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

#include "binaryAbsorptionEmission.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    namespace radiation
    {
        defineTypeNameAndDebug(binaryAbsorptionEmission, 0);

        addToRunTimeSelectionTable
        (
            absorptionEmissionModel,
            binaryAbsorptionEmission,
            dictionary
        );
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::radiation::binaryAbsorptionEmission::binaryAbsorptionEmission
(
    const dictionary& dict,
    const fvMesh& mesh
)
:
    absorptionEmissionModel(dict, mesh),
    coeffsDict_(dict.subDict(typeName + "Coeffs")),
    model1_
    (
        absorptionEmissionModel::New(coeffsDict_.subDict("model1"), mesh)
    ),
    model2_
    (
        absorptionEmissionModel::New(coeffsDict_.subDict("model2"), mesh)
    )
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::tmp<CML::volScalarField>
CML::radiation::binaryAbsorptionEmission::aCont(const label bandI) const
{
    return model1_->aCont(bandI) + model2_->aCont(bandI);
}


CML::tmp<CML::volScalarField>
CML::radiation::binaryAbsorptionEmission::aDisp(const label bandI) const
{
    return model1_->aDisp(bandI) + model2_->aDisp(bandI);
}


CML::tmp<CML::volScalarField>
CML::radiation::binaryAbsorptionEmission::eCont(const label bandI) const
{
    return model1_->eCont(bandI) + model2_->eCont(bandI);
}


CML::tmp<CML::volScalarField>
CML::radiation::binaryAbsorptionEmission::eDisp(const label bandI) const
{
    return model1_->eDisp(bandI) + model2_->eDisp(bandI);
}


CML::tmp<CML::volScalarField>
CML::radiation::binaryAbsorptionEmission::ECont(const label bandI) const
{
    return model1_->ECont(bandI) + model2_->ECont(bandI);
}


CML::tmp<CML::volScalarField>
CML::radiation::binaryAbsorptionEmission::EDisp(const label bandI) const
{
    return model1_->EDisp(bandI) + model2_->EDisp(bandI);
}
