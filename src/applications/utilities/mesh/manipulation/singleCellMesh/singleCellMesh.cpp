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
    singleCellMesh

Description
    Reads all fields and maps them to a mesh with all internal faces removed
    (singleCellFvMesh) which gets written to region "singleCell".

    Used to generate mesh and fields
    that can be used for boundary-only data.
    Might easily result in illegal mesh though so only look at boundaries
    in paraview.

\*---------------------------------------------------------------------------*/

#include "argList.hpp"
#include "fvMesh.hpp"
#include "volFields.hpp"
#include "Time.hpp"
#include "ReadFields_.hpp"
#include "singleCellFvMesh.hpp"
#include "timeSelector.hpp"

using namespace CML;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Name of region to create
const string singleCellName = "singleCell";


template<class GeoField>
void interpolateFields
(
    const singleCellFvMesh& scMesh,
    const PtrList<GeoField>& flds
)
{
    forAll(flds, i)
    {
        tmp<GeoField> scFld = scMesh.interpolate(flds[i]);
        GeoField* scFldPtr = scFld.ptr();
        scFldPtr->writeOpt() = IOobject::AUTO_WRITE;
        scFldPtr->store();
    }
}


// Main program:

int main(int argc, char *argv[])
{
    // constant, not false
    timeSelector::addOptions(true, false);

#   include "setRootCase.hpp"
#   include "createTime.hpp"

    instantList timeDirs = timeSelector::select0(runTime, args);

#   include "createNamedMesh.hpp"

    if (regionName == singleCellName)
    {
        FatalErrorInFunction
            << "Cannot convert region " << singleCellName
            << " since result would overwrite it. Please rename your region."
            << exit(FatalError);
    }

    // Create the mesh
    Info<< "Creating singleCell mesh" << nl << endl;
    autoPtr<singleCellFvMesh> scMesh
    (
        new singleCellFvMesh
        (
            IOobject
            (
                singleCellName,
                mesh.polyMesh::instance(),
                runTime,
                IOobject::NO_READ,
                IOobject::AUTO_WRITE
            ),
            mesh
        )
    );
    // For convenience create any fv* files
    if (!exists(scMesh().fvSolution::objectPath()))
    {
        mkDir(scMesh().fvSolution::path());
        ln("../fvSolution", scMesh().fvSolution::objectPath());
    }
    if (!exists(scMesh().fvSchemes::objectPath()))
    {
        mkDir(scMesh().fvSolution::path());
        ln("../fvSchemes", scMesh().fvSchemes::objectPath());
    }


    forAll(timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI], timeI);

        Info<< nl << "Time = " << runTime.timeName() << endl;


        // Check for new mesh
        if (mesh.readUpdate() != polyMesh::UNCHANGED)
        {
            Info<< "Detected changed mesh. Recreating singleCell mesh." << endl;
            scMesh.clear(); // remove any registered objects
            scMesh.reset
            (
                new singleCellFvMesh
                (
                    IOobject
                    (
                        singleCellName,
                        mesh.polyMesh::instance(),
                        runTime,
                        IOobject::NO_READ,
                        IOobject::AUTO_WRITE
                    ),
                    mesh
                )
            );
        }


        // Read objects in time directory
        IOobjectList objects(mesh, runTime.timeName());

        // Read vol fields.
        PtrList<volScalarField> vsFlds;
        ReadFields(mesh, objects, vsFlds);

        PtrList<volVectorField> vvFlds;
        ReadFields(mesh, objects, vvFlds);

        PtrList<volSphericalTensorField> vstFlds;
        ReadFields(mesh, objects, vstFlds);

        PtrList<volSymmTensorField> vsymtFlds;
        ReadFields(mesh, objects, vsymtFlds);

        PtrList<volTensorField> vtFlds;
        ReadFields(mesh, objects, vtFlds);

        // Map and store the fields on the scMesh.
        interpolateFields(scMesh(), vsFlds);
        interpolateFields(scMesh(), vvFlds);
        interpolateFields(scMesh(), vstFlds);
        interpolateFields(scMesh(), vsymtFlds);
        interpolateFields(scMesh(), vtFlds);


        // Write
        Info<< "Writing mesh to time " << runTime.timeName() << endl;
        scMesh().write();
    }


    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
