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

#include "pressureInletOutletVelocityFvPatchVectorField.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "volFields.hpp"
#include "surfaceFields.hpp"


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::pressureInletOutletVelocityFvPatchVectorField::
pressureInletOutletVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    directionMixedFvPatchVectorField(p, iF),
    phiName_("phi")
{
    refValue() = vector::zero;
    refGrad() = vector::zero;
    valueFraction() = symmTensor::zero;
}


CML::pressureInletOutletVelocityFvPatchVectorField::
pressureInletOutletVelocityFvPatchVectorField
(
    const pressureInletOutletVelocityFvPatchVectorField& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    directionMixedFvPatchVectorField(ptf, p, iF, mapper),
    phiName_(ptf.phiName_)
{
    if (ptf.tangentialVelocity_.size())
    {
        tangentialVelocity_ = mapper(ptf.tangentialVelocity_);
    }
}


CML::pressureInletOutletVelocityFvPatchVectorField::
pressureInletOutletVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    directionMixedFvPatchVectorField(p, iF),
    phiName_(dict.lookupOrDefault<word>("phi", "phi"))
{
    fvPatchVectorField::operator=(vectorField("value", dict, p.size()));

    if (dict.found("tangentialVelocity"))
    {
        setTangentialVelocity
        (
            vectorField("tangentialVelocity", dict, p.size())
        );
    }
    else
    {
        refValue() = vector::zero;
    }

    refGrad() = vector::zero;
    valueFraction() = symmTensor::zero;
}


CML::pressureInletOutletVelocityFvPatchVectorField::
pressureInletOutletVelocityFvPatchVectorField
(
    const pressureInletOutletVelocityFvPatchVectorField& pivpvf
)
:
    directionMixedFvPatchVectorField(pivpvf),
    phiName_(pivpvf.phiName_),
    tangentialVelocity_(pivpvf.tangentialVelocity_)
{}


CML::pressureInletOutletVelocityFvPatchVectorField::
pressureInletOutletVelocityFvPatchVectorField
(
    const pressureInletOutletVelocityFvPatchVectorField& pivpvf,
    const DimensionedField<vector, volMesh>& iF
)
:
    directionMixedFvPatchVectorField(pivpvf, iF),
    phiName_(pivpvf.phiName_),
    tangentialVelocity_(pivpvf.tangentialVelocity_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::pressureInletOutletVelocityFvPatchVectorField::
setTangentialVelocity(const vectorField& tangentialVelocity)
{
    tangentialVelocity_ = tangentialVelocity;
    const vectorField n(patch().nf());
    refValue() = tangentialVelocity_ - n*(n & tangentialVelocity_);
}


void CML::pressureInletOutletVelocityFvPatchVectorField::autoMap
(
    const fvPatchFieldMapper& m
)
{
    directionMixedFvPatchVectorField::autoMap(m);
    if (tangentialVelocity_.size())
    {
        tangentialVelocity_.autoMap(m);
    }
}


void CML::pressureInletOutletVelocityFvPatchVectorField::rmap
(
    const fvPatchVectorField& ptf,
    const labelList& addr
)
{
    directionMixedFvPatchVectorField::rmap(ptf, addr);

    if (tangentialVelocity_.size())
    {
        const pressureInletOutletVelocityFvPatchVectorField& tiptf =
            refCast<const pressureInletOutletVelocityFvPatchVectorField>(ptf);

        tangentialVelocity_.rmap(tiptf.tangentialVelocity_, addr);
    }
}


void CML::pressureInletOutletVelocityFvPatchVectorField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    const fvsPatchField<scalar>& phip =
        patch().lookupPatchField<surfaceScalarField, scalar>(phiName_);

    valueFraction() = neg(phip)*(I - sqr(patch().nf()));

    directionMixedFvPatchVectorField::updateCoeffs();
    directionMixedFvPatchVectorField::evaluate();
}


void CML::pressureInletOutletVelocityFvPatchVectorField::write
(
    Ostream& os
)
const
{
    fvPatchVectorField::write(os);
    writeEntryIfDifferent<word>(os, "phi", "phi", phiName_);
    if (tangentialVelocity_.size())
    {
        tangentialVelocity_.writeEntry("tangentialVelocity", os);
    }
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

void CML::pressureInletOutletVelocityFvPatchVectorField::operator=
(
    const fvPatchField<vector>& pvf
)
{
    tmp<vectorField> normalValue = transform(valueFraction(), refValue());
    tmp<vectorField> transformGradValue = transform(I - valueFraction(), pvf);
    fvPatchField<vector>::operator=(normalValue + transformGradValue);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
    makePatchTypeField
    (
        fvPatchVectorField,
        pressureInletOutletVelocityFvPatchVectorField
    );
}

// ************************************************************************* //
