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

#include "surfaceIntersection.hpp"
#include "triSurfaceSearch.hpp"
#include "labelPairLookup.hpp"
#include "OFstream.hpp"
#include "HashSet.hpp"
#include "triSurface.hpp"
#include "pointIndexHit.hpp"
#include "meshTools.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::surfaceIntersection::writeOBJ(const point& pt, Ostream& os)
{
    os << "v " << pt.x() << ' ' << pt.y() << ' ' << pt.z() << endl;
}


void CML::surfaceIntersection::writeOBJ
(
    const List<point>& pts,
    const List<edge>& edges,
    Ostream& os
)
{
    forAll(pts, i)
    {
        writeOBJ(pts[i], os);
    }
    forAll(edges, i)
    {
        const edge& e = edges[i];

        os << "l " << e.start()+1 << ' ' << e.end()+1 << endl;
    }
}


// Get minimum length of all edges connected to point
CML::scalar CML::surfaceIntersection::minEdgeLen
(
    const triSurface& surf,
    const label pointI
)
{
    const labelList& pEdges = surf.pointEdges()[pointI];

    scalar minLen = GREAT;

    forAll(pEdges, pEdgeI)
    {
        const edge& e = surf.edges()[pEdges[pEdgeI]];

        minLen = min(minLen, e.mag(surf.localPoints()));
    }

    return minLen;
}


// Get edge between fp and fp+1 on faceI.
CML::label CML::surfaceIntersection::getEdge
(
    const triSurface& surf,
    const label faceI,
    const label fp
)
{
    const edge faceEdge = surf.localFaces()[faceI].faceEdge(fp);

    const labelList& eLabels = surf.faceEdges()[faceI];

    forAll(eLabels, eI)
    {
        const label edgeI = eLabels[eI];

        if (surf.edges()[edgeI] == faceEdge)
        {
            return edgeI;
        }
    }

    FatalErrorInFunction
        << "Problem:: Cannot find edge with vertices " << faceEdge
        << " in face " << faceI
        << abort(FatalError);

    return -1;
}


// Given a map remove all consecutive duplicate elements.
void CML::surfaceIntersection::removeDuplicates
(
    const labelList& map,
    labelList& elems
)
{
    bool hasDuplicate = false;

    label prevVertI = -1;

    forAll(elems, elemI)
    {
        label newVertI = map[elems[elemI]];

        if (newVertI == prevVertI)
        {
            hasDuplicate = true;

            break;
        }
        prevVertI = newVertI;
    }

    if (hasDuplicate)
    {
        // Create copy
        labelList oldElems(elems);

        label elemI = 0;

        // Insert first
        elems[elemI++] = map[oldElems[0]];

        for (label vertI = 1; vertI < oldElems.size(); vertI++)
        {
            // Insert others only if they differ from one before
            label newVertI = map[oldElems[vertI]];

            if (newVertI != elems.last())
            {
                elems[elemI++] = newVertI;
            }
        }
        elems.setSize(elemI);
    }
}


// Remap.
void CML::surfaceIntersection::inlineRemap
(
    const labelList& map,
    labelList& elems
)
{
    forAll(elems, elemI)
    {
        elems[elemI] = map[elems[elemI]];
    }
}


// Remove all duplicate and degenerate elements. Return unique elements and
// map from old to new.
CML::edgeList CML::surfaceIntersection::filterEdges
(
    const edgeList& edges,
    labelList& map
)
{
    HashSet<edge, Hash<edge> > uniqueEdges(10*edges.size());

    edgeList newEdges(edges.size());

    map.setSize(edges.size());
    map = -1;

    label newEdgeI = 0;

    forAll(edges, edgeI)
    {
        const edge& e = edges[edgeI];

        if
        (
            (e.start() != e.end())
         && (uniqueEdges.find(e) == uniqueEdges.end())
        )
        {
            // Edge is -non degenerate and -not yet seen.
            uniqueEdges.insert(e);

            map[edgeI] = newEdgeI;

            newEdges[newEdgeI++] = e;
        }
    }

    newEdges.setSize(newEdgeI);

    return newEdges;
}


// Remove all duplicate elements.
CML::labelList CML::surfaceIntersection::filterLabels
(
    const labelList& elems,
    labelList& map
)
{
    labelHashSet uniqueElems(10*elems.size());

    labelList newElems(elems.size());

    map.setSize(elems.size());
    map = -1;

    label newElemI = 0;

    forAll(elems, elemI)
    {
        label elem = elems[elemI];

        if (uniqueElems.find(elem) == uniqueElems.end())
        {
            // First time elem is seen
            uniqueElems.insert(elem);

            map[elemI] = newElemI;

            newElems[newElemI++] = elem;
        }
    }

    newElems.setSize(newElemI);

    return newElems;
}


void CML::surfaceIntersection::writeIntersectedEdges
(
    const triSurface& surf,
    const labelListList& edgeCutVerts,
    Ostream& os
) const
{
    // Dump all points (surface followed by cutPoints)
    const pointField& pts = surf.localPoints();

    forAll(pts, pointI)
    {
        writeOBJ(pts[pointI], os);
    }
    forAll(cutPoints(), cutPointI)
    {
        writeOBJ(cutPoints()[cutPointI], os);
    }

    forAll(edgeCutVerts, edgeI)
    {
        const labelList& extraVerts = edgeCutVerts[edgeI];

        if (extraVerts.size())
        {
            const edge& e = surf.edges()[edgeI];

            // Start of original edge to first extra point
            os  << "l " << e.start()+1 << ' '
                << extraVerts[0] + surf.nPoints() + 1 << endl;

            for (label i = 1; i < extraVerts.size(); i++)
            {
                os  << "l " << extraVerts[i-1] + surf.nPoints() + 1  << ' '
                    << extraVerts[i] + surf.nPoints() + 1 << endl;
            }

            os  << "l " << extraVerts.last() + surf.nPoints() + 1
                << ' ' << e.end()+1 << endl;
        }
    }
}


// Return 0 (p close to start), 1(close to end) or -1.
CML::label CML::surfaceIntersection::classify
(
    const scalar startTol,
    const scalar endTol,
    const point& p,
    const edge& e,
    const pointField& points
)
{
    if (mag(p - points[e.start()]) < startTol)
    {
        return 0;
    }
    else if (mag(p - points[e.end()]) < endTol)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}


// ************************************************************************* //
