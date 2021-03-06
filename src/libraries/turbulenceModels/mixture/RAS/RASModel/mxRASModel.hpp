/*---------------------------------------------------------------------------*\
Copyright (C) 2011 OpenFOAM Foundation
Copyright (C) 2019 Applied CCM Pty Ltd
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

Namespace
    CML::mixture::RASModels

Description
    Namespace for compressible RAS turbulence models.

Class
    CML::mixture::RASModel

Description
    Abstract base class for turbulence models for multiphase mixture flows.

SourceFiles
    mxRASModel.cpp

\*---------------------------------------------------------------------------*/

#ifndef mxRASModel_HPP
#define mxRASModel_HPP

#include "mixture/turbulenceModel/mxTurbulenceModel.hpp"
#include "volFields.hpp"
#include "surfaceFields.hpp"
#include "fvm.hpp"
#include "fvc.hpp"
#include "fvMatrices.hpp"
#include "IOdictionary.hpp"
#include "Switch.hpp"
#include "bound.hpp"
#include "autoPtr.hpp"
#include "runTimeSelectionTables.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace mixture
{

/*---------------------------------------------------------------------------*\
                           Class RASModel Declaration
\*---------------------------------------------------------------------------*/

class RASModel
:
    public turbulenceModel,
    public IOdictionary
{

protected:

    //- Turbulence on/off flag
    Switch turbulence_;

    //- Flag to print the model coeffs at run-time
    Switch printCoeffs_;

    //- Model coefficients dictionary
    dictionary coeffDict_;

    //- Lower limit of k
    dimensionedScalar kMin_;

    //- Lower limit of epsilon
    dimensionedScalar epsilonMin_;

    //- Lower limit for omega
    dimensionedScalar omegaMin_;


    //- Print model coefficients
    virtual void printCoeffs();


private:

    //- Disallow default bitwise copy construct
    RASModel(const RASModel&);

    //- Disallow default bitwise assignment
    void operator=(const RASModel&);


public:

    //- Runtime type information
    TypeName("RASModel");


    // Declare run-time constructor selection table
    declareRunTimeSelectionTable
    (
        autoPtr,
        RASModel,
        dictionary,
        (
            const volScalarField& rho,
            const volVectorField& U,
            const surfaceScalarField& phi,
            const twoPhaseMixture& transport,
            const word& turbulenceModelName
        ),
        (rho, U, phi, transport, turbulenceModelName)
    );


    //- Construct from components
    RASModel
    (
        const word& type,
        const volScalarField& rho,
        const volVectorField& U,
        const surfaceScalarField& phi,
        const twoPhaseMixture& transport,
        const word& turbulenceModelName = turbulenceModel::typeName
    );


    //- Return a reference to the selected RAS model
    static autoPtr<RASModel> New
    (
        const volScalarField& rho,
        const volVectorField& U,
        const surfaceScalarField& phi,
        const twoPhaseMixture& transport,
        const word& turbulenceModelName = turbulenceModel::typeName
    );


    //- Destructor
    virtual ~RASModel()
    {}


    // Member Functions

    //- Return the lower allowable limit for k (default: SMALL)
    const dimensionedScalar& kMin() const
    {
        return kMin_;
    }

    //- Return the lower allowable limit for epsilon (default: SMALL)
    const dimensionedScalar& epsilonMin() const
    {
        return epsilonMin_;
    }

    //- Return the lower allowable limit for omega (default: SMALL)
    const dimensionedScalar& omegaMin() const
    {
        return omegaMin_;
    }

    //- Allow kMin to be changed
    dimensionedScalar& kMin()
    {
        return kMin_;
    }

    //- Allow epsilonMin to be changed
    dimensionedScalar& epsilonMin()
    {
        return epsilonMin_;
    }

    //- Allow omegaMin to be changed
    dimensionedScalar& omegaMin()
    {
        return omegaMin_;
    }

    //- Const access to the coefficients dictionary
    virtual const dictionary& coeffDict() const
    {
        return coeffDict_;
    }

    //- Return the effective viscosity
    virtual tmp<volScalarField> muEff() const
    {
        return tmp<volScalarField>
        (
            new volScalarField("muEff", mut() + mu())
        );
    }

    //- Solve the turbulence equations and correct the turbulence viscosity
    virtual void correct();

    //- Read RASProperties dictionary
    virtual bool read();
};


} // End namespace mixture
} // End namespace CML


#endif
