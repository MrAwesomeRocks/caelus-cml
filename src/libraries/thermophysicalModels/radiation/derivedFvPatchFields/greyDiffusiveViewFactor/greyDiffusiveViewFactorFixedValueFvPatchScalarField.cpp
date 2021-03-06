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

\*---------------------------------------------------------------------------*/

#include "greyDiffusiveViewFactorFixedValueFvPatchScalarField.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "fvPatchFieldMapper.hpp"
#include "volFields.hpp"


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::radiation::greyDiffusiveViewFactorFixedValueFvPatchScalarField::
greyDiffusiveViewFactorFixedValueFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(p, iF),
    radiationCoupledBase(patch(), "undefined", scalarField::null()),
    qro_(p.size(), 0.0)
{}


CML::radiation::greyDiffusiveViewFactorFixedValueFvPatchScalarField::
greyDiffusiveViewFactorFixedValueFvPatchScalarField
(
    const greyDiffusiveViewFactorFixedValueFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchScalarField(ptf, p, iF, mapper),
    radiationCoupledBase
    (
        patch(),
        ptf.emissivityMethod(),
        ptf.emissivity_
    ),
    qro_(ptf.qro_)
{}


CML::radiation::greyDiffusiveViewFactorFixedValueFvPatchScalarField::
greyDiffusiveViewFactorFixedValueFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchScalarField(p, iF),
    radiationCoupledBase(p, dict),
    qro_("qro", dict, p.size())
{
    if (dict.found("value"))
    {
        fvPatchScalarField::operator=
        (
            scalarField("value", dict, p.size())
        );

    }
    else
    {
         fvPatchScalarField::operator=(0.0);
    }
}


CML::radiation::greyDiffusiveViewFactorFixedValueFvPatchScalarField::
greyDiffusiveViewFactorFixedValueFvPatchScalarField
(
    const greyDiffusiveViewFactorFixedValueFvPatchScalarField& ptf
)
:
    fixedValueFvPatchScalarField(ptf),
    radiationCoupledBase
    (
        ptf.patch(),
        ptf.emissivityMethod(),
        ptf.emissivity_
    ),
    qro_(ptf.qro_)
{}


CML::radiation::greyDiffusiveViewFactorFixedValueFvPatchScalarField::
greyDiffusiveViewFactorFixedValueFvPatchScalarField
(
    const greyDiffusiveViewFactorFixedValueFvPatchScalarField& ptf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(ptf, iF),
    radiationCoupledBase
    (
        ptf.patch(),
        ptf.emissivityMethod(),
        ptf.emissivity_
    ),
    qro_(ptf.qro_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::radiation::greyDiffusiveViewFactorFixedValueFvPatchScalarField::
updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    if (debug)
    {
        scalar Q = gSum((*this)*patch().magSf());

        Info<< patch().boundaryMesh().mesh().name() << ':'
            << patch().name() << ':'
            << this->dimensionedInternalField().name() << " <- "
            << " heat transfer rate:" << Q
            << " wall radiative heat flux "
            << " min:" << gMin(*this)
            << " max:" << gMax(*this)
            << " avg:" << gAverage(*this)
            << endl;
    }

    fixedValueFvPatchScalarField::updateCoeffs();
}


void CML::radiation::greyDiffusiveViewFactorFixedValueFvPatchScalarField::
write
(
    Ostream& os
) const
{
    fixedValueFvPatchScalarField::write(os);
    radiationCoupledBase::write(os);
    qro_.writeEntry("qro", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace radiation
{
    makePatchTypeField
    (
        fvPatchScalarField,
        greyDiffusiveViewFactorFixedValueFvPatchScalarField
    );
}
}
