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

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Null constructor
inline CML::pointData::pointData()
:
    pointEdgePoint(),
    s_(GREAT),
    v_(point::max)
{}


// Construct from origin, distance
inline CML::pointData::pointData
(
    const point& origin,
    const scalar distSqr,
    const scalar s,
    const vector& v
)
:
    pointEdgePoint(origin, distSqr),
    s_(s),
    v_(v)
{}


// Construct as copy
inline CML::pointData::pointData(const pointData& wpt)
:
    pointEdgePoint(wpt),
    s_(wpt.s()),
    v_(wpt.v())
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

inline CML::scalar CML::pointData::s() const
{
    return s_;
}


inline const CML::vector& CML::pointData::v() const
{
    return v_;
}


template <class TrackingData>
inline void CML::pointData::transform
(
    const tensor& rotTensor,
    TrackingData& td
)
{
    pointEdgePoint::transform(rotTensor, td);
    v_ = CML::transform(rotTensor, v_);
}


// Update this with information from connected edge
template <class TrackingData>
inline bool CML::pointData::updatePoint
(
    const polyMesh& mesh,
    const label pointI,
    const label edgeI,
    const pointData& edgeInfo,
    const scalar tol,
    TrackingData& td
)
{
    if
    (
        pointEdgePoint::updatePoint
        (
            mesh,
            pointI,
            edgeI,
            edgeInfo,
            tol,
            td
        )
    )
    {
        s_ = edgeInfo.s_;
        v_ = edgeInfo.v_;
        return true;
    }
    else
    {
        return false;
    }
}

// Update this with new information on same point
template <class TrackingData>
inline bool CML::pointData::updatePoint
(
    const polyMesh& mesh,
    const label pointI,
    const pointData& newPointInfo,
    const scalar tol,
    TrackingData& td
)
{
    if
    (
        pointEdgePoint::updatePoint
        (
            mesh,
            pointI,
            newPointInfo,
            tol,
            td
        )
    )
    {
        s_ = newPointInfo.s_;
        v_ = newPointInfo.v_;
        return true;
    }
    else
    {
        return false;
    }
}


// Update this with new information on same point. No extra information.
template <class TrackingData>
inline bool CML::pointData::updatePoint
(
    const pointData& newPointInfo,
    const scalar tol,
    TrackingData& td
)
{
    if (pointEdgePoint::updatePoint(newPointInfo, tol, td))
    {
        s_ = newPointInfo.s_;
        v_ = newPointInfo.v_;
        return true;
    }
    else
    {
        return false;
    }
}


// Update this with information from connected point
template <class TrackingData>
inline bool CML::pointData::updateEdge
(
    const polyMesh& mesh,
    const label edgeI,
    const label pointI,
    const pointData& pointInfo,
    const scalar tol,
    TrackingData& td

)
{
    if
    (
        pointEdgePoint::updateEdge
        (
            mesh,
            edgeI,
            pointI,
            pointInfo,
            tol,
            td
        )
    )
    {
        s_ = pointInfo.s_;
        v_ = pointInfo.v_;
        return true;
    }
    else
    {
        return false;
    }
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

inline bool CML::pointData::operator==(const CML::pointData& rhs)
const
{
    return
        pointEdgePoint::operator==(rhs)
     && (s() == rhs.s())
     && (v() == rhs.v());
}


inline bool CML::pointData::operator!=(const CML::pointData& rhs)
const
{
    return !(*this == rhs);
}


// ************************************************************************* //
