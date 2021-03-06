/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
Copyright (C) 2011-2017 OpenFOAM Foundation
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
    CML::NonInertialFrameForce

Description
    Calculates particle non-inertial reference frame force.  Variable names as
    from Landau and Lifshitz, Mechanics, 3rd Ed, p126-129.


\*---------------------------------------------------------------------------*/

#ifndef NonInertialFrameForce_H
#define NonInertialFrameForce_H

#include "ParticleForce.hpp"
#include "uniformDimensionedFields.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

class fvMesh;

/*---------------------------------------------------------------------------*\
                  Class NonInertialFrameForce Declaration
\*---------------------------------------------------------------------------*/

template<class CloudType>
class NonInertialFrameForce
:
    public ParticleForce<CloudType>
{
    // Private data

        //- Name of the linear acceleration field
        word WName_;

        //- The linear acceleration of the reference frame
        vector W_;

        //- Name of the angular velocity field
        word omegaName_;

        //- The angular velocity of the reference frame
        vector omega_;

        //- Name of the angular acceleration field
        word omegaDotName_;

        //- The angular acceleration of the reference frame
        vector omegaDot_;

        //- Name of the centre of rotation field
        word centreOfRotationName_;

        //- The centre of rotation of the reference frame
        vector centreOfRotation_;


public:

    //- Runtime type information
    TypeName("nonInertialFrame");


    // Constructors

        //- Construct from mesh
        NonInertialFrameForce
        (
            CloudType& owner,
            const fvMesh& mesh,
            const dictionary& dict
        );

        //- Construct copy
        NonInertialFrameForce(const NonInertialFrameForce& niff);

        //- Construct and return a clone
        virtual autoPtr<ParticleForce<CloudType> > clone() const
        {
            return autoPtr<ParticleForce<CloudType> >
            (
                new ParticleForce<CloudType>(*this)
            );
        }


    //- Destructor
    virtual ~NonInertialFrameForce();


    // Member Functions

        // Access

            //- Return the linear acceleration of the reference frame
            inline const vector& W() const;

            //- Return the angular velocity of the reference frame
            inline const vector& omega() const;

            //- Return the angular acceleration of the reference frame
            inline const vector& omegaDot() const;

            //- Return the centre of rotation of the reference frame
            inline const vector& centreOfRotation() const;


        // Evaluation

            //- Cache fields
            virtual void cacheFields(const bool store);

            //- Calculate the non-coupled force
            virtual forceSuSp calcNonCoupled
            (
                const typename CloudType::parcelType& p,
                const typename CloudType::parcelType::trackingData& td,
                const scalar dt,
                const scalar mass,
                const scalar Re,
                const scalar muc
            ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class CloudType>
inline const CML::vector& CML::NonInertialFrameForce<CloudType>::W() const
{
    return W_;
}


template<class CloudType>
inline const CML::vector& CML::NonInertialFrameForce<CloudType>::omega() const
{
    return omega_;
}


template<class CloudType>
inline const CML::vector&
CML::NonInertialFrameForce<CloudType>::omegaDot() const
{
    return omegaDot_;
}


template<class CloudType>
inline const CML::vector&
CML::NonInertialFrameForce<CloudType>::centreOfRotation() const
{
    return centreOfRotation_;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
CML::NonInertialFrameForce<CloudType>::NonInertialFrameForce
(
    CloudType& owner,
    const fvMesh& mesh,
    const dictionary& dict
)
:
    ParticleForce<CloudType>(owner, mesh, dict, typeName, true),
    WName_
    (
        this->coeffs().template lookupOrDefault<word>
        (
            "linearAccelerationName",
            "linearAcceleration"
        )
    ),
    W_(Zero),
    omegaName_
    (
        this->coeffs().template lookupOrDefault<word>
        (
            "angularVelocityName",
            "angularVelocity"
        )
    ),
    omega_(Zero),
    omegaDotName_
    (
        this->coeffs().template lookupOrDefault<word>
        (
            "angularAccelerationName",
            "angularAcceleration"
        )
    ),
    omegaDot_(Zero),
    centreOfRotationName_
    (
        this->coeffs().template lookupOrDefault<word>
        (
            "centreOfRotationName",
            "centreOfRotation"
        )
    ),
    centreOfRotation_(Zero)
{}


template<class CloudType>
CML::NonInertialFrameForce<CloudType>::NonInertialFrameForce
(
    const NonInertialFrameForce& niff
)
:
    ParticleForce<CloudType>(niff),
    WName_(niff.WName_),
    W_(niff.W_),
    omegaName_(niff.omegaName_),
    omega_(niff.omega_),
    omegaDotName_(niff.omegaDotName_),
    omegaDot_(niff.omegaDot_),
    centreOfRotationName_(niff.centreOfRotationName_),
    centreOfRotation_(niff.centreOfRotation_)
{}


// * * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * //

template<class CloudType>
CML::NonInertialFrameForce<CloudType>::~NonInertialFrameForce()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
void CML::NonInertialFrameForce<CloudType>::cacheFields(const bool store)
{
    W_ = Zero;
    omega_ = Zero;
    omegaDot_ = Zero;
    centreOfRotation_ = Zero;

    if (store)
    {
        if
        (
            this->mesh().template foundObject<uniformDimensionedVectorField>
            (
                WName_
            )
        )
        {
            const uniformDimensionedVectorField& W = this->mesh().template
                lookupObject<uniformDimensionedVectorField>(WName_);

            W_ = W.value();
        }

        if
        (
            this->mesh().template foundObject<uniformDimensionedVectorField>
            (
                omegaName_
            )
        )
        {
            const uniformDimensionedVectorField& omega = this->mesh().template
                lookupObject<uniformDimensionedVectorField>(omegaName_);

            omega_ = omega.value();
        }

        if
        (
            this->mesh().template foundObject<uniformDimensionedVectorField>
            (
                omegaDotName_
            )
        )
        {
            const uniformDimensionedVectorField& omegaDot =
                this->mesh().template
                lookupObject<uniformDimensionedVectorField>(omegaDotName_);

            omegaDot_ = omegaDot.value();
        }

        if
        (
            this->mesh().template foundObject<uniformDimensionedVectorField>
            (
                centreOfRotationName_
            )
        )
        {
            const uniformDimensionedVectorField& centreOfRotation =
                this->mesh().template
                lookupObject<uniformDimensionedVectorField>
                (
                    centreOfRotationName_
                );

            centreOfRotation_ = centreOfRotation.value();
        }
    }
}


template<class CloudType>
CML::forceSuSp CML::NonInertialFrameForce<CloudType>::calcNonCoupled
(
    const typename CloudType::parcelType& p,
    const typename CloudType::parcelType::trackingData& td,
    const scalar dt,
    const scalar mass,
    const scalar Re,
    const scalar muc
) const
{
    forceSuSp value(Zero, 0.0);

    const vector r = p.position() - centreOfRotation_;

    value.Su() =
        mass
       *(
           -W_
          + (r ^ omegaDot_)
          + 2.0*(p.U() ^ omega_)
          + (omega_ ^ (r ^ omega_))
        );

    return value;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
