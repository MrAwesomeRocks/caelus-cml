/*---------------------------------------------------------------------------*\
Copyright Niels Gjøl Jacobsen, Technical University of Denmark.
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

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

#include "wavePressureFvPatchScalarField.hpp"
#include "addToRunTimeSelectionTable.hpp"

#include "Time.hpp"
#include "dimensionedVector.hpp"
#include "volMesh.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

wavePressureFvPatchScalarField::wavePressureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    mixedFvPatchField<scalar>(p, iF),
    convexPolyhedral(this->dimensionedInternalField().mesh(), true),
    waveProps_
    (
        waveTheories::waveTheory::New
        (
            this->patch().name(),
            this->dimensionedInternalField().mesh()
        )
    )
{
    this->refValue() = pTraits<scalar>::zero;
    this->refGrad() = pTraits<scalar>::zero;
    this->valueFraction() = 0.0;
}


wavePressureFvPatchScalarField::wavePressureFvPatchScalarField
(
    const wavePressureFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    mixedFvPatchField<scalar>(ptf, p, iF, mapper),
    convexPolyhedral(this->dimensionedInternalField().mesh(), true),
    waveProps_(ptf.waveProps_)
{
}


wavePressureFvPatchScalarField::wavePressureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    mixedFvPatchField<scalar>(p, iF),
    convexPolyhedral(this->dimensionedInternalField().mesh(), true),
    waveProps_
    (
        waveTheories::waveTheory::New
        (
            this->patch().name(),
            this->dimensionedInternalField().mesh()
        )
    )
{
    evaluate();
}


wavePressureFvPatchScalarField::wavePressureFvPatchScalarField
(
    const wavePressureFvPatchScalarField& ptf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    mixedFvPatchField<scalar>(ptf, iF),
    convexPolyhedral(this->dimensionedInternalField().mesh(), true),
    waveProps_
    (
        waveTheories::waveTheory::New
        (
            this->patch().name(),
            this->dimensionedInternalField().mesh()
        )
    )
{
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void wavePressureFvPatchScalarField::signedPointToSurfaceDistance
(
    const pointField& pp,
    scalarField& sd
)
{
    forAll (pp, pointi)
    {
        sd[pointi] = signedPointToSurfaceDistance(pp[pointi]);
    }
}


scalar wavePressureFvPatchScalarField::signedPointToSurfaceDistance
(
    const point& pp
) const
{
    scalar temp = waveProps_->eta(pp, db().time().value());
    temp += (waveProps_->returnDir() & pp);
    temp *= -1.0;

    return temp;
}


// Update the coefficients associated with the patch field
void wavePressureFvPatchScalarField::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    const fvMesh& mesh = this->dimensionedInternalField().mesh();
    const word patchName = this->patch().name();
    const label patchID = mesh.boundaryMesh().findPatchID(patchName);
    const scalarField& magSf(mesh.magSf().boundaryField()[patchID]);
    const vectorField& Sf (mesh.Sf().boundaryField()[patchID]);

    const label start = patch().patch().start();

    vector centre( vector::zero );
    vector normal( vector::zero );

    forAll (magSf, facei)
    {
        localFace lf = this->divideFace(facei + start);

        if (lf.isNegFace())
        {
            centre = lf.negCentre();
            normal = Sf[facei]/magSf[facei];
            this->refGrad()[facei]
                = waveProps_->ddxPd(centre, db().time().value(), normal);
        }
        else
        {
            this->refGrad()[facei] = 0.0;
        }

        this->refValue()[facei] = 0.0;
        this->valueFraction()[facei] = 1.0;
    }

    mixedFvPatchField<scalar>::updateCoeffs();
}


// Evaluate the field on the patch
void wavePressureFvPatchScalarField::evaluate()
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }

    Field<scalar>::operator=
    (
        this->valueFraction()*this->refValue()
      +
        (1.0 - this->valueFraction())*
        (
            this->patchInternalField()
          + this->refGrad()/this->patch().deltaCoeffs()
        )
    );

    fvPatchField<scalar>::evaluate();
}


// Write
void wavePressureFvPatchScalarField::write(Ostream& os) const
{
    fvPatchField<scalar>::write(os);
    this->refValue().writeEntry("refValue", os);
    this->refGrad().writeEntry("refGradient", os);
    this->valueFraction().writeEntry("valueFraction", os);
    this->writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField
(
    fvPatchScalarField,
    wavePressureFvPatchScalarField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
