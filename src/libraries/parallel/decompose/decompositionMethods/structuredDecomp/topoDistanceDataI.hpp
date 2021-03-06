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

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Null constructor
inline CML::topoDistanceData::topoDistanceData()
:
    data_(-1),
    distance_(-1)
{}


// Construct from components
inline CML::topoDistanceData::topoDistanceData
(
    const label data,
    const label distance
)
:
    data_(data),
    distance_(distance)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template <class TrackingData>
inline bool CML::topoDistanceData::valid(TrackingData& td) const
{
    return distance_ != -1;
}


// No geometric data so never any problem on cyclics
template <class TrackingData>
inline bool CML::topoDistanceData::sameGeometry
(
    const polyMesh&,
    const topoDistanceData&,
    const scalar,
    TrackingData&
) const
{
    return true;
}


// No geometric data.
template <class TrackingData>
inline void CML::topoDistanceData::leaveDomain
(
    const polyMesh&,
    const polyPatch& patch,
    const label patchFaceI,
    const point& faceCentre,
    TrackingData&
)
{}


// No geometric data.
template <class TrackingData>
inline void CML::topoDistanceData::transform
(
    const polyMesh&,
    const tensor& rotTensor,
    TrackingData&
)
{}


// No geometric data.
template <class TrackingData>
inline void CML::topoDistanceData::enterDomain
(
    const polyMesh&,
    const polyPatch& patch,
    const label patchFaceI,
    const point& faceCentre,
    TrackingData&
)
{}


// Update cell with neighbouring face information
template <class TrackingData>
inline bool CML::topoDistanceData::updateCell
(
    const polyMesh&,
    const label thisCellI,
    const label neighbourFaceI,
    const topoDistanceData& neighbourInfo,
    const scalar tol,
    TrackingData&
)
{
    if (distance_ == -1)
    {
        data_ = neighbourInfo.data_;
        distance_ = neighbourInfo.distance_ + 1;
        return true;
    }
    else
    {
        return false;
    }
}


// Update face with neighbouring cell information
template <class TrackingData>
inline bool CML::topoDistanceData::updateFace
(
    const polyMesh& mesh,
    const label thisFaceI,
    const label neighbourCellI,
    const topoDistanceData& neighbourInfo,
    const scalar tol,
    TrackingData&
)
{
    // From cell to its faces.

    if (distance_ == -1)
    {
        operator=(neighbourInfo);
        return true;
    }
    else
    {
        return false;
    }
}


// Update face with coupled face information
template <class TrackingData>
inline bool CML::topoDistanceData::updateFace
(
    const polyMesh&,
    const label thisFaceI,
    const topoDistanceData& neighbourInfo,
    const scalar tol,
    TrackingData&
)
{
    // From face to face (e.g. coupled faces)
    if (distance_ == -1)
    {
        operator=(neighbourInfo);
        return true;
    }
    else
    {
        return false;
    }
}


template <class TrackingData>
inline bool CML::topoDistanceData::equal
(
    const topoDistanceData& rhs,
    TrackingData& td
) const
{
    return operator==(rhs);
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

inline bool CML::topoDistanceData::operator==
(
    const CML::topoDistanceData& rhs
) const
{
    return data() == rhs.data() && distance() == rhs.distance();
}


inline bool CML::topoDistanceData::operator!=
(
    const CML::topoDistanceData& rhs
) const
{
    return !(*this == rhs);
}


// ************************************************************************* //
