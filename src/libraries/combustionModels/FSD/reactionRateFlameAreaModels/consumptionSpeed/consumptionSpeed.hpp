/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2018 OpenFOAM Foundation
-------------------------------------------------------------------------------
License

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
    CML::consumptionSpeed

Description
    Correlation function for laminar consumption speed obtained from flamelet
    solution at increasing strain rates.

SourceFiles
    consumptionSpeed.cpp

\*---------------------------------------------------------------------------*/

#ifndef consumptionSpeed_HPP
#define consumptionSpeed_HPP

#include "IOdictionary.hpp"
#include "volFields.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class consumptionSpeed Declaration
\*---------------------------------------------------------------------------*/

class consumptionSpeed
{

    //- Maximum consumption speed
    scalar omega0_;

    //- Exponential factor
    scalar eta_;

    //- Extinction strain
    scalar sigmaExt_;

    //- Minimum consumption speed
    scalar omegaMin_;


    // Private member functions

    //- Return consumption rate
    scalar omega0Sigma(scalar sigma, scalar a) const;

    //- Disallow copy construct
    consumptionSpeed(const consumptionSpeed&);

    //- Disallow default bitwise assignment
    void operator=(const consumptionSpeed&);


public:

    //- Runtime type information
    TypeName("consumptionSpeed");


    //- Construct from dictionary
    consumptionSpeed(const dictionary& dict);


    //- Destructor
    virtual ~consumptionSpeed()
    {}


    // Member functions

    //- Return speed consumption rate temp
    tmp<volScalarField> omega0Sigma(const volScalarField& sigma);


    // Access functions

    scalar omega0() const
    {
        return omega0_;
    }

    scalar eta() const
    {
        return eta_;
    }

    scalar sigmaExt() const
    {
        return sigmaExt_;
    }

    scalar omegaMin() const
    {
        return omegaMin_;
    }


    // IO

    //- Update properties
    void read(const dictionary& dict);

};


} // End namespace CML


#endif
