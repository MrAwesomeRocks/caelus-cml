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

Application
    caelusToSurface

Description
    Reads an CAELUS mesh and writes the boundaries in a surface format.

Usage
    - caelusToSurface [OPTION] \n
    Reads an CAELUS mesh and writes the boundaries in a surface format.

    \param -scale \<factor\>\n
    Specify an alternative geometry scaling factor.
    Eg, use \b 1000 to scale \em [m] to \em [mm].

    \param -tri \n
    Triangulate surface.

\*---------------------------------------------------------------------------*/

#include "argList.hpp"
#include "timeSelector.hpp"
#include "Time.hpp"
#include "polyMesh.hpp"

#include "MeshedSurfaces.hpp"

using namespace CML;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
// Main program:

int main(int argc, char *argv[])
{
    argList::noParallel();
    argList::validArgs.append("outputFile.ext");
    timeSelector::addOptions();

    argList::addOption
    (
        "scale",
        "factor",
        "geometry scaling factor - default is 1"
    );
    argList::addBoolOption
    (
        "tri",
        "triangulate surface"
    );

#   include "setRootCase.hpp"

    fileName exportName = args[1];

    scalar scaleFactor = 0;
    args.optionReadIfPresent<scalar>("scale", scaleFactor);
    const bool doTriangulate = args.optionFound("tri");

    fileName exportBase = exportName.lessExt();
    word exportExt = exportName.ext();

    if (!meshedSurface::canWriteType(exportExt, true))
    {
        return 1;
    }

#   include "createTime.hpp"
    instantList timeDirs = timeSelector::select0(runTime, args);
#   include "createPolyMesh.hpp"

    forAll(timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI], timeI);
#       include "getTimeIndex.hpp"

        polyMesh::readUpdateState state = mesh.readUpdate();

        if (timeI == 0 || state != polyMesh::UNCHANGED)
        {
            if (state == polyMesh::UNCHANGED)
            {
                exportName = exportBase + "." + exportExt;
            }
            else
            {
                exportName =
                    exportBase + '_' + runTime.timeName() + "." + exportExt;
            }

            meshedSurface surf(mesh.boundaryMesh());
            surf.scalePoints(scaleFactor);

            Info<< "writing " << exportName;
            if (doTriangulate)
            {
                Info<< " triangulated";
                surf.triangulate();
            }

            if (scaleFactor <= 0)
            {
                Info<< " without scaling" << endl;
            }
            else
            {
                Info<< " with scaling " << scaleFactor << endl;
            }
            surf.write(exportName);
        }

        Info<< nl << endl;
    }

    Info<< "End\n" << endl;

    return 0;
}

// ************************************************************************* //
