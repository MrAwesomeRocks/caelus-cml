/*---------------------------------------------------------------------------*\
Copyright (C) 2011 OpenFOAM Foundation
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
    CML::compressible::RASModels::kEpsilon

Description
    Standard k-epsilon turbulence model for compressible flows.

    Reference:
    \verbatim
        Launder, B.E., and Spalding, D.B.,
        "Mathematical Models of Turbulence",
        Academic Press, 1972.

        Launder, B.E., and Spalding, D.B.,
        "Computational Methods for Turbulent Flows",
        Comp. Meth in Appl Mech & Eng'g, Vol 3, 1974, pp 269-289.
    \endverbatim

    The default model coefficients correspond to the following:
    \verbatim
        kEpsilonCoeffs
        {
            Cmu         0.09;
            C1          1.44;
            C2          1.92;
            sigmak      1.0; 
            sigmaEps    1.3;
            Prt         1.0;    // only for compressible
        }
    \endverbatim

SourceFiles
    kEpsilon.cpp

\*---------------------------------------------------------------------------*/

#ifndef compressiblekEpsilon_H
#define compressiblekEpsilon_H

#include "compressibleRASModel.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace compressible
{
namespace RASModels
{

/*---------------------------------------------------------------------------*\
                        Class kEpsilon Declaration
\*---------------------------------------------------------------------------*/

class kEpsilon
:
    public RASModel
{

protected:

    // Protected data

        // Model coefficients

            dimensionedScalar Cmu_;
            dimensionedScalar C1_;
            dimensionedScalar C2_;
            dimensionedScalar sigmak_;
            dimensionedScalar sigmaEps_;
            dimensionedScalar Prt_;

        // Fields

            volScalarField k_;
            volScalarField epsilon_;
            volScalarField mut_;
            volScalarField alphat_;


public:

    //- Runtime type information
    TypeName("kEpsilon");

    // Constructors

        //- Construct from components
        kEpsilon
        (
            const volScalarField& rho,
            const volVectorField& U,
            const surfaceScalarField& phi,
            const fluidThermo& thermophysicalModel,
            const word& turbulenceModelName = turbulenceModel::typeName,
            const word& modelName = typeName
        );


    //- Destructor
    virtual ~kEpsilon()
    {}


    // Member Functions

        //- Return the effective diffusivity for k
        tmp<volScalarField> DkEff() const
        {
            return tmp<volScalarField>
            (
                new volScalarField("DkEff", mut_/sigmak_ + mu())
            );
        }

        //- Return the effective diffusivity for epsilon
        tmp<volScalarField> DepsilonEff() const
        {
            return tmp<volScalarField>
            (
                new volScalarField("DepsilonEff", mut_/sigmaEps_ + mu())
            );
        }

        //- Return the turbulence viscosity
        virtual tmp<volScalarField> mut() const
        {
            return mut_;
        }

        //- Return the turbulence thermal diffusivity
        virtual tmp<volScalarField> alphat() const
        {
            return alphat_;
        }

        //- Return the turbulence kinetic energy
        virtual tmp<volScalarField> k() const
        {
            return k_;
        }

        //- Return the turbulence kinetic energy dissipation rate
        virtual tmp<volScalarField> epsilon() const
        {
            return epsilon_;
        }

        //- Return the Reynolds stress tensor
        virtual tmp<volSymmTensorField> R() const;

        //- Return the effective stress tensor including the laminar stress
        virtual tmp<volSymmTensorField> devRhoReff() const;

        //- Return the source term for the momentum equation
        virtual tmp<fvVectorMatrix> divDevRhoReff(volVectorField& U) const;

        //- Solve the turbulence equations and correct the turbulence viscosity
        virtual void correct();

        //- Read RASProperties dictionary
        virtual bool read();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace RASModels
} // End namespace compressible
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
