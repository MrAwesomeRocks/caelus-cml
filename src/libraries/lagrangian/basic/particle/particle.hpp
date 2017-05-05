/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
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
    CML::particle

Description
    Base particle class

\*---------------------------------------------------------------------------*/

#ifndef particle_H
#define particle_H

#include "vector.hpp"
#include "Cloud.hpp"
#include "IDLList.hpp"
#include "labelList.hpp"
#include "pointField.hpp"
#include "faceList.hpp"
#include "OFstream.hpp"
#include "tetPointRef.hpp"
#include "FixedList.hpp"
#include "polyMeshTetDecomposition.hpp"
#include "polyMesh.hpp"
#include "Time.hpp"

#include "IOPosition.hpp"
#include "cyclicPolyPatch.hpp"
#include "processorPolyPatch.hpp"
#include "symmetryPolyPatch.hpp"
#include "wallPolyPatch.hpp"
#include "wedgePolyPatch.hpp"
#include "meshTools.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes
class particle;

class polyPatch;

class cyclicPolyPatch;
class processorPolyPatch;
class symmetryPolyPatch;
class wallPolyPatch;
class wedgePolyPatch;

// Forward declaration of friend functions and operators

Ostream& operator<<
(
    Ostream&,
    const particle&
);

bool operator==(const particle&, const particle&);

bool operator!=(const particle&, const particle&);

/*---------------------------------------------------------------------------*\
                          Class Particle Declaration
\*---------------------------------------------------------------------------*/

class particle
:
    public IDLList<particle>::link
{
public:

    template<class CloudType>
    class TrackingData
    {
        // Private data

            //- Reference to the cloud containing (this) particle
            CloudType& cloud_;


    public:

        // Public data

            typedef CloudType cloudType;

            //- Flag to switch processor
            bool switchProcessor;

            //- Flag to indicate whether to keep particle (false = delete)
            bool keepParticle;


        // Constructor
        TrackingData(CloudType& cloud)
        :
            cloud_(cloud)
        {}


        // Member functions

            //- Return a reference to the cloud
            CloudType& cloud()
            {
                return cloud_;
            }
    };


protected:

    // Protected data

        //- Reference to the polyMesh database
        const polyMesh& mesh_;

        //- Position of particle
        vector position_;

        //- Index of the cell it is in
        label cellI_;

        //- Face index if the particle is on a face otherwise -1
        label faceI_;

        //- Fraction of time-step completed
        scalar stepFraction_;

        //- Index of the face that owns the decomposed tet that the
        //  particle is in
        label tetFaceI_;

        //- Index of the point on the face that defines the decomposed
        //  tet that the particle is in.  Relative to the face base
        //  point.
        label tetPtI_;

        //- Originating processor id
        label origProc_;

        //- Local particle id on originating processor
        label origId_;


    // Private Member Functions

        //- Find the tet tri faces between position and tet centre
        void findTris
        (
            const vector& position,
            DynamicList<label>& faceList,
            const tetPointRef& tet,
            const FixedList<vector, 4>& tetAreas,
            const FixedList<label, 4>& tetPlaneBasePtIs,
            const scalar tol
        ) const;

        //- Find the lambda value for the line to-from across the
        //  given tri face, where p = from + lambda*(to - from)
        inline scalar tetLambda
        (
            const vector& from,
            const vector& to,
            const label triI,
            const vector& tetArea,
            const label tetPlaneBasePtI,
            const label cellI,
            const label tetFaceI,
            const label tetPtI,
            const scalar tol
        ) const;

        //- Find the lambda value for a moving tri face
        inline scalar movingTetLambda
        (
            const vector& from,
            const vector& to,
            const label triI,
            const vector& tetArea,
            const label tetPlaneBasePtI,
            const label cellI,
            const label tetFaceI,
            const label tetPtI,
            const scalar tol
        ) const;

        //- Modify the tet owner data by crossing triI
        inline void tetNeighbour(label triI);

        //- Cross the from the given face across the given edge of the
        //  given cell to find the resulting face and tetPtI
        inline void crossEdgeConnectedFace
        (
            const label& cellI,
            label& tetFaceI,
            label& tetPtI,
            const edge& e
        );

        //- Hit wall faces in the current cell if the
        //- wallImpactDistance is non-zero.  They may not be in
        //- different tets to the current.
        template<class CloudType>
        inline void hitWallFaces
        (
            const CloudType& td,
            const vector& from,
            const vector& to,
            scalar& lambdaMin,
            tetIndices& closestTetIs
        );


    // Patch interactions

        //- Overridable function to handle the particle hitting a face
        template<class TrackData>
        void hitFace(TrackData& td);

        //- Overridable function to handle the particle hitting a
        //  patch.  Executed before other patch-hitting functions.
        //  trackFraction is passed in to allow mesh motion to
        //  interpolate in time to the correct face state.
        template<class TrackData>
        bool hitPatch
        (
            const polyPatch&,
            TrackData& td,
            const label patchI,
            const scalar trackFraction,
            const tetIndices& tetIs
        );

        //- Overridable function to handle the particle hitting a wedgePatch
        template<class TrackData>
        void hitWedgePatch(const wedgePolyPatch&, TrackData& td);

        //- Overridable function to handle the particle hitting a
        //  symmetryPatch
        template<class TrackData>
        void hitSymmetryPatch(const symmetryPolyPatch&, TrackData& td);

        //- Overridable function to handle the particle hitting a cyclicPatch
        template<class TrackData>
        void hitCyclicPatch(const cyclicPolyPatch&, TrackData& td);

        //- Overridable function to handle the particle hitting a
        //  processorPatch
        template<class TrackData>
        void hitProcessorPatch(const processorPolyPatch&, TrackData& td);

        //- Overridable function to handle the particle hitting a wallPatch
        template<class TrackData>
        void hitWallPatch
        (
            const wallPolyPatch&,
            TrackData& td,
            const tetIndices& tetIs
        );

        //- Overridable function to handle the particle hitting a
        //  general patch
        template<class TrackData>
        void hitPatch(const polyPatch&, TrackData& td);


public:

    // Static data members

        //- Runtime type information
        TypeName("particle");

        //- String representation of properties
        static string propHeader;

        //- Cumulative particle counter - used to provode unique ID
        static label particleCount_;

        //- Fraction of distance to tet centre to move a particle to
        // 'rescue' it from a tracking problem
        static const scalar trackingCorrectionTol;

        //- Fraction of the cell volume to use in determining tolerance values
        //  for the denominator and numerator of lambda
        static const scalar lambdaDistanceToleranceCoeff;


    // Constructors

        //- Construct from components
        particle
        (
            const polyMesh& mesh,
            const vector& position,
            const label cellI,
            const label tetFaceI,
            const label tetPtI
        );

        //- Construct from components, tetFaceI_ and tetPtI_ are not
        //  supplied so they will be deduced by a search
        particle
        (
            const polyMesh& mesh,
            const vector& position,
            const label cellI,
            bool doCellFacePt = true
        );

        //- Construct from Istream
        particle(const polyMesh& mesh, Istream&, bool readFields = true);

        //- Construct as a copy
        particle(const particle& p);

        //- Construct as a copy with refernce to a new mesh
        particle(const particle& p, const polyMesh& mesh);

        //- Construct a clone
        virtual autoPtr<particle> clone() const
        {
            return autoPtr<particle>(new particle(*this));
        }


        //- Factory class to read-construct particles used for
        //  parallel transfer
        class iNew
        {
            const polyMesh& mesh_;

        public:

            iNew(const polyMesh& mesh)
            :
                mesh_(mesh)
            {}

            autoPtr<particle> operator()(Istream& is) const
            {
                return autoPtr<particle>(new particle(mesh_, is, true));
            }
        };


    //- Destructor
    virtual ~particle()
    {}


    // Member Functions

        // Access

            //- Get unique particle creation id
            inline label getNewParticleID() const;

            //- Return the mesh database
            inline const polyMesh& mesh() const;

            //- Return current particle position
            inline const vector& position() const;

            //- Return current particle position
            inline vector& position();

            //- Return current cell particle is in
            inline label& cell();

            //- Return current cell particle is in
            inline label cell() const;

            //- Return current tet face particle is in
            inline label& tetFace();

            //- Return current tet face particle is in
            inline label tetFace() const;

            //- Return current tet face particle is in
            inline label& tetPt();

            //- Return current tet face particle is in
            inline label tetPt() const;

            //- Return the indices of the current tet that the
            //  particle occupies.
            inline tetIndices currentTetIndices() const;

            //- Return the geometry of the current tet that the
            //  particle occupies.
            inline tetPointRef currentTet() const;

            //- Return the normal of the tri on tetFaceI_ for the
            //  current tet.
            inline vector normal() const;

            //- Return the normal of the tri on tetFaceI_ for the
            //  current tet at the start of the timestep, i.e. based
            //  on oldPoints
            inline vector oldNormal() const;

            //- Return current face particle is on otherwise -1
            inline label& face();

            //- Return current face particle is on otherwise -1
            inline label face() const;

            //- Return the impact model to be used, soft or hard (default).
            inline bool softImpact() const;

            //- Return the particle current time
            inline scalar currentTime() const;


        // Check

            //- Check the stored cell value (setting if necessary) and
            //  initialise the tetFace and tetPt values
            inline void initCellFacePt();

            //- Is the particle on the boundary/(or outside the domain)?
            inline bool onBoundary() const;

            //- Is this global face an internal face?
            inline bool internalFace(const label faceI) const;

            //- Is this global face a boundary face?
            inline bool boundaryFace(const label faceI) const;

            //- Which patch is particle on
            inline label patch(const label faceI) const;

            //- Which face of this patch is this particle on
            inline label patchFace
            (
                const label patchI,
                const label faceI
            ) const;

            //- Return the fraction of time-step completed
            inline scalar& stepFraction();

            //-  Return the fraction of time-step completed
            inline scalar stepFraction() const;

            //- Return const access to the originating processor id
            inline label origProc() const;

            //- Return the originating processor id for manipulation
            inline label& origProc();

            //- Return const access to  the particle id on originating processor
            inline label origId() const;

            //- Return the particle id on originating processor for manipulation
            inline label& origId();


        // Track

            //- Track particle to end of trajectory
            //  or until it hits the boundary.
            //  On entry 'stepFraction()' should be set to the fraction of the
            //  time-step at which the tracking starts and on exit it contains
            //  the fraction of the time-step completed.
            //  Returns the boundary face index if the track stops at the
            //  boundary, -1 otherwise.
            template<class TrackData>
            label track(const vector& endPosition, TrackData& td);

            //- Track particle to a given position and returns 1.0 if the
            //  trajectory is completed without hitting a face otherwise
            //  stops at the face and returns the fraction of the trajectory
            //  completed.
            //  on entry 'stepFraction()' should be set to the fraction of the
            //  time-step at which the tracking starts.
            template<class TrackData>
            scalar trackToFace(const vector& endPosition, TrackData& td);

            //- Return the index of the face to be used in the interpolation
            //  routine
            inline label faceInterpolation() const;


    // Transformations

        //- Transform the physical properties of the particle
        //  according to the given transformation tensor
        virtual void transformProperties(const tensor& T);

        //- Transform the physical properties of the particle
        //  according to the given separation vector
        virtual void transformProperties(const vector& separation);

        //- The nearest distance to a wall that
        //  the particle can be in the n direction
        virtual scalar wallImpactDistance(const vector& n) const;


    // Parallel transfer

        //- Convert global addressing to the processor patch
        //  local equivalents
        template<class TrackData>
        void prepareForParallelTransfer(const label patchI, TrackData& td);

        //- Convert processor patch addressing to the global equivalents
        //  and set the cellI to the face-neighbour
        template<class TrackData>
        void correctAfterParallelTransfer(const label patchI, TrackData& td);


    // I-O

        //- Read the fields associated with the owner cloud
        template<class CloudType>
        static void readFields(CloudType& c);

        //- Write the fields associated with the owner cloud
        template<class CloudType>
        static void writeFields(const CloudType& c);

        //- Write the particle data
        void write(Ostream& os, bool writeFields) const;


    // Friend Operators

        friend Ostream& operator<<(Ostream&, const particle&);

        friend bool operator==(const particle& pA, const particle& pB);

        friend bool operator!=(const particle& pA, const particle& pB);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

inline void CML::particle::findTris
(
    const vector& position,
    DynamicList<label>& faceList,
    const tetPointRef& tet,
    const FixedList<vector, 4>& tetAreas,
    const FixedList<label, 4>& tetPlaneBasePtIs,
    const scalar tol
) const
{
    faceList.clear();

    const point Ct = tet.centre();

    for (label i = 0; i < 4; i++)
    {
        scalar lambda = tetLambda
        (
            Ct,
            position,
            i,
            tetAreas[i],
            tetPlaneBasePtIs[i],
            cellI_,
            tetFaceI_,
            tetPtI_,
            tol
        );

        if ((lambda > 0.0) && (lambda < 1.0))
        {
            faceList.append(i);
        }
    }
}


inline CML::scalar CML::particle::tetLambda
(
    const vector& from,
    const vector& to,
    const label triI,
    const vector& n,
    const label tetPlaneBasePtI,
    const label cellI,
    const label tetFaceI,
    const label tetPtI,
    const scalar tol
) const
{
    const pointField& pPts = mesh_.points();

    if (mesh_.moving())
    {
        return movingTetLambda
        (
            from,
            to,
            triI,
            n,
            tetPlaneBasePtI,
            cellI,
            tetFaceI,
            tetPtI,
            tol
        );
    }

    const point& base = pPts[tetPlaneBasePtI];

    scalar lambdaNumerator = (base - from) & n;
    scalar lambdaDenominator = (to - from) & n;

    // n carries the area of the tet faces, so the dot product with a
    // delta-length has the units of volume.  Comparing the component of each
    // delta-length in the direction of n times the face area to a fraction of
    // the cell volume.

    if (mag(lambdaDenominator) < tol)
    {
        if (mag(lambdaNumerator) < tol)
        {
            // Track starts on the face, and is potentially
            // parallel to it.  +-tol/+-tol is not a good
            // comparison, return 0.0, in anticipation of tet
            // centre correction.

            return 0.0;
        }
        else
        {
            if (mag((to - from)) < tol/mag(n))
            {
                // 'Zero' length track (compared to the tolerance, which is
                // based on the cell volume, divided by the tet face area), not
                // along the face, face cannot be crossed.

                return GREAT;
            }
            else
            {
                // Trajectory is non-zero and parallel to face

                lambdaDenominator = sign(lambdaDenominator)*SMALL;
            }
        }
    }

    return lambdaNumerator/lambdaDenominator;
}


inline CML::scalar CML::particle::movingTetLambda
(
    const vector& from,
    const vector& to,
    const label triI,
    const vector& n,
    const label tetPlaneBasePtI,
    const label cellI,
    const label tetFaceI,
    const label tetPtI,
    const scalar tol
) const
{
    const pointField& pPts = mesh_.points();
    const pointField& oldPPts = mesh_.oldPoints();

    // Base point of plane at end of motion
    const point& b = pPts[tetPlaneBasePtI];

    // n: Normal of plane at end of motion

    // Base point of plane at start of timestep
    const point& b00 = oldPPts[tetPlaneBasePtI];

    // Base point of plane at start of tracking portion (cast forward by
    // stepFraction)
    point b0 = b00 + stepFraction_*(b - b00);

    // Normal of plane at start of tracking portion
    vector n0 = vector::zero;

    {
        tetIndices tetIs(cellI, tetFaceI, tetPtI, mesh_);

        // tet at timestep start
        tetPointRef tet00 = tetIs.oldTet(mesh_);

        // tet at timestep end
        tetPointRef tet = tetIs.tet(mesh_);

        point tet0PtA = tet00.a() + stepFraction_*(tet.a() - tet00.a());
        point tet0PtB = tet00.b() + stepFraction_*(tet.b() - tet00.b());
        point tet0PtC = tet00.c() + stepFraction_*(tet.c() - tet00.c());
        point tet0PtD = tet00.d() + stepFraction_*(tet.d() - tet00.d());

        // Tracking portion start tet (cast forward by stepFraction)
        tetPointRef tet0(tet0PtA, tet0PtB, tet0PtC, tet0PtD);

        switch (triI)
        {
            case 0:
            {
                n0 = tet0.Sa();
                break;
            }
            case 1:
            {
                n0 = tet0.Sb();
                break;
            }
            case 2:
            {
                n0 = tet0.Sc();
                break;
            }
            case 3:
            {
                n0 = tet0.Sd();
                break;
            }
            default:
            {
                break;
            }
        }
    }

    if (mag(n0) < SMALL)
    {
        // If the old normal is zero (for example in layer addition)
        // then use the current normal;

        n0 = n;
    }

    scalar lambdaNumerator = 0;
    scalar lambdaDenominator = 0;

    vector dP = to - from;
    vector dN = n - n0;
    vector dB = b - b0;
    vector dS = from - b0;

    if (mag(dN) > SMALL)
    {
        scalar a = (dP - dB) & dN;
        scalar b = ((dP - dB) & n0) + (dS & dN);
        scalar c = dS & n0;

        if (mag(a) > SMALL)
        {

            // Solve quadratic for lambda
            scalar discriminant = sqr(b) - 4.0*a*c;

            if (discriminant < 0)
            {
                // Imaginary roots only - face not crossed
                return GREAT;
            }
            else
            {
                // Numerical Recipes in C,
                // Second Edition (1992),
                // Section 5.6.
                // q = -0.5*(b + sgn(b)*sqrt(b^2 - 4ac))
                // x1 = q/a
                // x2 = c/q

                scalar q = -0.5*(b + sign(b)*CML::sqrt(discriminant));

                if (mag(q) < VSMALL)
                {
                    // If q is zero, then l1 = q/a is the required
                    // value of lambda, and is zero.

                    return 0.0;
                }

                scalar l1 = q/a;
                scalar l2 = c/q;

                // There will be two roots, a big one and a little
                // one, choose the little one.

                if (mag(l1) < mag(l2))
                {
                    return l1;
                }
                else
                {
                    return l2;
                }
            }
        }
        {
            // When a is zero, solve the first order polynomial

            lambdaNumerator = -c;
            lambdaDenominator = b;
        }
    }
    else
    {
        // when n = n0 is zero, there is no plane rotation, solve the
        // first order polynomial

        lambdaNumerator = -(dS & n0);
        lambdaDenominator = ((dP - dB) & n0);

    }

    if (mag(lambdaDenominator) < tol)
    {
        if (mag(lambdaNumerator) < tol)
        {
            // Track starts on the face, and is potentially
            // parallel to it.  +-tol)/+-tol is not a good
            // comparison, return 0.0, in anticipation of tet
            // centre correction.

            return 0.0;
        }
        else
        {
            if (mag((to - from)) < tol/mag(n))
            {
                // Zero length track, not along the face, face
                // cannot be crossed.

                return GREAT;
            }
            else
            {
                // Trajectory is non-zero and parallel to face

                lambdaDenominator = sign(lambdaDenominator)*SMALL;
            }
        }
    }

    return lambdaNumerator/lambdaDenominator;
}



inline void CML::particle::tetNeighbour(label triI)
{
    const labelList& pOwner = mesh_.faceOwner();
    const faceList& pFaces = mesh_.faces();

    bool own = (pOwner[tetFaceI_] == cellI_);

    const CML::face& f = pFaces[tetFaceI_];

    label tetBasePtI = mesh_.tetBasePtIs()[tetFaceI_];

    if (tetBasePtI == -1)
    {
        FatalErrorIn
        (
            "inline void CML::particle::tetNeighbour(label triI)"
        )
            << "No base point for face " << tetFaceI_ << ", " << f
            << ", produces a valid tet decomposition."
            << abort(FatalError);
    }

    label facePtI = (tetPtI_ + tetBasePtI) % f.size();
    label otherFacePtI = f.fcIndex(facePtI);

    switch (triI)
    {
        case 0:
        {
            // Crossing this triangle changes tet to that in the
            // neighbour cell over tetFaceI

            // Modification of cellI_ will happen by other indexing,
            // tetFaceI_ and tetPtI don't change.

            break;
        }
        case 1:
        {
            crossEdgeConnectedFace
            (
                cellI_,
                tetFaceI_,
                tetPtI_,
                CML::edge(f[facePtI], f[otherFacePtI])
            );

            break;
        }
        case 2:
        {
            if (own)
            {
                if (tetPtI_ < f.size() - 2)
                {
                    tetPtI_ = f.fcIndex(tetPtI_);
                }
                else
                {
                    crossEdgeConnectedFace
                    (
                        cellI_,
                        tetFaceI_,
                        tetPtI_,
                        CML::edge(f[tetBasePtI], f[otherFacePtI])
                    );
                }
            }
            else
            {
                if (tetPtI_ > 1)
                {
                    tetPtI_ = f.rcIndex(tetPtI_);
                }
                else
                {
                    crossEdgeConnectedFace
                    (
                        cellI_,
                        tetFaceI_,
                        tetPtI_,
                        CML::edge(f[tetBasePtI], f[facePtI])
                    );
                }
            }

            break;
        }
        case 3:
        {
            if (own)
            {
                if (tetPtI_ > 1)
                {
                    tetPtI_ = f.rcIndex(tetPtI_);
                }
                else
                {
                    crossEdgeConnectedFace
                    (
                        cellI_,
                        tetFaceI_,
                        tetPtI_,
                        CML::edge(f[tetBasePtI], f[facePtI])
                    );
                }
            }
            else
            {
                if (tetPtI_ < f.size() - 2)
                {
                    tetPtI_ = f.fcIndex(tetPtI_);
                }
                else
                {
                    crossEdgeConnectedFace
                    (
                        cellI_,
                        tetFaceI_,
                        tetPtI_,
                        CML::edge(f[tetBasePtI], f[otherFacePtI])
                    );
                }
            }

            break;
        }
        default:
        {
            FatalErrorIn
            (
                "inline void "
                "CML::particle::tetNeighbour(label triI)"
            )
                << "Tet tri face index error, can only be 0..3, supplied "
                << triI << abort(FatalError);

            break;
        }
    }
}


inline void CML::particle::crossEdgeConnectedFace
(
    const label& cellI,
    label& tetFaceI,
    label& tetPtI,
    const edge& e
)
{
    const faceList& pFaces = mesh_.faces();
    const cellList& pCells = mesh_.cells();

    const CML::face& f = pFaces[tetFaceI];

    const CML::cell& thisCell = pCells[cellI];

    forAll(thisCell, cFI)
    {
        // Loop over all other faces of this cell and
        // find the one that shares this edge

        label fI = thisCell[cFI];

        if (tetFaceI == fI)
        {
            continue;
        }

        const CML::face& otherFace = pFaces[fI];

        label edDir = otherFace.edgeDirection(e);

        if (edDir == 0)
        {
            continue;
        }
        else if (f == pFaces[fI])
        {
            // This is a necessary condition if using duplicate baffles
            // (so coincident faces). We need to make sure we don't cross into
            // the face with the same vertices since we might enter a tracking
            // loop where it never exits. This test should be cheap
            // for most meshes so can be left in for 'normal' meshes.

            continue;
        }
        else
        {
            //Found edge on other face
            tetFaceI = fI;

            label eIndex = -1;

            if (edDir == 1)
            {
                // Edge is in the forward circulation of this face, so
                // work with the start point of the edge

                eIndex = findIndex(otherFace, e.start());
            }
            else
            {
                // edDir == -1, so the edge is in the reverse
                // circulation of this face, so work with the end
                // point of the edge

                eIndex = findIndex(otherFace, e.end());
            }

            label tetBasePtI = mesh_.tetBasePtIs()[fI];

            if (tetBasePtI == -1)
            {
                FatalErrorIn
                (
                    "inline void "
                    "CML::particle::crossEdgeConnectedFace"
                    "("
                        "const label& cellI,"
                        "label& tetFaceI,"
                        "label& tetPtI,"
                        "const edge& e"
                    ")"
                )
                    << "No base point for face " << fI << ", " << f
                    << ", produces a decomposition that has a minimum "
                    << "volume greater than tolerance."
                    << abort(FatalError);
            }

            // Find eIndex relative to the base point on new face
            eIndex -= tetBasePtI;

            if (neg(eIndex))
            {
                eIndex = (eIndex + otherFace.size()) % otherFace.size();
            }

            if (eIndex == 0)
            {
                // The point is the base point, so this is first tet
                // in the face circulation

                tetPtI = 1;
            }
            else if (eIndex == otherFace.size() - 1)
            {
                // The point is the last before the base point, so
                // this is the last tet in the face circulation

                tetPtI = otherFace.size() - 2;
            }
            else
            {
                tetPtI = eIndex;
            }

            break;
        }
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

inline CML::label CML::particle::getNewParticleID() const
{
    label id = particleCount_++;

    if (id == labelMax)
    {
        WarningIn("particle::getNewParticleID() const")
            << "Particle counter has overflowed. This might cause problems"
            << " when reconstructing particle tracks." << endl;
    }
    return id;
}


inline const CML::polyMesh& CML::particle::mesh() const
{
    return mesh_;
}


inline const CML::vector& CML::particle::position() const
{
    return position_;
}


inline CML::vector& CML::particle::position()
{
    return position_;
}


inline CML::label CML::particle::cell() const
{
    return cellI_;
}


inline CML::label& CML::particle::cell()
{
    return cellI_;
}


inline CML::label CML::particle::tetFace() const
{
    return tetFaceI_;
}


inline CML::label& CML::particle::tetFace()
{
    return tetFaceI_;
}


inline CML::label CML::particle::tetPt() const
{
    return tetPtI_;
}


inline CML::label& CML::particle::tetPt()
{
    return tetPtI_;
}


inline CML::tetIndices CML::particle::currentTetIndices() const
{
    return tetIndices(cellI_, tetFaceI_, tetPtI_, mesh_);
}


inline CML::tetPointRef CML::particle::currentTet() const
{
    return currentTetIndices().tet(mesh_);
}


inline CML::vector CML::particle::normal() const
{
    return currentTetIndices().faceTri(mesh_).normal();
}


inline CML::vector CML::particle::oldNormal() const
{
    return currentTetIndices().oldFaceTri(mesh_).normal();
}


inline CML::label CML::particle::face() const
{
    return faceI_;
}


inline CML::label& CML::particle::face()
{
    return faceI_;
}


inline void CML::particle::initCellFacePt()
{
    if (cellI_ == -1)
    {
        mesh_.findCellFacePt
        (
            position_,
            cellI_,
            tetFaceI_,
            tetPtI_
        );

        if (cellI_ == -1)
        {
            FatalErrorIn("void CML::particle::initCellFacePt()")
                << "cell, tetFace and tetPt search failure at position "
                << position_ << abort(FatalError);
        }
    }
    else
    {
        mesh_.findTetFacePt(cellI_, position_, tetFaceI_, tetPtI_);

        if (tetFaceI_ == -1 || tetPtI_ == -1)
        {
            label oldCellI = cellI_;

            mesh_.findCellFacePt
            (
                position_,
                cellI_,
                tetFaceI_,
                tetPtI_
            );

            if (cellI_ == -1 || tetFaceI_ == -1 || tetPtI_ == -1)
            {
                // The particle has entered this function with a cell
                // number, but hasn't been able to find a cell to
                // occupy.

                if(!mesh_.pointInCellBB(position_, oldCellI, 0.1))
                {
                    // If the position is not inside the (slightly
                    // extended) bound-box of the cell that it thought
                    // it should be in, then this is considered an
                    // error.

                    FatalErrorIn("void CML::particle::initCellFacePt()")
                        << "cell, tetFace and tetPt search failure at position "
                        << position_ << nl << "for requested cell " << oldCellI
                        << abort(FatalError);
                }

                // The position is in the (slightly extended)
                // bound-box of the cell.  This situation may arise
                // because the face decomposition of the cell is not
                // the same as when the particle acquired the cell
                // index.  For example, it has been read into a mesh
                // that has made a different face base-point decision
                // for a boundary face and now this particle is in a
                // position that is not in the mesh.  Gradually move
                // the particle towards the centre of the cell that it
                // thought that it was in.

                cellI_ = oldCellI;

                point newPosition = position_;

                const point& cC = mesh_.cellCentres()[cellI_];

                label trap(1.0/trackingCorrectionTol + 1);

                label iterNo = 0;

                do
                {
                    newPosition += trackingCorrectionTol*(cC - position_);

                    mesh_.findTetFacePt
                    (
                        cellI_,
                        newPosition,
                        tetFaceI_,
                        tetPtI_
                    );

                    iterNo++;

                } while (tetFaceI_ < 0  && iterNo <= trap);

                if (tetFaceI_ == -1)
                {
                    FatalErrorIn("void CML::particle::initCellFacePt()")
                        << "cell, tetFace and tetPt search failure at position "
                        << position_ << abort(FatalError);
                }

                if (debug)
                {
                    WarningIn("void CML::particle::initCellFacePt()")
                        << "Particle moved from " << position_
                        << " to " << newPosition
                        << " in cell " << cellI_
                        << " tetFace " << tetFaceI_
                        << " tetPt " << tetPtI_ << nl
                        << "    (A fraction of "
                        << 1.0 - mag(cC - newPosition)/mag(cC - position_)
                        << " of the distance to the cell centre)"
                        << " because a decomposition tetFace and tetPt "
                        << "could not be found."
                        << endl;
                }

                position_ = newPosition;
            }

            if (debug && cellI_ != oldCellI)
            {
                WarningIn("void CML::particle::initCellFacePt()")
                    << "Particle at position " << position_
                    << " searched for a cell, tetFace and tetPt." << nl
                    << "    Found"
                    << " cell " << cellI_
                    << " tetFace " << tetFaceI_
                    << " tetPt " << tetPtI_ << nl
                    << "    This is a different cell to that which was supplied"
                    << " (" << oldCellI << ")." << nl
                    << endl;
            }
        }
    }
}


inline bool CML::particle::onBoundary() const
{
    return faceI_ != -1 && faceI_ >= mesh_.nInternalFaces();
}


inline CML::scalar& CML::particle::stepFraction()
{
    return stepFraction_;
}


inline CML::scalar CML::particle::stepFraction() const
{
    return stepFraction_;
}


inline CML::label CML::particle::origProc() const
{
    return origProc_;
}


inline CML::label& CML::particle::origProc()
{
    return origProc_;
}


inline CML::label CML::particle::origId() const
{
    return origId_;
}


inline CML::label& CML::particle::origId()
{
    return origId_;
}


inline bool CML::particle::softImpact() const
{
    return false;
}


inline CML::scalar CML::particle::currentTime() const
{
    return
        mesh_.time().value()
      + stepFraction_*mesh_.time().deltaTValue();
}


inline bool CML::particle::internalFace(const label faceI) const
{
    return mesh_.isInternalFace(faceI);
}


bool CML::particle::boundaryFace(const label faceI) const
{
    return !internalFace(faceI);
}


inline CML::label CML::particle::patch(const label faceI) const
{
    return mesh_.boundaryMesh().whichPatch(faceI);
}


inline CML::label CML::particle::patchFace
(
    const label patchI,
    const label faceI
) const
{
    return mesh_.boundaryMesh()[patchI].whichFace(faceI);
}


inline CML::label CML::particle::faceInterpolation() const
{
    return faceI_;
}
/*
#define defineParticleTypeNameAndDebug(Type, DebugSwitch)                     \
    template<>                                                                \
    const CML::word Particle<Type>::typeName(#Type);                         \
    template<>                                                                \
    int Particle<Type>::debug(CML::debug::debugSwitch(#Type, DebugSwitch));
*/


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class TrackData>
void CML::particle::prepareForParallelTransfer
(
    const label patchI,
    TrackData& td
)
{
    // Convert the face index to be local to the processor patch
    faceI_ = patchFace(patchI, faceI_);
}


template<class TrackData>
void CML::particle::correctAfterParallelTransfer
(
    const label patchI,
    TrackData& td
)
{
    const coupledPolyPatch& ppp =
        refCast<const coupledPolyPatch>(mesh_.boundaryMesh()[patchI]);

    cellI_ = ppp.faceCells()[faceI_];

    // Have patch transform the position
    ppp.transformPosition(position_, faceI_);

    // Transform the properties
    if (!ppp.parallel())
    {
        const tensor& T =
        (
            ppp.forwardT().size() == 1
          ? ppp.forwardT()[0]
          : ppp.forwardT()[faceI_]
        );
        transformProperties(T);
    }
    else if (ppp.separated())
    {
        const vector& s =
        (
            (ppp.separation().size() == 1)
          ? ppp.separation()[0]
          : ppp.separation()[faceI_]
        );
        transformProperties(-s);
    }

    tetFaceI_ = faceI_ + ppp.start();

    // Faces either side of a coupled patch have matched base indices,
    // tetPtI is specified relative to the base point, already and
    // opposite circulation directions by design, so if the vertices
    // are:
    // source:
    // face    (a b c d e f)
    // fPtI     0 1 2 3 4 5
    //            +
    // destination:
    // face    (a f e d c b)
    // fPtI     0 1 2 3 4 5
    //                  +
    // where a is the base point of the face are matching , and we
    // have fPtI = 1 on the source processor face, i.e. vertex b, then
    // this because of the face circulation direction change, vertex c
    // is the characterising point on the destination processor face,
    // giving the destination fPtI as:
    //     fPtI_d = f.size() - 1 - fPtI_s = 6 - 1 - 1 = 4
    // This relationship can be verified for other points and sizes of
    // face.

    tetPtI_ = mesh_.faces()[tetFaceI_].size() - 1 - tetPtI_;

    // Reset the face index for the next tracking operation
    if (stepFraction_ > (1.0 - SMALL))
    {
        stepFraction_ = 1.0;
        faceI_ = -1;
    }
    else
    {
        faceI_ += ppp.start();
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
void CML::particle::readFields(CloudType& c)
{
    if (!c.size())
    {
        return;
    }

    IOobject procIO(c.fieldIOobject("origProcId", IOobject::MUST_READ));

    if (procIO.headerOk())
    {
        IOField<label> origProcId(procIO);
        c.checkFieldIOobject(c, origProcId);
        IOField<label> origId(c.fieldIOobject("origId", IOobject::MUST_READ));
        c.checkFieldIOobject(c, origId);

        label i = 0;
        forAllIter(typename CloudType, c, iter)
        {
            particle& p = iter();

            p.origProc_ = origProcId[i];
            p.origId_ = origId[i];
            i++;
        }
    }
}


template<class CloudType>
void CML::particle::writeFields(const CloudType& c)
{
    // Write the cloud position file
    IOPosition<CloudType> ioP(c);
    ioP.write();

    label np =  c.size();

    IOField<label> origProc
    (
        c.fieldIOobject("origProcId", IOobject::NO_READ),
        np
    );
    IOField<label> origId(c.fieldIOobject("origId", IOobject::NO_READ), np);

    label i = 0;
    forAllConstIter(typename CloudType, c, iter)
    {
        origProc[i] = iter().origProc_;
        origId[i] = iter().origId_;
        i++;
    }

    origProc.write();
    origId.write();
}


template<class TrackData>
CML::label CML::particle::track(const vector& endPosition, TrackData& td)
{
    faceI_ = -1;

    // Tracks to endPosition or stop on boundary
    while (!onBoundary() && stepFraction_ < 1.0 - SMALL)
    {
        stepFraction_ += trackToFace(endPosition, td)*(1.0 - stepFraction_);
    }

    return faceI_;
}


template<class TrackData>
CML::scalar CML::particle::trackToFace
(
    const vector& endPosition,
    TrackData& td
)
{
    typedef typename TrackData::cloudType cloudType;
    typedef typename cloudType::particleType particleType;

    cloudType& cloud = td.cloud();


    const faceList& pFaces = mesh_.faces();
    const pointField& pPts = mesh_.points();
    const vectorField& pC = mesh_.cellCentres();

    faceI_ = -1;

    // Pout<< "Particle " << origId_ << " " << origProc_
    //     << " Tracking from " << position_
    //     << " to " << endPosition
    //     << endl;

    // Pout<< "stepFraction " << stepFraction_ << nl
    //     << "cellI " << cellI_ << nl
    //     << "tetFaceI " << tetFaceI_ << nl
    //     << "tetPtI " << tetPtI_
    //     << endl;

    scalar trackFraction = 0.0;

    // Minimum tetrahedron decomposition of each cell of the mesh into
    // using the cell centre, base point on face, and further two
    // points on the face.  For each face of n points, there are n - 2
    // tets generated.

    // The points for each tet are organised to match those used in the
    // tetrahedron class, supplying them in the order:
    //     Cc, basePt, pA, pB
    // where:
    //   + Cc is the cell centre;
    //   + basePt is the base point on the face;
    //   + pA and pB are the remaining points on the face, such that
    //     the circulation, {basePt, pA, pB} produces a positive
    //     normal by the right-hand rule.  pA and pB are chosen from
    //     tetPtI_ do accomplish this depending if the cell owns the
    //     face, tetPtI_ is the vertex that characterises the tet, and
    //     is the first vertex on the tet when circulating around the
    //     face. Therefore, the same tetPtI represents the same face
    //     triangle for both the owner and neighbour cell.
    //
    // Each tet has its four triangles represented in the same order:
    // 0) tri joining a tet to the tet across the face in next cell.
    //    This is the triangle opposite Cc.
    // 1) tri joining a tet to the tet that is in the same cell, but
    //    belongs to the face that shares the edge of the current face
    //    that doesn't contain basePt.  This is the triangle opposite
    //    basePt.

    // 2) tri joining a tet to the tet that is in the same cell, but
    //    belongs to the face that shares the tet-edge (basePt - pB).
    //    This may be on the same face, or a different one.  This is
    //    the triangle opposite basePt.  This is the triangle opposite
    //    pA.

    // 4) tri joining a tet to the tet that is in the same cell, but
    //    belongs to the face that shares the tet-edge (basePt - pA).
    //    This may be on the same face, or a different one.  This is
    //    the triangle opposite basePt.  This is the triangle opposite
    //    pA.

    // Which tri (0..3) of the tet has been crossed
    label triI = -1;

    // Determine which face was actually crossed.  lambdaMin < SMALL
    // is considered a trigger for a tracking correction towards the
    // current tet centre.
    scalar lambdaMin = VGREAT;

    DynamicList<label>& tris = cloud.labels();

    // Tet indices that will be set by hitWallFaces if a wall face is
    // to be hit, or are set when any wall tri of a tet is hit.
    // Carries the description of the tet on which the cell face has
    // been hit.  For the case of being set in hitWallFaces, this may
    // be a different tet to the one that the particle occupies.
    tetIndices faceHitTetIs;

    // What tolerance is appropriate the minimum lambda numerator and
    // denominator for tracking in this cell.
    scalar lambdaDistanceTolerance =
        lambdaDistanceToleranceCoeff*mesh_.cellVolumes()[cellI_];

    do
    {
        if (triI != -1)
        {
            // Change tet ownership because a tri face has been crossed
            tetNeighbour(triI);
        }

        const CML::face& f = pFaces[tetFaceI_];

        bool own = (mesh_.faceOwner()[tetFaceI_] == cellI_);

        label tetBasePtI = mesh_.tetBasePtIs()[tetFaceI_];

        label basePtI = f[tetBasePtI];

        label facePtI = (tetPtI_ + tetBasePtI) % f.size();
        label otherFacePtI = f.fcIndex(facePtI);

        label fPtAI = -1;
        label fPtBI = -1;

        if (own)
        {
            fPtAI = facePtI;
            fPtBI = otherFacePtI;
        }
        else
        {
            fPtAI = otherFacePtI;
            fPtBI = facePtI;
        }

        tetPointRef tet
        (
            pC[cellI_],
            pPts[basePtI],
            pPts[f[fPtAI]],
            pPts[f[fPtBI]]
        );

        if (lambdaMin < SMALL)
        {
            // Apply tracking correction towards tet centre

            if (debug)
            {
                Pout<< "tracking rescue using tetCentre from " << position();
            }

            position_ += trackingCorrectionTol*(tet.centre() - position_);

            if (debug)
            {
                Pout<< " to " << position() << " due to "
                    << (tet.centre() - position_) << endl;
            }

            cloud.trackingRescue();

            return trackFraction;
        }

        if (triI != -1 && mesh_.moving())
        {
            // Mesh motion requires stepFraction to be correct for
            // each tracking portion, so trackToFace must return after
            // every lambda calculation.
            return trackFraction;
        }

        FixedList<vector, 4> tetAreas;

        tetAreas[0] = tet.Sa();
        tetAreas[1] = tet.Sb();
        tetAreas[2] = tet.Sc();
        tetAreas[3] = tet.Sd();

        FixedList<label, 4> tetPlaneBasePtIs;

        tetPlaneBasePtIs[0] = basePtI;
        tetPlaneBasePtIs[1] = f[fPtAI];
        tetPlaneBasePtIs[2] = basePtI;
        tetPlaneBasePtIs[3] = basePtI;

        findTris
        (
            endPosition,
            tris,
            tet,
            tetAreas,
            tetPlaneBasePtIs,
            lambdaDistanceTolerance
        );

        // Reset variables for new track
        triI = -1;
        lambdaMin = VGREAT;

        // Pout<< "tris " << tris << endl;

        // Sets a value for lambdaMin and faceI_ if a wall face is hit
        // by the track.
        hitWallFaces
        (
            cloud,
            position_,
            endPosition,
            lambdaMin,
            faceHitTetIs
        );

        // Did not hit any tet tri faces, and no wall face has been
        // found to hit.
        if (tris.empty() && faceI_ < 0)
        {
            position_ = endPosition;

            return 1.0;
        }
        else
        {
            // Loop over all found tris and see if any of them find a
            // lambda value smaller than that found for a wall face.
            forAll(tris, i)
            {
                label tI = tris[i];

                scalar lam = tetLambda
                (
                    position_,
                    endPosition,
                    triI,
                    tetAreas[tI],
                    tetPlaneBasePtIs[tI],
                    cellI_,
                    tetFaceI_,
                    tetPtI_,
                    lambdaDistanceTolerance
                );

                if (lam < lambdaMin)
                {
                    lambdaMin = lam;

                    triI = tI;
                }
            }
        }

        if (triI == 0)
        {
            // This must be a cell face crossing
            faceI_ = tetFaceI_;

            // Set the faceHitTetIs to those for the current tet in case a
            // wall interaction is required with the cell face
            faceHitTetIs = tetIndices
            (
                cellI_,
                tetFaceI_,
                tetBasePtI,
                fPtAI,
                fPtBI,
                tetPtI_
            );
        }
        else if (triI > 0)
        {
            // A tri was found to be crossed before a wall face was hit (if any)
            faceI_ = -1;
        }

        // Pout<< "track loop " << position_ << " " << endPosition << nl
        //     << "    " << cellI_
        //     << "    " << faceI_
        //     << " " << tetFaceI_
        //     << " " << tetPtI_
        //     << " " << triI
        //     << " " << lambdaMin
        //     << " " << trackFraction
        //     << endl;

        // Pout<< "# Tracking loop tet "
        //     << origId_ << " " << origProc_<< nl
        //     << "# face: " << tetFaceI_ << nl
        //     << "# tetPtI: " << tetPtI_ << nl
        //     << "# tetBasePtI: " << mesh_.tetBasePtIs()[tetFaceI_] << nl
        //     << "# tet.mag(): " << tet.mag() << nl
        //     << "# tet.quality(): " << tet.quality()
        //     << endl;

        // meshTools::writeOBJ(Pout, tet.a());
        // meshTools::writeOBJ(Pout, tet.b());
        // meshTools::writeOBJ(Pout, tet.c());
        // meshTools::writeOBJ(Pout, tet.d());

        // Pout<< "f 1 3 2" << nl
        //     << "f 2 3 4" << nl
        //     << "f 1 4 3" << nl
        //     << "f 1 2 4" << endl;

        // The particle can be 'outside' the tet.  This will yield a
        // lambda larger than 1, or smaller than 0.  For values < 0,
        // the particle travels away from the tet and we don't move
        // the particle, only change tet/cell.  For values larger than
        // 1, we move the particle to endPosition before the tet/cell
        // change.
        if (lambdaMin > SMALL)
        {
            if (lambdaMin <= 1.0)
            {
                trackFraction += lambdaMin*(1 - trackFraction);

                position_ += lambdaMin*(endPosition - position_);
            }
            else
            {
                trackFraction = 1.0;

                position_ = endPosition;
            }
        }
        else
        {
            // Set lambdaMin to zero to force a towards-tet-centre
            // correction.
            lambdaMin = 0.0;
        }

    } while (faceI_ < 0);

    particleType& p = static_cast<particleType&>(*this);
    p.hitFace(td);

    if (internalFace(faceI_))
    {
        // Change tet ownership because a tri face has been crossed,
        // in general this is:
        //     tetNeighbour(triI);
        // but triI must be 0;
        // No modifications are required for triI = 0, no call required to
        //     tetNeighbour(0);

        if (cellI_ == mesh_.faceOwner()[faceI_])
        {
            cellI_ = mesh_.faceNeighbour()[faceI_];
        }
        else if (cellI_ == mesh_.faceNeighbour()[faceI_])
        {
            cellI_ = mesh_.faceOwner()[faceI_];
        }
        else
        {
            FatalErrorIn("Particle::trackToFace(const vector&, TrackData&)")
                << "addressing failure" << abort(FatalError);
        }
    }
    else
    {
        label origFaceI = faceI_;
        label patchI = patch(faceI_);

        // No action taken for tetPtI_ for tetFaceI_ here, handled by
        // patch interaction call or later during processor transfer.

        if
        (
            !p.hitPatch
            (
                mesh_.boundaryMesh()[patchI],
                td,
                patchI,
                trackFraction,
                faceHitTetIs
            )
        )
        {
            // Did patch interaction model switch patches?
            if (faceI_ != origFaceI)
            {
                patchI = patch(faceI_);
            }

            const polyPatch& patch = mesh_.boundaryMesh()[patchI];

            if (isA<wedgePolyPatch>(patch))
            {
                p.hitWedgePatch
                (
                    static_cast<const wedgePolyPatch&>(patch), td
                );
            }
            else if (isA<symmetryPolyPatch>(patch))
            {
                p.hitSymmetryPatch
                (
                    static_cast<const symmetryPolyPatch&>(patch), td
                );
            }
            else if (isA<cyclicPolyPatch>(patch))
            {
                p.hitCyclicPatch
                (
                    static_cast<const cyclicPolyPatch&>(patch), td
                );
            }
            else if (isA<processorPolyPatch>(patch))
            {
                p.hitProcessorPatch
                (
                    static_cast<const processorPolyPatch&>(patch), td
                );
            }
            else if (isA<wallPolyPatch>(patch))
            {
                p.hitWallPatch
                (
                    static_cast<const wallPolyPatch&>(patch), td, faceHitTetIs
                );
            }
            else
            {
                p.hitPatch(patch, td);
            }
        }
    }

    if (lambdaMin < SMALL)
    {
        // Apply tracking correction towards tet centre.
        // Generate current tet to find centre to apply correction.

        tetPointRef tet = currentTet();

        if (debug)
        {
            Pout<< "tracking rescue for lambdaMin:" << lambdaMin
                << "from " << position();
        }

        position_ += trackingCorrectionTol*(tet.centre() - position_);

        if
        (
            cloud.hasWallImpactDistance()
         && !internalFace(faceHitTetIs.face())
         && cloud.cellHasWallFaces()[faceHitTetIs.cell()]
        )
        {
            const polyBoundaryMesh& patches = mesh_.boundaryMesh();

            label fI = faceHitTetIs.face();

            label patchI = patches.patchID()[fI - mesh_.nInternalFaces()];

            if (isA<wallPolyPatch>(patches[patchI]))
            {
                // In the case of collision with a wall where there is
                // a non-zero wallImpactDistance, it is possible for
                // there to be a tracking correction required to bring
                // the particle into the domain, but the position of
                // the particle is further from the wall than the tet
                // centre, in which case the normal correction can be
                // counter-productive, i.e. pushes the particle
                // further out of the domain.  In this case it is the
                // position that hit the wall that is in need of a
                // rescue correction.

                triPointRef wallTri = faceHitTetIs.faceTri(mesh_);

                tetPointRef wallTet = faceHitTetIs.tet(mesh_);

                vector nHat = wallTri.normal();
                nHat /= mag(nHat);

                const scalar r = p.wallImpactDistance(nHat);

                // Removing (approximately) the wallTri normal
                // component of the existing correction, to avoid the
                // situation where the existing correction in the wall
                // normal direction is larger towards the wall than
                // the new correction is away from it.
                position_ +=
                    trackingCorrectionTol
                   *(
                        (wallTet.centre() - (position_ + r*nHat))
                      - (nHat & (tet.centre() - position_))*nHat
                    );
            }
        }

        if (debug)
        {
            Pout<< " to " << position() << endl;
        }

        cloud.trackingRescue();
    }

    return trackFraction;
}


template<class CloudType>
void CML::particle::hitWallFaces
(
    const CloudType& cloud,
    const vector& from,
    const vector& to,
    scalar& lambdaMin,
    tetIndices& closestTetIs
)
{
    typedef typename CloudType::particleType particleType;

    if (!(cloud.hasWallImpactDistance() && cloud.cellHasWallFaces()[cellI_]))
    {
        return;
    }

    particleType& p = static_cast<particleType&>(*this);

    const faceList& pFaces = mesh_.faces();

    const CML::cell& thisCell = mesh_.cells()[cellI_];

    scalar lambdaDistanceTolerance =
        lambdaDistanceToleranceCoeff*mesh_.cellVolumes()[cellI_];

    const polyBoundaryMesh& patches = mesh_.boundaryMesh();

    forAll(thisCell, cFI)
    {
        label fI = thisCell[cFI];

        if (internalFace(fI))
        {
            continue;
        }

        label patchI = patches.patchID()[fI - mesh_.nInternalFaces()];

        if (isA<wallPolyPatch>(patches[patchI]))
        {
            // Get the decomposition of this wall face

            const List<tetIndices> faceTetIs =
                polyMeshTetDecomposition::faceTetIndices(mesh_, fI, cellI_);

            const CML::face& f = pFaces[fI];

            forAll(faceTetIs, tI)
            {
                const tetIndices& tetIs = faceTetIs[tI];

                triPointRef tri = tetIs.faceTri(mesh_);

                vector n = tri.normal();

                vector nHat = n/mag(n);

                // Radius of particle with respect to this wall face
                // triangle.  Assuming that the wallImpactDistance
                // does not change as the particle or the mesh moves
                // forward in time.
                scalar r = p.wallImpactDistance(nHat);

                vector toPlusRNHat = to + r*nHat;

                // triI = 0 because it is the cell face tri of the tet
                // we are concerned with.
                scalar tetClambda = tetLambda
                (
                    tetIs.tet(mesh_).centre(),
                    toPlusRNHat,
                    0,
                    n,
                    f[tetIs.faceBasePt()],
                    cellI_,
                    fI,
                    tetIs.tetPt(),
                    lambdaDistanceTolerance
                );

                if ((tetClambda <= 0.0) || (tetClambda >= 1.0))
                {
                    // toPlusRNHat is not on the outside of the plane of
                    // the wall face tri, the tri cannot be hit.
                    continue;
                }

                // Check if the actual trajectory of the near-tri
                // points intersects the triangle.

                vector fromPlusRNHat = from + r*nHat;

                // triI = 0 because it is the cell face tri of the tet
                // we are concerned with.
                scalar lambda = tetLambda
                (
                    fromPlusRNHat,
                    toPlusRNHat,
                    0,
                    n,
                    f[tetIs.faceBasePt()],
                    cellI_,
                    fI,
                    tetIs.tetPt(),
                    lambdaDistanceTolerance
                );

                pointHit hitInfo(vector::zero);

                if (mesh_.moving())
                {
                    // For a moving mesh, the position of wall
                    // triangle needs to be moved in time to be
                    // consistent with the moment defined by the
                    // current value of stepFraction and the value of
                    // lambda just calculated.

                    // Total fraction thought the timestep of the
                    // motion, including stepFraction before the
                    // current tracking step and the current
                    // lambda
                    // i.e.
                    // let s = stepFraction, l = lambda
                    // Motion of x in time:
                    // |-----------------|---------|---------|
                    // x00               x0        xi        x
                    //
                    // where xi is the correct value of x at the required
                    // tracking instant.
                    //
                    // x0 = x00 + s*(x - x00) = s*x + (1 - s)*x00
                    //
                    // i.e. the motion covered by previous tracking portions
                    // within this timestep, and
                    //
                    // xi = x0 + l*(x - x0)
                    //    = l*x + (1 - l)*x0
                    //    = l*x + (1 - l)*(s*x + (1 - s)*x00)
                    //    = (s + l - s*l)*x + (1 - (s + l - s*l))*x00
                    //
                    // let m = (s + l - s*l)
                    //
                    // xi = m*x + (1 - m)*x00 = x00 + m*(x - x00);
                    //
                    // In the same form as before.

                    // Clip lambda to 0.0-1.0 to ensure that sensible
                    // positions are used for triangle intersections.
                    scalar lam = max(0.0, min(1.0, lambda));

                    scalar m = stepFraction_ + lam - (stepFraction_*lam);

                    triPointRef tri00 = tetIs.oldFaceTri(mesh_);

                    // Use SMALL positive tolerance to make the triangle
                    // slightly "fat" to improve robustness.  Intersection
                    // is calculated as the ray (from + r*nHat) -> (to +
                    // r*nHat).

                    point tPtA = tri00.a() + m*(tri.a() - tri00.a());
                    point tPtB = tri00.b() + m*(tri.b() - tri00.b());
                    point tPtC = tri00.c() + m*(tri.c() - tri00.c());

                    triPointRef t(tPtA, tPtB, tPtC);

                    // The point fromPlusRNHat + m*(to - from) is on the
                    // plane of the triangle.  Determine the
                    // intersection with this triangle by testing if
                    // this point is inside or outside of the triangle.
                    hitInfo = t.intersection
                    (
                        fromPlusRNHat + m*(to - from),
                        t.normal(),
                        intersection::FULL_RAY,
                        SMALL
                    );
                }
                else
                {
                    // Use SMALL positive tolerance to make the triangle
                    // slightly "fat" to improve robustness.  Intersection
                    // is calculated as the ray (from + r*nHat) -> (to +
                    // r*nHat).
                    hitInfo = tri.intersection
                    (
                        fromPlusRNHat,
                        (to - from),
                        intersection::FULL_RAY,
                        SMALL
                    );
                }

                if (hitInfo.hit())
                {
                    if (lambda < lambdaMin)
                    {
                        lambdaMin = lambda;

                        faceI_ = fI;

                        closestTetIs = tetIs;
                    }
                }
            }
        }
    }
}


template<class TrackData>
void CML::particle::hitFace(TrackData&)
{}


template<class TrackData>
bool CML::particle::hitPatch
(
    const polyPatch&,
    TrackData&,
    const label,
    const scalar,
    const tetIndices&
)
{
    return false;
}


template<class TrackData>
void CML::particle::hitWedgePatch
(
    const wedgePolyPatch& wpp,
    TrackData&
)
{
    FatalErrorIn
    (
        "void CML::particle::hitWedgePatch"
        "("
            "const wedgePolyPatch& wpp, "
            "TrackData&"
        ")"
    )   << "Hitting a wedge patch should not be possible."
        << abort(FatalError);

    vector nf = normal();
    nf /= mag(nf);

    transformProperties(I - 2.0*nf*nf);
}


template<class TrackData>
void CML::particle::hitSymmetryPatch
(
    const symmetryPolyPatch& spp,
    TrackData&
)
{
    vector nf = normal();
    nf /= mag(nf);

    transformProperties(I - 2.0*nf*nf);
}


template<class TrackData>
void CML::particle::hitCyclicPatch
(
    const cyclicPolyPatch& cpp,
    TrackData& td
)
{
    faceI_ = cpp.transformGlobalFace(faceI_);

    cellI_ = mesh_.faceOwner()[faceI_];

    tetFaceI_ = faceI_;

    // See note in correctAfterParallelTransfer for tetPtI_ addressing.
    tetPtI_ = mesh_.faces()[tetFaceI_].size() - 1 - tetPtI_;

    const cyclicPolyPatch& receiveCpp = cpp.neighbPatch();
    label patchFacei = receiveCpp.whichFace(faceI_);

    // Now the particle is on the receiving side

    // Have patch transform the position
    receiveCpp.transformPosition(position_, patchFacei);

    // Transform the properties
    if (!receiveCpp.parallel())
    {
        const tensor& T =
        (
            receiveCpp.forwardT().size() == 1
          ? receiveCpp.forwardT()[0]
          : receiveCpp.forwardT()[patchFacei]
        );
        transformProperties(T);
    }
    else if (receiveCpp.separated())
    {
        const vector& s =
        (
            (receiveCpp.separation().size() == 1)
          ? receiveCpp.separation()[0]
          : receiveCpp.separation()[patchFacei]
        );
        transformProperties(-s);
    }
}


template<class TrackData>
void CML::particle::hitProcessorPatch(const processorPolyPatch&, TrackData&)
{}


template<class TrackData>
void CML::particle::hitWallPatch
(
    const wallPolyPatch&,
    TrackData&,
    const tetIndices&
)
{}


template<class TrackData>
void CML::particle::hitPatch(const polyPatch&, TrackData&)
{}



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
