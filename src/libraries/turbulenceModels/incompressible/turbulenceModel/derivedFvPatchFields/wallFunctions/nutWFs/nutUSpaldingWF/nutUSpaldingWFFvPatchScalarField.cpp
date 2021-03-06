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

#include "nutUSpaldingWFFvPatchScalarField.hpp"
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

tmp<scalarField> nutUSpaldingWallFunctionFvPatchScalarField::calcNut() const
{
    const label patchI = patch().index();

    const turbulenceModel& turbModel =
        db().lookupObject<turbulenceModel>("turbulenceModel");
    const fvPatchVectorField& Uw = turbModel.U().boundaryField()[patchI];
    const scalarField magGradU(mag(Uw.snGrad()));
    const tmp<volScalarField> tnu = turbModel.nu();
    const volScalarField& nu = tnu();
    const scalarField& nuw = nu.boundaryField()[patchI];

    return max
    (
        scalar(0),
        sqr(calcUTau(magGradU))/(magGradU + ROOTVSMALL) - nuw
    );
}


tmp<scalarField> nutUSpaldingWallFunctionFvPatchScalarField::calcUTau
(
    const scalarField& magGradU
) const
{
    const label patchI = patch().index();

    const turbulenceModel& turbModel =
        db().lookupObject<turbulenceModel>("turbulenceModel");
    const scalarField& y = turbModel.y()[patchI];

    const fvPatchVectorField& Uw = turbModel.U().boundaryField()[patchI];
    const scalarField magUp(mag(Uw.patchInternalField() - Uw));

    const tmp<volScalarField> tnu = turbModel.nu();
    const volScalarField& nu = tnu();
    const scalarField& nuw = nu.boundaryField()[patchI];

    const scalarField& nutw = *this;

    tmp<scalarField> tuTau(new scalarField(patch().size(), 0.0));
    scalarField& uTau = tuTau();

    forAll(uTau, faceI)
    {
        scalar ut = sqrt((nutw[faceI] + nuw[faceI])*magGradU[faceI]);

        if (ut > ROOTVSMALL)
        {
            int iter = 0;
            scalar err = GREAT;

            do
            {
                scalar kUu = min(kappa_*magUp[faceI]/ut, 50);
                scalar fkUu = exp(kUu) - 1 - kUu*(1 + 0.5*kUu);

                scalar f =
                    - ut*y[faceI]/nuw[faceI]
                    + magUp[faceI]/ut
                    + 1/E_*(fkUu - 1.0/6.0*kUu*sqr(kUu));

                scalar df =
                    y[faceI]/nuw[faceI]
                  + magUp[faceI]/sqr(ut)
                  + 1/E_*kUu*fkUu/ut;

                scalar uTauNew = ut + f/df;
                err = mag((ut - uTauNew)/ut);
                ut = uTauNew;

            } while (ut > ROOTVSMALL && err > 0.01 && ++iter < 10);

            uTau[faceI] = max(0.0, ut);
        }
    }

    return tuTau;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

nutUSpaldingWallFunctionFvPatchScalarField::
nutUSpaldingWallFunctionFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    nutWallFunctionFvPatchScalarField(p, iF)
{}


nutUSpaldingWallFunctionFvPatchScalarField::
nutUSpaldingWallFunctionFvPatchScalarField
(
    const nutUSpaldingWallFunctionFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    nutWallFunctionFvPatchScalarField(ptf, p, iF, mapper)
{}


nutUSpaldingWallFunctionFvPatchScalarField::
nutUSpaldingWallFunctionFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    nutWallFunctionFvPatchScalarField(p, iF, dict)
{}


nutUSpaldingWallFunctionFvPatchScalarField::
nutUSpaldingWallFunctionFvPatchScalarField
(
    const nutUSpaldingWallFunctionFvPatchScalarField& wfpsf
)
:
    nutWallFunctionFvPatchScalarField(wfpsf)
{}


nutUSpaldingWallFunctionFvPatchScalarField::
nutUSpaldingWallFunctionFvPatchScalarField
(
    const nutUSpaldingWallFunctionFvPatchScalarField& wfpsf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    nutWallFunctionFvPatchScalarField(wfpsf, iF)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

tmp<scalarField> nutUSpaldingWallFunctionFvPatchScalarField::yPlus() const
{
    const label patchi = patch().index();

    const turbulenceModel& turbModel =
        db().lookupObject<turbulenceModel>("turbulenceModel");
    const scalarField& y = turbModel.y()[patchi];
    const fvPatchVectorField& Uw = turbModel.U().boundaryField()[patchi];
    const tmp<volScalarField> tnu = turbModel.nu();
    const volScalarField& nu = tnu();
    const scalarField& nuw = nu.boundaryField()[patchi];

    return y*calcUTau(mag(Uw.snGrad()))/nuw;
}


void nutUSpaldingWallFunctionFvPatchScalarField::write(Ostream& os) const
{
    fvPatchField<scalar>::write(os);
    writeLocalEntries(os);
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField
(
    fvPatchScalarField,
    nutUSpaldingWallFunctionFvPatchScalarField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace incompressible
} // End namespace CML

// ************************************************************************* //
