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
    CML::EulerImplicit

Description
    An Euler implicit solver for chemistry


\*---------------------------------------------------------------------------*/

#ifndef EulerImplicit_HPP
#define EulerImplicit_HPP

#include "chemistrySolver.hpp"
#include "Switch.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

template <class Type>
class simpleMatrix;

/*---------------------------------------------------------------------------*\
                       Class EulerImplicit Declaration
\*---------------------------------------------------------------------------*/

template<class ChemistryModel>
class EulerImplicit
:
    public chemistrySolver<ChemistryModel>
{

    //- Coefficients dictionary
    dictionary coeffsDict_;


    // Model constants

    //- Chemistry timescale
    scalar cTauChem_;

    //- Equilibrium rate limiter flag (on/off)
    Switch eqRateLimiter_;

    // Solver data
    mutable scalarField cTp_;


public:

    //- Runtime type information
    TypeName("EulerImplicit");


    //- Construct from thermo
    EulerImplicit(typename ChemistryModel::reactionThermo& thermo);


    //- Destructor
    virtual ~EulerImplicit()
    {}


    // Member Functions

    void updateRRInReactionI
    (
        const label index,
        const scalar pr,
        const scalar pf,
        const scalar corr,
        const label lRef,
        const label rRef,
        const scalar p,
        const scalar T,
        simpleMatrix<scalar>& RR
    ) const;

    //- Update the concentrations and return the chemical time
    virtual void solve
    (
        scalarField& c,
        scalar& T,
        scalar& p,
        scalar& deltaT,
        scalar& subDeltaT
    ) const;

};


} // End namespace CML

#include "addToRunTimeSelectionTable.hpp"
#include "simpleMatrix.hpp"
#include "Reaction.hpp"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class ChemistryModel>
CML::EulerImplicit<ChemistryModel>::EulerImplicit
(
    typename ChemistryModel::reactionThermo& thermo
)
:
    chemistrySolver<ChemistryModel>(thermo),
    coeffsDict_(this->subDict("EulerImplicitCoeffs")),
    cTauChem_(readScalar(coeffsDict_.lookup("cTauChem"))),
    eqRateLimiter_(coeffsDict_.lookup("equilibriumRateLimiter")),
    cTp_(this->nEqns())
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class ChemistryModel>
void CML::EulerImplicit<ChemistryModel>::updateRRInReactionI
(
    const label index,
    const scalar pr,
    const scalar pf,
    const scalar corr,
    const label lRef,
    const label rRef,
    const scalar p,
    const scalar T,
    simpleMatrix<scalar>& RR
) const
{
    const Reaction<typename ChemistryModel::thermoType>& R =
        this->reactions_[index];

    forAll(R.lhs(), s)
    {
        const label si = R.lhs()[s].index;
        const scalar sl = R.lhs()[s].stoichCoeff;
        RR[si][rRef] -= sl*pr*corr;
        RR[si][lRef] += sl*pf*corr;
    }

    forAll(R.rhs(), s)
    {
        const label si = R.rhs()[s].index;
        const scalar sr = R.rhs()[s].stoichCoeff;
        RR[si][lRef] -= sr*pf*corr;
        RR[si][rRef] += sr*pr*corr;
    }
}


template<class ChemistryModel>
void CML::EulerImplicit<ChemistryModel>::solve
(
    scalarField& c,
    scalar& T,
    scalar& p,
    scalar& deltaT,
    scalar& subDeltaT
) const
{
    const label nSpecie = this->nSpecie();
    simpleMatrix<scalar> RR(nSpecie, 0, 0);

    for (label i=0; i<nSpecie; i++)
    {
        c[i] = max(0, c[i]);
    }

    // Calculate the absolute enthalpy
    const scalar cTot = sum(c);
    typename ChemistryModel::thermoType mixture
    (
        (this->specieThermo_[0].W()*c[0])*this->specieThermo_[0]
    );
    for (label i=1; i<nSpecie; i++)
    {
        mixture += (this->specieThermo_[i].W()*c[i])*this->specieThermo_[i];
    }
    const scalar ha = mixture.Ha(p, T);
    const scalar deltaTEst = min(deltaT, subDeltaT);

    forAll(this->reactions(), i)
    {
        scalar pf, cf, pr, cr;
        label lRef, rRef;

        const scalar omegai =
            this->omegaI(i, c, T, p, pf, cf, lRef, pr, cr, rRef);

        scalar corr = 1;
        if (eqRateLimiter_)
        {
            if (omegai < 0)
            {
                corr = 1/(1 + pr*deltaTEst);
            }
            else
            {
                corr = 1/(1 + pf*deltaTEst);
            }
        }

        updateRRInReactionI(i, pr, pf, corr, lRef, rRef, p, T, RR);
    }

    // Calculate the stable/accurate time-step
    scalar tMin = GREAT;

    for (label i=0; i<nSpecie; i++)
    {
        scalar d = 0;
        for (label j=0; j<nSpecie; j++)
        {
            d -= RR(i, j)*c[j];
        }

        if (d < -SMALL)
        {
            tMin = min(tMin, -(c[i] + SMALL)/d);
        }
        else
        {
            d = max(d, SMALL);
            const scalar cm = max(cTot - c[i], 1e-5);
            tMin = min(tMin, cm/d);
        }
    }

    subDeltaT = cTauChem_*tMin;
    deltaT = min(deltaT, subDeltaT);

    // Add the diagonal and source contributions from the time-derivative
    for (label i=0; i<nSpecie; i++)
    {
        RR(i, i) += 1/deltaT;
        RR.source()[i] = c[i]/deltaT;
    }

    // Solve for the new composition
    c = RR.LUsolve();

    // Limit the composition
    for (label i=0; i<nSpecie; i++)
    {
        c[i] = max(0, c[i]);
    }

    // Update the temperature
    mixture = (this->specieThermo_[0].W()*c[0])*this->specieThermo_[0];
    for (label i=1; i<nSpecie; i++)
    {
        mixture += (this->specieThermo_[i].W()*c[i])*this->specieThermo_[i];
    }
    T = mixture.THa(ha, p, T);
}


#endif
