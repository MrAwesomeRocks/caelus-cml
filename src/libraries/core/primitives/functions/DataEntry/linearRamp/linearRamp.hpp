/*---------------------------------------------------------------------------*\
Copyright (C) 2017 OpenFOAM Foundation
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
    CML::DataEntryTypes::linearRamp

Description
    Linear ramp function starting from 0 and increasing linearRamply to 1 from
    \c start over the \c duration and remaining at 1 thereafter.

See also
    CML::DataEntryTypes::ramp

SourceFiles
    linearRamp.cpp

\*---------------------------------------------------------------------------*/

#ifndef linearRamp_HPP
#define linearRamp_HPP

#include "ramp.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace DataEntryTypes
{

/*---------------------------------------------------------------------------*\
                           Class linearRamp Declaration
\*---------------------------------------------------------------------------*/

class linearRamp
:
    public ramp
{
    // Private Member Functions

        //- Disallow default bitwise assignment
        void operator=(const linearRamp&);


public:

    // Runtime type information
    TypeName("linearRamp");


    // Constructors

        //- Construct from entry name and dictionary
        linearRamp
        (
            const word& entryName,
            const dictionary& dict
        );

        //- Construct and return a clone
        virtual tmp<DataEntry<scalar> > clone() const
        {
            return tmp<DataEntry<scalar> >(new linearRamp(*this));
        }


    //- Destructor
    virtual ~linearRamp();


    // Member Functions

        //- Return value for time t
        scalar value(const scalar t) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace DataEntryTypes
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
