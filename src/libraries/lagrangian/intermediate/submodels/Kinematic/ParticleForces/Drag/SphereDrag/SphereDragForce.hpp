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
    CML::SphereDragForce

Description
    Drag model based on assumption of solid spheres

\*---------------------------------------------------------------------------*/

#ifndef SphereDragForce_H
#define SphereDragForce_H

#include "ParticleForce.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
/*---------------------------------------------------------------------------*\
                       Class SphereDragForce Declaration
\*---------------------------------------------------------------------------*/

template<class CloudType>
class SphereDragForce
:
    public ParticleForce<CloudType>
{
    // Private Member Functions

        //- Drag coefficient multiplied by Reynolds number
        scalar CdRe(const scalar Re) const;


public:

    //- Runtime type information
    TypeName("sphereDrag");


    // Constructors

        //- Construct from mesh
        SphereDragForce
        (
            CloudType& owner,
            const fvMesh& mesh,
            const dictionary& dict
        );

        //- Construct copy
        SphereDragForce(const SphereDragForce<CloudType>& df);

        //- Construct and return a clone
        virtual autoPtr<ParticleForce<CloudType> > clone() const
        {
            return autoPtr<ParticleForce<CloudType> >
            (
                new SphereDragForce<CloudType>(*this)
            );
        }


    //- Destructor
    virtual ~SphereDragForce();


    // Member Functions

        // Evaluation

            //- Calculate the coupled force
            virtual forceSuSp calcCoupled
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

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class CloudType>
CML::scalar CML::SphereDragForce<CloudType>::CdRe(const scalar Re) const
{
    if (Re > 1000.0)
    {
        return 0.424*Re;
    }
    else
    {
        return 24.0*(1.0 + 1.0/6.0*pow(Re, 2.0/3.0));
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
CML::SphereDragForce<CloudType>::SphereDragForce
(
    CloudType& owner,
    const fvMesh& mesh,
    const dictionary& dict
)
:
    ParticleForce<CloudType>(owner, mesh, dict, typeName, false)
{}


template<class CloudType>
CML::SphereDragForce<CloudType>::SphereDragForce
(
    const SphereDragForce<CloudType>& df
)
:
    ParticleForce<CloudType>(df)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class CloudType>
CML::SphereDragForce<CloudType>::~SphereDragForce()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
CML::forceSuSp CML::SphereDragForce<CloudType>::calcCoupled
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

    value.Sp() = mass*0.75*muc*CdRe(Re)/(p.rho()*sqr(p.d()));

    return value;
}




// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
