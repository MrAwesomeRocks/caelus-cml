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

#include "externalWallHeatFluxTemperatureFvPatchScalarField.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "fvPatchFieldMapper.hpp"
#include "volFields.hpp"
#include "mappedPatchBase.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

template<>
const char*
NamedEnum
<externalWallHeatFluxTemperatureFvPatchScalarField::operationMode, 3>::names[]=
{
    "fixed_heat_flux",
    "fixed_heat_transfer_coefficient",
    "unknown"
};

const NamedEnum
<
    externalWallHeatFluxTemperatureFvPatchScalarField::operationMode, 3
>
externalWallHeatFluxTemperatureFvPatchScalarField::operationModeNames;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::externalWallHeatFluxTemperatureFvPatchScalarField::
externalWallHeatFluxTemperatureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    mixedFvPatchScalarField(p, iF),
    temperatureCoupledBase(patch(), "undefined", "undefined-K"),
    oldMode_(unknown),
    q_(p.size(), 0.0),
    h_(p.size(), 0.0),
    Ta_(p.size(), 0.0)
{
    this->refValue() = 0.0;
    this->refGrad() = 0.0;
    this->valueFraction() = 1.0;
}


CML::externalWallHeatFluxTemperatureFvPatchScalarField::
externalWallHeatFluxTemperatureFvPatchScalarField
(
    const externalWallHeatFluxTemperatureFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    mixedFvPatchScalarField(ptf, p, iF, mapper),
    temperatureCoupledBase(patch(), ptf.KMethod(), ptf.KName()),
    oldMode_(ptf.oldMode_),
    q_(ptf.q_, mapper),
    h_(ptf.h_, mapper),
    Ta_(ptf.Ta_, mapper)
{}


CML::externalWallHeatFluxTemperatureFvPatchScalarField::
externalWallHeatFluxTemperatureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    mixedFvPatchScalarField(p, iF),
    temperatureCoupledBase(patch(), dict),
    oldMode_(unknown),
    q_(p.size(), 0.0),
    h_(p.size(), 0.0),
    Ta_(p.size(), 0.0)
{
    if (dict.found("q") && !dict.found("h") && !dict.found("Ta"))
    {
        oldMode_ = fixedHeatFlux;
        q_ = scalarField("q", dict, p.size());
    }
    else if (dict.found("h") && dict.found("Ta") && !dict.found("q"))
    {
        oldMode_ = fixedHeatTransferCoeff;
        h_ = scalarField("h", dict, p.size());
        Ta_ = scalarField("Ta", dict, p.size());
    }
    else
    {
        FatalErrorIn
        (
            "externalWallHeatFluxTemperatureFvPatchScalarField::"
            "externalWallHeatFluxTemperatureFvPatchScalarField\n"
            "(\n"
            "    const fvPatch& p,\n"
            "    const DimensionedField<scalar, volMesh>& iF,\n"
            "    const dictionary& dict\n"
            ")\n"
        )   << "\n patch type '" << p.type()
            << "' either q or h and Ta were not found '"
            << "\n for patch " << p.name()
            << " of field " << dimensionedInternalField().name()
            << " in file " << dimensionedInternalField().objectPath()
            << exit(FatalError);
    }

    fvPatchScalarField::operator=(scalarField("value", dict, p.size()));

    if (dict.found("refValue"))
    {
        // Full restart
        refValue() = scalarField("refValue", dict, p.size());
        refGrad() = scalarField("refGradient", dict, p.size());
        valueFraction() = scalarField("valueFraction", dict, p.size());
    }
    else
    {
        // Start from user entered data. Assume fixedValue.
        refValue() = *this;
        refGrad() = 0.0;
        valueFraction() = 1.0;
    }
}


CML::externalWallHeatFluxTemperatureFvPatchScalarField::
externalWallHeatFluxTemperatureFvPatchScalarField
(
    const externalWallHeatFluxTemperatureFvPatchScalarField& tppsf
)
:
    mixedFvPatchScalarField(tppsf),
    temperatureCoupledBase(tppsf),
    oldMode_(tppsf.oldMode_),
    q_(tppsf.q_),
    h_(tppsf.h_),
    Ta_(tppsf.Ta_)
{}


CML::externalWallHeatFluxTemperatureFvPatchScalarField::
externalWallHeatFluxTemperatureFvPatchScalarField
(
    const externalWallHeatFluxTemperatureFvPatchScalarField& tppsf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    mixedFvPatchScalarField(tppsf, iF),
    temperatureCoupledBase(patch(), tppsf.KMethod(), tppsf.KName()),
    oldMode_(tppsf.oldMode_),
    q_(tppsf.q_),
    h_(tppsf.h_),
    Ta_(tppsf.Ta_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::externalWallHeatFluxTemperatureFvPatchScalarField::autoMap
(
    const fvPatchFieldMapper& m
)
{
    mixedFvPatchScalarField::autoMap(m);
    q_.autoMap(m);
    h_.autoMap(m);
    Ta_.autoMap(m);
}


void CML::externalWallHeatFluxTemperatureFvPatchScalarField::rmap
(
    const fvPatchScalarField& ptf,
    const labelList& addr
)
{
    mixedFvPatchScalarField::rmap(ptf, addr);

    const externalWallHeatFluxTemperatureFvPatchScalarField& tiptf =
        refCast<const externalWallHeatFluxTemperatureFvPatchScalarField>(ptf);

    q_.rmap(tiptf.q_, addr);
    h_.rmap(tiptf.h_, addr);
    Ta_.rmap(tiptf.Ta_, addr);
}


void CML::externalWallHeatFluxTemperatureFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    scalarField q(size(), 0.0);
    scalarField KWall(K(*this));
    scalarField KDelta(KWall*patch().deltaCoeffs());
    scalarField Tc(patchInternalField());

    if (oldMode_ == fixedHeatFlux)
    {
        q = q_;
    }
    else if (oldMode_ == fixedHeatTransferCoeff)
    {
        q = (Ta_ - Tc)*h_;
    }
    else
    {
        FatalErrorIn
        (
            "externalWallHeatFluxTemperatureFvPatchScalarField"
            "::updateCoeffs()"
        )   << "Illegal heat flux mode " << operationModeNames[oldMode_]
            << exit(FatalError);
    }

    forAll (*this, i)
    {
        if (q[i] > 0) //in
        {
            this->refGrad()[i] = q[i]/KWall[i];
            this->refValue()[i] = 0.0;
            this->valueFraction()[i] = 0.0;
        }
        else //out
        {
            this->refGrad()[i] = 0.0;
            this->refValue()[i] = q[i]/KDelta[i] + Tc[i];
            this->valueFraction()[i] = 1.0;
        }
    }

    mixedFvPatchScalarField::updateCoeffs();

    if (debug)
    {
        scalar Q = gSum(KWall*patch().magSf()*snGrad());

        Info<< patch().boundaryMesh().mesh().name() << ':'
            << patch().name() << ':'
            << this->dimensionedInternalField().name() << " :"
            << " heat transfer rate:" << Q
            << " walltemperature "
            << " min:" << gMin(*this)
            << " max:" << gMax(*this)
            << " avg:" << gAverage(*this)
            << endl;
    }
}


void CML::externalWallHeatFluxTemperatureFvPatchScalarField::write
(
    Ostream& os
) const
{
    mixedFvPatchScalarField::write(os);
    temperatureCoupledBase::write(os);

    switch (oldMode_)
    {
        case fixedHeatFlux:
        {
            q_.writeEntry("q", os);
            break;
        }
        case fixedHeatTransferCoeff:
        {
            h_.writeEntry("h", os);
            Ta_.writeEntry("Ta", os);
            break;
        }
        default:
        {
            FatalErrorIn
            (
                "void externalWallHeatFluxTemperatureFvPatchScalarField::write"
                "("
                    "Ostream& os"
                ") const"
            )   << "Illegal heat flux mode " << operationModeNames[oldMode_]
                << abort(FatalError);
        }
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
    makePatchTypeField
    (
        fvPatchScalarField,
        externalWallHeatFluxTemperatureFvPatchScalarField
    );
}

// ************************************************************************* //
