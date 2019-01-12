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

Description
    Selects a section of mesh based on a cellSet.

    The utility sub-sets the mesh to choose only a part of interest. Check
    the setSet/cellSet utilities to see how to select cells based on various.

    The mesh will subset all points, faces and cells needed to make a sub-mesh
    but will not preserve attached boundary types.

\*---------------------------------------------------------------------------*/

#include "fvMeshSubset.hpp"
#include "argList.hpp"
#include "cellSet.hpp"
#include "IOobjectList.hpp"
#include "volFields.hpp"

using namespace CML;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


template<class Type>
void subsetVolFields
(
    const fvMeshSubset& subsetter,
    const wordList& fieldNames,
    PtrList<GeometricField<Type, fvPatchField, volMesh> >& subFields
)
{
    const fvMesh& baseMesh = subsetter.baseMesh();

    forAll(fieldNames, i)
    {
        const word& fieldName = fieldNames[i];

        Info<< "Subsetting field " << fieldName << endl;

        GeometricField<Type, fvPatchField, volMesh> fld
        (
            IOobject
            (
                fieldName,
                baseMesh.time().timeName(),
                baseMesh,
                IOobject::MUST_READ,
                IOobject::NO_WRITE
            ),
            baseMesh
        );

        subFields.set(i, subsetter.interpolate(fld));
    }
}


template<class Type>
void subsetSurfaceFields
(
    const fvMeshSubset& subsetter,
    const wordList& fieldNames,
    PtrList<GeometricField<Type, fvsPatchField, surfaceMesh> >& subFields
)
{
    const fvMesh& baseMesh = subsetter.baseMesh();

    forAll(fieldNames, i)
    {
        const word& fieldName = fieldNames[i];

        Info<< "Subsetting field " << fieldName << endl;

        GeometricField<Type, fvsPatchField, surfaceMesh> fld
        (
            IOobject
            (
                fieldName,
                baseMesh.time().timeName(),
                baseMesh,
                IOobject::MUST_READ,
                IOobject::NO_WRITE
            ),
            baseMesh
        );

        subFields.set(i, subsetter.interpolate(fld));
    }
}


template<class Type>
void subsetPointFields
(
    const fvMeshSubset& subsetter,
    const pointMesh& pMesh,
    const wordList& fieldNames,
    PtrList<GeometricField<Type, pointPatchField, pointMesh> >& subFields
)
{
    const fvMesh& baseMesh = subsetter.baseMesh();

    forAll(fieldNames, i)
    {
        const word& fieldName = fieldNames[i];

        Info<< "Subsetting field " << fieldName << endl;

        GeometricField<Type, pointPatchField, pointMesh> fld
        (
            IOobject
            (
                fieldName,
                baseMesh.time().timeName(),
                baseMesh,
                IOobject::MUST_READ,
                IOobject::NO_WRITE
            ),
            pMesh
        );

        subFields.set(i, subsetter.interpolate(fld));
    }
}


template<class Type>
void subsetDimensionedFields
(
    const fvMeshSubset& subsetter,
    const wordList& fieldNames,
    PtrList<DimensionedField<Type, volMesh> >& subFields
)
{
    const fvMesh& baseMesh = subsetter.baseMesh();

    forAll(fieldNames, i)
    {
        const word& fieldName = fieldNames[i];

        Info<< "Subsetting field " << fieldName << endl;

        DimensionedField<Type, volMesh> fld
        (
            IOobject
            (
                fieldName,
                baseMesh.time().timeName(),
                baseMesh,
                IOobject::MUST_READ,
                IOobject::NO_WRITE
            ),
            baseMesh
        );

        subFields.set(i, subsetter.interpolate(fld));
    }
}



int main(int argc, char *argv[])
{
    argList::addNote
    (
        "select a mesh subset based on a cellSet"
    );

    #include "addOverwriteOption.hpp"
    #include "addRegionOption.hpp"
    argList::validArgs.append("cellSet");
    argList::addOption
    (
        "patch",
        "name",
        "add exposed internal faces to specified patch instead of to "
        "'oldInternalFaces'"
    );
    argList::addOption
    (
        "patches",
        "names",
        "add exposed internal faces to nearest of specified patches"
        " instead of to 'oldInternalFaces'"
    );
    argList::addOption
    (
        "resultTime",
        "time",
        "specify a time for the resulting mesh"
    );
    #include "setRootCase.hpp"
    #include "createTime.hpp"
    runTime.functionObjects().off();

    CML::word meshRegionName = polyMesh::defaultRegion;
    args.optionReadIfPresent("region", meshRegionName);

    #include "createNamedMesh.hpp"


    const word setName = args[1];

    word meshInstance = mesh.pointsInstance();
    word fieldsInstance = runTime.timeName();

    const bool overwrite = args.optionFound("overwrite");
    const bool specifiedInstance = args.optionReadIfPresent
    (
        "resultTime",
        fieldsInstance
    );
    if (specifiedInstance)
    {
        // Set both mesh and field to this time
        meshInstance = fieldsInstance;
    }


    Info<< "Reading cell set from " << setName << endl << endl;

    // Create mesh subsetting engine
    fvMeshSubset subsetter(mesh);

    label patchI = -1;

    if (args.optionFound("patch"))
    {
        const word patchName = args["patch"];

        patchI = mesh.boundaryMesh().findPatchID(patchName);

        if (patchI == -1)
        {
            FatalErrorInFunction << "Illegal patch " << patchName
                << nl << "Valid patches are " << mesh.boundaryMesh().names()
                << exit(FatalError);
        }

        Info<< "Adding exposed internal faces to patch " << patchName << endl
            << endl;
    }
    else
    {
        Info<< "Adding exposed internal faces to a patch called"
            << " \"oldInternalFaces\" (created if necessary)" << endl
            << endl;
    }


    cellSet currentSet(mesh, setName);

    subsetter.setLargeCellSubset(currentSet, patchI, true);

    IOobjectList objects(mesh, runTime.timeName());


    // Read vol fields and subset
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~

    wordList scalarNames(objects.names(volScalarField::typeName));
    PtrList<volScalarField> scalarFlds(scalarNames.size());
    subsetVolFields(subsetter, scalarNames, scalarFlds);

    wordList vectorNames(objects.names(volVectorField::typeName));
    PtrList<volVectorField> vectorFlds(vectorNames.size());
    subsetVolFields(subsetter, vectorNames, vectorFlds);

    wordList sphericalTensorNames
    (
        objects.names(volSphericalTensorField::typeName)
    );
    PtrList<volSphericalTensorField> sphericalTensorFlds
    (
        sphericalTensorNames.size()
    );
    subsetVolFields(subsetter, sphericalTensorNames, sphericalTensorFlds);

    wordList symmTensorNames(objects.names(volSymmTensorField::typeName));
    PtrList<volSymmTensorField> symmTensorFlds(symmTensorNames.size());
    subsetVolFields(subsetter, symmTensorNames, symmTensorFlds);

    wordList tensorNames(objects.names(volTensorField::typeName));
    PtrList<volTensorField> tensorFlds(tensorNames.size());
    subsetVolFields(subsetter, tensorNames, tensorFlds);


    // Read surface fields and subset
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    wordList surfScalarNames(objects.names(surfaceScalarField::typeName));
    PtrList<surfaceScalarField> surfScalarFlds(surfScalarNames.size());
    subsetSurfaceFields(subsetter, surfScalarNames, surfScalarFlds);

    wordList surfVectorNames(objects.names(surfaceVectorField::typeName));
    PtrList<surfaceVectorField> surfVectorFlds(surfVectorNames.size());
    subsetSurfaceFields(subsetter, surfVectorNames, surfVectorFlds);

    wordList surfSphericalTensorNames
    (
        objects.names(surfaceSphericalTensorField::typeName)
    );
    PtrList<surfaceSphericalTensorField> surfSphericalTensorFlds
    (
        surfSphericalTensorNames.size()
    );
    subsetSurfaceFields
    (
        subsetter,
        surfSphericalTensorNames,
        surfSphericalTensorFlds
    );

    wordList surfSymmTensorNames
    (
        objects.names(surfaceSymmTensorField::typeName)
    );
    PtrList<surfaceSymmTensorField> surfSymmTensorFlds
    (
        surfSymmTensorNames.size()
    );
    subsetSurfaceFields(subsetter, surfSymmTensorNames, surfSymmTensorFlds);

    wordList surfTensorNames(objects.names(surfaceTensorField::typeName));
    PtrList<surfaceTensorField> surfTensorFlds(surfTensorNames.size());
    subsetSurfaceFields(subsetter, surfTensorNames, surfTensorFlds);


    // Read point fields and subset
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    const pointMesh& pMesh = pointMesh::New(mesh);

    wordList pointScalarNames(objects.names(pointScalarField::typeName));
    PtrList<pointScalarField> pointScalarFlds(pointScalarNames.size());
    subsetPointFields(subsetter, pMesh, pointScalarNames, pointScalarFlds);

    wordList pointVectorNames(objects.names(pointVectorField::typeName));
    PtrList<pointVectorField> pointVectorFlds(pointVectorNames.size());
    subsetPointFields(subsetter, pMesh, pointVectorNames, pointVectorFlds);

    wordList pointSphericalTensorNames
    (
        objects.names(pointSphericalTensorField::typeName)
    );
    PtrList<pointSphericalTensorField> pointSphericalTensorFlds
    (
        pointSphericalTensorNames.size()
    );
    subsetPointFields
    (
        subsetter,
        pMesh,
        pointSphericalTensorNames,
        pointSphericalTensorFlds
    );

    wordList pointSymmTensorNames
    (
        objects.names(pointSymmTensorField::typeName)
    );
    PtrList<pointSymmTensorField> pointSymmTensorFlds
    (
        pointSymmTensorNames.size()
    );
    subsetPointFields
    (
        subsetter,
        pMesh,
        pointSymmTensorNames,
        pointSymmTensorFlds
    );

    wordList pointTensorNames(objects.names(pointTensorField::typeName));
    PtrList<pointTensorField> pointTensorFlds(pointTensorNames.size());
    subsetPointFields(subsetter, pMesh, pointTensorNames, pointTensorFlds);


    // Read dimensioned fields and subset
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    typedef volScalarField::DimensionedInternalField dimScalType;
    wordList scalarDimNames(objects.names(dimScalType::typeName));
    PtrList<dimScalType> scalarDimFlds(scalarDimNames.size());
    subsetDimensionedFields(subsetter, scalarDimNames, scalarDimFlds);

    typedef volVectorField::DimensionedInternalField dimVecType;
    wordList vectorDimNames(objects.names(dimVecType::typeName));
    PtrList<dimVecType> vectorDimFlds(vectorDimNames.size());
    subsetDimensionedFields(subsetter, vectorDimNames, vectorDimFlds);

    typedef volSphericalTensorField::DimensionedInternalField dimSphereType;
    wordList sphericalTensorDimNames(objects.names(dimSphereType::typeName));
    PtrList<dimSphereType> sphericalTensorDimFlds
    (
        sphericalTensorDimNames.size()
    );
    subsetDimensionedFields
    (
        subsetter,
        sphericalTensorDimNames,
        sphericalTensorDimFlds
    );

    typedef volSymmTensorField::DimensionedInternalField dimSymmTensorType;
    wordList symmTensorDimNames(objects.names(dimSymmTensorType::typeName));
    PtrList<dimSymmTensorType> symmTensorDimFlds(symmTensorDimNames.size());
    subsetDimensionedFields(subsetter, symmTensorDimNames, symmTensorDimFlds);

    typedef volTensorField::DimensionedInternalField dimTensorType;
    wordList tensorDimNames(objects.names(dimTensorType::typeName));
    PtrList<dimTensorType> tensorDimFlds(tensorDimNames.size());
    subsetDimensionedFields(subsetter, tensorDimNames, tensorDimFlds);


    // Write mesh and fields to new time
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    if (overwrite || specifiedInstance)
    {
        runTime.setTime(instant(fieldsInstance), 0);
        subsetter.subMesh().setInstance(meshInstance);
    }
    else
    {
        runTime++;
    }

    Info<< "Writing subsetted mesh and fields to time " << runTime.timeName()
        << endl;
    subsetter.subMesh().write();


    // Subsetting adds 'subset' prefix. Rename field to be like original.
    forAll(scalarFlds, i)
    {
        scalarFlds[i].rename(scalarNames[i]);
        scalarFlds[i].write();
    }
    forAll(vectorFlds, i)
    {
        vectorFlds[i].rename(vectorNames[i]);
        vectorFlds[i].write();
    }
    forAll(sphericalTensorFlds, i)
    {
        sphericalTensorFlds[i].rename(sphericalTensorNames[i]);
        sphericalTensorFlds[i].write();
    }
    forAll(symmTensorFlds, i)
    {
        symmTensorFlds[i].rename(symmTensorNames[i]);
        symmTensorFlds[i].write();
    }
    forAll(tensorFlds, i)
    {
        tensorFlds[i].rename(tensorNames[i]);
        tensorFlds[i].write();
    }

    // Surface ones.
    forAll(surfScalarFlds, i)
    {
        surfScalarFlds[i].rename(surfScalarNames[i]);
        surfScalarFlds[i].write();
    }
    forAll(surfVectorFlds, i)
    {
        surfVectorFlds[i].rename(surfVectorNames[i]);
        surfVectorFlds[i].write();
    }
    forAll(surfSphericalTensorFlds, i)
    {
        surfSphericalTensorFlds[i].rename(surfSphericalTensorNames[i]);
        surfSphericalTensorFlds[i].write();
    }
    forAll(surfSymmTensorFlds, i)
    {
        surfSymmTensorFlds[i].rename(surfSymmTensorNames[i]);
        surfSymmTensorFlds[i].write();
    }
    forAll(surfTensorNames, i)
    {
        surfTensorFlds[i].rename(surfTensorNames[i]);
        surfTensorFlds[i].write();
    }

    // Point ones
    forAll(pointScalarFlds, i)
    {
        pointScalarFlds[i].rename(pointScalarNames[i]);
        pointScalarFlds[i].write();
    }
    forAll(pointVectorFlds, i)
    {
        pointVectorFlds[i].rename(pointVectorNames[i]);
        pointVectorFlds[i].write();
    }
    forAll(pointSphericalTensorFlds, i)
    {
        pointSphericalTensorFlds[i].rename(pointSphericalTensorNames[i]);
        pointSphericalTensorFlds[i].write();
    }
    forAll(pointSymmTensorFlds, i)
    {
        pointSymmTensorFlds[i].rename(pointSymmTensorNames[i]);
        pointSymmTensorFlds[i].write();
    }
    forAll(pointTensorNames, i)
    {
        pointTensorFlds[i].rename(pointTensorNames[i]);
        pointTensorFlds[i].write();
    }

    // DimensionedFields
    forAll(scalarDimFlds, i)
    {
        scalarDimFlds[i].rename(scalarDimNames[i]);
        scalarDimFlds[i].write();
    }
    forAll(vectorDimFlds, i)
    {
        vectorDimFlds[i].rename(vectorDimNames[i]);
        vectorDimFlds[i].write();
    }
    forAll(sphericalTensorDimFlds, i)
    {
        sphericalTensorDimFlds[i].rename(sphericalTensorDimNames[i]);
        sphericalTensorDimFlds[i].write();
    }
    forAll(symmTensorDimFlds, i)
    {
        symmTensorDimFlds[i].rename(symmTensorDimNames[i]);
        symmTensorDimFlds[i].write();
    }
    forAll(tensorDimFlds, i)
    {
        tensorDimFlds[i].rename(tensorDimNames[i]);
        tensorDimFlds[i].write();
    }


    Info<< "\nEnd\n" << endl;

    return 0;
}


// ************************************************************************* //
