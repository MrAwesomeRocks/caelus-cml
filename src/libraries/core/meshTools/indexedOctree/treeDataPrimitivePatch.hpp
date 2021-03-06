/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
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

Class
    CML::treeDataPrimitivePatch

Description
    Encapsulation of data needed to search on PrimitivePatches

SourceFiles
    treeDataPrimitivePatch.cpp

\*---------------------------------------------------------------------------*/

#ifndef treeDataPrimitivePatch_H
#define treeDataPrimitivePatch_H

#include "treeBoundBoxList.hpp"
#include "volumeType.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes
template<class Type> class indexedOctree;


/*---------------------------------------------------------------------------*\
                 Class treeDataPrimitivePatchName Declaration
\*---------------------------------------------------------------------------*/

TemplateName(treeDataPrimitivePatch);


/*---------------------------------------------------------------------------*\
                   Class treeDataPrimitivePatch Declaration
\*---------------------------------------------------------------------------*/

template<class PatchType>
class treeDataPrimitivePatch
:
    public treeDataPrimitivePatchName
{
    // Private data

        //- Underlying geometry
        const PatchType& patch_;

        //- Whether to precalculate and store face bounding box
        const bool cacheBb_;

        //- Tolerance to use for intersection tests
        const scalar planarTol_;

        //- face bounding boxes (valid only if cacheBb_)
        treeBoundBoxList bbs_;


    // Private Member Functions

        //- Calculate face bounding box
        static treeBoundBox calcBb(const pointField&, const face&);

        //- Initialise all member data
        void update();


public:


    class findNearestOp
    {
        const indexedOctree<treeDataPrimitivePatch>& tree_;

    public:

        findNearestOp(const indexedOctree<treeDataPrimitivePatch>& tree);

        void operator()
        (
            const labelUList& indices,
            const point& sample,

            scalar& nearestDistSqr,
            label& minIndex,
            point& nearestPoint
        ) const;

        //- Calculates nearest (to line) point in shape.
        //  Returns point and distance (squared)
        void operator()
        (
            const labelUList& indices,
            const linePointRef& ln,

            treeBoundBox& tightest,
            label& minIndex,
            point& linePoint,
            point& nearestPoint
        ) const;
    };


    class findIntersectOp
    {
        const indexedOctree<treeDataPrimitivePatch>& tree_;

    public:

        findIntersectOp(const indexedOctree<treeDataPrimitivePatch>& tree);

        //- Calculate intersection of any face with ray. Sets result
        //  accordingly. Used to find first intersection.
        bool operator()
        (
            const label index,
            const point& start,
            const point& end,
            point& intersectionPoint
        ) const;
    };


    class findAllIntersectOp
    {
        const indexedOctree<treeDataPrimitivePatch>& tree_;

        DynamicList<label>& shapeMask_;

    public:

        findAllIntersectOp
        (
            const indexedOctree<treeDataPrimitivePatch>& tree,
            DynamicList<label>& shapeMask
        );

        //- Calculate intersection of unique face with ray. Sets result
        //  accordingly. Used to find all faces.
        bool operator()
        (
            const label index,
            const point& start,
            const point& end,
            point& intersectionPoint
        ) const;
    };


    class findSelfIntersectOp
    {
        const indexedOctree<treeDataPrimitivePatch>& tree_;

        const label edgeID_;

    public:

        findSelfIntersectOp
        (
            const indexedOctree<treeDataPrimitivePatch>& tree,
            const label edgeID
        );

        //- Calculate intersection of face with edge of patch. Excludes
        //  faces that use edgeID. Used to find self intersection.
        bool operator()
        (
            const label index,
            const point& start,
            const point& end,
            point& intersectionPoint
        ) const;
    };


    // Constructors

        //- Construct from patch.
        treeDataPrimitivePatch
        (
            const bool cacheBb,
            const PatchType&,
            const scalar planarTol
        );


    // Member Functions

        // Access

            label size() const
            {
                return patch_.size();
            }

            //- Get representative point cloud for all shapes inside
            //  (one point per shape)
            pointField shapePoints() const;

            //- Return access to the underlying patch
            const PatchType& patch() const
            {
                return patch_;
            }


        // Search

            //- Get type (inside,outside,mixed,unknown) of point w.r.t. surface.
            //  Only makes sense for closed surfaces.
            volumeType getVolumeType
            (
                const indexedOctree<treeDataPrimitivePatch<PatchType> >&,
                const point&
            ) const;

            //- Does shape at index overlap bb
            bool overlaps
            (
                const label index,
                const treeBoundBox& sampleBb
            ) const;

            //- Does shape at index overlap sphere
            bool overlaps
            (
                const label index,
                const point& centre,
                const scalar radiusSqr
            ) const;

            //- Helper: find intersection of line with shapes
            static bool findIntersection
            (
                const indexedOctree<treeDataPrimitivePatch<PatchType> >& tree,
                const label index,
                const point& start,
                const point& end,
                point& intersectionPoint
            );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "indexedOctree.hpp"
#include "triangleFuncs.hpp"
#include "triSurfaceTools.hpp"
#include "triFace.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class PatchType>
CML::treeBoundBox CML::treeDataPrimitivePatch<PatchType>::calcBb
(
    const pointField& points,
    const face& f
)
{
    treeBoundBox bb(points[f[0]], points[f[0]]);

    for (label fp = 1; fp < f.size(); fp++)
    {
        const point& p = points[f[fp]];

        bb.min() = min(bb.min(), p);
        bb.max() = max(bb.max(), p);
    }
    return bb;
}


template<class PatchType>
void CML::treeDataPrimitivePatch<PatchType>::update()
{
    if (cacheBb_)
    {
        bbs_.setSize(patch_.size());

        forAll(patch_, i)
        {
            bbs_[i] = calcBb(patch_.points(), patch_[i]);
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from components
template<class PatchType>
CML::treeDataPrimitivePatch<PatchType>::treeDataPrimitivePatch
(
    const bool cacheBb,
    const PatchType& patch,
    const scalar planarTol
)
:
    patch_(patch),
    cacheBb_(cacheBb),
    planarTol_(planarTol)
{
    update();
}


template<class PatchType>
CML::treeDataPrimitivePatch<PatchType>::findNearestOp::findNearestOp
(
    const indexedOctree<treeDataPrimitivePatch<PatchType> >& tree
)
:
    tree_(tree)
{}


template<class PatchType>
CML::treeDataPrimitivePatch<PatchType>::findIntersectOp::findIntersectOp
(
    const indexedOctree<treeDataPrimitivePatch<PatchType> >& tree
)
:
    tree_(tree)
{}


template<class PatchType>
CML::treeDataPrimitivePatch<PatchType>::findAllIntersectOp::findAllIntersectOp
(
    const indexedOctree<treeDataPrimitivePatch<PatchType> >& tree,
    DynamicList<label>& shapeMask
)
:
    tree_(tree),
    shapeMask_(shapeMask)
{}


template<class PatchType>
CML::treeDataPrimitivePatch<PatchType>::
findSelfIntersectOp::findSelfIntersectOp
(
    const indexedOctree<treeDataPrimitivePatch<PatchType> >& tree,
    const label edgeID
)
:
    tree_(tree),
    edgeID_(edgeID)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class PatchType>
CML::pointField CML::treeDataPrimitivePatch<PatchType>::shapePoints() const
{
    pointField cc(patch_.size());

    forAll(patch_, i)
    {
        cc[i] = patch_[i].centre(patch_.points());
    }

    return cc;
}


//- Get type (inside,outside,mixed,unknown) of point w.r.t. surface.
//  Only makes sense for closed surfaces.
template<class PatchType>
CML::volumeType CML::treeDataPrimitivePatch<PatchType>::getVolumeType
(
    const indexedOctree<treeDataPrimitivePatch<PatchType> >& oc,
    const point& sample
) const
{
    // Need to determine whether sample is 'inside' or 'outside'
    // Done by finding nearest face. This gives back a face which is
    // guaranteed to contain nearest point. This point can be
    // - in interior of face: compare to face normal
    // - on edge of face: compare to edge normal
    // - on point of face: compare to point normal
    // Unfortunately the octree does not give us back the intersection point
    // or where on the face it has hit so we have to recreate all that
    // information.


    // Find nearest face to sample
    pointIndexHit info = oc.findNearest(sample, sqr(GREAT));

    if (info.index() == -1)
    {
        FatalErrorInFunction
            << "Could not find " << sample << " in octree."
            << abort(FatalError);
    }

    // Get actual intersection point on face
    label facei = info.index();

    if (debug & 2)
    {
        Pout<< "getSampleType : sample:" << sample
            << " nearest face:" << facei;
    }

    const pointField& points = patch_.localPoints();
    const typename PatchType::FaceType& f = patch_.localFaces()[facei];

    // Retest to classify where on face info is. Note: could be improved. We
    // already have point.

    pointHit curHit = f.nearestPoint(sample, points);
    const vector area = f.area(points);
    const point& curPt = curHit.rawPoint();

    //
    // 1] Check whether sample is above face
    //

    if (curHit.hit())
    {
        // Nearest point inside face. Compare to face normal.

        if (debug & 2)
        {
            Pout<< " -> face hit:" << curPt
                << " comparing to face normal " << area << endl;
        }
        return indexedOctree<treeDataPrimitivePatch>::getSide
        (
            area,
            sample - curPt
        );
    }

    if (debug & 2)
    {
        Pout<< " -> face miss:" << curPt;
    }

    //
    // 2] Check whether intersection is on one of the face vertices or
    //    face centre
    //

    const scalar typDimSqr = mag(area) + VSMALL;


    forAll(f, fp)
    {
        if ((magSqr(points[f[fp]] - curPt)/typDimSqr) < planarTol_)
        {
            // Face intersection point equals face vertex fp

            // Calculate point normal (wrong: uses face normals instead of
            // triangle normals)

            return indexedOctree<treeDataPrimitivePatch>::getSide
            (
                patch_.pointNormals()[f[fp]],
                sample - curPt
            );
        }
    }

    const point fc(f.centre(points));

    if ((magSqr(fc - curPt)/typDimSqr) < planarTol_)
    {
        // Face intersection point equals face centre. Normal at face centre
        // is already average of face normals

        if (debug & 2)
        {
            Pout<< " -> centre hit:" << fc
                << " distance:" << magSqr(fc - curPt)/typDimSqr << endl;
        }

        return indexedOctree<treeDataPrimitivePatch>::getSide
        (
            area,
            sample - curPt
        );
    }



    //
    // 3] Get the 'real' edge the face intersection is on
    //

    const labelList& fEdges = patch_.faceEdges()[facei];

    forAll(fEdges, fEdgeI)
    {
        label edgeI = fEdges[fEdgeI];
        const edge& e = patch_.edges()[edgeI];

        pointHit edgeHit = e.line(points).nearestDist(sample);

        if ((magSqr(edgeHit.rawPoint() - curPt)/typDimSqr) < planarTol_)
        {
            // Face intersection point lies on edge e

            // Calculate edge normal (wrong: uses face normals instead of
            // triangle normals)
            const labelList& eFaces = patch_.edgeFaces()[edgeI];

            vector edgeNormal(vector::zero);

            forAll(eFaces, i)
            {
                edgeNormal += patch_.faceNormals()[eFaces[i]];
            }

            if (debug & 2)
            {
                Pout<< " -> real edge hit point:" << edgeHit.rawPoint()
                    << " comparing to edge normal:" << edgeNormal
                    << endl;
            }

            // Found face intersection point on this edge. Compare to edge
            // normal
            return indexedOctree<treeDataPrimitivePatch>::getSide
            (
                edgeNormal,
                sample - curPt
            );
        }
    }


    //
    // 4] Get the internal edge the face intersection is on
    //

    forAll(f, fp)
    {
        pointHit edgeHit = linePointRef
        (
            points[f[fp]],
            fc
        ).nearestDist(sample);

        if ((magSqr(edgeHit.rawPoint() - curPt)/typDimSqr) < planarTol_)
        {
            // Face intersection point lies on edge between two face triangles

            // Calculate edge normal as average of the two triangle normals
            vector e = points[f[fp]] - fc;
            vector ePrev = points[f[f.rcIndex(fp)]] - fc;
            vector eNext = points[f[f.fcIndex(fp)]] - fc;

            vector nLeft = ePrev ^ e;
            nLeft /= mag(nLeft) + VSMALL;

            vector nRight = e ^ eNext;
            nRight /= mag(nRight) + VSMALL;

            if (debug & 2)
            {
                Pout<< " -> internal edge hit point:" << edgeHit.rawPoint()
                    << " comparing to edge normal "
                    << 0.5*(nLeft + nRight)
                    << endl;
            }

            // Found face intersection point on this edge. Compare to edge
            // normal
            return indexedOctree<treeDataPrimitivePatch>::getSide
            (
                0.5*(nLeft + nRight),
                sample - curPt
            );
        }
    }

    if (debug & 2)
    {
        Pout<< "Did not find sample " << sample
            << " anywhere related to nearest face " << facei << endl
            << "Face:";

        forAll(f, fp)
        {
            Pout<< "    vertex:" << f[fp] << "  coord:" << points[f[fp]]
                << endl;
        }
    }

    // Can't determine status of sample with respect to nearest face.
    // Either
    // - tolerances are wrong. (if e.g. face has zero area)
    // - or (more likely) surface is not closed.

    return volumeType::UNKNOWN;
}


// Check if any point on shape is inside cubeBb.
template<class PatchType>
bool CML::treeDataPrimitivePatch<PatchType>::overlaps
(
    const label index,
    const treeBoundBox& cubeBb
) const
{
    // 1. Quick rejection: bb does not intersect face bb at all
    if (cacheBb_)
    {
        if (!cubeBb.overlaps(bbs_[index]))
        {
            return false;
        }
    }
    else
    {
        if (!cubeBb.overlaps(calcBb(patch_.points(), patch_[index])))
        {
            return false;
        }
    }


    // 2. Check if one or more face points inside

    const pointField& points = patch_.points();
    const typename PatchType::FaceType& f = patch_[index];

    if (cubeBb.containsAny(points, f))
    {
        return true;
    }

    // 3. Difficult case: all points are outside but connecting edges might
    // go through cube. Use triangle-bounding box intersection.
    const point fc = f.centre(points);

    if (f.size() == 3)
    {
        return triangleFuncs::intersectBb
        (
            points[f[0]],
            points[f[1]],
            points[f[2]],
            cubeBb
        );
    }
    else
    {
        forAll(f, fp)
        {
            bool triIntersects = triangleFuncs::intersectBb
            (
                points[f[fp]],
                points[f[f.fcIndex(fp)]],
                fc,
                cubeBb
            );

            if (triIntersects)
            {
                return true;
            }
        }
    }

    return false;
}


// Check if any point on shape is inside sphere.
template<class PatchType>
bool CML::treeDataPrimitivePatch<PatchType>::overlaps
(
    const label index,
    const point& centre,
    const scalar radiusSqr
) const
{
    // 1. Quick rejection: sphere does not intersect face bb at all
    if (cacheBb_)
    {
        if (!bbs_[index].overlaps(centre, radiusSqr))
        {
            return false;
        }
    }
    else
    {
        if (!calcBb(patch_.points(), patch_[index]).overlaps(centre, radiusSqr))
        {
            return false;
        }
    }

    const pointField& points = patch_.points();
    const face& f = patch_[index];

    pointHit nearHit = f.nearestPoint(centre, points);

    // If the distance to the nearest point on the face from the sphere centres
    // is within the radius, then the sphere touches the face.
    if (sqr(nearHit.distance()) < radiusSqr)
    {
        return true;
    }

    return false;
}


template<class PatchType>
void CML::treeDataPrimitivePatch<PatchType>::findNearestOp::operator()
(
    const labelUList& indices,
    const point& sample,

    scalar& nearestDistSqr,
    label& minIndex,
    point& nearestPoint
) const
{
    const treeDataPrimitivePatch<PatchType>& shape = tree_.shapes();
    const PatchType& patch = shape.patch();

    const pointField& points = patch.points();

    forAll(indices, i)
    {
        const label index = indices[i];
        const typename PatchType::FaceType& f = patch[index];

        pointHit nearHit = f.nearestPoint(sample, points);
        scalar distSqr = sqr(nearHit.distance());

        if (distSqr < nearestDistSqr)
        {
            nearestDistSqr = distSqr;
            minIndex = index;
            nearestPoint = nearHit.rawPoint();
        }
    }
}


template<class PatchType>
void CML::treeDataPrimitivePatch<PatchType>::findNearestOp::operator()
(
    const labelUList& indices,
    const linePointRef& ln,

    treeBoundBox& tightest,
    label& minIndex,
    point& linePoint,
    point& nearestPoint
) const
{
    NotImplemented;
}


template<class PatchType>
bool CML::treeDataPrimitivePatch<PatchType>::findIntersectOp::operator()
(
    const label index,
    const point& start,
    const point& end,
    point& intersectionPoint
) const
{
    return findIntersection(tree_, index, start, end, intersectionPoint);
}


template<class PatchType>
bool CML::treeDataPrimitivePatch<PatchType>::findAllIntersectOp::operator()
(
    const label index,
    const point& start,
    const point& end,
    point& intersectionPoint
) const
{
    if (!shapeMask_.empty() && findIndex(shapeMask_, index) != -1)
    {
        return false;
    }

    return findIntersection(tree_, index, start, end, intersectionPoint);
}


template<class PatchType>
bool CML::treeDataPrimitivePatch<PatchType>::findSelfIntersectOp::operator()
(
    const label index,
    const point& start,
    const point& end,
    point& intersectionPoint
) const
{
    if (edgeID_ == -1)
    {
        FatalErrorInFunction
            << "EdgeID not set. Please set edgeID to the index of"
            << " the edge you are testing"
            << exit(FatalError);
    }

    const treeDataPrimitivePatch<PatchType>& shape = tree_.shapes();
    const PatchType& patch = shape.patch();

    const typename PatchType::FaceType& f = patch.localFaces()[index];
    const edge& e = patch.edges()[edgeID_];

    if (findIndex(f, e[0]) == -1 && findIndex(f, e[1]) == -1)
    {
        return findIntersection(tree_, index, start, end, intersectionPoint);
    }
    else
    {
        return false;
    }
}


template<class PatchType>
bool CML::treeDataPrimitivePatch<PatchType>::findIntersection
(
    const indexedOctree<treeDataPrimitivePatch<PatchType> >& tree,
    const label index,
    const point& start,
    const point& end,
    point& intersectionPoint
)
{
    const treeDataPrimitivePatch<PatchType>& shape = tree.shapes();
    const PatchType& patch = shape.patch();

    const pointField& points = patch.points();
    const typename PatchType::FaceType& f = patch[index];

    // Do quick rejection test
    if (shape.cacheBb_)
    {
        const treeBoundBox& faceBb = shape.bbs_[index];

        if ((faceBb.posBits(start) & faceBb.posBits(end)) != 0)
        {
            // start and end in same block outside of faceBb.
            return false;
        }
    }

    const vector dir(end - start);
    pointHit inter;

    if (f.size() == 3)
    {
        inter = triPointRef
        (
            points[f[0]],
            points[f[1]],
            points[f[2]]
        ).intersection(start, dir, intersection::HALF_RAY, shape.planarTol_);
    }
    else
    {
        const pointField& faceCentres = patch.faceCentres();

        inter = f.intersection
        (
            start,
            dir,
            faceCentres[index],
            points,
            intersection::HALF_RAY,
            shape.planarTol_
        );
    }

    if (inter.hit() && inter.distance() <= 1)
    {
        // Note: no extra test on whether intersection is in front of us
        // since using half_ray
        intersectionPoint = inter.hitPoint();

        return true;
    }
    else
    {
        return false;
    }
}



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
