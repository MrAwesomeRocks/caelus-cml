/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2018 OpenFOAM Foundation
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
    CML::regionModels::surfaceFilmModels::drippingInjection

Description
    Film Dripping mass transfer model.

    If the film mass exceeds that needed to generate a valid parcel, the
    equivalent mass is removed from the film.

    New parcel diameters are sampled from a PDF.

SourceFiles
    drippingInjection.C

\*---------------------------------------------------------------------------*/

#ifndef drippingInjection_H
#define drippingInjection_H

#include "injectionModel.hpp"
#include "distributionModel.hpp"
#include "Random.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace regionModels
{
namespace surfaceFilmModels
{

/*---------------------------------------------------------------------------*\
                    Class drippingInjection Declaration
\*---------------------------------------------------------------------------*/

class drippingInjection
:
    public injectionModel
{
private:

    // Private member functions

        //- Disallow default bitwise copy construct
        drippingInjection(const drippingInjection&);

        //- Disallow default bitwise assignment
        void operator=(const drippingInjection&);


protected:

    // Protected data

        //- Stable film thickness - drips only formed if thickness
        //  exceeds this threshold value
        scalar deltaStable_;

        //- Number of particles per parcel
        scalar particlesPerParcel_;

        //- Random number generator
        Random rndGen_;

        //- Parcel size PDF model
        const autoPtr<distributionModel>
            parcelDistribution_;

        //- Diameters of particles to inject into the dripping
        scalarList diameter_;


public:

    //- Runtime type information
    TypeName("drippingInjection");


    // Constructors

        //- Construct from surface film model
        drippingInjection
        (
            surfaceFilmRegionModel& film,
            const dictionary& dict
        );


    //- Destructor
    virtual ~drippingInjection();


    // Member Functions

        // Evolution

            //- Correct
            virtual void correct
            (
                scalarField& availableMass,
                scalarField& massToInject,
                scalarField& diameterToInject
            );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmModels
} // End namespace regionModels
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
