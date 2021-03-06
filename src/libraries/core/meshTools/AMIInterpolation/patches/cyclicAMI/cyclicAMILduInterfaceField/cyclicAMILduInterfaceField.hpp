/*---------------------------------------------------------------------------*\
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
    CML::cyclicAMILduInterfaceField

Description
    Abstract base class for cyclic AMI coupled interfaces

SourceFiles
    cyclicAMILduInterfaceField.cpp

\*---------------------------------------------------------------------------*/

#ifndef cyclicAMILduInterfaceField_H
#define cyclicAMILduInterfaceField_H

#include "primitiveFieldsFwd.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                  Class cyclicAMILduInterfaceField Declaration
\*---------------------------------------------------------------------------*/

class cyclicAMILduInterfaceField
{

public:

    //- Runtime type information
    TypeName("cyclicAMILduInterfaceField");


    // Constructors

        //- Construct null
        cyclicAMILduInterfaceField()
        {}


    //- Destructor
    virtual ~cyclicAMILduInterfaceField();


    // Member Functions

        // Access

            //- Is the transform required
            virtual bool doTransform() const = 0;

            //- Return face transformation tensor
            virtual const tensorField& forwardT() const = 0;

            //- Return neighbour-cell transformation tensor
            virtual const tensorField& reverseT() const = 0;

            //- Return rank of component for transform
            virtual int rank() const = 0;


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
void CML::cyclicAMILduInterfaceField::transformCoupleField
(
    Field<Type>& f
) const
{
    if (doTransform())
    {
        if (forwardT().size() == 1)
        {
            transform(f, forwardT()[0], f);
        }
        else
        {
            transform(f, forwardT(), f);
        }
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
