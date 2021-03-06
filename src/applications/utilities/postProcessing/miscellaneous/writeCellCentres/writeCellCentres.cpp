/*---------------------------------------------------------------------------*\
Copyright (C) 2011 OpenFOAM Foundation
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
    Write the three components of the cell centres as volScalarFields so
    they can be used in postprocessing in thresholding.

\*---------------------------------------------------------------------------*/

#include "argList.hpp"
#include "timeSelector.hpp"
#include "Time.hpp"
#include "fvMesh.hpp"
#include "vectorIOField.hpp"
#include "volFields.hpp"

using namespace CML;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Main program:

int main(int argc, char *argv[])
{
    timeSelector::addOptions();
#   include "addRegionOption.hpp"

#   include "setRootCase.hpp"
#   include "createTime.hpp"

    instantList timeDirs = timeSelector::select0(runTime, args);

#   include "createNamedMesh.hpp"

    forAll(timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI], timeI);

        Info<< "Time = " << runTime.timeName() << endl;

        // Check for new mesh
        mesh.readUpdate();

        volVectorField cc
        (
            IOobject
            (
                "cellCentres",
                runTime.timeName(),
                mesh,
                IOobject::NO_READ,
                IOobject::AUTO_WRITE
            ),
            mesh.C()
        );

        // Info<< "Writing cellCentre positions to " << cc.name() << " in "
        //     << runTime.timeName() << endl;
        //
        // cc.write();

        Info<< "Writing components of cellCentre positions to volScalarFields"
            << " ccx, ccy, ccz in " <<  runTime.timeName() << endl;

        for (direction i=0; i<vector::nComponents; i++)
        {
            volScalarField cci
            (
                IOobject
                (
                    "cc" + word(vector::componentNames[i]),
                    runTime.timeName(),
                    mesh,
                    IOobject::NO_READ,
                    IOobject::AUTO_WRITE
                ),
                mesh.C().component(i)
            );

            cci.write();
        }
    }

    Info<< "\nEnd\n" << endl;

    return 0;
}


// ************************************************************************* //
