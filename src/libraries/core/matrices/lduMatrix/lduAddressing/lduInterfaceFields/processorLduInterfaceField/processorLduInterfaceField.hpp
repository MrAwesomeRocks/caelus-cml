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
    CML::processorLduInterfaceField

Description
    Abstract base class for processor coupled interfaces.

SourceFiles
    processorLduInterfaceField.cpp

\*---------------------------------------------------------------------------*/

#ifndef processorLduInterfaceField_H
#define processorLduInterfaceField_H

#include "primitiveFieldsFwd.hpp"
#include "typeInfo.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                  Class processorLduInterfaceField Declaration
\*---------------------------------------------------------------------------*/

class processorLduInterfaceField
{

public:

    //- Runtime type information
    TypeName("processorLduInterfaceField");


    // Constructors

        //- Construct given coupled patch
        processorLduInterfaceField()
        {}


    //- Destructor
    virtual ~processorLduInterfaceField();


    // Member Functions

        // Access

            //- Return processor number
            virtual int myProcNo() const = 0;

            //- Return neighbour processor number
            virtual int neighbProcNo() const = 0;

            //- Is the transform required
            virtual bool doTransform() const = 0;

            //- Return face transformation tensor
            virtual const tensorField& forwardT() const = 0;

            //- Return rank of component for transform
            virtual int rank() const = 0;


        //- Transform given patch component field
        void transformCoupleField
        (
            scalarField& f,
            const direction cmpt
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
