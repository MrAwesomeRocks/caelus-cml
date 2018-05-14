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

#include "radiationModel.hpp"
#include "absorptionEmissionModel.hpp"
#include "scatterModel.hpp"
#include "fvmSup.hpp"
#include "basicThermo.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    namespace radiation
    {
        defineTypeNameAndDebug(radiationModel, 0);
        defineRunTimeSelectionTable(radiationModel, T);
        defineRunTimeSelectionTable(radiationModel, dictionary);
    }
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

CML::IOobject CML::radiation::radiationModel::createIOobject
(
    const fvMesh& mesh
) const
{
    IOobject io
    (
        "radiationProperties",
        mesh.time().constant(),
        mesh,
        IOobject::MUST_READ,
        IOobject::NO_WRITE
    );

    if (io.headerOk())
    {
        io.readOpt() = IOobject::MUST_READ_IF_MODIFIED;
        return io;
    }
    else
    {
        io.readOpt() = IOobject::NO_READ;
        return io;
    }
}


void CML::radiation::radiationModel::initialise()
{
    if (radiation_)
    {
        solverFreq_ = max(1, lookupOrDefault<label>("solverFreq", 1));

        absorptionEmission_.reset
        (
            absorptionEmissionModel::New(*this, mesh_).ptr()
        );

        scatter_.reset(scatterModel::New(*this, mesh_).ptr());
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::radiation::radiationModel::radiationModel(const volScalarField& T)
:
    IOdictionary
    (
        IOobject
        (
            "radiationProperties",
            T.time().constant(),
            T.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        )
    ),
    mesh_(T.mesh()),
    time_(T.time()),
    T_(T),
    radiation_(false),
    coeffs_(dictionary::null),
    solverFreq_(0),
    firstIter_(true),
    absorptionEmission_(NULL),
    scatter_(NULL)
{}


CML::radiation::radiationModel::radiationModel
(
    const word& type,
    const volScalarField& T
)
:
    IOdictionary(createIOobject(T.mesh())),
    mesh_(T.mesh()),
    time_(T.time()),
    T_(T),
    radiation_(lookupOrDefault("radiation", true)),
    coeffs_(subOrEmptyDict(type + "Coeffs")),
    solverFreq_(1),
    firstIter_(true),
    absorptionEmission_(NULL),
    scatter_(NULL)
{
    if (readOpt() == IOobject::NO_READ)
    {
        radiation_ = false;
    }

    initialise();
}


CML::radiation::radiationModel::radiationModel
(
    const word& type,
    const dictionary& dict,
    const volScalarField& T
)
:
    IOdictionary
    (
        IOobject
        (
            "radiationProperties",
            T.time().constant(),
            T.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        dict
    ),
    mesh_(T.mesh()),
    time_(T.time()),
    T_(T),
    radiation_(lookupOrDefault("radiation", true)),
    coeffs_(subOrEmptyDict(type + "Coeffs")),
    solverFreq_(1),
    firstIter_(true),
    absorptionEmission_(NULL),
    scatter_(NULL)
{
    initialise();
}


// * * * * * * * * * * * * * * * * Destructor    * * * * * * * * * * * * * * //

CML::radiation::radiationModel::~radiationModel()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool CML::radiation::radiationModel::read()
{
    if (regIOobject::read())
    {
        lookup("radiation") >> radiation_;
        coeffs_ = subOrEmptyDict(type() + "Coeffs");

        solverFreq_ = lookupOrDefault<label>("solverFreq", 1);
        solverFreq_ = max(1, solverFreq_);

        return true;
    }
    else
    {
        return false;
    }
}


void CML::radiation::radiationModel::correct()
{
    if (!radiation_)
    {
        return;
    }

    if (firstIter_ || (time_.timeIndex() % solverFreq_ == 0))
    {
        calculate();
        firstIter_ = false;
    }
}


CML::tmp<CML::fvScalarMatrix> CML::radiation::radiationModel::Sh
(
    basicThermo& thermo
) const
{
    volScalarField& h = thermo.h();
    const volScalarField Cp(thermo.Cp());
    const volScalarField T3(pow3(T_));

    return
    (
        Ru()
      - fvm::Sp(4.0*Rp()*T3/Cp, h)
      - Rp()*T3*(T_ - 4.0*h/Cp)
    );
}


CML::tmp<CML::fvScalarMatrix> CML::radiation::radiationModel::Shs
(
    basicThermo& thermo
) const
{
    volScalarField& hs = thermo.hs();
    const volScalarField Cp(thermo.Cp());
    const volScalarField T3(pow3(T_));

    return
    (
        Ru()
      - fvm::Sp(4.0*Rp()*T3/Cp, hs)
      - Rp()*T3*(T_ - 4.0*hs/Cp)
    );
}


CML::tmp<CML::fvScalarMatrix> CML::radiation::radiationModel::ST
(
    const dimensionedScalar& rhoCp,
    volScalarField& T
) const
{
    return
    (
        Ru()/rhoCp
      - fvm::Sp(Rp()*pow3(T)/rhoCp, T)
    );
}


const CML::radiation::absorptionEmissionModel&
CML::radiation::radiationModel::absorptionEmission() const
{
    if (!absorptionEmission_.valid())
    {
        FatalErrorIn("radiation::radiationModel::absorptionEmission() const")
            << "Requested radiation absorptionEmission model, but model is "
            << "not activate" << abort(FatalError);
    }

    return absorptionEmission_();
}


// ************************************************************************* //
