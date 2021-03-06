/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2012 OpenFOAM Foundation
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

#include "nutUTabWFFvPatchScalarField.hpp"
#include "incompressible/turbulenceModel/turbulenceModel.hpp"
#include "fvPatchFieldMapper.hpp"
#include "volFields.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace incompressible
{

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

tmp<scalarField> nutUTabulatedWallFunctionFvPatchScalarField::calcNut() const
{
    const label patchi = patch().index();

    const turbulenceModel& turbModel =
        db().lookupObject<turbulenceModel>("turbulenceModel");
    const scalarField& y = turbModel.y()[patchi];
    const fvPatchVectorField& Uw = turbModel.U().boundaryField()[patchi];
    const scalarField magUp(mag(Uw.patchInternalField() - Uw));
    const scalarField magGradU(mag(Uw.snGrad()));
    const tmp<volScalarField> tnu = turbModel.nu();
    const volScalarField& nu = tnu();
    const scalarField& nuw = nu.boundaryField()[patchi];

    return
        max
        (
            scalar(0),
            sqr(magUp/(calcUPlus(magUp*y/nuw) + ROOTVSMALL))
           /(magGradU + ROOTVSMALL)
          - nuw
        );
}


tmp<scalarField> nutUTabulatedWallFunctionFvPatchScalarField::calcUPlus
(
    const scalarField& Rey
) const
{
    tmp<scalarField> tuPlus(new scalarField(patch().size(), 0.0));
    scalarField& uPlus = tuPlus();

    forAll(uPlus, faceI)
    {
        uPlus[faceI] = uPlusTable_.interpolateLog10(Rey[faceI]);
    }

    return tuPlus;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

nutUTabulatedWallFunctionFvPatchScalarField::
nutUTabulatedWallFunctionFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    nutWallFunctionFvPatchScalarField(p, iF),
    uPlusTableName_("undefined-uPlusTableName"),
    uPlusTable_
    (
        IOobject
        (
            uPlusTableName_,
            patch().boundaryMesh().mesh().time().constant(),
            patch().boundaryMesh().mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            false
        ),
        false
    )
{}


nutUTabulatedWallFunctionFvPatchScalarField::
nutUTabulatedWallFunctionFvPatchScalarField
(
    const nutUTabulatedWallFunctionFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    nutWallFunctionFvPatchScalarField(ptf, p, iF, mapper),
    uPlusTableName_(ptf.uPlusTableName_),
    uPlusTable_(ptf.uPlusTable_)
{}


nutUTabulatedWallFunctionFvPatchScalarField::
nutUTabulatedWallFunctionFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    nutWallFunctionFvPatchScalarField(p, iF, dict),
    uPlusTableName_(dict.lookup("uPlusTable")),
    uPlusTable_
    (
        IOobject
        (
            uPlusTableName_,
            patch().boundaryMesh().mesh().time().constant(),
            patch().boundaryMesh().mesh(),
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE,
            false
        ),
        true
    )
{}


nutUTabulatedWallFunctionFvPatchScalarField::
nutUTabulatedWallFunctionFvPatchScalarField
(
    const nutUTabulatedWallFunctionFvPatchScalarField& wfpsf
)
:
    nutWallFunctionFvPatchScalarField(wfpsf),
    uPlusTableName_(wfpsf.uPlusTableName_),
    uPlusTable_(wfpsf.uPlusTable_)
{}


nutUTabulatedWallFunctionFvPatchScalarField::
nutUTabulatedWallFunctionFvPatchScalarField
(
    const nutUTabulatedWallFunctionFvPatchScalarField& wfpsf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    nutWallFunctionFvPatchScalarField(wfpsf, iF),
    uPlusTableName_(wfpsf.uPlusTableName_),
    uPlusTable_(wfpsf.uPlusTable_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

tmp<scalarField> nutUTabulatedWallFunctionFvPatchScalarField::yPlus() const
{
    const label patchi = patch().index();

    const turbulenceModel& turbModel =
        db().lookupObject<turbulenceModel>("turbulenceModel");
    const scalarField& y = turbModel.y()[patchi];
    const fvPatchVectorField& Uw = turbModel.U().boundaryField()[patchi];
    const scalarField magUp(mag(Uw.patchInternalField() - Uw));
    const tmp<volScalarField> tnu = turbModel.nu();
    const volScalarField& nu = tnu();
    const scalarField& nuw = nu.boundaryField()[patchi];
    const scalarField Rey(magUp*y/nuw);

    return Rey/(calcUPlus(Rey) + ROOTVSMALL);
}


void nutUTabulatedWallFunctionFvPatchScalarField::write(Ostream& os) const
{
    fvPatchField<scalar>::write(os);
    os.writeKeyword("uPlusTable") << uPlusTableName_
        << token::END_STATEMENT << nl;
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField
(
    fvPatchScalarField,
    nutUTabulatedWallFunctionFvPatchScalarField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace incompressible
} // End namespace CML

// ************************************************************************* //
