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

#include "surfaceFeatures.hpp"
#include "triSurface.hpp"
#include "indexedOctree.hpp"
#include "treeDataEdge.hpp"
#include "treeDataPoint.hpp"
#include "meshTools.hpp"
#include "linePointRef.hpp"
#include "OFstream.hpp"
#include "IFstream.hpp"
#include "unitConversion.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


namespace CML
{
    defineTypeNameAndDebug(surfaceFeatures, 0);
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

//- Get nearest point on edge and classify position on edge.
CML::pointIndexHit CML::surfaceFeatures::edgeNearest
(
    const point& start,
    const point& end,
    const point& sample
)
{
    pointHit eHit = linePointRef(start, end).nearestDist(sample);

    // Classification of position on edge.
    label endPoint;

    if (eHit.hit())
    {
        // Nearest point is on edge itself.
        // Note: might be at or very close to endpoint. We should use tolerance
        // here.
        endPoint = -1;
    }
    else
    {
        // Nearest point has to be one of the end points. Find out
        // which one.
        if
        (
            mag(eHit.rawPoint() - start)
          < mag(eHit.rawPoint() - end)
        )
        {
            endPoint = 0;
        }
        else
        {
            endPoint = 1;
        }
    }

    return pointIndexHit(eHit.hit(), eHit.rawPoint(), endPoint);
}


// Go from selected edges only to a value for every edge
CML::List<CML::surfaceFeatures::edgeStatus> CML::surfaceFeatures::toStatus()
 const
{
    List<edgeStatus> edgeStat(surf_.nEdges(), NONE);

    // Region edges first
    for (label i = 0; i < externalStart_; i++)
    {
        edgeStat[featureEdges_[i]] = REGION;
    }

    // External edges
    for (label i = externalStart_; i < internalStart_; i++)
    {
        edgeStat[featureEdges_[i]] = EXTERNAL;
    }

    // Internal edges
    for (label i = internalStart_; i < featureEdges_.size(); i++)
    {
        edgeStat[featureEdges_[i]] = INTERNAL;
    }

    return edgeStat;
}


// Set from value for every edge
void CML::surfaceFeatures::setFromStatus(const List<edgeStatus>& edgeStat)
{
    // Count

    label nRegion = 0;
    label nExternal = 0;
    label nInternal = 0;

    forAll(edgeStat, edgeI)
    {
        if (edgeStat[edgeI] == REGION)
        {
            nRegion++;
        }
        else if (edgeStat[edgeI] == EXTERNAL)
        {
            nExternal++;
        }
        else if (edgeStat[edgeI] == INTERNAL)
        {
            nInternal++;
        }
    }

    externalStart_ = nRegion;
    internalStart_ = externalStart_ + nExternal;


    // Copy

    featureEdges_.setSize(internalStart_ + nInternal);

    label regionI = 0;
    label externalI = externalStart_;
    label internalI = internalStart_;

    forAll(edgeStat, edgeI)
    {
        if (edgeStat[edgeI] == REGION)
        {
            featureEdges_[regionI++] = edgeI;
        }
        else if (edgeStat[edgeI] == EXTERNAL)
        {
            featureEdges_[externalI++] = edgeI;
        }
        else if (edgeStat[edgeI] == INTERNAL)
        {
            featureEdges_[internalI++] = edgeI;
        }
    }

    // Calculate consistent feature points
    calcFeatPoints(edgeStat);
}


//construct feature points where more than 2 feature edges meet
void CML::surfaceFeatures::calcFeatPoints(const List<edgeStatus>& edgeStat)
{
    DynamicList<label> featurePoints(surf_.nPoints()/1000);

    const labelListList& pointEdges = surf_.pointEdges();

    forAll(pointEdges, pointI)
    {
        const labelList& pEdges = pointEdges[pointI];

        label nFeatEdges = 0;

        forAll(pEdges, i)
        {
            if (edgeStat[pEdges[i]] != NONE)
            {
                nFeatEdges++;
            }
        }

        if (nFeatEdges > 2)
        {
            featurePoints.append(pointI);
        }
    }

    featurePoints_.transfer(featurePoints);
}


// Returns next feature edge connected to pointI with correct value.
CML::label CML::surfaceFeatures::nextFeatEdge
(
    const List<edgeStatus>& edgeStat,
    const labelList& featVisited,
    const label unsetVal,
    const label prevEdgeI,
    const label vertI
) const
{
    const labelList& pEdges = surf_.pointEdges()[vertI];

    label nextEdgeI = -1;

    forAll(pEdges, i)
    {
        label edgeI = pEdges[i];

        if
        (
            edgeI != prevEdgeI
         && edgeStat[edgeI] != NONE
         && featVisited[edgeI] == unsetVal
        )
        {
            if (nextEdgeI == -1)
            {
                nextEdgeI = edgeI;
            }
            else
            {
                // More than one feature edge to choose from. End of segment.
                return -1;
            }
        }
    }

    return nextEdgeI;
}


// Finds connected feature edges by walking from prevEdgeI in direction of
// prevPointI. Marks feature edges visited in featVisited by assigning them
// the current feature line number. Returns cumulative length of edges walked.
// Works in one of two modes:
// - mark : step to edges with featVisited = -1.
//          Mark edges visited with currentFeatI.
// - clear : step to edges with featVisited = currentFeatI
//           Mark edges visited with -2 and erase from feature edges.
CML::surfaceFeatures::labelScalar CML::surfaceFeatures::walkSegment
(
    const bool mark,
    const List<edgeStatus>& edgeStat,
    const label startEdgeI,
    const label startPointI,
    const label currentFeatI,
    labelList& featVisited
)
{
    label edgeI = startEdgeI;

    label vertI = startPointI;

    scalar visitedLength = 0.0;

    label nVisited = 0;

    if (findIndex(featurePoints_, startPointI) >= 0)
    {
        // Do not walk across feature points

        return labelScalar(nVisited, visitedLength);
    }

    //
    // Now we have:
    //    edgeI : first edge on this segment
    //    vertI : one of the endpoints of this segment
    //
    // Start walking, marking off edges (in featVisited)
    // as we go along.
    //

    label unsetVal;

    if (mark)
    {
        unsetVal = -1;
    }
    else
    {
        unsetVal = currentFeatI;
    }

    do
    {
        // Step to next feature edge with value unsetVal
        edgeI = nextFeatEdge(edgeStat, featVisited, unsetVal, edgeI, vertI);

        if (edgeI == -1 || edgeI == startEdgeI)
        {
            break;
        }

        // Mark with current value. If in clean mode also remove feature edge.

        if (mark)
        {
            featVisited[edgeI] = currentFeatI;
        }
        else
        {
            featVisited[edgeI] = -2;
        }


        // Step to next vertex on edge
        const edge& e = surf_.edges()[edgeI];

        vertI = e.otherVertex(vertI);


        // Update cumulative length
        visitedLength += e.mag(surf_.localPoints());

        nVisited++;

        if (nVisited > surf_.nEdges())
        {
            Warning<< "walkSegment : reached iteration limit in walking "
                << "feature edges on surface from edge:" << startEdgeI
                << " vertex:" << startPointI << nl
                << "Returning with large length" << endl;

            return labelScalar(nVisited, GREAT);
        }
    }
    while (true);

    return labelScalar(nVisited, visitedLength);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::surfaceFeatures::surfaceFeatures(const triSurface& surf)
:
    surf_(surf),
    featurePoints_(0),
    featureEdges_(0),
    externalStart_(0),
    internalStart_(0)
{}


// Construct from components.
CML::surfaceFeatures::surfaceFeatures
(
    const triSurface& surf,
    const labelList& featurePoints,
    const labelList& featureEdges,
    const label externalStart,
    const label internalStart
)
:
    surf_(surf),
    featurePoints_(featurePoints),
    featureEdges_(featureEdges),
    externalStart_(externalStart),
    internalStart_(externalStart)
{}


// Construct from surface, angle and min length
CML::surfaceFeatures::surfaceFeatures
(
    const triSurface& surf,
    const scalar includedAngle,
    const scalar minLen,
    const label minElems
)
:
    surf_(surf),
    featurePoints_(0),
    featureEdges_(0),
    externalStart_(0),
    internalStart_(0)
{
    findFeatures(includedAngle);

    if (minLen > 0 || minElems > 0)
    {
        trimFeatures(minLen, minElems);
    }
}


//- Construct from dictionary
CML::surfaceFeatures::surfaceFeatures
(
    const triSurface& surf,
    const dictionary& featInfoDict
)
:
    surf_(surf),
    featurePoints_(featInfoDict.lookup("featurePoints")),
    featureEdges_(featInfoDict.lookup("featureEdges")),
    externalStart_(readLabel(featInfoDict.lookup("externalStart"))),
    internalStart_(readLabel(featInfoDict.lookup("internalStart")))
{}


//- Construct from file
CML::surfaceFeatures::surfaceFeatures
(
    const triSurface& surf,
    const fileName& fName
)
:
    surf_(surf),
    featurePoints_(0),
    featureEdges_(0),
    externalStart_(0),
    internalStart_(0)
{
    IFstream str(fName);

    dictionary featInfoDict(str);

    featureEdges_ = labelList(featInfoDict.lookup("featureEdges"));
    featurePoints_ = labelList(featInfoDict.lookup("featurePoints"));
    externalStart_ = readLabel(featInfoDict.lookup("externalStart"));
    internalStart_ = readLabel(featInfoDict.lookup("internalStart"));
}


//- Construct as copy
CML::surfaceFeatures::surfaceFeatures(const surfaceFeatures& sf)
:
    surf_(sf.surface()),
    featurePoints_(sf.featurePoints()),
    featureEdges_(sf.featureEdges()),
    externalStart_(sf.externalStart()),
    internalStart_(sf.internalStart())
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::labelList CML::surfaceFeatures::selectFeatureEdges
(
    const bool regionEdges,
    const bool externalEdges,
    const bool internalEdges
) const
{
    DynamicList<label> selectedEdges;

    if (regionEdges)
    {
        selectedEdges.setCapacity(selectedEdges.size() + nRegionEdges());

        for (label i = 0; i < externalStart_; i++)
        {
            selectedEdges.append(featureEdges_[i]);
        }
    }

    if (externalEdges)
    {
        selectedEdges.setCapacity(selectedEdges.size() + nExternalEdges());

        for (label i = externalStart_; i < internalStart_; i++)
        {
            selectedEdges.append(featureEdges_[i]);
        }
    }

    if (internalEdges)
    {
        selectedEdges.setCapacity(selectedEdges.size() + nInternalEdges());

        for (label i = internalStart_; i < featureEdges_.size(); i++)
        {
            selectedEdges.append(featureEdges_[i]);
        }
    }

    return selectedEdges.shrink();
}


void CML::surfaceFeatures::findFeatures(const scalar includedAngle)
{
    scalar minCos = CML::cos(degToRad(180.0 - includedAngle));

    const labelListList& edgeFaces = surf_.edgeFaces();
    const vectorField& faceNormals = surf_.faceNormals();
    const pointField& points = surf_.points();

    // Per edge whether is feature edge.
    List<edgeStatus> edgeStat(surf_.nEdges(), NONE);

    forAll(edgeFaces, edgeI)
    {
        const labelList& eFaces = edgeFaces[edgeI];

        if (eFaces.size() != 2)
        {
            // Non-manifold. What to do here? Is region edge? external edge?
            edgeStat[edgeI] = REGION;
        }
        else
        {
            label face0 = eFaces[0];
            label face1 = eFaces[1];

            if (surf_[face0].region() != surf_[face1].region())
            {
                edgeStat[edgeI] = REGION;
            }
            else
            {
                if ((faceNormals[face0] & faceNormals[face1]) < minCos)
                {

                    // Check if convex or concave by looking at angle
                    // between face centres and normal
                    vector f0Tof1 =
                        surf_[face1].centre(points)
                        - surf_[face0].centre(points);

                    if ((f0Tof1 & faceNormals[face0]) > 0.0)
                    {
                        edgeStat[edgeI] = INTERNAL;
                    }
                    else
                    {
                        edgeStat[edgeI] = EXTERNAL;
                    }
                }
            }
        }
    }

    setFromStatus(edgeStat);
}


// Remove small strings of edges. First assigns string number to
// every edge and then works out the length of them.
CML::labelList CML::surfaceFeatures::trimFeatures
(
    const scalar minLen,
    const label minElems
)
{
    // Convert feature edge list to status per edge.
    List<edgeStatus> edgeStat(toStatus());


    // Mark feature edges according to connected lines.
    // -1: unassigned
    // -2: part of too small a feature line
    // >0: feature line number
    labelList featLines(surf_.nEdges(), -1);

    // Current featureline number.
    label featI = 0;

    // Current starting edge
    label startEdgeI = 0;

    do
    {
        // Find unset featureline
        for (; startEdgeI < edgeStat.size(); startEdgeI++)
        {
            if
            (
                edgeStat[startEdgeI] != NONE
             && featLines[startEdgeI] == -1
            )
            {
                // Found unassigned feature edge.
                break;
            }
        }

        if (startEdgeI == edgeStat.size())
        {
            // No unset feature edge found. All feature edges have line number
            // assigned.
            break;
        }

        featLines[startEdgeI] = featI;

        const edge& startEdge = surf_.edges()[startEdgeI];

        // Walk 'left' and 'right' from startEdge.
        labelScalar leftPath =
            walkSegment
            (
                true,           // 'mark' mode
                edgeStat,
                startEdgeI,     // edge, not yet assigned to a featureLine
                startEdge[0],   // start of edge
                featI,          // Mark value
                featLines       // Mark for all edges
            );

        labelScalar rightPath =
            walkSegment
            (
                true,
                edgeStat,
                startEdgeI,
                startEdge[1],   // end of edge
                featI,
                featLines
            );

        if
        (
            (
                leftPath.len_
              + rightPath.len_
              + startEdge.mag(surf_.localPoints())
              < minLen
            )
         || (leftPath.n_ + rightPath.n_ + 1 < minElems)
        )
        {
            // Rewalk same route (recognizable by featLines == featI)
            // to reset featLines.

            featLines[startEdgeI] = -2;

            walkSegment
            (
                false,          // 'clean' mode
                edgeStat,
                startEdgeI,     // edge, not yet assigned to a featureLine
                startEdge[0],   // start of edge
                featI,          // Unset value
                featLines       // Mark for all edges
            );

            walkSegment
            (
                false,
                edgeStat,
                startEdgeI,
                startEdge[1],   // end of edge
                featI,
                featLines
            );
        }
        else
        {
            featI++;
        }
    }
    while (true);

    // Unmark all feature lines that have featLines=-2
    forAll(featureEdges_, i)
    {
        label edgeI = featureEdges_[i];

        if (featLines[edgeI] == -2)
        {
            edgeStat[edgeI] = NONE;
        }
    }

    // Convert back to edge labels
    setFromStatus(edgeStat);

    return featLines;
}


void CML::surfaceFeatures::writeDict(Ostream& writeFile) const
{

    dictionary featInfoDict;
    featInfoDict.add("externalStart", externalStart_);
    featInfoDict.add("internalStart", internalStart_);
    featInfoDict.add("featureEdges", featureEdges_);
    featInfoDict.add("featurePoints", featurePoints_);

    featInfoDict.write(writeFile);
}


void CML::surfaceFeatures::write(const fileName& fName) const
{
    OFstream str(fName);

    writeDict(str);
}


void CML::surfaceFeatures::writeObj(const fileName& prefix) const
{
    OFstream regionStr(prefix + "_regionEdges.obj");
    Pout<< "Writing region edges to " << regionStr.name() << endl;

    label verti = 0;
    for (label i = 0; i < externalStart_; i++)
    {
        const edge& e = surf_.edges()[featureEdges_[i]];

        meshTools::writeOBJ(regionStr, surf_.localPoints()[e[0]]); verti++;
        meshTools::writeOBJ(regionStr, surf_.localPoints()[e[1]]); verti++;
        regionStr << "l " << verti-1 << ' ' << verti << endl;
    }


    OFstream externalStr(prefix + "_externalEdges.obj");
    Pout<< "Writing external edges to " << externalStr.name() << endl;

    verti = 0;
    for (label i = externalStart_; i < internalStart_; i++)
    {
        const edge& e = surf_.edges()[featureEdges_[i]];

        meshTools::writeOBJ(externalStr, surf_.localPoints()[e[0]]); verti++;
        meshTools::writeOBJ(externalStr, surf_.localPoints()[e[1]]); verti++;
        externalStr << "l " << verti-1 << ' ' << verti << endl;
    }

    OFstream internalStr(prefix + "_internalEdges.obj");
    Pout<< "Writing internal edges to " << internalStr.name() << endl;

    verti = 0;
    for (label i = internalStart_; i < featureEdges_.size(); i++)
    {
        const edge& e = surf_.edges()[featureEdges_[i]];

        meshTools::writeOBJ(internalStr, surf_.localPoints()[e[0]]); verti++;
        meshTools::writeOBJ(internalStr, surf_.localPoints()[e[1]]); verti++;
        internalStr << "l " << verti-1 << ' ' << verti << endl;
    }

    OFstream pointStr(prefix + "_points.obj");
    Pout<< "Writing feature points to " << pointStr.name() << endl;

    forAll(featurePoints_, i)
    {
        label pointI = featurePoints_[i];

        meshTools::writeOBJ(pointStr, surf_.localPoints()[pointI]);
    }
}


// Get nearest vertex on patch for every point of surf in pointSet.
CML::Map<CML::label> CML::surfaceFeatures::nearestSamples
(
    const labelList& pointLabels,
    const pointField& samples,
    const scalarField& maxDistSqr
) const
{
    // Build tree out of all samples.

    // Note: cannot be done one the fly - gcc4.4 compiler bug.
    treeBoundBox bb(samples);

    indexedOctree<treeDataPoint> ppTree
    (
        treeDataPoint(samples),   // all information needed to do checks
        bb,                       // overall search domain
        8,      // maxLevel
        10,     // leafsize
        3.0     // duplicity
    );

    // From patch point to surface point
    Map<label> nearest(2*pointLabels.size());

    const pointField& surfPoints = surf_.localPoints();

    forAll(pointLabels, i)
    {
        label surfPointI = pointLabels[i];

        const point& surfPt = surfPoints[surfPointI];

        pointIndexHit info = ppTree.findNearest
        (
            surfPt,
            maxDistSqr[i]
        );

        if (!info.hit())
        {
            FatalErrorInFunction
                << "Problem for point "
                << surfPointI << " in tree " << ppTree.bb()
                << abort(FatalError);
        }

        label sampleI = info.index();

        if (magSqr(samples[sampleI] - surfPt) < maxDistSqr[sampleI])
        {
            nearest.insert(sampleI, surfPointI);
        }
    }


    if (debug)
    {
        //
        // Dump to obj file
        //

        Pout<< endl
            << "Dumping nearest surface feature points to nearestSamples.obj"
            << endl
            << "View this Lightwave-OBJ file with e.g. javaview" << endl
            << endl;

        OFstream objStream("nearestSamples.obj");

        label vertI = 0;
        forAllConstIter(Map<label>, nearest, iter)
        {
            meshTools::writeOBJ(objStream, samples[iter.key()]); vertI++;
            meshTools::writeOBJ(objStream, surfPoints[iter()]); vertI++;
            objStream<< "l " << vertI-1 << ' ' << vertI << endl;
        }
    }

    return nearest;
}


// Get nearest sample point for regularly sampled points along
// selected edges. Return map from sample to edge label
CML::Map<CML::label> CML::surfaceFeatures::nearestSamples
(
    const labelList& selectedEdges,
    const pointField& samples,
    const scalarField& sampleDist,
    const scalarField& maxDistSqr,
    const scalar minSampleDist
) const
{
    const pointField& surfPoints = surf_.localPoints();
    const edgeList& surfEdges = surf_.edges();

    scalar maxSearchSqr = max(maxDistSqr);

    //Note: cannot be done one the fly - gcc4.4 compiler bug.
    treeBoundBox bb(samples);

    indexedOctree<treeDataPoint> ppTree
    (
        treeDataPoint(samples),   // all information needed to do checks
        bb,                         // overall search domain
        8,      // maxLevel
        10,     // leafsize
        3.0     // duplicity
    );

    // From patch point to surface edge.
    Map<label> nearest(2*selectedEdges.size());

    forAll(selectedEdges, i)
    {
        label surfEdgeI = selectedEdges[i];

        const edge& e = surfEdges[surfEdgeI];

        if (debug && (i % 1000) == 0)
        {
            Pout<< "looking at surface feature edge " << surfEdgeI
                << " verts:" << e << " points:" << surfPoints[e[0]]
                << ' ' << surfPoints[e[1]] << endl;
        }

        // Normalized edge vector
        vector eVec = e.vec(surfPoints);
        scalar eMag = mag(eVec);
        eVec /= eMag;


        //
        // Sample along edge
        //

        bool exit = false;

        // Coordinate along edge (0 .. eMag)
        scalar s = 0.0;

        while (true)
        {
            point edgePoint(surfPoints[e.start()] + s*eVec);

            pointIndexHit info = ppTree.findNearest
            (
                edgePoint,
                maxSearchSqr
            );

            if (!info.hit())
            {
                // No point close enough to surface edge.
                break;
            }

            label sampleI = info.index();

            if (magSqr(info.hitPoint() - edgePoint) < maxDistSqr[sampleI])
            {
                nearest.insert(sampleI, surfEdgeI);
            }

            if (exit)
            {
                break;
            }

            // Step to next sample point using local distance.
            // Truncate to max 1/minSampleDist samples per feature edge.
            s += max(minSampleDist*eMag, sampleDist[sampleI]);

            if (s >= (1-minSampleDist)*eMag)
            {
                // Do one more sample, at edge endpoint
                s = eMag;
                exit = true;
            }
        }
    }



    if (debug)
    {
        // Dump to obj file

        Pout<< "Dumping nearest surface edges to nearestEdges.obj\n"
            << "View this Lightwave-OBJ file with e.g. javaview\n" << endl;

        OFstream objStream("nearestEdges.obj");

        label vertI = 0;
        forAllConstIter(Map<label>, nearest, iter)
        {
            const label sampleI = iter.key();

            meshTools::writeOBJ(objStream, samples[sampleI]); vertI++;

            const edge& e = surfEdges[iter()];

            point nearPt =
                e.line(surfPoints).nearestDist(samples[sampleI]).rawPoint();

            meshTools::writeOBJ(objStream, nearPt); vertI++;

            objStream<< "l " << vertI-1 << ' ' << vertI << endl;
        }
    }

    return nearest;
}


// Get nearest edge on patch for regularly sampled points along the feature
// edges. Return map from patch edge to feature edge.
//
// Q: using point-based sampleDist and maxDist (distance to look around
// each point). Should they be edge-based e.g. by averaging or max()?
CML::Map<CML::pointIndexHit> CML::surfaceFeatures::nearestEdges
(
    const labelList& selectedEdges,
    const edgeList& sampleEdges,
    const labelList& selectedSampleEdges,
    const pointField& samplePoints,
    const scalarField& sampleDist,
    const scalarField& maxDistSqr,
    const scalar minSampleDist
) const
{
    // Build tree out of selected sample edges.
    indexedOctree<treeDataEdge> ppTree
    (
        treeDataEdge
        (
            false,
            sampleEdges,
            samplePoints,
            selectedSampleEdges
        ),                          // geometric info container for edges
        treeBoundBox(samplePoints), // overall search domain
        8,      // maxLevel
        10,     // leafsize
        3.0     // duplicity
    );

    const pointField& surfPoints = surf_.localPoints();
    const edgeList& surfEdges = surf_.edges();

    scalar maxSearchSqr = max(maxDistSqr);

    Map<pointIndexHit> nearest(2*sampleEdges.size());

    //
    // Loop over all selected edges. Sample at regular intervals. Find nearest
    // sampleEdges (using octree)
    //

    forAll(selectedEdges, i)
    {
        label surfEdgeI = selectedEdges[i];

        const edge& e = surfEdges[surfEdgeI];

        if (debug && (i % 1000) == 0)
        {
            Pout<< "looking at surface feature edge " << surfEdgeI
                << " verts:" << e << " points:" << surfPoints[e[0]]
                << ' ' << surfPoints[e[1]] << endl;
        }

        // Normalized edge vector
        vector eVec = e.vec(surfPoints);
        scalar eMag = mag(eVec);
        eVec /= eMag;


        //
        // Sample along edge
        //

        bool exit = false;

        // Coordinate along edge (0 .. eMag)
        scalar s = 0.0;

        while (true)
        {
            point edgePoint(surfPoints[e.start()] + s*eVec);

            pointIndexHit info = ppTree.findNearest
            (
                edgePoint,
                maxSearchSqr
            );

            if (!info.hit())
            {
                // No edge close enough to surface edge.
                break;
            }

            label index = info.index();

            label sampleEdgeI = ppTree.shapes().edgeLabels()[index];

            const edge& e = sampleEdges[sampleEdgeI];

            if (magSqr(info.hitPoint() - edgePoint) < maxDistSqr[e.start()])
            {
                nearest.insert
                (
                    sampleEdgeI,
                    pointIndexHit(true, edgePoint, surfEdgeI)
                );
            }

            if (exit)
            {
                break;
            }

            // Step to next sample point using local distance.
            // Truncate to max 1/minSampleDist samples per feature edge.
            // s += max(minSampleDist*eMag, sampleDist[e.start()]);
            s += 0.01*eMag;

            if (s >= (1-minSampleDist)*eMag)
            {
                // Do one more sample, at edge endpoint
                s = eMag;
                exit = true;
            }
        }
    }


    if (debug)
    {
        // Dump to obj file

        Pout<< "Dumping nearest surface feature edges to nearestEdges.obj\n"
            << "View this Lightwave-OBJ file with e.g. javaview\n" << endl;

        OFstream objStream("nearestEdges.obj");

        label vertI = 0;
        forAllConstIter(Map<pointIndexHit>, nearest, iter)
        {
            const label sampleEdgeI = iter.key();

            const edge& sampleEdge = sampleEdges[sampleEdgeI];

            // Write line from edgeMid to point on feature edge
            meshTools::writeOBJ(objStream, sampleEdge.centre(samplePoints));
            vertI++;

            meshTools::writeOBJ(objStream, iter().rawPoint());
            vertI++;

            objStream<< "l " << vertI-1 << ' ' << vertI << endl;
        }
    }

    return nearest;
}


// Get nearest surface edge for every sample. Return in form of
// labelLists giving surfaceEdge label&intersectionpoint.
void CML::surfaceFeatures::nearestSurfEdge
(
    const labelList& selectedEdges,
    const pointField& samples,
    scalar searchSpanSqr,   // Search span
    labelList& edgeLabel,
    labelList& edgeEndPoint,
    pointField& edgePoint
) const
{
    edgeLabel.setSize(samples.size());
    edgeEndPoint.setSize(samples.size());
    edgePoint.setSize(samples.size());

    const pointField& localPoints = surf_.localPoints();

    indexedOctree<treeDataEdge> ppTree
    (
        treeDataEdge
        (
            false,
            surf_.edges(),
            localPoints,
            selectedEdges
        ),          // all information needed to do geometric checks
        treeBoundBox(localPoints),  // overall search domain
        8,      // maxLevel
        10,     // leafsize
        3.0     // duplicity
    );

    forAll(samples, i)
    {
        const point& sample = samples[i];

        pointIndexHit info = ppTree.findNearest
        (
            sample,
            searchSpanSqr
        );

        if (!info.hit())
        {
            edgeLabel[i] = -1;
        }
        else
        {
            edgeLabel[i] = selectedEdges[info.index()];

            // Need to recalculate to classify edgeEndPoint
            const edge& e = surf_.edges()[edgeLabel[i]];

            pointIndexHit pHit = edgeNearest
            (
                localPoints[e.start()],
                localPoints[e.end()],
                sample
            );

            edgePoint[i] = pHit.rawPoint();
            edgeEndPoint[i] = pHit.index();
        }
    }
}


// Get nearest point on nearest feature edge for every sample (is edge)
void CML::surfaceFeatures::nearestSurfEdge
(
    const labelList& selectedEdges,

    const edgeList& sampleEdges,
    const labelList& selectedSampleEdges,
    const pointField& samplePoints,

    const vector& searchSpan,   // Search span
    labelList& edgeLabel,       // label of surface edge or -1
    pointField& pointOnEdge,    // point on above edge
    pointField& pointOnFeature  // point on sample edge
) const
{
    edgeLabel.setSize(selectedSampleEdges.size());
    pointOnEdge.setSize(selectedSampleEdges.size());
    pointOnFeature.setSize(selectedSampleEdges.size());

    indexedOctree<treeDataEdge> ppTree
    (
        treeDataEdge
        (
            false,
            surf_.edges(),
            surf_.localPoints(),
            selectedEdges
        ),          // all information needed to do geometric checks
        treeBoundBox(surf_.localPoints()),  // overall search domain
        8,      // maxLevel
        10,     // leafsize
        3.0     // duplicity
    );

    forAll(selectedSampleEdges, i)
    {
        const edge& e = sampleEdges[selectedSampleEdges[i]];

        linePointRef edgeLine = e.line(samplePoints);

        point eMid(edgeLine.centre());

        treeBoundBox tightest(eMid - searchSpan, eMid + searchSpan);

        pointIndexHit info = ppTree.findNearest
        (
            edgeLine,
            tightest,
            pointOnEdge[i]
        );

        if (!info.hit())
        {
            edgeLabel[i] = -1;
        }
        else
        {
            edgeLabel[i] = selectedEdges[info.index()];

            pointOnFeature[i] = info.hitPoint();
        }
    }
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

void CML::surfaceFeatures::operator=(const surfaceFeatures& rhs)
{
    // Check for assignment to self
    if (this == &rhs)
    {
        FatalErrorInFunction
            << "Attempted assignment to self"
            << abort(FatalError);
    }

    if (&surf_ != &rhs.surface())
    {
        FatalErrorInFunction
            << "Operating on different surfaces"
            << abort(FatalError);
    }

    featurePoints_ = rhs.featurePoints();
    featureEdges_ = rhs.featureEdges();
    externalStart_ = rhs.externalStart();
    internalStart_ = rhs.internalStart();
}


// ************************************************************************* //
