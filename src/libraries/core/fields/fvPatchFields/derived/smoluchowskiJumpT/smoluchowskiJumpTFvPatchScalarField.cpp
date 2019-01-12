/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2015 OpenFOAM Foundation
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

#include "smoluchowskiJumpTFvPatchScalarField.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "fvPatchFieldMapper.hpp"
#include "volFields.hpp"
#include "mathematicalConstants.hpp"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::smoluchowskiJumpTFvPatchScalarField::smoluchowskiJumpTFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    mixedFvPatchScalarField(p, iF),
    accommodationCoeff_(1.0),
    Twall_(p.size(), 0.0),
    gamma_(1.4)
{
    refValue() = 0.0;
    refGrad() = 0.0;
    valueFraction() = 0.0;
}


CML::smoluchowskiJumpTFvPatchScalarField::smoluchowskiJumpTFvPatchScalarField
(
    const smoluchowskiJumpTFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    mixedFvPatchScalarField(ptf, p, iF, mapper),
    accommodationCoeff_(ptf.accommodationCoeff_),
    Twall_(ptf.Twall_),
    gamma_(ptf.gamma_)
{}


CML::smoluchowskiJumpTFvPatchScalarField::smoluchowskiJumpTFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    mixedFvPatchScalarField(p, iF),
    accommodationCoeff_(readScalar(dict.lookup("accommodationCoeff"))),
    Twall_("Twall", dict, p.size()),
    gamma_(dict.lookupOrDefault<scalar>("gamma", 1.4))
{
    if
    (
        mag(accommodationCoeff_) < SMALL
     || mag(accommodationCoeff_) > 2.0
    )
    {
        FatalIOErrorInFunction(dict)
            << "unphysical accommodationCoeff specified"
            << "(0 < accommodationCoeff <= 1)" << endl
            << exit(FatalError);
    }

    if (dict.found("value"))
    {
        fvPatchField<scalar>::operator=
        (
            scalarField("value", dict, p.size())
        );
    }
    else
    {
        fvPatchField<scalar>::operator=(patchInternalField());
    }

    refValue() = *this;
    refGrad() = 0.0;
    valueFraction() = 0.0;
}


CML::smoluchowskiJumpTFvPatchScalarField::smoluchowskiJumpTFvPatchScalarField
(
    const smoluchowskiJumpTFvPatchScalarField& ptpsf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    mixedFvPatchScalarField(ptpsf, iF),
    accommodationCoeff_(ptpsf.accommodationCoeff_),
    Twall_(ptpsf.Twall_),
    gamma_(ptpsf.gamma_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// Map from self
void CML::smoluchowskiJumpTFvPatchScalarField::autoMap
(
    const fvPatchFieldMapper& m
)
{
    mixedFvPatchScalarField::autoMap(m);
}


// Reverse-map the given fvPatchField onto this fvPatchField
void CML::smoluchowskiJumpTFvPatchScalarField::rmap
(
    const fvPatchField<scalar>& ptf,
    const labelList& addr
)
{
    mixedFvPatchField<scalar>::rmap(ptf, addr);
}


// Update the coefficients associated with the patch field
void CML::smoluchowskiJumpTFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    const fvPatchScalarField& pmu =
        patch().lookupPatchField<volScalarField, scalar>("mu");
    const fvPatchScalarField& prho =
        patch().lookupPatchField<volScalarField, scalar>("rho");
    const fvPatchField<scalar>& ppsi =
        patch().lookupPatchField<volScalarField, scalar>("psi");
    const fvPatchVectorField& pU =
        patch().lookupPatchField<volVectorField, vector>("U");

    // Prandtl number reading consistent with rhoCentralCML
    const dictionary& thermophysicalProperties =
        db().lookupObject<IOdictionary>("thermophysicalProperties");

    dimensionedScalar Pr
    (
        dimensionedScalar::lookupOrDefault
        (
            "Pr",
            thermophysicalProperties,
            1.0
        )
    );

    Field<scalar> C2
    (
        pmu/prho
        *sqrt(ppsi*constant::mathematical::piByTwo)
        *2.0*gamma_/Pr.value()/(gamma_ + 1.0)
        *(2.0 - accommodationCoeff_)/accommodationCoeff_
    );

    Field<scalar> aCoeff(prho.snGrad() - prho/C2);
    Field<scalar> KEbyRho(0.5*magSqr(pU));

    valueFraction() = (1.0/(1.0 + patch().deltaCoeffs()*C2));
    refValue() = Twall_;
    refGrad() = 0.0;

    mixedFvPatchScalarField::updateCoeffs();
}


// Write
void CML::smoluchowskiJumpTFvPatchScalarField::write(Ostream& os) const
{
    fvPatchScalarField::write(os);
    os.writeKeyword("accommodationCoeff")
        << accommodationCoeff_ << token::END_STATEMENT << nl;
    Twall_.writeEntry("Twall", os);
    os.writeKeyword("gamma")
        << gamma_ << token::END_STATEMENT << nl;
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
    makePatchTypeField
    (
        fvPatchScalarField,
        smoluchowskiJumpTFvPatchScalarField
    );
}


// ************************************************************************* //
