/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2012 OpenFOAM Foundation
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
    CML::compressible::LESModels::oneEqEddy

Description
    One Equation Eddy Viscosity Model for compressible flows

    Eddy viscosity SGS model using a modeled balance equation to simulate the
    behaviour of k, hence,
    \verbatim
        d/dt(rho*k) + div(rho*U*k) - div(muEff*grad(k))
        =
        -rho*D:B - ce*rho*k^(3/2)/delta

    and

        B = 2/3*k*I - 2*nuSgs*dev(D)

    where

        D = symm(grad(U));
        nuSgs = ck*sqrt(k)*delta
        muSgs = rho*nuSgs
        muEff = muSgs + mu
    \endverbatim

SourceFiles
    compressibleOneEqEddy.cpp

\*---------------------------------------------------------------------------*/

#ifndef compressibleOneEqEddy_H
#define compressibleOneEqEddy_H

#include "compressibleGenEddyVisc.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace compressible
{
namespace LESModels
{

/*---------------------------------------------------------------------------*\
                           Class oneEqEddy Declaration
\*---------------------------------------------------------------------------*/

class oneEqEddy
:
    public GenEddyVisc
{
    // Private data

        volScalarField k_;

        dimensionedScalar ck_;


    // Private Member Functions

        //- Update sub-grid scale fields
        void updateSubGridScaleFields();

        // Disallow default bitwise copy construct and assignment
        oneEqEddy(const oneEqEddy&);
        oneEqEddy& operator=(const oneEqEddy&);


public:

    //- Runtime type information
    TypeName("oneEqEddy");


    // Constructors

        //- Constructor from components
        oneEqEddy
        (
            const volScalarField& rho,
            const volVectorField& U,
            const surfaceScalarField& phi,
            const fluidThermo& thermoPhysicalModel,
            const word& turbulenceModelName = turbulenceModel::typeName,
            const word& modelName = typeName
        );


    //- Destructor
    virtual ~oneEqEddy()
    {}


    // Member Functions

        //- Return SGS kinetic energy
        virtual tmp<volScalarField> k() const
        {
            return k_;
        }

        //- Return the effective diffusivity for k
        tmp<volScalarField> DkEff() const
        {
            return tmp<volScalarField>
            (
                new volScalarField("DkEff", muSgs_ + mu())
            );
        }

        //- Correct Eddy-Viscosity and related properties
        virtual void correct(const tmp<volTensorField>& gradU);

        //- Read LESProperties dictionary
        virtual bool read();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace LESModels
} // End namespace compressible
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
