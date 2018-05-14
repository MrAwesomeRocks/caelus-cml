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

#include "mutkWFFvPatchScalarField.hpp"
#include "compressible/turbulenceModel/turbulenceModel.hpp"
#include "fvPatchFieldMapper.hpp"
#include "volFields.hpp"
#include "wallFvPatch.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace compressible
{

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void mutkWallFunctionFvPatchScalarField::checkType()
{
    if (!isA<wallFvPatch>(patch()))
    {
        FatalErrorIn("mutkWallFunctionFvPatchScalarField::checkType()")
            << "Invalid wall function specification" << nl
            << "    Patch type for patch " << patch().name()
            << " must be wall" << nl
            << "    Current patch type is " << patch().type() << nl << endl
            << abort(FatalError);
    }
}


scalar mutkWallFunctionFvPatchScalarField::calcYPlusLam
(
    const scalar kappa,
    const scalar E
) const
{
    scalar ypl = 11.0;

    for (int i=0; i<10; i++)
    {
        ypl = log(E*ypl)/kappa;
    }

    return ypl;
}


tmp<scalarField> mutkWallFunctionFvPatchScalarField::calcMut() const
{
    const label patchI = patch().index();
    const turbulenceModel& turbModel =
        db().lookupObject<turbulenceModel>("turbulenceModel");
    const scalarField& y = turbModel.y()[patchI];
    const scalarField& rhow = turbModel.rho().boundaryField()[patchI];
    const tmp<volScalarField> tk = turbModel.k();
    const volScalarField& k = tk();
    const scalarField& muw = turbModel.mu().boundaryField()[patchI];

    const scalar Cmu25 = pow025(Cmu_);

    tmp<scalarField> tmutw(new scalarField(patch().size(), 0.0));
    scalarField& mutw = tmutw();

    forAll(mutw, faceI)
    {
        label faceCellI = patch().faceCells()[faceI];

        scalar yPlus =
            Cmu25*y[faceI]*sqrt(k[faceCellI])/(muw[faceI]/rhow[faceI]);

        if (yPlus > yPlusLam_)
        {
            mutw[faceI] = muw[faceI]*(yPlus*kappa_/log(E_*yPlus) - 1);
        }
    }

    return tmutw;
}


void mutkWallFunctionFvPatchScalarField::writeLocalEntries(Ostream& os) const
{
    os.writeKeyword("Cmu") << Cmu_ << token::END_STATEMENT << nl;
    os.writeKeyword("kappa") << kappa_ << token::END_STATEMENT << nl;
    os.writeKeyword("E") << E_ << token::END_STATEMENT << nl;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

mutkWallFunctionFvPatchScalarField::mutkWallFunctionFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(p, iF),
    Cmu_(0.09),
    kappa_(0.41),
    E_(9.8),
    yPlusLam_(calcYPlusLam(kappa_, E_))
{}


mutkWallFunctionFvPatchScalarField::mutkWallFunctionFvPatchScalarField
(
    const mutkWallFunctionFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchScalarField(ptf, p, iF, mapper),
    Cmu_(ptf.Cmu_),
    kappa_(ptf.kappa_),
    E_(ptf.E_),
    yPlusLam_(ptf.yPlusLam_)
{}


mutkWallFunctionFvPatchScalarField::mutkWallFunctionFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchScalarField(p, iF, dict),
    Cmu_(dict.lookupOrDefault<scalar>("Cmu", 0.09)),
    kappa_(dict.lookupOrDefault<scalar>("kappa", 0.41)),
    E_(dict.lookupOrDefault<scalar>("E", 9.8)),
    yPlusLam_(calcYPlusLam(kappa_, E_))
{}


mutkWallFunctionFvPatchScalarField::mutkWallFunctionFvPatchScalarField
(
    const mutkWallFunctionFvPatchScalarField& wfpsf
)
:
    fixedValueFvPatchScalarField(wfpsf),
    Cmu_(wfpsf.Cmu_),
    kappa_(wfpsf.kappa_),
    E_(wfpsf.E_),
    yPlusLam_(wfpsf.yPlusLam_)
{}


mutkWallFunctionFvPatchScalarField::mutkWallFunctionFvPatchScalarField
(
    const mutkWallFunctionFvPatchScalarField& wfpsf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(wfpsf, iF),
    Cmu_(wfpsf.Cmu_),
    kappa_(wfpsf.kappa_),
    E_(wfpsf.E_),
    yPlusLam_(wfpsf.yPlusLam_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void mutkWallFunctionFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    operator==(calcMut());

    fixedValueFvPatchScalarField::updateCoeffs();
}


tmp<scalarField> mutkWallFunctionFvPatchScalarField::yPlus() const
{
    const label patchI = patch().index();

    const turbulenceModel& turbModel =
        db().lookupObject<turbulenceModel>("turbulenceModel");
    const scalarField& y = turbModel.y()[patchI];

    const tmp<volScalarField> tk = turbModel.k();
    const volScalarField& k = tk();
    const scalarField kwc(k.boundaryField()[patchI].patchInternalField());
    const scalarField& muw = turbModel.mu().boundaryField()[patchI];
    const scalarField& rhow = turbModel.rho().boundaryField()[patchI];

    return pow025(Cmu_)*y*sqrt(kwc)/(muw/rhow);
}


void mutkWallFunctionFvPatchScalarField::write(Ostream& os) const
{
    fvPatchField<scalar>::write(os);
    writeLocalEntries(os);
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField
(
    fvPatchScalarField,
    mutkWallFunctionFvPatchScalarField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace compressible
} // End namespace CML

// ************************************************************************* //
