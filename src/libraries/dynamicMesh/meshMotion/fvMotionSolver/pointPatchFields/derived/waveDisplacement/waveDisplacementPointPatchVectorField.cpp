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

#include "waveDisplacementPointPatchVectorField.hpp"
#include "pointPatchFields.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "Time.hpp"
#include "polyMesh.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

waveDisplacementPointPatchVectorField::
waveDisplacementPointPatchVectorField
(
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF
)
:
    fixedValuePointPatchField<vector>(p, iF),
    amplitude_(vector::zero),
    omega_(0.0),
    waveNumber_(vector::zero)
{}


waveDisplacementPointPatchVectorField::
waveDisplacementPointPatchVectorField
(
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF,
    const dictionary& dict
)
:
    fixedValuePointPatchField<vector>(p, iF, dict),
    amplitude_(dict.lookup("amplitude")),
    omega_(readScalar(dict.lookup("omega"))),
    waveNumber_(dict.lookupOrDefault<vector>("waveLength", vector::zero))
{
    if (!dict.found("value"))
    {
        updateCoeffs();
    }
}


waveDisplacementPointPatchVectorField::
waveDisplacementPointPatchVectorField
(
    const waveDisplacementPointPatchVectorField& ptf,
    const pointPatch& p,
    const DimensionedField<vector, pointMesh>& iF,
    const pointPatchFieldMapper& mapper
)
:
    fixedValuePointPatchField<vector>(ptf, p, iF, mapper),
    amplitude_(ptf.amplitude_),
    omega_(ptf.omega_),
    waveNumber_(ptf.waveNumber_)
{}


waveDisplacementPointPatchVectorField::
waveDisplacementPointPatchVectorField
(
    const waveDisplacementPointPatchVectorField& ptf,
    const DimensionedField<vector, pointMesh>& iF
)
:
    fixedValuePointPatchField<vector>(ptf, iF),
    amplitude_(ptf.amplitude_),
    omega_(ptf.omega_),
    waveNumber_(ptf.waveNumber_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void waveDisplacementPointPatchVectorField::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    const polyMesh& mesh = this->dimensionedInternalField().mesh()();
    const Time& t = mesh.time();

    const scalarField points( waveNumber_ & patch().localPoints());

    Field<vector>::operator=
    (
        amplitude_*cos(omega_*t.value() - points)
    );

    fixedValuePointPatchField<vector>::updateCoeffs();
}


void waveDisplacementPointPatchVectorField::write(Ostream& os) const
{
    pointPatchField<vector>::write(os);
    os.writeKeyword("amplitude")
        << amplitude_ << token::END_STATEMENT << nl;
    os.writeKeyword("omega")
        << omega_ << token::END_STATEMENT << nl;
    os.writeKeyword("waveNumber")
        << waveNumber_ << token::END_STATEMENT << nl;
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePointPatchTypeField
(
    pointPatchVectorField,
    waveDisplacementPointPatchVectorField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
