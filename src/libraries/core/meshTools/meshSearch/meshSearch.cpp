/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2018 OpenFOAM Foundation
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

#include "meshSearch.hpp"
#include "polyMesh.hpp"
#include "indexedOctree.hpp"
#include "DynamicList.hpp"
#include "demandDrivenData.hpp"
#include "treeDataCell.hpp"
#include "treeDataFace.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


namespace CML
{
    defineTypeNameAndDebug(meshSearch, 0);

    scalar meshSearch::tol_ = 1e-3;
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

bool CML::meshSearch::findNearer
(
    const point& sample,
    const pointField& points,
    label& nearestI,
    scalar& nearestDistSqr
)
{
    bool nearer = false;

    forAll(points, pointi)
    {
        scalar distSqr = magSqr(points[pointi] - sample);

        if (distSqr < nearestDistSqr)
        {
            nearestDistSqr = distSqr;
            nearestI = pointi;
            nearer = true;
        }
    }

    return nearer;
}


bool CML::meshSearch::findNearer
(
    const point& sample,
    const pointField& points,
    const labelList& indices,
    label& nearestI,
    scalar& nearestDistSqr
)
{
    bool nearer = false;

    forAll(indices, i)
    {
        label pointi = indices[i];

        scalar distSqr = magSqr(points[pointi] - sample);

        if (distSqr < nearestDistSqr)
        {
            nearestDistSqr = distSqr;
            nearestI = pointi;
            nearer = true;
        }
    }

    return nearer;
}


// tree based searching
CML::label CML::meshSearch::findNearestCellTree(const point& location) const
{
    const indexedOctree<treeDataCell>& tree = cellTree();

    pointIndexHit info = tree.findNearest
    (
        location,
        magSqr(tree.bb().max()-tree.bb().min())
    );

    if (!info.hit())
    {
        info = tree.findNearest(location, CML::sqr(GREAT));
    }
    return info.index();
}


// linear searching
CML::label CML::meshSearch::findNearestCellLinear(const point& location) const
{
    const vectorField& centres = mesh_.cellCentres();

    label nearestIndex = 0;
    scalar minProximity = magSqr(centres[nearestIndex] - location);

    findNearer
    (
        location,
        centres,
        nearestIndex,
        minProximity
    );

    return nearestIndex;
}


// walking from seed
CML::label CML::meshSearch::findNearestCellWalk
(
    const point& location,
    const label seedCelli
) const
{
    if (seedCelli < 0)
    {
        FatalErrorInFunction
            << "illegal seedCell:" << seedCelli << exit(FatalError);
    }

    // Walk in direction of face that decreases distance

    label curCelli = seedCelli;
    scalar distanceSqr = magSqr(mesh_.cellCentres()[curCelli] - location);

    bool closer;

    do
    {
        // Try neighbours of curCelli
        closer = findNearer
        (
            location,
            mesh_.cellCentres(),
            mesh_.cellCells()[curCelli],
            curCelli,
            distanceSqr
        );
    } while (closer);

    return curCelli;
}


// tree based searching
CML::label CML::meshSearch::findNearestFaceTree(const point& location) const
{
    // Search nearest cell centre.
    const indexedOctree<treeDataCell>& tree = cellTree();

    // Search with decent span
    pointIndexHit info = tree.findNearest
    (
        location,
        magSqr(tree.bb().max()-tree.bb().min())
    );

    if (!info.hit())
    {
        // Search with disparate span
        info = tree.findNearest(location, CML::sqr(GREAT));
    }


    // Now check any of the faces of the nearest cell
    const vectorField& centres = mesh_.faceCentres();
    const cell& ownFaces = mesh_.cells()[info.index()];

    label nearestFacei = ownFaces[0];
    scalar minProximity = magSqr(centres[nearestFacei] - location);

    findNearer
    (
        location,
        centres,
        ownFaces,
        nearestFacei,
        minProximity
    );

    return nearestFacei;
}


// linear searching
CML::label CML::meshSearch::findNearestFaceLinear(const point& location) const
{
    const vectorField& centres = mesh_.faceCentres();

    label nearestFacei = 0;
    scalar minProximity = magSqr(centres[nearestFacei] - location);

    findNearer
    (
        location,
        centres,
        nearestFacei,
        minProximity
    );

    return nearestFacei;
}


// walking from seed
CML::label CML::meshSearch::findNearestFaceWalk
(
    const point& location,
    const label seedFacei
) const
{
    if (seedFacei < 0)
    {
        FatalErrorInFunction
            << "illegal seedFace:" << seedFacei << exit(FatalError);
    }

    const vectorField& centres = mesh_.faceCentres();


    // Walk in direction of face that decreases distance

    label curFacei = seedFacei;
    scalar distanceSqr = magSqr(centres[curFacei] - location);

    while (true)
    {
        label betterFacei = curFacei;

        findNearer
        (
            location,
            centres,
            mesh_.cells()[mesh_.faceOwner()[curFacei]],
            betterFacei,
            distanceSqr
        );

        if (mesh_.isInternalFace(curFacei))
        {
            findNearer
            (
                location,
                centres,
                mesh_.cells()[mesh_.faceNeighbour()[curFacei]],
                betterFacei,
                distanceSqr
            );
        }

        if (betterFacei == curFacei)
        {
            break;
        }

        curFacei = betterFacei;
    }

    return curFacei;
}


CML::label CML::meshSearch::findCellLinear(const point& location) const
{
    bool cellFound = false;
    label n = 0;

    label celli = -1;

    while ((!cellFound) && (n < mesh_.nCells()))
    {
        if (mesh_.pointInCell(location, n, cellDecompMode_))
        {
            cellFound = true;
            celli = n;
        }
        else
        {
            n++;
        }
    }
    if (cellFound)
    {
        return celli;
    }
    else
    {
        return -1;
    }
}


// walking from seed
CML::label CML::meshSearch::findCellWalk
(
    const point& location,
    const label seedCelli
) const
{
    if (seedCelli < 0)
    {
        FatalErrorInFunction
            << "illegal seedCell:" << seedCelli << exit(FatalError);
    }

    if (mesh_.pointInCell(location, seedCelli, cellDecompMode_))
    {
        return seedCelli;
    }

    // Walk in direction of face that decreases distance
    label curCelli = seedCelli;
    scalar nearestDistSqr = magSqr(mesh_.cellCentres()[curCelli] - location);

    while(true)
    {
        // Try neighbours of curCelli

        const cell& cFaces = mesh_.cells()[curCelli];

        label nearestCelli = -1;

        forAll(cFaces, i)
        {
            label facei = cFaces[i];

            if (mesh_.isInternalFace(facei))
            {
                label celli = mesh_.faceOwner()[facei];
                if (celli == curCelli)
                {
                    celli = mesh_.faceNeighbour()[facei];
                }

                // Check if this is the correct cell
                if (mesh_.pointInCell(location, celli, cellDecompMode_))
                {
                    return celli;
                }

                // Also calculate the nearest cell
                scalar distSqr = magSqr(mesh_.cellCentres()[celli] - location);

                if (distSqr < nearestDistSqr)
                {
                    nearestDistSqr = distSqr;
                    nearestCelli = celli;
                }
            }
        }

        if (nearestCelli == -1)
        {
            return -1;
        }

        // Continue with the nearest cell
        curCelli = nearestCelli;
    }

    return -1;
}


CML::label CML::meshSearch::findNearestBoundaryFaceWalk
(
    const point& location,
    const label seedFacei
) const
{
    if (seedFacei < 0)
    {
        FatalErrorInFunction
            << "illegal seedFace:" << seedFacei << exit(FatalError);
    }

    // Start off from seedFacei

    label curFacei = seedFacei;

    const face& f =  mesh_.faces()[curFacei];

    scalar minDist = f.nearestPoint
    (
        location,
        mesh_.points()
    ).distance();

    bool closer;

    do
    {
        closer = false;

        // Search through all neighbouring boundary faces by going
        // across edges

        label lastFacei = curFacei;

        const labelList& myEdges = mesh_.faceEdges()[curFacei];

        forAll(myEdges, myEdgeI)
        {
            const labelList& neighbours = mesh_.edgeFaces()[myEdges[myEdgeI]];

            // Check any face which uses edge, is boundary face and
            // is not curFacei itself.

            forAll(neighbours, nI)
            {
                label facei = neighbours[nI];

                if
                (
                    (facei >= mesh_.nInternalFaces())
                 && (facei != lastFacei)
                )
                {
                    const face& f =  mesh_.faces()[facei];

                    pointHit curHit = f.nearestPoint
                    (
                        location,
                        mesh_.points()
                    );

                    // If the face is closer, reset current face and distance
                    if (curHit.distance() < minDist)
                    {
                        minDist = curHit.distance();
                        curFacei = facei;
                        closer = true;  // a closer neighbour has been found
                    }
                }
            }
        }
    } while (closer);

    return curFacei;
}


CML::vector CML::meshSearch::offset
(
    const point& bPoint,
    const label bFacei,
    const vector& dir
) const
{
    // Get the neighbouring cell
    label ownerCelli = mesh_.faceOwner()[bFacei];

    const point& c = mesh_.cellCentres()[ownerCelli];

    // Typical dimension: distance from point on face to cell centre
    scalar typDim = mag(c - bPoint);

    return tol_*typDim*dir;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from components
CML::meshSearch::meshSearch
(
    const polyMesh& mesh,
    const polyMesh::cellRepresentation cellDecompMode
)
:
    mesh_(mesh),
    cellDecompMode_(cellDecompMode)
{
    if (cellDecompMode_ == polyMesh::FACEDIAGTETS)
    {
        // Force construction of face diagonals
        (void)mesh.tetBasePtIs();
    }
}


// Construct with a custom bounding box
CML::meshSearch::meshSearch
(
    const polyMesh& mesh,
    const treeBoundBox& bb,
    const polyMesh::cellRepresentation cellDecompMode
)
:
    mesh_(mesh),
    cellDecompMode_(cellDecompMode)
{
    overallBbPtr_.reset(new treeBoundBox(bb));

    if (cellDecompMode_ == polyMesh::FACEDIAGTETS)
    {
        // Force construction of face diagonals
        (void)mesh.tetBasePtIs();
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::meshSearch::~meshSearch()
{
    clearOut();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const CML::indexedOctree<CML::treeDataFace>& CML::meshSearch::boundaryTree()
 const
{
    if (!boundaryTreePtr_.valid())
    {
        //
        // Construct tree
        //

        if (!overallBbPtr_.valid())
        {
            overallBbPtr_.reset
            (
                new treeBoundBox(mesh_.points())
            );

            treeBoundBox& overallBb = overallBbPtr_();

            // Extend slightly and make 3D
            overallBb = overallBb.extend(1e-4);
        }

        // all boundary faces (not just walls)
        labelList bndFaces(mesh_.nFaces()-mesh_.nInternalFaces());
        forAll(bndFaces, i)
        {
            bndFaces[i] = mesh_.nInternalFaces() + i;
        }

        boundaryTreePtr_.reset
        (
            new indexedOctree<treeDataFace>
            (
                treeDataFace    // all information needed to search faces
                (
                    false,                      // do not cache bb
                    mesh_,
                    bndFaces                    // boundary faces only
                ),
                overallBbPtr_(),                // overall search domain
                8,                              // maxLevel
                10,                             // leafsize
                3.0                             // duplicity
            )
        );
    }

    return boundaryTreePtr_();
}


const CML::indexedOctree<CML::treeDataCell>& CML::meshSearch::cellTree()
const
{
    if (!cellTreePtr_.valid())
    {
        //
        // Construct tree
        //

        if (!overallBbPtr_.valid())
        {
            overallBbPtr_.reset
            (
                new treeBoundBox(mesh_.points())
            );

            treeBoundBox& overallBb = overallBbPtr_();

            // Extend slightly and make 3D
            overallBb = overallBb.extend(1e-4);
        }

        cellTreePtr_.reset
        (
            new indexedOctree<treeDataCell>
            (
                treeDataCell
                (
                    false,          // not cache bb
                    mesh_,
                    cellDecompMode_ // cell decomposition mode for inside tests
                ),
                overallBbPtr_(),
                8,              // maxLevel
                10,             // leafsize
                6.0             // duplicity
            )
        );
    }

    return cellTreePtr_();
}


//// Is the point in the cell
//// Works by checking if there is a face inbetween the point and the cell
//// centre.
//// Check for internal uses proper face decomposition or just average normal.
//bool CML::meshSearch::pointInCell(const point& p, label cellI) const
//{
//    if (faceDecomp_)
//    {
//        const point& ctr = mesh_.cellCentres()[cellI];
//
//        vector dir(p - ctr);
//        scalar magDir = mag(dir);
//
//        // Check if any faces are hit by ray from cell centre to p.
//        // If none -> p is in cell.
//        const labelList& cFaces = mesh_.cells()[cellI];
//
//        // Make sure half_ray does not pick up any faces on the wrong
//        // side of the ray.
//        scalar oldTol = intersection::setPlanarTol(0.0);
//
//        forAll(cFaces, i)
//        {
//            label faceI = cFaces[i];
//
//            pointHit inter = mesh_.faces()[faceI].ray
//            (
//                ctr,
//                dir,
//                mesh_.points(),
//                intersection::HALF_RAY,
//                intersection::VECTOR
//            );
//
//            if (inter.hit())
//            {
//                scalar dist = inter.distance();
//
//                if (dist < magDir)
//                {
//                    // Valid hit. Hit face so point is not in cell.
//                    intersection::setPlanarTol(oldTol);
//
//                    return false;
//                }
//            }
//        }
//
//        intersection::setPlanarTol(oldTol);
//
//        // No face inbetween point and cell centre so point is inside.
//        return true;
//    }
//    else
//    {
//        const labelList& f = mesh_.cells()[cellI];
//        const labelList& owner = mesh_.faceOwner();
//        const vectorField& cf = mesh_.faceCentres();
//        const vectorField& Sf = mesh_.faceAreas();
//
//        forAll(f, facei)
//        {
//            label nFace = f[facei];
//            vector proj = p - cf[nFace];
//            vector normal = Sf[nFace];
//            if (owner[nFace] == cellI)
//            {
//                if ((normal & proj) > 0)
//                {
//                    return false;
//                }
//            }
//            else
//            {
//                if ((normal & proj) < 0)
//                {
//                    return false;
//                }
//            }
//        }
//
//        return true;
//    }
//}


CML::label CML::meshSearch::findNearestCell
(
    const point& location,
    const label seedCelli,
    const bool useTreeSearch
) const
{
    if (seedCelli == -1)
    {
        if (useTreeSearch)
        {
            return findNearestCellTree(location);
        }
        else
        {
            return findNearestCellLinear(location);
        }
    }
    else
    {
        return findNearestCellWalk(location, seedCelli);
    }
}


CML::label CML::meshSearch::findNearestFace
(
    const point& location,
    const label seedFacei,
    const bool useTreeSearch
) const
{
    if (seedFacei == -1)
    {
        if (useTreeSearch)
        {
            return findNearestFaceTree(location);
        }
        else
        {
            return findNearestFaceLinear(location);
        }
    }
    else
    {
        return findNearestFaceWalk(location, seedFacei);
    }
}


CML::label CML::meshSearch::findCell
(
    const point& location,
    const label seedCelli,
    const bool useTreeSearch
) const
{
    // Find the nearest cell centre to this location
    if (seedCelli == -1)
    {
        if (useTreeSearch)
        {
            return cellTree().findInside(location);
        }
        else
        {
            return findCellLinear(location);
        }
    }
    else
    {
        return findCellWalk(location, seedCelli);
    }
}


CML::label CML::meshSearch::findNearestBoundaryFace
(
    const point& location,
    const label seedFacei,
    const bool useTreeSearch
) const
{
    if (seedFacei == -1)
    {
        if (useTreeSearch)
        {
            const indexedOctree<treeDataFace>& tree = boundaryTree();

            pointIndexHit info = boundaryTree().findNearest
            (
                location,
                magSqr(tree.bb().max()-tree.bb().min())
            );

            if (!info.hit())
            {
                info = boundaryTree().findNearest
                (
                    location,
                    CML::sqr(GREAT)
                );
            }

            return tree.shapes().faceLabels()[info.index()];
        }
        else
        {
            scalar minDist = GREAT;

            label minFacei = -1;

            for
            (
                label facei = mesh_.nInternalFaces();
                facei < mesh_.nFaces();
                facei++
            )
            {
                const face& f =  mesh_.faces()[facei];

                pointHit curHit =
                    f.nearestPoint
                    (
                        location,
                        mesh_.points()
                    );

                if (curHit.distance() < minDist)
                {
                    minDist = curHit.distance();
                    minFacei = facei;
                }
            }
            return minFacei;
        }
    }
    else
    {
        return findNearestBoundaryFaceWalk(location, seedFacei);
    }
}


CML::pointIndexHit CML::meshSearch::intersection
(
    const point& pStart,
    const point& pEnd
) const
{
    pointIndexHit curHit = boundaryTree().findLine(pStart, pEnd);

    if (curHit.hit())
    {
        // Change index into octreeData into face label
        curHit.setIndex(boundaryTree().shapes().faceLabels()[curHit.index()]);
    }
    return curHit;
}


CML::List<CML::pointIndexHit> CML::meshSearch::intersections
(
    const point& pStart,
    const point& pEnd
) const
{
    DynamicList<pointIndexHit> hits;

    vector edgeVec = pEnd - pStart;
    edgeVec /= mag(edgeVec);

    point pt = pStart;

    pointIndexHit bHit;
    do
    {
        bHit = intersection(pt, pEnd);

        if (bHit.hit())
        {
            hits.append(bHit);

            const vector& area = mesh_.faceAreas()[bHit.index()];

            scalar typDim = CML::sqrt(mag(area));

            if ((mag(bHit.hitPoint() - pEnd)/typDim) < SMALL)
            {
                break;
            }

            // Restart from hitPoint shifted a little bit in the direction
            // of the destination

            pt =
                bHit.hitPoint()
              + offset(bHit.hitPoint(), bHit.index(), edgeVec);
        }

    } while (bHit.hit());


    hits.shrink();

    return hits;
}


bool CML::meshSearch::isInside(const point& p) const
{
    return (boundaryTree().getVolumeType(p) == volumeType::INSIDE);
}


// Delete all storage
void CML::meshSearch::clearOut()
{
    boundaryTreePtr_.clear();
    cellTreePtr_.clear();
    overallBbPtr_.clear();
}


void CML::meshSearch::correct()
{
    clearOut();
}


// ************************************************************************* //
