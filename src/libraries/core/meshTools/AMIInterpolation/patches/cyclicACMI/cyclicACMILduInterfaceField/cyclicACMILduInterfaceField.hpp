/*---------------------------------------------------------------------------*\
Copyright (C) 2013 OpenFOAM Foundation
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
    CML::cyclicACMILduInterfaceField

Description
    Abstract base class for cyclic ACMI coupled interfaces

SourceFiles
    cyclicACMILduInterfaceField.C

\*---------------------------------------------------------------------------*/

#ifndef cyclicACMILduInterfaceField_H
#define cyclicACMILduInterfaceField_H

#include "cyclicAMILduInterfaceField.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                 Class cyclicACMILduInterfaceField Declaration
\*---------------------------------------------------------------------------*/

class cyclicACMILduInterfaceField
:
    public cyclicAMILduInterfaceField
{

public:

    //- Runtime type information
    TypeName("cyclicACMILduInterfaceField");


    // Constructors

        //- Construct null
        cyclicACMILduInterfaceField()
        {}


    //- Destructor
    virtual ~cyclicACMILduInterfaceField();


    // Member Functions

        //- Transform given patch field
        template<class Type>
        void transformCoupleField(Field<Type>& f) const;

        //- Transform given patch internal field
        void transformCoupleField
        (
            scalarField& psiInternal,
            const direction cmpt
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "tensorField.hpp"

template<class Type>
void CML::cyclicACMILduInterfaceField::transformCoupleField
(
    Field<Type>& f
) const
{
    cyclicAMILduInterfaceField::transformCoupleField(f);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
