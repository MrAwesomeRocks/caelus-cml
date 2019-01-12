/*---------------------------------------------------------------------------*\
Copyright (C) 2013-2018 OpenFOAM Foundation
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
    CML::seulex

Description
    An extrapolation-algorithm, based on the linearly implicit Euler method
    with step size control and order selection.

    Reference:
    \verbatim
        Hairer, E., Nørsett, S. P., & Wanner, G. (1996).
        Solving Ordinary Differential Equations II:
        Stiff and Differential-Algebraic Problems, second edition",
        Springer-Verlag, Berlin.
    \endverbatim

SourceFiles
    seulex.cpp

\*---------------------------------------------------------------------------*/

#ifndef seulex_HPP
#define seulex_HPP

#include "ODESolver.hpp"
#include "scalarMatrices.hpp"
#include "labelField.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class seulex Declaration
\*---------------------------------------------------------------------------*/

class seulex
:
    public ODESolver
{

    // Static constants
    static const label kMaxx_ = 12;
    static const label iMaxx_ = kMaxx_ + 1;
    static const scalar
        stepFactor1_, stepFactor2_, stepFactor3_,
        stepFactor4_, stepFactor5_,
        kFactor1_, kFactor2_;

    // Evaluated constants
    scalar jacRedo_;
    labelField nSeq_;
    scalarField cpu_;
    scalarSquareMatrix coeff_;

    // Temporary storage
    // held to avoid dynamic memory allocation between calls
    // and to transfer internal values between functions
    mutable scalar theta_;
    mutable label kTarg_;
    mutable scalarRectangularMatrix table_;

    mutable scalarField dfdx_;
    mutable scalarSquareMatrix dfdy_;
    mutable scalarSquareMatrix a_;
    mutable labelList pivotIndices_;

    // Fields space for "solve" function
    mutable scalarField dxOpt_, temp_;
    mutable scalarField y0_, ySequence_, scale_;

    // Fields used in "seul" function
    mutable scalarField dy_, yTemp_, dydx_;


    // Private Member Functions

    //- Computes the j-th line of the extrapolation table
    bool seul
    (
        const scalar x0,
        const scalarField& y0,
        const scalar dxTot,
        const label k,
        scalarField& y,
        const scalarField& scale
    ) const;

    //- Polynomial extrpolation
    void extrapolate
    (
        const label k,
        scalarRectangularMatrix& table,
        scalarField& y
    ) const;


public:

    //- Runtime type information
    TypeName("seulex");


    //- Construct from ODESystem
    seulex(const ODESystem& ode, const dictionary& dict);


    //- Destructor
    virtual ~seulex()
    {}


    // Member Functions

    //- Resize the ODE solver
    virtual bool resize();

    //- Solve the ODE system and the update the state
    virtual void solve
    (
        scalar& x,
        scalarField& y,
        stepState& step
    ) const;

};


} // End namespace CML


#endif
