/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
Copyright (C) 2011-2017 OpenFOAM Foundation
Copyright (C) 2018 OpenCFD Ltd.
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
    CML::PairSpringSliderDashpot

Description
    Pair forces between particles colliding with a spring, slider, damper model

\*---------------------------------------------------------------------------*/

#ifndef PairSpringSliderDashpot_H
#define PairSpringSliderDashpot_H

#include "PairModel.hpp"
#include "CollisionRecordList.hpp"
#include "mathematicalConstants.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
/*---------------------------------------------------------------------------*\
                    Class PairSpringSliderDashpot Declaration
\*---------------------------------------------------------------------------*/

template<class CloudType>
class PairSpringSliderDashpot
:
    public PairModel<CloudType>
{
    // Private data

        //- Effective Young's modulus value, assuming both particles have
        //  the same E value
        scalar Estar_;

        //- Effective shear modulus value, assuming both particles have
        //  the same Poisson's ratio and Young's modulus
        scalar Gstar_;

        //- alpha-coefficient, related to coefficient of restitution
        scalar alpha_;

        //- Spring power (b = 1 for linear, b = 3/2 for Hertzian)
        scalar b_;

        //- Coefficient of friction in for tangential sliding
        scalar mu_;

        //- Cohesion energy density [J/m^3]
        scalar cohesionEnergyDensity_;

        // Switch cohesion on and off
        bool cohesion_;

        //- The number of steps over which to resolve the minimum
        //  harmonic approximation of the collision period
        scalar collisionResolutionSteps_;

        //- Volume factor for determining the equivalent size of a
        //  parcel where nParticles is not 1.  The equivalent size of
        //  the parcel is
        //      parcelEquivVolume = volumeFactor*nParticles*p.volume()
        //  so
        //      parcelEquivD = cbrt(volumeFactor*nParticles)*p.d()
        //  + When volumeFactor = 1, the particles are compressed
        //    together so that the equivalent volume of the parcel is
        //    the sum of the constituent particles
        //  + When volumeFactor = 3*sqrt(2)/pi, the particles are
        //    close packed, but uncompressed.
        //  + When volumeFactor > 3*sqrt(2)/pi, the particles loosely
        //    grouped.
        // 3*sqrt(2)/pi = 1.350474 is the volume factor for close
        // packing, i.e pi/(3*sqrt(2)) is the maximum close packing
        // factor
        scalar volumeFactor_;

        //- Switch to control use of equivalent size particles.  Used
        //  because the calculation can be very expensive.
        bool useEquivalentSize_;


    // Private Member Functions

        //- Find the appropriate properties for determining the minimum
        //- Allowable timestep
        void findMinMaxProperties
        (
            scalar& RMin,
            scalar& rhoMax,
            scalar& vMagMax
        ) const;

public:

    //- Runtime type information
    TypeName("pairSpringSliderDashpot");


    // Constructors

        //- Construct from dictionary
        PairSpringSliderDashpot(const dictionary& dict, CloudType& cloud);


    //- Destructor
    virtual ~PairSpringSliderDashpot();


    // Member Functions

        //- Return the volumeFactor
        inline scalar volumeFactor() const
        {
            return volumeFactor_;
        }

        // Return the area of overlap between two spheres of radii rA and rB,
        // centres separated by a distance rAB.  Assumes rAB < (rA + rB).
        inline scalar overlapArea(scalar rA, scalar rB, scalar rAB) const
        {
            // Reference:
            // http://mathworld.wolfram.com/Sphere-SphereIntersection.html
            return
                mathematical::pi/4.0
               /sqr(rAB)
               *(
                    (-rAB + rA - rB)
                   *(-rAB - rA + rB)
                   *(-rAB + rA + rB)
                   *( rAB + rA + rB)
                );
        }

        //- Whether the PairModel has a timestep limit that will
        //  require subCycling
        virtual bool controlsTimestep() const;

        //- For PairModels that control the timestep, calculate the
        //  number of subCycles needed to satisfy the minimum
        //  allowable timestep
        virtual label nSubCycles() const;

        //- Calculate the pair interaction between parcels
        virtual void evaluatePair
        (
            typename CloudType::parcelType& pA,
            typename CloudType::parcelType& pB
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class CloudType>
void CML::PairSpringSliderDashpot<CloudType>::findMinMaxProperties
(
    scalar& RMin,
    scalar& rhoMax,
    scalar& UMagMax
) const
{
    RMin = VGREAT;
    rhoMax = -VGREAT;
    UMagMax = -VGREAT;

    forAllConstIter(typename CloudType, this->owner(), iter)
    {
        const typename CloudType::parcelType& p = iter();

        // Finding minimum diameter to avoid excessive arithmetic

        scalar dEff = p.d();

        if (useEquivalentSize_)
        {
            dEff *= cbrt(p.nParticle()*volumeFactor_);
        }

        RMin = min(dEff, RMin);

        rhoMax = max(p.rho(), rhoMax);

        UMagMax = max
        (
            mag(p.U()) + mag(p.omega())*dEff/2,
            UMagMax
        );
    }

    // Transform the minimum diameter into minimum radius
    //     rMin = dMin/2
    // then rMin into minimum R,
    //     1/RMin = 1/rMin + 1/rMin,
    //     RMin = rMin/2 = dMin/4

    RMin /= 4.0;

    // Multiply by two to create the worst-case relative velocity

    UMagMax = 2*UMagMax;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
CML::PairSpringSliderDashpot<CloudType>::PairSpringSliderDashpot
(
    const dictionary& dict,
    CloudType& cloud
)
:
    PairModel<CloudType>(dict, cloud, typeName),
    Estar_(),
    Gstar_(),
    alpha_(readScalar(this->coeffDict().lookup("alpha"))),
    b_(readScalar(this->coeffDict().lookup("b"))),
    mu_(readScalar(this->coeffDict().lookup("mu"))),
    cohesionEnergyDensity_
    (
        readScalar(this->coeffDict().lookup("cohesionEnergyDensity"))
    ),
    cohesion_(false),
    collisionResolutionSteps_
    (
        readScalar(this->coeffDict().lookup("collisionResolutionSteps"))
    ),
    volumeFactor_(1.0),
    useEquivalentSize_(Switch(this->coeffDict().lookup("useEquivalentSize")))
{
    if (useEquivalentSize_)
    {
        volumeFactor_ = readScalar(this->coeffDict().lookup("volumeFactor"));
    }

    scalar nu = this->owner().constProps().poissonsRatio();

    scalar E = this->owner().constProps().youngsModulus();

    Estar_ = E/(2.0*(1.0 - sqr(nu)));

    scalar G = E/(2.0*(1.0 + nu));

    Gstar_ = G/(2.0*(2.0 - nu));

    cohesion_ = (mag(cohesionEnergyDensity_) > VSMALL);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class CloudType>
CML::PairSpringSliderDashpot<CloudType>::~PairSpringSliderDashpot()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
bool CML::PairSpringSliderDashpot<CloudType>::controlsTimestep() const
{
    return true;
}


template<class CloudType>
CML::label CML::PairSpringSliderDashpot<CloudType>::nSubCycles() const
{
    if (!(this->owner().size()))
    {
        return 1;
    }

    scalar RMin;
    scalar rhoMax;
    scalar UMagMax;

    findMinMaxProperties(RMin, rhoMax, UMagMax);

    // Note:  pi^(7/5)*(5/4)^(2/5) = 5.429675
    scalar minCollisionDeltaT =
        5.429675
       *RMin
       *pow(rhoMax/(Estar_*sqrt(UMagMax) + VSMALL), 0.4)
       /collisionResolutionSteps_;

    return ceil(this->owner().time().deltaTValue()/minCollisionDeltaT);
}


template<class CloudType>
void CML::PairSpringSliderDashpot<CloudType>::evaluatePair
(
    typename CloudType::parcelType& pA,
    typename CloudType::parcelType& pB
) const
{
    vector r_AB = (pA.position() - pB.position());

    scalar dAEff = pA.d();

    if (useEquivalentSize_)
    {
        dAEff *= cbrt(pA.nParticle()*volumeFactor_);
    }

    scalar dBEff = pB.d();

    if (useEquivalentSize_)
    {
        dBEff *= cbrt(pB.nParticle()*volumeFactor_);
    }

    scalar r_AB_mag = mag(r_AB);

    scalar normalOverlapMag = 0.5*(dAEff + dBEff) - r_AB_mag;

    if (normalOverlapMag > 0)
    {
        // Particles in collision

        vector rHat_AB = r_AB/(r_AB_mag + VSMALL);

        vector U_AB = pA.U() - pB.U();

        // Effective radius
        scalar R = 0.5*dAEff*dBEff/(dAEff + dBEff);

        // Effective mass
        scalar M = pA.mass()*pB.mass()/(pA.mass() + pB.mass());

        scalar kN = (4.0/3.0)*sqrt(R)*Estar_;

        scalar etaN = alpha_*sqrt(M*kN)*pow025(normalOverlapMag);

        // Normal force
        vector fN_AB =
            rHat_AB
           *(kN*pow(normalOverlapMag, b_) - etaN*(U_AB & rHat_AB));

        // Cohesion force, energy density multiplied by the area of
        // particle-particle overlap
        if (cohesion_)
        {
            fN_AB +=
                -cohesionEnergyDensity_
                *overlapArea(dAEff/2.0, dBEff/2.0, r_AB_mag)
                *rHat_AB;
        }

        pA.f() += fN_AB;
        pB.f() += -fN_AB;

        vector USlip_AB =
            U_AB - (U_AB & rHat_AB)*rHat_AB
          - ((dAEff/2*pA.omega() + dBEff/2*pB.omega()) ^ rHat_AB);

        scalar deltaT = this->owner().mesh().time().deltaTValue();

        vector& tangentialOverlap_AB =
            pA.collisionRecords().matchPairRecord
            (
                pB.origProc(),
                pB.origId()
            ).collisionData();

        vector& tangentialOverlap_BA =
            pB.collisionRecords().matchPairRecord
            (
                pA.origProc(),
                pA.origId()
            ).collisionData();

        vector deltaTangentialOverlap_AB = USlip_AB*deltaT;

        tangentialOverlap_AB += deltaTangentialOverlap_AB;
        tangentialOverlap_BA += -deltaTangentialOverlap_AB;

        scalar tangentialOverlapMag = mag(tangentialOverlap_AB);

        if (tangentialOverlapMag > VSMALL)
        {
            scalar kT = 8.0*sqrt(R*normalOverlapMag)*Gstar_;

            scalar etaT = etaN;

            // Tangential force
            vector fT_AB;

            if (kT*tangentialOverlapMag > mu_*mag(fN_AB))
            {
                // Tangential force greater than sliding friction,
                // particle slips

                fT_AB = -mu_*mag(fN_AB)*USlip_AB/mag(USlip_AB);

                tangentialOverlap_AB = Zero;
                tangentialOverlap_BA = Zero;
            }
            else
            {
                fT_AB = - kT*tangentialOverlap_AB - etaT*USlip_AB;
            }

            pA.f() += fT_AB;
            pB.f() += -fT_AB;

            pA.torque() += (dAEff/2*-rHat_AB) ^ fT_AB;
            pB.torque() += (dBEff/2*rHat_AB) ^ -fT_AB;
        }
    }
}


#endif

// ************************************************************************* //
