/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2012 OpenFOAM Foundation
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

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// Update this with w2 if w2 nearer to pt.
template<class TrackingData>
inline bool wallPointYPlus::update
(
    const point& pt,
    const wallPointYPlus& w2,
    const scalar tol,
    TrackingData& td
)
{
    scalar dist2 = magSqr(pt - w2.origin());

    if (valid(td))
    {
        scalar diff = distSqr() - dist2;

        if (diff < 0)
        {
            // already nearer to pt
            return false;
        }

        if ((diff < SMALL) || ((distSqr() > SMALL) && (diff/distSqr() < tol)))
        {
            // don't propagate small changes
            return false;
        }
    }


    // Either *this is not yet valid or w2 is closer
    {
        // only propagate if interesting (i.e. y+ < 100)
        scalar yPlus = CML::sqrt(dist2)/w2.data();

        if (yPlus < yPlusCutOff)
        {
            // update with new values
            distSqr() = dist2;
            origin() = w2.origin();
            data() = w2.data();

            return true;
        }
        else
        {
            return false;
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Null constructor
inline wallPointYPlus::wallPointYPlus()
:
    wallPointData<scalar>()
{
    // Important: value of yStar where meshWave does not come.
    data() = 1.0;
}


// Construct from components
inline wallPointYPlus::wallPointYPlus
(
    const point& origin,
    const scalar yStar,
    const scalar distSqr
)
:
    wallPointData<scalar>(origin, yStar, distSqr)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// Update this with w2 if w2 nearer to pt.
template<class TrackingData>
inline bool wallPointYPlus::updateCell
(
    const polyMesh& mesh,
    const label thisCellI,
    const label neighbourFaceI,
    const wallPointYPlus& neighbourWallInfo,
    const scalar tol,
    TrackingData& td
)
{
    const vectorField& cellCentres = mesh.primitiveMesh::cellCentres();

    return update
    (
        cellCentres[thisCellI],
        neighbourWallInfo,
        tol,
        td
    );
}


// Update this with w2 if w2 nearer to pt.
template<class TrackingData>
inline bool wallPointYPlus::updateFace
(
    const polyMesh& mesh,
    const label thisFaceI,
    const label neighbourCellI,
    const wallPointYPlus& neighbourWallInfo,
    const scalar tol,
    TrackingData& td
)
{
    const vectorField& faceCentres = mesh.faceCentres();

    return update
    (
        faceCentres[thisFaceI],
        neighbourWallInfo,
        tol,
        td
    );
}


// Update this with w2 if w2 nearer to pt.
template<class TrackingData>
inline bool wallPointYPlus::updateFace
(
    const polyMesh& mesh,
    const label thisFaceI,
    const wallPointYPlus& neighbourWallInfo,
    const scalar tol,
    TrackingData& td
)
{
    const vectorField& faceCentres = mesh.faceCentres();

    return update
    (
        faceCentres[thisFaceI],
        neighbourWallInfo,
        tol,
        td
    );
}


// ************************************************************************* //

} // End namespace CML

// ************************************************************************* //
