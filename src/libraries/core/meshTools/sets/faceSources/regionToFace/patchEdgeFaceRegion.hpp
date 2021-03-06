/*---------------------------------------------------------------------------*\
Copyright (C) 2012-2016 OpenFOAM Foundation
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
    CML::patchEdgeFaceRegion

Description
    Transport of region for use in PatchEdgeFaceWave.

    Set element to -2 to denote blocked.

SourceFiles
    patchEdgeFaceRegionI.hpp
    patchEdgeFaceRegion.cpp

\*---------------------------------------------------------------------------*/

#ifndef patchEdgeFaceRegion_H
#define patchEdgeFaceRegion_H

#include "point.hpp"
#include "label.hpp"
#include "scalar.hpp"
#include "tensor.hpp"
#include "indirectPrimitivePatch.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes
class polyPatch;
class polyMesh;

// Forward declaration of friend functions and operators

class patchEdgeFaceRegion;

Istream& operator>>(Istream&, patchEdgeFaceRegion&);
Ostream& operator<<(Ostream&, const patchEdgeFaceRegion&);


/*---------------------------------------------------------------------------*\
                           Class patchEdgeFaceRegion Declaration
\*---------------------------------------------------------------------------*/

class patchEdgeFaceRegion
{
    // Private data

        //- region
        label region_;

    // Private Member Functions

        //- Combine current with w2. Update region_ if w2 has smaller
        //  quantities and returns true.
        template<class TrackingData>
        inline bool update
        (
            const patchEdgeFaceRegion& w2,
            const scalar tol,
            TrackingData& td
        );


public:

    // Constructors

        //- Construct null
        inline patchEdgeFaceRegion();

        //- Construct from origin, distance
        inline patchEdgeFaceRegion(const label);


    // Member Functions

        // Access

            inline label region() const;


        // Needed by meshWave

            //- Check whether origin has been changed at all or
            //  still contains original (invalid) value.
            template<class TrackingData>
            inline bool valid(TrackingData& td) const;

            //- Apply rotation matrix
            template<class TrackingData>
            inline void transform
            (
                const polyMesh& mesh,
                const indirectPrimitivePatch& patch,
                const tensor& rotTensor,
                const scalar tol,
                TrackingData& td
            );

            //- Influence of face on edge
            template<class TrackingData>
            inline bool updateEdge
            (
                const polyMesh& mesh,
                const indirectPrimitivePatch& patch,
                const label edgeI,
                const label faceI,
                const patchEdgeFaceRegion& faceInfo,
                const scalar tol,
                TrackingData& td
            );

            //- New information for edge (from e.g. coupled edge)
            template<class TrackingData>
            inline bool updateEdge
            (
                const polyMesh& mesh,
                const indirectPrimitivePatch& patch,
                const patchEdgeFaceRegion& edgeInfo,
                const bool sameOrientation,
                const scalar tol,
                TrackingData& td
            );

            //- Influence of edge on face.
            template<class TrackingData>
            inline bool updateFace
            (
                const polyMesh& mesh,
                const indirectPrimitivePatch& patch,
                const label faceI,
                const label edgeI,
                const patchEdgeFaceRegion& edgeInfo,
                const scalar tol,
                TrackingData& td
            );

            //- Same (like operator==)
            template<class TrackingData>
            inline bool equal(const patchEdgeFaceRegion&, TrackingData& td) const;


    // Member Operators

        // Needed for List IO
        inline bool operator==(const patchEdgeFaceRegion&) const;
        inline bool operator!=(const patchEdgeFaceRegion&) const;


    // IOstream Operators

        friend Ostream& operator<<(Ostream&, const patchEdgeFaceRegion&);
        friend Istream& operator>>(Istream&, patchEdgeFaceRegion&);
};


//- Data associated with patchEdgeFaceRegion type are contiguous
template<>
inline bool contiguous<patchEdgeFaceRegion>()
{
    return true;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "patchEdgeFaceRegionI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
