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
    CML::distributionModels::normal

Description
    A normal distribution model

    \verbatim
        model = strength * exp(-0.5*((x - expectation)/variance)^2 )
    \endverbatim

    strength only has meaning if there's more than one distribution model

SourceFiles
    normal.cpp

\*---------------------------------------------------------------------------*/

#ifndef normal_H
#define normal_H

#include "distributionModel.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace distributionModels
{

/*---------------------------------------------------------------------------*\
                           Class normal Declaration
\*---------------------------------------------------------------------------*/

class normal
:
    public distributionModel
{
    // Private data


        //- Distribution minimum
        scalar minValue_;

        //- Distribution maximum
        scalar maxValue_;


        // Model coefficients

            scalar expectation_;
            scalar variance_;

            scalar a_;


public:

    //- Runtime type information
    TypeName("normal");


    // Constructors

        //- Construct from components
        normal(const dictionary& dict, Random& rndGen);

        //- Construct copy
        normal(const normal& p);

        //- Construct and return a clone
        virtual autoPtr<distributionModel> clone() const
        {
            return autoPtr<distributionModel>(new normal(*this));
        }


    //- Destructor
    virtual ~normal();


    // Member Functions

        //- Sample the distributionModel
        virtual scalar sample() const;

        //- Return the minimum value
        virtual scalar minValue() const;

        //- Return the maximum value
        virtual scalar maxValue() const;

        //- Return the mean value
        virtual scalar meanValue() const;

        virtual scalar erfInv(const scalar y) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace distributionModels
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
