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

\*---------------------------------------------------------------------------*/

#include "OBJedgeFormat.hpp"
#include "clock.hpp"
#include "IFstream.hpp"
#include "IStringStream.hpp"
#include "Ostream.hpp"
#include "OFstream.hpp"
#include "ListOps.hpp"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::fileFormats::OBJedgeFormat::OBJedgeFormat
(
    const fileName& filename
)
{
    read(filename);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool CML::fileFormats::OBJedgeFormat::read
(
    const fileName& filename
)
{
    clear();

    IFstream is(filename);
    if (!is.good())
    {
        FatalErrorInFunction
            << "Cannot read file " << filename
            << exit(FatalError);
    }

    DynamicList<point> dynPoints;
    DynamicList<edge> dynEdges;
    DynamicList<label> dynUsedPoints;

    while (is.good())
    {
        string line = this->getLineNoComment(is);

        // handle continuations
        if (line[line.size()-1] == '\\')
        {
            line.substr(0, line.size()-1);
            line += this->getLineNoComment(is);
        }

        // Read first word
        IStringStream lineStream(line);
        word cmd;
        lineStream >> cmd;

        if (cmd == "v")
        {
            scalar x, y, z;
            lineStream >> x >> y >> z;

            dynPoints.append(point(x, y, z));
            dynUsedPoints.append(-1);
        }
        else if (cmd == "l")
        {
            edge edgeRead;

            // Assume 'l' is followed by space.
            string::size_type endNum = 1;

            int nVerts = 0;
            for (int count = 0; count < 2; ++count)
            {
                string::size_type startNum =
                    line.find_first_not_of(' ', endNum);

                if (startNum == string::npos)
                {
                    break;
                }

                endNum = line.find(' ', startNum);

                string vertexSpec;
                if (endNum != string::npos)
                {
                    vertexSpec = line.substr(startNum, endNum-startNum);
                }
                else
                {
                    vertexSpec = line.substr(startNum, line.size() - startNum);
                }

                string::size_type slashPos = vertexSpec.find('/');

                label vertI = 0;
                if (slashPos != string::npos)
                {
                    IStringStream intStream(vertexSpec.substr(0, slashPos));

                    intStream >> vertI;
                }
                else
                {
                    IStringStream intStream(vertexSpec);

                    intStream >> vertI;
                }

                edgeRead[nVerts++] = (vertI - 1); // change to zero-offset
            }

            if (nVerts >= 2)
            {
                dynUsedPoints[edgeRead[0]] = edgeRead[0];
                dynUsedPoints[edgeRead[1]] = edgeRead[1];

                dynEdges.append(edgeRead);
            }
        }
    }

    // cull unused points
    label nUsed = 0;

    forAll(dynPoints, pointI)
    {
        if (dynUsedPoints[pointI] >= 0)
        {
            if (nUsed != pointI)
            {
                dynPoints[nUsed] = dynPoints[pointI];
                dynUsedPoints[pointI] = nUsed;   // new position
            }
            ++nUsed;
        }
    }

    dynPoints.setSize(nUsed);

    // transfer to normal lists
    storedPoints().transfer(dynPoints);

    // renumber edge vertices
    if (nUsed != dynUsedPoints.size())
    {
        forAll(dynEdges, edgeI)
        {
            edge& e = dynEdges[edgeI];

            e[0] = dynUsedPoints[e[0]];
            e[1] = dynUsedPoints[e[1]];
        }
    }
    storedEdges().transfer(dynEdges);

    return true;
}


void CML::fileFormats::OBJedgeFormat::write
(
    const fileName& filename,
    const edgeMesh& mesh
)
{
    const pointField& pointLst = mesh.points();
    const edgeList& edgeLst = mesh.edges();

    OFstream os(filename);
    if (!os.good())
    {
        FatalErrorInFunction
            << "Cannot open file for writing " << filename
            << exit(FatalError);
    }


    os  << "# Wavefront OBJ file written " << clock::dateTime().c_str() << nl
        << "o " << os.name().lessExt().name() << nl
        << nl
        << "# points : " << pointLst.size() << nl
        << "# lines  : " << edgeLst.size() << nl;

    os  << nl
        << "# <points count=\"" << pointLst.size() << "\">" << nl;

    // Write vertex coords
    forAll(pointLst, ptI)
    {
        const point& p = pointLst[ptI];

        os  << "v " << p.x() << ' ' << p.y() << ' ' << p.z() << nl;
    }

    os  << "# </points>" << nl
        << nl
        << "# <edges count=\"" << edgeLst.size() << "\">" << endl;

    // Write line connectivity
    forAll(edgeLst, edgeI)
    {
        const edge& e = edgeLst[edgeI];

        os << "l " << (e[0] + 1) << " " << (e[1] + 1) << nl;
    }
    os << "# </edges>" << endl;
}


// ************************************************************************* //
