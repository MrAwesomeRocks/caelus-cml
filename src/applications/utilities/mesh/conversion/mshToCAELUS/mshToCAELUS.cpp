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
    Converts .msh file generated by the Adventure system.

    Note: the .msh format does not contain any boundary information. It is
    purely a description of the internal mesh.

    Can read both linear-tet format (i.e. 4 verts per tet) and linear-hex
    format (8 verts per hex) (if provided with the -hex (at your option)
    (Note: will bomb out if not supplied with the correct option for the
     file format)

    Not extensively tested.

\*---------------------------------------------------------------------------*/

#include "argList.hpp"
#include "Time.hpp"
#include "polyMesh.hpp"
#include "IFstream.hpp"
#include "polyPatch.hpp"
#include "ListOps.hpp"
#include "cellModeller.hpp"

#include <fstream>

using namespace CML;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Main program:

int main(int argc, char *argv[])
{
    argList::noParallel();
    argList::validArgs.append(".msh file");
    argList::addBoolOption
    (
        "hex",
        "treat input as containing hex instead of tet cells"
    );

#   include "setRootCase.hpp"
#   include "createTime.hpp"

    const bool readHex = args.optionFound("hex");
    IFstream mshStream(args[1]);

    label nCells;
    mshStream >> nCells;

    if (readHex)
    {
        Info<< "Trying to read " << nCells << " hexes." << nl << endl;
    }
    else
    {
        Info<< "Trying to read " << nCells << " tets." << nl << endl;
    }

    cellShapeList cells(nCells);

    const cellModel& tet = *(cellModeller::lookup("tet"));
    const cellModel& hex = *(cellModeller::lookup("hex"));

    labelList tetPoints(4);
    labelList hexPoints(8);

    if (readHex)
    {
        for (label cellI = 0; cellI < nCells; cellI++)
        {
            for (label cp = 0; cp < 8; cp++)
            {
                mshStream >> hexPoints[cp];
            }
            cells[cellI] = cellShape(hex, hexPoints);
        }
    }
    else
    {
        for (label cellI = 0; cellI < nCells; cellI++)
        {
            for (label cp = 0; cp < 4; cp++)
            {
                mshStream >> tetPoints[cp];
            }
            cells[cellI] = cellShape(tet, tetPoints);
        }
    }


    label nPoints;

    mshStream >> nPoints;

    Info<< "Trying to read " << nPoints << " points." << endl << endl;

    pointField points(nPoints);


    for (label pointI = 0; pointI < nPoints; pointI++)
    {
        scalar x, y, z;

        mshStream >> x >> y >> z;

        points[pointI] = point(x, y, z);
    }


    polyMesh mesh
    (
        IOobject
        (
            polyMesh::defaultRegion,
            runTime.constant(),
            runTime
        ),
        xferMove(points),
        cells,
        faceListList(0),
        wordList(0),
        wordList(0),
        "defaultFaces",
        polyPatch::typeName,
        wordList(0)
    );

    Info<< "Writing mesh ..." << endl;

    mesh.write();


    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
