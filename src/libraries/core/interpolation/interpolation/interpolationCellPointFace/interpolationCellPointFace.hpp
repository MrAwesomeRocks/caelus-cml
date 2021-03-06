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

Class
    CML::interpolationCellPointFace

Description
    CML::interpolationCellPointFace

\*---------------------------------------------------------------------------*/

#ifndef interpolationCellPointFace_H
#define interpolationCellPointFace_H

#include "interpolation.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class interpolationCellPointFace Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class interpolationCellPointFace
:
    public interpolation<Type>
{
    // Private data

        //- Interpolated volfield
        const GeometricField<Type, pointPatchField, pointMesh> psip_;

        //- Linearly interpolated volfield
        const GeometricField<Type, fvsPatchField, surfaceMesh> psis_;

        bool findTet
        (
            const vector& position,
            const label nFace,
            vector tetPoints[],
            label tetLabelCandidate[],
            label tetPointLabels[],
            scalar phi[],
            scalar phiCandidate[],
            label& closestFace,
            scalar& minDistance
        ) const;

        bool findTriangle
        (
            const vector& position,
            const label nFace,
            label tetPointLabels[],
            scalar phi[]
        ) const;


public:

    //- Runtime type information
    TypeName("cellPointFace");


    // Constructors

        //- Construct from components
        interpolationCellPointFace
        (
            const GeometricField<Type, fvPatchField, volMesh>& psi
        );


    // Member Functions

        //- Interpolate field to the given point in the given cell
        Type interpolate
        (
            const vector& position,
            const label cellI,
            const label faceI = -1
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "volFields.hpp"
#include "polyMesh.hpp"
#include "volPointInterpolation.hpp"
#include "linear.hpp"
#include "findCellPointFaceTet.hpp"
#include "findCellPointFaceTriangle.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * Constructor * * * * * * * * * * * * * * * //

template<class Type>
interpolationCellPointFace<Type>::interpolationCellPointFace
(
    const GeometricField<Type, fvPatchField, volMesh>& psi
)
:
    interpolation<Type>(psi),
    psip_(volPointInterpolation::New(psi.mesh()).interpolate(psi)),
    psis_(linearInterpolate(psi))
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
Type interpolationCellPointFace<Type>::interpolate
(
    const vector& position,
    const label cellI,
    const label faceI
) const
{
    Type ts[4];
    vector tetPoints[4];
    scalar phi[4], phiCandidate[4];
    label tetLabelCandidate[2], tetPointLabels[2];

    Type t = pTraits<Type>::zero;

    // only use face information when the position is on a face
    if (faceI < 0)
    {
        const vector& cellCentre = this->pMesh_.cellCentres()[cellI];
        const labelList& cellFaces = this->pMesh_.cells()[cellI];

        vector projection = position - cellCentre;
        tetPoints[3] = cellCentre;

        // *********************************************************************
        // project the cell-center through the point onto the face
        // and get the closest face, ...
        // *********************************************************************

        bool foundTet = false;
        label closestFace = -1;
        scalar minDistance = GREAT;

        forAll(cellFaces, faceI)
        {
            label nFace = cellFaces[faceI];

            vector normal = this->pMeshFaceAreas_[nFace];
            normal /= mag(normal);

            const vector& faceCentreTmp = this->pMeshFaceCentres_[nFace];

            scalar multiplierNumerator = (faceCentreTmp - cellCentre) & normal;
            scalar multiplierDenominator = projection & normal;

            // if normal and projection are not orthogonal this could
            // be the one...
            if (mag(multiplierDenominator) > SMALL)
            {
                scalar multiplier = multiplierNumerator/multiplierDenominator;
                vector iPoint = cellCentre + multiplier*projection;
                scalar dist = mag(position - iPoint);

                if (dist < minDistance)
                {
                    closestFace = nFace;
                    minDistance = dist;
                }
            }
        }

        // *********************************************************************
        // find the tetrahedron containing 'position'
        // from the cell center, face center and
        // two other points on the face
        // *********************************************************************

        minDistance = GREAT;
        if (closestFace != -1)
        {
            label nFace = closestFace;
            foundTet = findTet
            (
                position,
                nFace,
                tetPoints,
                tetLabelCandidate,
                tetPointLabels,
                phi,
                phiCandidate,
                closestFace,
                minDistance
            );
        }

        if (!foundTet)
        {
            // check if the position is 'just' outside a tet
            if (minDistance < 1.0e-5)
            {
                foundTet = true;
                for (label i=0; i<4; i++)
                {
                    phi[i] = phiCandidate[i];
                }
                tetPointLabels[0] = tetLabelCandidate[0];
                tetPointLabels[1] = tetLabelCandidate[1];
            }
        }

        // *********************************************************************
        // if the search failed check all the cell-faces
        // *********************************************************************

        if (!foundTet)
        {
            minDistance = GREAT;

            label faceI = 0;
            while (faceI < cellFaces.size() && !foundTet)
            {
                label nFace = cellFaces[faceI];
                if (nFace < this->pMeshFaceAreas_.size())
                {
                    foundTet = findTet
                    (
                        position,
                        nFace,
                        tetPoints,
                        tetLabelCandidate,
                        tetPointLabels,
                        phi,
                        phiCandidate,
                        closestFace,
                        minDistance
                    );
                }
                faceI++;
            }
        }

        if (!foundTet)
        {
            // check if the position is 'just' outside a tet
            // this time with a more tolerant limit
            if (minDistance < 1.0e-3)
            {
                foundTet = true;
                for (label i=0; i<4; i++)
                {
                    phi[i] = phiCandidate[i];
                }
                tetPointLabels[0] = tetLabelCandidate[0];
                tetPointLabels[1] = tetLabelCandidate[1];
            }
        }

        // *********************************************************************
        // if the tet was found do the interpolation,
        // otherwise use the closest face value
        // *********************************************************************

        if (foundTet)
        {
            for (label i=0; i<2; i++)
            {
                ts[i] = psip_[tetPointLabels[i]];
            }

            if (closestFace < psis_.size())
            {
                ts[2] = psis_[closestFace];
            }
            else
            {
                label patchI =
                    this->pMesh_.boundaryMesh().whichPatch(closestFace);

                // If the boundary patch is not empty use the face value
                // else use the cell value
                if (this->psi_.boundaryField()[patchI].size())
                {
                    ts[2] = this->psi_.boundaryField()[patchI]
                    [
                        this->pMesh_.boundaryMesh()[patchI].whichFace
                        (
                            closestFace
                        )
                    ];
                }
                else
                {
                    ts[2] = this->psi_[cellI];
                }
            }

            ts[3] = this->psi_[cellI];

            for (label n=0; n<4; n++)
            {
                phi[n] = min(1.0, phi[n]);
                phi[n] = max(0.0, phi[n]);

                t += phi[n]*ts[n];
            }
        }
        else
        {
            Info<< "interpolationCellPointFace<Type>::interpolate"
                << "(const vector&, const label cellI) const : "
                << "search failed; using closest cellFace value" << endl
                << "cell number " << cellI << tab
                << "position " << position << endl;

            if (closestFace < psis_.size())
            {
                t = psis_[closestFace];
            }
            else
            {
                label patchI =
                    this->pMesh_.boundaryMesh().whichPatch(closestFace);

                // If the boundary patch is not empty use the face value
                // else use the cell value
                if (this->psi_.boundaryField()[patchI].size())
                {
                    t = this->psi_.boundaryField()[patchI]
                    [
                        this->pMesh_.boundaryMesh()[patchI].whichFace
                        (
                            closestFace
                        )
                    ];
                }
                else
                {
                    t = this->psi_[cellI];
                }
            }
        }
    }
    else
    {
        bool foundTriangle = findTriangle
        (
            position,
            faceI,
            tetPointLabels,
            phi
        );

        if (foundTriangle)
        {
            // add up the point values ...
            for (label i=0; i<2; i++)
            {
                Type vel = psip_[tetPointLabels[i]];
                t += phi[i]*vel;
            }

            // ... and the face value
            if (faceI < psis_.size())
            {
                t += phi[2]*psis_[faceI];
            }
            else
            {
                label patchI = this->pMesh_.boundaryMesh().whichPatch(faceI);

                // If the boundary patch is not empty use the face value
                // else use the cell value
                if (this->psi_.boundaryField()[patchI].size())
                {
                    t += phi[2]*this->psi_.boundaryField()[patchI]
                        [this->pMesh_.boundaryMesh()[patchI].whichFace(faceI)];
                }
                else
                {
                    t += phi[2]*this->psi_[cellI];
                }
            }
        }
        else
        {
            // use face value only
            if (faceI < psis_.size())
            {
                t = psis_[faceI];
            }
            else
            {
                label patchI = this->pMesh_.boundaryMesh().whichPatch(faceI);

                // If the boundary patch is not empty use the face value
                // else use the cell value
                if (this->psi_.boundaryField()[patchI].size())
                {
                    t = this->psi_.boundaryField()[patchI]
                        [this->pMesh_.boundaryMesh()[patchI].whichFace(faceI)];
                }
                else
                {
                    t = this->psi_[cellI];
                }
            }
        }
    }

    return t;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
