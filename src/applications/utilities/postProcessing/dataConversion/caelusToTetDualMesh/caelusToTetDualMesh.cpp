/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2015 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of CAELUS.

    CAELUS is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CAELUS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with CAELUS.  If not, see <http://www.gnu.org/licenses/>.

Application
    caelusToTetDualMesh

Description
    Converts polyMesh results to tetDualMesh.

\*---------------------------------------------------------------------------*/


#include "argList.hpp"
#include "fvMesh.hpp"
#include "volFields.hpp"
#include "pointFields.hpp"
#include "Time.hpp"
#include "IOobjectList.hpp"

using namespace CML;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class ReadGeoField, class MappedGeoField>
void ReadAndMapFields
(
    const fvMesh& mesh,
    const IOobjectList& objects,
    const fvMesh& tetDualMesh,
    const labelList& map,
    const typename MappedGeoField::value_type& nullValue,
    PtrList<MappedGeoField>& tetFields
)
{
    typedef typename MappedGeoField::value_type Type;

    // Search list of objects for wanted type
    IOobjectList fieldObjects(objects.lookupClass(ReadGeoField::typeName));

    tetFields.setSize(fieldObjects.size());

    label i = 0;
    forAllConstIter(IOobjectList, fieldObjects, iter)
    {
        Info<< "Converting " << ReadGeoField::typeName << ' ' << iter.key()
            << endl;

        ReadGeoField readField(*iter(), mesh);

        tetFields.set
        (
            i,
            new MappedGeoField
            (
                IOobject
                (
                    readField.name(),
                    readField.instance(),
                    readField.local(),
                    tetDualMesh,
                    IOobject::NO_READ,
                    IOobject::AUTO_WRITE,
                    readField.registerObject()
                ),
                pointMesh::New(tetDualMesh),
                dimensioned<Type>
                (
                    "zero",
                    readField.dimensions(),
                    pTraits<Type>::zero
                )
            )
        );

        Field<Type>& fld = tetFields[i].internalField();

        // Map from read field. Set unmapped entries to nullValue.
        fld.setSize(map.size(), nullValue);
        forAll(map, pointI)
        {
            label index = map[pointI];

            if (index > 0)
            {
                label cellI = index-1;
                fld[pointI] = readField[cellI];
            }
            else if (index < 0)
            {
                label faceI = -index-1;
                label bFaceI = faceI - mesh.nInternalFaces();
                if (bFaceI >= 0)
                {
                    label patchI = mesh.boundaryMesh().patchID()[bFaceI];
                    label localFaceI = mesh.boundaryMesh()[patchI].whichFace
                    (
                        faceI
                    );
                    fld[pointI] = readField.boundaryField()[patchI][localFaceI];
                }
                //else
                //{
                //    FatalErrorInFunction
                //        << "Face " << faceI << " from index " << index
                //        << " is not a boundary face." << abort(FatalError);
                //}

            }
            //else
            //{
            //    WarningInFunction
            //        << "Point " << pointI << " at "
            //        << tetDualMesh.points()[pointI]
            //        << " has no dual correspondence." << endl;
            //}
        }
        tetFields[i].correctBoundaryConditions();

        i++;
    }
}



// Main program:

int main(int argc, char *argv[])
{
#   include "addOverwriteOption.hpp"
#   include "addTimeOptions.hpp"

#   include "setRootCase.hpp"
#   include "createTime.hpp"
    // Get times list
    instantList Times = runTime.times();
#   include "checkTimeOptions.hpp"
    runTime.setTime(Times[startTime], startTime);


    // Read the mesh
#   include "createMesh.hpp"

    // Read the tetDualMesh
    Info<< "Create tetDualMesh for time = "
        << runTime.timeName() << nl << endl;

    fvMesh tetDualMesh
    (
        IOobject
        (
            "tetDualMesh",
            runTime.timeName(),
            runTime,
            IOobject::MUST_READ
        )
    );
    // From tet vertices to poly cells/faces
    const labelIOList pointDualAddressing
    (
        IOobject
        (
            "pointDualAddressing",
            tetDualMesh.facesInstance(),
            tetDualMesh.meshSubDir,
            tetDualMesh,
            IOobject::MUST_READ
        )
    );

    if (pointDualAddressing.size() != tetDualMesh.nPoints())
    {
            FatalErrorInFunction
                << "Size " << pointDualAddressing.size()
                << " of addressing map " << pointDualAddressing.objectPath()
                << " differs from number of points in mesh "
                << tetDualMesh.nPoints()
                << exit(FatalError);
    }


    // Some stats on addressing
    label nCells = 0;
    label nPatchFaces = 0;
    label nUnmapped = 0;
    forAll(pointDualAddressing, pointI)
    {
        label index = pointDualAddressing[pointI];

        if (index > 0)
        {
            nCells++;
        }
        else if (index == 0)
        {
            nUnmapped++;
        }
        else
        {
            label faceI = -index-1;
            if (faceI < mesh.nInternalFaces())
            {
                FatalErrorInFunction
                    << "Face " << faceI << " from index " << index
                    << " is not a boundary face."
                    << " nInternalFaces:" << mesh.nInternalFaces()
                    << exit(FatalError);
            }
            else
            {
                nPatchFaces++;
            }
        }
    }

    reduce(nCells, sumOp<label>());
    reduce(nPatchFaces, sumOp<label>());
    reduce(nUnmapped, sumOp<label>());
    Info<< "tetDualMesh points : " << tetDualMesh.nPoints()
        << " of which mapped to" << nl
        << "    cells       : " << nCells << nl
        << "    patch faces : " << nPatchFaces << nl
        << "    not mapped  : " << nUnmapped << nl
        << endl;


    // Read objects in time directory
    IOobjectList objects(mesh, runTime.timeName());

    // Read vol fields, interpolate onto tet points
    PtrList<pointScalarField> psFlds;
    ReadAndMapFields<volScalarField, pointScalarField>
    (
        mesh,
        objects,
        tetDualMesh,
        pointDualAddressing,
        pTraits<scalar>::zero,  // nullValue
        psFlds
    );

    PtrList<pointVectorField> pvFlds;
    ReadAndMapFields<volVectorField, pointVectorField>
    (
        mesh,
        objects,
        tetDualMesh,
        pointDualAddressing,
        pTraits<vector>::zero,  // nullValue
        pvFlds
    );

    PtrList<pointSphericalTensorField> pstFlds;
    ReadAndMapFields<volSphericalTensorField, pointSphericalTensorField>
    (
        mesh,
        objects,
        tetDualMesh,
        pointDualAddressing,
        pTraits<sphericalTensor>::zero,  // nullValue
        pstFlds
    );

    PtrList<pointSymmTensorField> psymmtFlds;
    ReadAndMapFields<volSymmTensorField, pointSymmTensorField>
    (
        mesh,
        objects,
        tetDualMesh,
        pointDualAddressing,
        pTraits<symmTensor>::zero,  // nullValue
        psymmtFlds
    );

    PtrList<pointTensorField> ptFlds;
    ReadAndMapFields<volTensorField, pointTensorField>
    (
        mesh,
        objects,
        tetDualMesh,
        pointDualAddressing,
        pTraits<tensor>::zero,  // nullValue
        ptFlds
    );

    tetDualMesh.objectRegistry::write();

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
