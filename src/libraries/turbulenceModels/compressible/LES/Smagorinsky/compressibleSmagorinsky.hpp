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
    CML::compressible::LESModels::Smagorinsky

Description
    Smagorinsky Model for compressible flows.

    Algebraic eddy viscosity SGS model founded on the assumption that
    local equilibrium prevails.
    Thus,
    \verbatim

        B = 2/3*k*I - 2*nuSgs*dev(D)

    where

        D = symm(grad(U));
        k from rho*D:B + ce*rho*k^3/2/delta = 0
        muSgs = ck*rho*sqrt(k)*delta
    \endverbatim

SourceFiles
    compressibleSmagorinsky.cpp

\*---------------------------------------------------------------------------*/

#ifndef compressibleSmagorinsky_H
#define compressibleSmagorinsky_H

#include "compressibleGenEddyVisc.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace compressible
{
namespace LESModels
{

/*---------------------------------------------------------------------------*\
                           Class Smagorinsky Declaration
\*---------------------------------------------------------------------------*/

class Smagorinsky
:
    public GenEddyVisc
{
    // Private data

        dimensionedScalar ck_;


    // Private Member Functions

        //- Update sub-grid scale fields
        void updateSubGridScaleFields(const volTensorField& gradU);

        // Disallow default bitwise copy construct and assignment
        Smagorinsky(const Smagorinsky&);
        Smagorinsky& operator=(const Smagorinsky&);


public:

    //- Runtime type information
    TypeName("Smagorinsky");

    // Constructors

        //- Construct from components
        Smagorinsky
        (
            const volScalarField& rho,
            const volVectorField& U,
            const surfaceScalarField& phi,
            const fluidThermo& thermoPhysicalModel,
            const word& turbulenceModelName = turbulenceModel::typeName,
            const word& modelName = typeName
        );


    //- Destructor
    virtual ~Smagorinsky()
    {}


    // Member Functions

    //  calculated from the given velocity gradient
    tmp<volScalarField> k(const tmp<volTensorField>& gradU) const
    {
        volSymmTensorField D(symm(gradU));

        volScalarField a(ce_/delta());
        volScalarField b((2.0/3.0)*tr(D));
        volScalarField c(2*ck_*delta()*(dev(D) && D));

        return sqr((-b + sqrt(sqr(b) + 4*a*c))/(2*a));
    }

    //- Return SGS kinetic energy
    virtual tmp<volScalarField> k() const
    {
        return k(fvc::grad(U()));
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
