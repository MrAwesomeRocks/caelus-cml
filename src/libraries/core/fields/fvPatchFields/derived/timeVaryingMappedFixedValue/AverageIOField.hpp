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
    CML::AverageIOField

Description
    A primitive field + average with IO.


\*---------------------------------------------------------------------------*/

#ifndef AverageIOField_H
#define AverageIOField_H

#include "regIOobject.hpp"
#include "Field.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                        Class AverageIOField Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class AverageIOField
:
    public regIOobject,
    public Field<Type>
{
    // Private data

        //- The average of the field
        Type average_;


public:

    TypeName("AverageField");


    // Constructors

        //- Construct from IOobject
        AverageIOField
        (
            const IOobject&
        );

        //- Construct from IOobject and size (does not set values)
        AverageIOField
        (
            const IOobject&,
            const label size
        );

        //- Construct from components
        AverageIOField
        (
            const IOobject&,
            const Type& average,
            const Field<Type>&
        );


    // Member functions

        const Type& average() const
        {
            return average_;
        }

        Type& average()
        {
            return average_;
        }

        bool writeData(Ostream& os) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
CML::AverageIOField<Type>::AverageIOField
(
    const IOobject& io
)
:
    regIOobject(io)
{
    readStream(typeName) >> average_;
    readStream(typeName) >> static_cast<Field<Type>&>(*this);
    close();
}


template<class Type>
CML::AverageIOField<Type>::AverageIOField
(
    const IOobject& io,
    const label size
)
:
    regIOobject(io),
    Field<Type>(size),
    average_(pTraits<Type>::zero)
{}


template<class Type>
CML::AverageIOField<Type>::AverageIOField
(
    const IOobject& io,
    const Type& average,
    const Field<Type>& f
)
:
    regIOobject(io),
    Field<Type>(f),
    average_(average)
{
    if (io.readOpt() == IOobject::READ_IF_PRESENT && headerOk())
    {
        readStream(typeName)
            >> average_
            >> static_cast<Field<Type>&>(*this);
        close();
    }
}


template<class Type>
bool CML::AverageIOField<Type>::writeData(Ostream& os) const
{
    os  << average_
        << token::NL
        << static_cast<const Field<Type>&>(*this);

    return os.good();
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
