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

#include "laplaceFilter.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "calculatedFvPatchFields.hpp"
#include "fvm.hpp"
#include "fvc.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(laplaceFilter, 0);
    addToRunTimeSelectionTable(LESfilter, laplaceFilter, dictionary);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::laplaceFilter::laplaceFilter(const fvMesh& mesh, scalar widthCoeff)
:
    LESfilter(mesh),
    widthCoeff_(widthCoeff),
    coeff_
    (
        IOobject
        (
            "laplaceFilterCoeff",
            mesh.time().timeName(),
            mesh
        ),
        mesh,
        dimensionedScalar("zero", dimLength*dimLength, 0),
        calculatedFvPatchScalarField::typeName
    )
{
    coeff_.dimensionedInternalField() = pow(mesh.V(), 2.0/3.0)/widthCoeff_;
}


CML::laplaceFilter::laplaceFilter(const fvMesh& mesh, const dictionary& bd)
:
    LESfilter(mesh),
    widthCoeff_(readScalar(bd.subDict(type() + "Coeffs").lookup("widthCoeff"))),
    coeff_
    (
        IOobject
        (
            "laplaceFilterCoeff",
            mesh.time().timeName(),
            mesh
        ),
        mesh,
        dimensionedScalar("zero", dimLength*dimLength, 0),
        calculatedFvPatchScalarField::typeName
    )
{
    coeff_.dimensionedInternalField() = pow(mesh.V(), 2.0/3.0)/widthCoeff_;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::laplaceFilter::read(const dictionary& bd)
{
    bd.subDict(type() + "Coeffs").lookup("widthCoeff") >> widthCoeff_;
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

CML::tmp<CML::volScalarField> CML::laplaceFilter::operator()
(
    const tmp<volScalarField>& unFilteredField
) const
{
    tmp<volScalarField> filteredField =
        unFilteredField() + fvc::laplacian(coeff_, unFilteredField());

    unFilteredField.clear();

    return filteredField;
}


CML::tmp<CML::volVectorField> CML::laplaceFilter::operator()
(
    const tmp<volVectorField>& unFilteredField
) const
{
    tmp<volVectorField> filteredField =
        unFilteredField() + fvc::laplacian(coeff_, unFilteredField());

    unFilteredField.clear();

    return filteredField;
}


CML::tmp<CML::volSymmTensorField> CML::laplaceFilter::operator()
(
    const tmp<volSymmTensorField>& unFilteredField
) const
{
    tmp<volSymmTensorField> filteredField =
        unFilteredField() + fvc::laplacian(coeff_, unFilteredField());

    unFilteredField.clear();

    return filteredField;
}


CML::tmp<CML::volTensorField> CML::laplaceFilter::operator()
(
    const tmp<volTensorField>& unFilteredField
) const
{
    tmp<volTensorField> filteredField =
        unFilteredField() + fvc::laplacian(coeff_, unFilteredField());

    unFilteredField.clear();

    return filteredField;
}


// ************************************************************************* //
