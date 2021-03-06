/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2018 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Caelus is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "greyMeanSolidAbsorptionEmission.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "unitConversion.hpp"
#include "extrapolatedCalculatedFvPatchFields.hpp"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    namespace radiation
    {
        defineTypeNameAndDebug(greyMeanSolidAbsorptionEmission, 0);

        addToRunTimeSelectionTable
        (
            absorptionEmissionModel,
            greyMeanSolidAbsorptionEmission,
            dictionary
        );
    }
}

// * * * * * * * * * * * * * * Private Member Functions * * * * * * * * * * //

CML::tmp<CML::scalarField> CML::radiation::
greyMeanSolidAbsorptionEmission::X(const word specie) const
{
    const volScalarField& T = thermo_.T();
    const volScalarField& p = thermo_.p();

    tmp<scalarField> tXj(new scalarField(T.internalField().size(), 0.0));
    scalarField& Xj = tXj();

    tmp<scalarField> tRhoInv(new scalarField(T.internalField().size(), 0.0));
    scalarField& rhoInv = tRhoInv();

    forAll(mixture_.Y(), specieI)
    {
        const scalarField& Yi = mixture_.Y()[specieI];

        forAll(rhoInv, iCell)
        {
            rhoInv[iCell] +=
                Yi[iCell]/mixture_.rho(specieI, p[iCell], T[iCell]);
        }
    }
    const scalarField& Yj = mixture_.Y(specie);
    const label mySpecieI = mixture_.species()[specie];
    forAll(Xj, iCell)
    {
        Xj[iCell] = Yj[iCell]/mixture_.rho(mySpecieI, p[iCell], T[iCell]);
    }

    return (Xj/rhoInv);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::radiation::greyMeanSolidAbsorptionEmission::
greyMeanSolidAbsorptionEmission
(
    const dictionary& dict,
    const fvMesh& mesh
)
:
    absorptionEmissionModel(dict, mesh),
    coeffsDict_((dict.subDict(typeName + "Coeffs"))),
    thermo_(mesh.lookupObject<solidThermo>(basicThermo::dictName)),
    speciesNames_(0),
    mixture_(dynamic_cast<const basicSpecieMixture&>(thermo_)),
    solidData_(mixture_.Y().size())
{
    if (!isA<basicSpecieMixture>(thermo_))
    {
        FatalErrorInFunction
            << "Model requires a multi-component thermo package"
            << abort(FatalError);
    }

    label nFunc = 0;
    const dictionary& functionDicts = dict.subDict(typeName + "Coeffs");

    forAllConstIter(dictionary, functionDicts, iter)
    {
        // safety:
        if (!iter().isDict())
        {
            continue;
        }
        const word& key = iter().keyword();
        if (!mixture_.contains(key))
        {
            WarningInFunction
                << " specie: " << key << " is not found in the solid mixture"
                << nl
                << " specie is the mixture are:" << mixture_.species() << nl
                << nl << endl;
        }
        speciesNames_.insert(key, nFunc);
        const dictionary& dict = iter().dict();
        dict.lookup("absorptivity") >> solidData_[nFunc][absorptivity];
        dict.lookup("emissivity") >> solidData_[nFunc][emissivity];

        nFunc++;
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::tmp<CML::volScalarField>
CML::radiation::greyMeanSolidAbsorptionEmission::
calc(const label propertyId) const
{
    tmp<volScalarField> ta
    (
        new volScalarField
        (
            IOobject
            (
                "a",
                mesh().time().timeName(),
                mesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            mesh(),
            dimensionedScalar("a", dimless/dimLength, 0.0),
            extrapolatedCalculatedFvPatchVectorField::typeName
        )
    );

    scalarField& a = ta().internalField();

    forAllConstIter(HashTable<label>, speciesNames_, iter)
    {
        if (mixture_.contains(iter.key()))
        {
            a += solidData_[iter()][propertyId]*X(iter.key());
        }
    }

    ta().correctBoundaryConditions();
    return ta;
}


CML::tmp<CML::volScalarField>
CML::radiation::greyMeanSolidAbsorptionEmission::eCont
(
    const label bandI
) const
{
   return calc(emissivity);
}


CML::tmp<CML::volScalarField>
CML::radiation::greyMeanSolidAbsorptionEmission::aCont
(
    const label bandI
) const
{
   return calc(absorptivity);
}
