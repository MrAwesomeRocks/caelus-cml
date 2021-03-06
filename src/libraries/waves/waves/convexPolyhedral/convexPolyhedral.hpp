/*---------------------------------------------------------------------------*\
Copyright Niels Gjoel Jacobsen, Technical University of Denmark
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

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
    convexPolyhedral

Description
    This class allows for the computation of the intersections between
    geometric objects (polyhedral, polygon) and a given surface. Based on
    these, the following intersection types can be defined:

    - Polygon (face) and surface              => another polygon
    - Polyhedral (cell) and surface           => another polyhedral
    - Polygon (face) and polygon (face)       => edge/face
    - Polyhedral (cell) and polyhedral (cell) => face/cell

    Requirement to the cell: All faces must consist of co-planar points!

SourceFiles
    convexPolyhedral.cpp

Author
    Niels Gjoel Jacobsen, Technical University of Denmark

\*---------------------------------------------------------------------------*/

#ifndef convexPolyhedral_HPP
#define convexPolyhedral_HPP

#include "fvMesh.hpp"
#include "volFields.hpp"
#include "surfaceFields.hpp"

#include "localFace.hpp"
#include "localCell.hpp"
#include "localCellNeg.hpp"

namespace CML
{

/*---------------------------------------------------------------------------*\
                      Class convexPolyhedral Declaration
\*---------------------------------------------------------------------------*/


class convexPolyhedral
{

private:

    // Private member variables
        point pointOnPlane_;
        vector normalToPlane_;
        bool iterateDistance_;

    // Private member functions
        inline void setPoint(const point& p) {pointOnPlane_ = p;};

        inline void setNormal(const vector& n)
        {
            normalToPlane_ = n/CML::mag(n);
        };

        labelList edgeCutLabel
        (
            const edgeList&,
            const labelList&,
            const scalarField&,
            pointField&
        );

        void faceCut
        (
            const labelList& pType,
            const edgeList& eL,
            const labelList& edgeCut,
            localCell& lc
        );

        void faceCut
        (
            const labelList& pType,
            const edgeList& eL,
            const labelList& edgeCut,
            localCellNeg& lc
        );

        void faceCut
        (
            const labelList&,
            const face&,
            const edgeList&,
            const labelList&,
            const pointField&,
            localFace&,
            edge&
        );

        face combineEdgeList(const edgeList&);


protected:
    const fvMesh& mesh_;

    // Protected member functions

        //- Returns a scalarField of the signed distance to
        //  an arbitrary surface
        virtual void signedPointToSurfaceDistance
        (
            const pointField&,
            scalarField&
        );

        //- Returns a scalar of the signed distance to an arbitrary surface
        virtual scalar signedPointToSurfaceDistance
        (
            const point&
        ) const;

        // Take care of floating point logic, so it is only considered once per
        // intersection call
        void floatingPointToLabel
        (
            const scalarField&,
            labelList&
        );

        label floatingPointToLabel
        (
            const scalar&
        );

public:

    //- Constructur - only switch on iterateDistance, if not using the
    //  signedPointToSurfaceDistance computation in convecPolyhedral AND
    //  if the surfaces are non-planar.
        convexPolyhedral
        (
            const fvMesh& mesh,
            const bool iterateDistance = false
        )
        :
            iterateDistance_(iterateDistance),
            mesh_(mesh)
        {};


    // Destructor

        virtual ~convexPolyhedral()
        {}

    // Public member functions

        localFace divideFace
        (
            const label&,
            const point&,
            const vector&
        );

        localFace divideFace
        (
            const label&
        );


        localCell dividePolyhedral
        (
            const label&,
            const point&,
            const vector&
        );

        void dividePolyhedral
        (
            const point&,
            const vector&,
            localCell&
        );

        void dividePolyhedral
        (
            const point&,
            const vector&,
            localCellNeg&
        );

        void unionSet
        (
            const localCell&,
            localCell&
        );

        void unionSet
        (
            const localCellNeg&,
            localCellNeg&
        );
};

} // End namespace


#endif
