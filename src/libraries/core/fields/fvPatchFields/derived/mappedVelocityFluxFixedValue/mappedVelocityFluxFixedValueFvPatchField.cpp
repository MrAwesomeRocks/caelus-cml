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

#include "mappedVelocityFluxFixedValueFvPatchField.hpp"
#include "fvPatchFieldMapper.hpp"
#include "mappedPatchBase.hpp"
#include "volFields.hpp"
#include "surfaceFields.hpp"
#include "addToRunTimeSelectionTable.hpp"


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::mappedVelocityFluxFixedValueFvPatchField::
mappedVelocityFluxFixedValueFvPatchField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(p, iF),
    phiName_("phi")
{}


CML::mappedVelocityFluxFixedValueFvPatchField::
mappedVelocityFluxFixedValueFvPatchField
(
    const mappedVelocityFluxFixedValueFvPatchField& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchVectorField(ptf, p, iF, mapper),
    phiName_(ptf.phiName_)
{
    if (!isA<mappedPatchBase>(this->patch().patch()))
    {
        FatalErrorInFunction
            << "Patch type '" << p.type()
            << "' not type '" << mappedPatchBase::typeName << "'"
            << " for patch " << p.name()
            << " of field " << dimensionedInternalField().name()
            << " in file " << dimensionedInternalField().objectPath()
            << exit(FatalError);
    }
}


CML::mappedVelocityFluxFixedValueFvPatchField::
mappedVelocityFluxFixedValueFvPatchField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchVectorField(p, iF, dict),
    phiName_(dict.lookupOrDefault<word>("phi", "phi"))
{
    if (!isA<mappedPatchBase>(this->patch().patch()))
    {
        FatalErrorInFunction
            << "Patch type '" << p.type()
            << "' not type '" << mappedPatchBase::typeName << "'"
            << " for patch " << p.name()
            << " of field " << dimensionedInternalField().name()
            << " in file " << dimensionedInternalField().objectPath()
            << exit(FatalError);
    }

    const mappedPatchBase& mpp = refCast<const mappedPatchBase>
    (
        this->patch().patch()
    );
    if (mpp.mode() == mappedPolyPatch::NEARESTCELL)
    {
        FatalErrorInFunction
            << "Patch " << p.name()
            << " of type '" << p.type()
            << "' can not be used in 'nearestCell' mode"
            << " of field " << dimensionedInternalField().name()
            << " in file " << dimensionedInternalField().objectPath()
            << exit(FatalError);
    }
}


CML::mappedVelocityFluxFixedValueFvPatchField::
mappedVelocityFluxFixedValueFvPatchField
(
    const mappedVelocityFluxFixedValueFvPatchField& ptf
)
:
    fixedValueFvPatchVectorField(ptf),
    phiName_(ptf.phiName_)
{}


CML::mappedVelocityFluxFixedValueFvPatchField::
mappedVelocityFluxFixedValueFvPatchField
(
    const mappedVelocityFluxFixedValueFvPatchField& ptf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(ptf, iF),
    phiName_(ptf.phiName_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::mappedVelocityFluxFixedValueFvPatchField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    // Since we're inside initEvaluate/evaluate there might be processor
    // comms underway. Change the tag we use.
    int oldTag = UPstream::msgType();
    UPstream::msgType() = oldTag+1;

    // Get the mappedPatchBase
    const mappedPatchBase& mpp = refCast<const mappedPatchBase>
    (
        mappedVelocityFluxFixedValueFvPatchField::patch().patch()
    );
    const fvMesh& nbrMesh = refCast<const fvMesh>(mpp.sampleMesh());
    const word& fieldName = dimensionedInternalField().name();
    const volVectorField& UField =
        nbrMesh.lookupObject<volVectorField>(fieldName);

    surfaceScalarField& phiField = const_cast<surfaceScalarField&>
    (
        nbrMesh.lookupObject<surfaceScalarField>(phiName_)
    );

    vectorField newUValues;
    scalarField newPhiValues;

    switch (mpp.mode())
    {
        case mappedPolyPatch::NEARESTFACE:
        {
            vectorField allUValues(nbrMesh.nFaces(), vector::zero);
            scalarField allPhiValues(nbrMesh.nFaces(), 0.0);

            forAll(UField.boundaryField(), patchI)
            {
                const fvPatchVectorField& Upf = UField.boundaryField()[patchI];
                const scalarField& phipf = phiField.boundaryField()[patchI];

                label faceStart = Upf.patch().start();

                forAll(Upf, faceI)
                {
                    allUValues[faceStart + faceI] = Upf[faceI];
                    allPhiValues[faceStart + faceI] = phipf[faceI];
                }
            }

            mpp.distribute(allUValues);
            newUValues.transfer(allUValues);

            mpp.distribute(allPhiValues);
            newPhiValues.transfer(allPhiValues);

            break;
        }
        case mappedPolyPatch::NEARESTPATCHFACE:
        case mappedPolyPatch::NEARESTPATCHFACEAMI:
        {
            const label nbrPatchID =
                nbrMesh.boundaryMesh().findPatchID(mpp.samplePatch());

            newUValues = UField.boundaryField()[nbrPatchID];
            mpp.distribute(newUValues);

            newPhiValues = phiField.boundaryField()[nbrPatchID];
            mpp.distribute(newPhiValues);

            break;
        }
        default:
        {
            FatalErrorInFunction
                << "patch can only be used in NEARESTPATCHFACE, "
                << "NEARESTPATCHFACEAMI or NEARESTFACE mode" << nl
                << abort(FatalError);
        }
    }

    operator==(newUValues);
    phiField.boundaryField()[patch().index()] == newPhiValues;

    // Restore tag
    UPstream::msgType() = oldTag;

    fixedValueFvPatchVectorField::updateCoeffs();
}


void CML::mappedVelocityFluxFixedValueFvPatchField::write
(
    Ostream& os
) const
{
    fvPatchVectorField::write(os);
    writeEntryIfDifferent<word>(os, "phi", "phi", phiName_);
    this->writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
    makePatchTypeField
    (
        fvPatchVectorField,
        mappedVelocityFluxFixedValueFvPatchField
    );
}


// ************************************************************************* //
