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

\*---------------------------------------------------------------------------*/

#include "polyMesh.hpp"
#include "transform.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// Update this with w2 if w2 nearer to pt.
template<class TrackingData>
inline bool CML::pointEdgePoint::update
(
    const point& pt,
    const pointEdgePoint& w2,
    const scalar tol,
    TrackingData& td
)
{
    scalar dist2 = magSqr(pt - w2.origin());

    if (!valid(td))
    {
        // current not yet set so use any value
        distSqr_ = dist2;
        origin_ = w2.origin();

        return true;
    }

    scalar diff = distSqr_ - dist2;

    if (diff < 0)
    {
        // already nearer to pt
        return false;
    }

    if ((diff < SMALL) || ((distSqr_ > SMALL) && (diff/distSqr_ < tol)))
    {
        // don't propagate small changes
        return false;
    }
    else
    {
        // update with new values
        distSqr_ = dist2;
        origin_ = w2.origin();

        return true;
    }
}


// Update this with w2 (information on same point)
template<class TrackingData>
inline bool CML::pointEdgePoint::update
(
    const pointEdgePoint& w2,
    const scalar tol,
    TrackingData& td
)
{
    if (!valid(td))
    {
        // current not yet set so use any value
        distSqr_ = w2.distSqr();
        origin_ = w2.origin();

        return true;
    }

    scalar diff = distSqr_ - w2.distSqr();

    if (diff < 0)
    {
        // already nearer to pt
        return false;
    }

    if ((diff < SMALL) || ((distSqr_ > SMALL) && (diff/distSqr_ < tol)))
    {
        // don't propagate small changes
        return false;
    }
    else
    {
        // update with new values
        distSqr_ =  w2.distSqr();
        origin_ = w2.origin();

        return true;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Null constructor
inline CML::pointEdgePoint::pointEdgePoint()
:
    origin_(point::max),
    distSqr_(GREAT)
{}


// Construct from origin, distance
inline CML::pointEdgePoint::pointEdgePoint
(
    const point& origin,
    const scalar distSqr
)
:
    origin_(origin),
    distSqr_(distSqr)
{}


// Construct as copy
inline CML::pointEdgePoint::pointEdgePoint(const pointEdgePoint& wpt)
:
    origin_(wpt.origin()),
    distSqr_(wpt.distSqr())
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

inline const CML::point& CML::pointEdgePoint::origin() const
{
    return origin_;
}


inline CML::scalar CML::pointEdgePoint::distSqr() const
{
    return distSqr_;
}


template<class TrackingData>
inline bool CML::pointEdgePoint::valid(TrackingData& td) const
{
    return origin_ != point::max;
}


// Checks for cyclic points
template<class TrackingData>
inline bool CML::pointEdgePoint::sameGeometry
(
    const pointEdgePoint& w2,
    const scalar tol,
    TrackingData& td
) const
{
    scalar diff = CML::mag(distSqr() - w2.distSqr());

    if (diff < SMALL)
    {
        return true;
    }
    else
    {
        if ((distSqr() > SMALL) && ((diff/distSqr()) < tol))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}


template<class TrackingData>
inline void CML::pointEdgePoint::leaveDomain
(
    const polyPatch& patch,
    const label patchPointI,
    const point& coord,
    TrackingData& td
)
{
    origin_ -= coord;
}


template<class TrackingData>
inline void CML::pointEdgePoint::transform
(
    const tensor& rotTensor,
    TrackingData& td
)
{
    origin_ = CML::transform(rotTensor, origin_);
}


// Update absolute geometric quantities. Note that distance (distSqr_)
// is not affected by leaving/entering domain.
template<class TrackingData>
inline void CML::pointEdgePoint::enterDomain
(
    const polyPatch& patch,
    const label patchPointI,
    const point& coord,
    TrackingData& td
)
{
    // back to absolute form
    origin_ += coord;
}


// Update this with information from connected edge
template<class TrackingData>
inline bool CML::pointEdgePoint::updatePoint
(
    const polyMesh& mesh,
    const label pointI,
    const label edgeI,
    const pointEdgePoint& edgeInfo,
    const scalar tol,
    TrackingData& td
)
{
    return update(mesh.points()[pointI], edgeInfo, tol, td);
}


// Update this with new information on same point
template<class TrackingData>
inline bool CML::pointEdgePoint::updatePoint
(
    const polyMesh& mesh,
    const label pointI,
    const pointEdgePoint& newPointInfo,
    const scalar tol,
    TrackingData& td
)
{
    return update(mesh.points()[pointI], newPointInfo, tol, td);
}


// Update this with new information on same point. No extra information.
template<class TrackingData>
inline bool CML::pointEdgePoint::updatePoint
(
    const pointEdgePoint& newPointInfo,
    const scalar tol,
    TrackingData& td
)
{
    return update(newPointInfo, tol, td);
}


// Update this with information from connected point
template<class TrackingData>
inline bool CML::pointEdgePoint::updateEdge
(
    const polyMesh& mesh,
    const label edgeI,
    const label pointI,
    const pointEdgePoint& pointInfo,
    const scalar tol,
    TrackingData& td
)
{
    const edge& e = mesh.edges()[edgeI];
    return update(e.centre(mesh.points()), pointInfo, tol, td);
}


template <class TrackingData>
inline bool CML::pointEdgePoint::equal
(
    const pointEdgePoint& rhs,
    TrackingData& td
) const
{
    return operator==(rhs);
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

inline bool CML::pointEdgePoint::operator==(const CML::pointEdgePoint& rhs)
 const
{
    return origin() == rhs.origin();
}


inline bool CML::pointEdgePoint::operator!=(const CML::pointEdgePoint& rhs)
 const
{
    return !(*this == rhs);
}


// ************************************************************************* //
