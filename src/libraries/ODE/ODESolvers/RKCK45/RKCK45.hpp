/*---------------------------------------------------------------------------*\
Copyright (C) 2013-2018 OpenFOAM Foundation
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
    CML::RKCK45

Description
    4/5th Order Cash-Karp Runge-Kutta ODE solver.

    References:
    \verbatim
        Cash, J. R., & Karp, A. H. (1990).
        A variable order Runge-Kutta method for initial value problems
        with rapidly varying right-hand sides.
        ACM Transactions on Mathematical Software (TOMS), 16(3), 201-222.

        Hairer, E., Nørsett, S. P., & Wanner, G. (1993).
        Solving Ordinary Differential Equations I: Nonstiff Problems,
        second edition.
        Springer-Verlag, Berlin.
    \endverbatim

SourceFiles
    RKCK45.cpp

\*---------------------------------------------------------------------------*/

#ifndef RKCK45_HPP
#define RKCK45_HPP

#include "ODESolver.hpp"
#include "adaptiveSolver.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class RKCK45 Declaration
\*---------------------------------------------------------------------------*/

class RKCK45
:
    public ODESolver,
    public adaptiveSolver
{

    //- RKCK Constants
    static const scalar
        c2, c3, c4, c5, c6,
        a21, a31, a32, a41, a42, a43, a51, a52, a53, a54,
        a61, a62, a63, a64, a65,
        b1, b3, b4, b6,
        e1, e3, e4, e5, e6;

    // Temporary fields
    mutable scalarField yTemp_;
    mutable scalarField k2_;
    mutable scalarField k3_;
    mutable scalarField k4_;
    mutable scalarField k5_;
    mutable scalarField k6_;

    //- Error-estimate field
    mutable scalarField err_;


public:

    //- Runtime type information
    TypeName("RKCK45");


    //- Construct from ODESystem
    RKCK45(const ODESystem& ode, const dictionary& dict);


    //- Destructor
    virtual ~RKCK45()
    {}


    // Member Functions

    //- Inherit solve from ODESolver
    using ODESolver::solve;

    //- Resize the ODE solver
    virtual bool resize();

    //- Solve a single step dx and return the error
    virtual scalar solve
    (
        const scalar x0,
        const scalarField& y0,
        const scalarField& dydx0,
        const scalar dx,
        scalarField& y
    ) const;

    //- Solve the ODE system and the update the state
    virtual void solve
    (
        scalar& x,
        scalarField& y,
        scalar& dxTry
    ) const;
};


} // End namespace CML


#endif
