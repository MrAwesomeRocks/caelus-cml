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

#include "muSgsUSpaldingWFFvPatchScalarField.hpp"
#include "compressibleLESModel.hpp"
#include "fvPatchFieldMapper.hpp"
#include "volFields.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace compressible
{
namespace LESModels
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

muSgsUSpaldingWallFunctionFvPatchScalarField::
muSgsUSpaldingWallFunctionFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(p, iF),
    UName_("U"),
    rhoName_("rho"),
    muName_("mu"),
    kappa_(0.41),
    E_(9.8)
{}


muSgsUSpaldingWallFunctionFvPatchScalarField::
muSgsUSpaldingWallFunctionFvPatchScalarField
(
    const muSgsUSpaldingWallFunctionFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchScalarField(ptf, p, iF, mapper),
    UName_(ptf.UName_),
    rhoName_(ptf.rhoName_),
    muName_(ptf.muName_),
    kappa_(ptf.kappa_),
    E_(ptf.E_)
{}


muSgsUSpaldingWallFunctionFvPatchScalarField::
muSgsUSpaldingWallFunctionFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchScalarField(p, iF, dict),
    UName_(dict.lookupOrDefault<word>("U", "U")),
    rhoName_(dict.lookupOrDefault<word>("rho", "rho")),
    muName_(dict.lookupOrDefault<word>("mu", "mu")),
    kappa_(dict.lookupOrDefault<scalar>("kappa", 0.41)),
    E_(dict.lookupOrDefault<scalar>("E", 9.8))
{}


muSgsUSpaldingWallFunctionFvPatchScalarField::
muSgsUSpaldingWallFunctionFvPatchScalarField
(
    const muSgsUSpaldingWallFunctionFvPatchScalarField& mwfpsf
)
:
    fixedValueFvPatchScalarField(mwfpsf),
    UName_(mwfpsf.UName_),
    rhoName_(mwfpsf.rhoName_),
    muName_(mwfpsf.muName_),
    kappa_(mwfpsf.kappa_),
    E_(mwfpsf.E_)
{}


muSgsUSpaldingWallFunctionFvPatchScalarField::
muSgsUSpaldingWallFunctionFvPatchScalarField
(
    const muSgsUSpaldingWallFunctionFvPatchScalarField& mwfpsf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(mwfpsf, iF),
    UName_(mwfpsf.UName_),
    rhoName_(mwfpsf.rhoName_),
    muName_(mwfpsf.muName_),
    kappa_(mwfpsf.kappa_),
    E_(mwfpsf.E_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void muSgsUSpaldingWallFunctionFvPatchScalarField::evaluate
(
    const Pstream::commsTypes
)
{
    const scalarField& ry = patch().deltaCoeffs();

    const fvPatchVectorField& U =
        patch().lookupPatchField<volVectorField, vector>(UName_);

    const scalarField magUp(mag(U.patchInternalField() - U));

    const scalarField& muw =
        patch().lookupPatchField<volScalarField, scalar>(muName_);

    const scalarField& rhow =
        patch().lookupPatchField<volScalarField, scalar>(rhoName_);

    scalarField& muSgsw = *this;

    const scalarField magFaceGradU(mag(U.snGrad()));

    forAll(muSgsw, facei)
    {
        scalar magUpara = magUp[facei];

        scalar utau =
            sqrt((muSgsw[facei] + muw[facei])*magFaceGradU[facei]/rhow[facei]);

        if (utau > 0)
        {
            int iter = 0;
            scalar err = GREAT;

            do
            {
                scalar kUu = kappa_*magUpara/utau;
                scalar fkUu = exp(kUu) - 1 - kUu*(1 + 0.5*kUu);

                scalar f =
                    - utau/(ry[facei]*muw[facei]/rhow[facei])
                    + magUpara/utau
                    + 1/E_*(fkUu - 1.0/6.0*kUu*sqr(kUu));

                scalar df =
                    - 1.0/(ry[facei]*muw[facei]/rhow[facei])
                    - magUpara/sqr(utau)
                    - 1/E_*kUu*fkUu/utau;

                scalar utauNew = utau - f/df;
                err = mag((utau - utauNew)/utau);
                utau = utauNew;

            } while (utau > VSMALL && err > 0.01 && ++iter < 10);

            muSgsw[facei] =
                max
                (
                    rhow[facei]*sqr(utau)/magFaceGradU[facei] - muw[facei],
                    0.0
                );
        }
        else
        {
            muSgsw[facei] = 0;
        }
    }
}


void muSgsUSpaldingWallFunctionFvPatchScalarField::write(Ostream& os) const
{
    fvPatchField<scalar>::write(os);
    writeEntryIfDifferent<word>(os, "U", "U", UName_);
    writeEntryIfDifferent<word>(os, "rho", "rho", rhoName_);
    writeEntryIfDifferent<word>(os, "mu", "mu", muName_);
    os.writeKeyword("kappa") << kappa_ << token::END_STATEMENT << nl;
    os.writeKeyword("E") << E_ << token::END_STATEMENT << nl;
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField
(
    fvPatchScalarField,
    muSgsUSpaldingWallFunctionFvPatchScalarField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace LESModels
} // End namespace compressible
} // End namespace CML

// ************************************************************************* //
