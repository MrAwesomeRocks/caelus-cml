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

\*---------------------------------------------------------------------------*/

#include "ODESolver.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(ODESolver, 0);
    defineRunTimeSelectionTable(ODESolver, dictionary);
}


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

CML::scalar CML::ODESolver::normalizeError
(
    const scalarField& y0,
    const scalarField& y,
    const scalarField& err
) const
{
    // Calculate the maximum error
    scalar maxErr = 0.0;
    forAll(err, i)
    {
        scalar tol = absTol_[i] + relTol_[i]*max(mag(y0[i]), mag(y[i]));
        maxErr = max(maxErr, mag(err[i])/tol);
    }

    return maxErr;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::ODESolver::ODESolver(const ODESystem& ode, const dictionary& dict)
:
    odes_(ode),
    maxN_(ode.nEqns()),
    n_(ode.nEqns()),
    absTol_(n_, dict.lookupOrDefault<scalar>("absTol", SMALL)),
    relTol_(n_, dict.lookupOrDefault<scalar>("relTol", 1e-4)),
    maxSteps_(dict.lookupOrDefault<scalar>("maxSteps", 10000))
{}


CML::ODESolver::ODESolver
(
    const ODESystem& ode,
    const scalarField& absTol,
    const scalarField& relTol
)
:
    odes_(ode),
    maxN_(ode.nEqns()),
    n_(ode.nEqns()),
    absTol_(absTol),
    relTol_(relTol),
    maxSteps_(10000)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool CML::ODESolver::resize()
{
    if (odes_.nEqns() != n_)
    {
        if (odes_.nEqns() > maxN_)
        {
            FatalErrorInFunction
                << "Specified number of equations " << odes_.nEqns()
                << " greater than maximum " << maxN_
                << abort(FatalError);
        }

        n_ = odes_.nEqns();

        resizeField(absTol_);
        resizeField(relTol_);

        return true;
    }
    else
    {
        return false;
    }
}


void CML::ODESolver::solve
(
    scalar& x,
    scalarField& y,
    scalar& dxTry
) const
{
    stepState step(dxTry);
    solve(x, y, step);
    dxTry = step.dxTry;
}


void CML::ODESolver::solve
(
    scalar& x,
    scalarField& y,
    stepState& step
) const
{
    scalar x0 = x;
    solve(x, y, step.dxTry);
    step.dxDid = x - x0;
}


void CML::ODESolver::solve
(
    const scalar xStart,
    const scalar xEnd,
    scalarField& y,
    scalar& dxTry
) const
{
    stepState step(dxTry);
    scalar x = xStart;

    for (label nStep=0; nStep<maxSteps_; nStep++)
    {
        // Store previous iteration dxTry
        scalar dxTry0 = step.dxTry;

        step.reject = false;

        // Check if this is a truncated step and set dxTry to integrate to xEnd
        if ((x + step.dxTry - xEnd)*(x + step.dxTry - xStart) > 0)
        {
            step.last = true;
            step.dxTry = xEnd - x;
        }

        // Integrate as far as possible up to step.dxTry
        solve(x, y, step);

        // Check if reached xEnd
        if ((x - xEnd)*(xEnd - xStart) >= 0)
        {
            if (nStep > 0 && step.last)
            {
                step.dxTry = dxTry0;
            }

            dxTry = step.dxTry;

            return;
        }

        step.first = false;

        // If the step.dxTry was reject set step.prevReject
        if (step.reject)
        {
            step.prevReject = true;
        }
    }

    FatalErrorInFunction
        << "Integration steps greater than maximum " << maxSteps_ << nl
        << "    xStart = " << xStart << ", xEnd = " << xEnd
        << ", x = " << x << ", dxDid = " << step.dxDid << nl
        << "    y = " << y
        << exit(FatalError);
}
