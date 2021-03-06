/*---------------------------------------------------------------------------*\
Copyright (C) 2017-2018 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Caelus is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

Class
    CML::regionModels::surfaceFilmModels::distributionContactAngleForce

Description
    PDF distribution based film contact angle force

See also
    CML::regionModels::surfaceFilmModels::contactAngleForce
    CML::distributionModel

SourceFiles
    distributionContactAngleForce.cpp

\*---------------------------------------------------------------------------*/

#ifndef distributionContactAngleForce_HPP
#define distributionContactAngleForce_HPP

#include "contactAngleForce.hpp"
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
                Class distributionContactAngleForce Declaration
\*---------------------------------------------------------------------------*/

class distributionContactAngleForce
:
    public contactAngleForce
{
    // Private Data

        //- Random number generator
        Random rndGen_;

        //- Parcel size PDF model
        const autoPtr<distributionModel> distribution_;


    // Private member functions

        //- Disallow default bitwise copy construct
        distributionContactAngleForce(const distributionContactAngleForce&);

        //- Disallow default bitwise assignment
        void operator=(const distributionContactAngleForce&);


protected:

        //- Return the contact angle field
        virtual tmp<volScalarField> theta() const;


public:

    //- Runtime type information
    TypeName("distributionContactAngle");


    // Constructors

        //- Construct from surface film model
        distributionContactAngleForce
        (
            surfaceFilmRegionModel& film,
            const dictionary& dict
        );


    //- Destructor
    virtual ~distributionContactAngleForce();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmModels
} // End namespace regionModels
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
