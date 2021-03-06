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
    CML::ODESystem

Description
     Abstract base class for the systems of ordinary differential equations.

\*---------------------------------------------------------------------------*/

#ifndef ODESystem_H
#define ODESystem_H

#include "scalarField.hpp"
#include "scalarMatrices.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class ODESystem Declaration
\*---------------------------------------------------------------------------*/

class ODESystem
{

public:

    // Constructors

        //- Construct null
        ODESystem()
        {}


    //- Destructor
    virtual ~ODESystem()
    {}


    // Member Functions
        //- Return the number of equations in the system
        virtual label nEqns() const = 0;

        //- Calculate the derivatives in dydx
        virtual void derivatives
        (
            const scalar x,
            const scalarField& y,
            scalarField& dydx
        ) const = 0;

        //- Calculate the Jacobian of the system
        //  Need by the stiff-system solvers
        virtual void jacobian
        (
            const scalar x,
            const scalarField& y,
            scalarField& dfdx,
            scalarSquareMatrix& dfdy
        ) const = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
