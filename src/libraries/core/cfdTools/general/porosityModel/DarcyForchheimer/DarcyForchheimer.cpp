/*---------------------------------------------------------------------------*\
Copyright (C) 2012 OpenFOAM Foundation
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

#include "addToRunTimeSelectionTable.hpp"
#include "DarcyForchheimer.hpp"
#include "geometricOneField.hpp"
#include "fvMatrices.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    namespace porosityModels
    {
        defineTypeNameAndDebug(DarcyForchheimer, 0);
        addToRunTimeSelectionTable(porosityModel, DarcyForchheimer, mesh);
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::porosityModels::DarcyForchheimer::DarcyForchheimer
(
    const word& name,
    const word& modelType,
    const fvMesh& mesh,
    const dictionary& dict,
    const word& cellZoneName
)
:
    porosityModel(name, modelType, mesh, dict, cellZoneName),
    dXYZ_(coeffs_.lookup("d")),
    fXYZ_(coeffs_.lookup("f")),
    D_(cellZoneIDs_.size()),
    F_(cellZoneIDs_.size()),
    rhoName_(coeffs_.lookupOrDefault<word>("rho", "rho")),
    muName_(coeffs_.lookupOrDefault<word>("mu", "thermo:mu")),
    nuName_(coeffs_.lookupOrDefault<word>("nu", "nu"))
{
    adjustNegativeResistance(dXYZ_);
    adjustNegativeResistance(fXYZ_);

    calcTranformModelData();
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::porosityModels::DarcyForchheimer::~DarcyForchheimer()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::porosityModels::DarcyForchheimer::calcTranformModelData()
{
    if (coordSys_.R().uniform())
    {
        forAll (cellZoneIDs_, zoneI)
        {
            D_[zoneI].setSize(1);
            F_[zoneI].setSize(1);

            D_[zoneI][0] = tensor::zero;
            D_[zoneI][0].xx() = dXYZ_.value().x();
            D_[zoneI][0].yy() = dXYZ_.value().y();
            D_[zoneI][0].zz() = dXYZ_.value().z();

            D_[zoneI][0] = coordSys_.R().transformTensor(D_[zoneI][0]);

            // leading 0.5 is from 1/2*rho
            F_[zoneI][0] = tensor::zero;
            F_[zoneI][0].xx() = 0.5*fXYZ_.value().x();
            F_[zoneI][0].yy() = 0.5*fXYZ_.value().y();
            F_[zoneI][0].zz() = 0.5*fXYZ_.value().z();

            F_[zoneI][0] = coordSys_.R().transformTensor(F_[zoneI][0]);
        }
    }
    else
    {
        forAll(cellZoneIDs_, zoneI)
        {
            const labelList& cells = mesh_.cellZones()[cellZoneIDs_[zoneI]];

            D_[zoneI].setSize(cells.size());
            F_[zoneI].setSize(cells.size());

            forAll(cells, i)
            {
                D_[zoneI][i] = tensor::zero;
                D_[zoneI][i].xx() = dXYZ_.value().x();
                D_[zoneI][i].yy() = dXYZ_.value().y();
                D_[zoneI][i].zz() = dXYZ_.value().z();

                // leading 0.5 is from 1/2*rho
                F_[zoneI][i] = tensor::zero;
                F_[zoneI][i].xx() = 0.5*fXYZ_.value().x();
                F_[zoneI][i].yy() = 0.5*fXYZ_.value().y();
                F_[zoneI][i].zz() = 0.5*fXYZ_.value().z();
            }

            const coordinateRotation& R = coordSys_.R(mesh_, cells);

            D_[zoneI] = R.transformTensor(D_[zoneI], cells);
            F_[zoneI] = R.transformTensor(F_[zoneI], cells);
        }
    }

    if (debug && mesh_.time().outputTime())
    {
        volTensorField Dout
        (
            IOobject
            (
                typeName + ":D",
                mesh_.time().timeName(),
                mesh_,
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            mesh_,
            dimensionedTensor("0", dXYZ_.dimensions(), tensor::zero)
        );
        volTensorField Fout
        (
            IOobject
            (
                typeName + ":F",
                mesh_.time().timeName(),
                mesh_,
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            mesh_,
            dimensionedTensor("0", fXYZ_.dimensions(), tensor::zero)
        );

        UIndirectList<tensor>(Dout, mesh_.cellZones()[cellZoneIDs_[0]]) = D_[0];
        UIndirectList<tensor>(Fout, mesh_.cellZones()[cellZoneIDs_[0]]) = F_[0];

        Dout.write();
        Fout.write();
    }
}


void CML::porosityModels::DarcyForchheimer::calcForce
(
    const volVectorField& U,
    const volScalarField& rho,
    const volScalarField& mu,
    vectorField& force
) const
{
    scalarField Udiag(U.size(), 0.0);
    vectorField Usource(U.size(), vector::zero);
    const scalarField& V = mesh_.V();

    apply(Udiag, Usource, V, rho, mu, U);

    force = Udiag*U - Usource;
}


void CML::porosityModels::DarcyForchheimer::correct
(
    fvVectorMatrix& UEqn
) const
{
    const volVectorField& U = UEqn.psi();
    const scalarField& V = mesh_.V();
    scalarField& Udiag = UEqn.diag();
    vectorField& Usource = UEqn.source();
 
    if (UEqn.dimensions() == dimForce)
    {
        const volScalarField& rho = mesh_.lookupObject<volScalarField>(rhoName_);

        if (mesh_.foundObject<volScalarField>(muName_))
        {
            const volScalarField& mu =
                mesh_.lookupObject<volScalarField>(muName_);

            apply(Udiag, Usource, V, rho, mu, U);
        }
        else
        {
            const volScalarField& nu =
                mesh_.lookupObject<volScalarField>(nuName_);

            apply(Udiag, Usource, V, rho, rho*nu, U);
        }
    }
    else
    {
        if (mesh_.foundObject<volScalarField>(nuName_))
        {
            const volScalarField& nu =
                mesh_.lookupObject<volScalarField>(nuName_);

            apply(Udiag, Usource, V, geometricOneField(), nu, U);
        }
        else
        {
            const volScalarField& rho =
                mesh_.lookupObject<volScalarField>(rhoName_);
            const volScalarField& mu =
                mesh_.lookupObject<volScalarField>(muName_);

            apply(Udiag, Usource, V, geometricOneField(), mu/rho, U);
        }
    }
}


void CML::porosityModels::DarcyForchheimer::correct
(
    fvVectorMatrix& UEqn,
    const volScalarField& rho,
    const volScalarField& mu
) const
{
    const vectorField& U = UEqn.psi();
    const scalarField& V = mesh_.V();
    scalarField& Udiag = UEqn.diag();
    vectorField& Usource = UEqn.source();

    apply(Udiag, Usource, V, rho, mu, U);
}


void CML::porosityModels::DarcyForchheimer::correct
(
    const fvVectorMatrix& UEqn,
    volTensorField& AU
) const
{
    const volVectorField& U = UEqn.psi();


    if (UEqn.dimensions() == dimForce)
    {
        const volScalarField& rho = mesh_.lookupObject<volScalarField>(rhoName_);
        const volScalarField& mu = mesh_.lookupObject<volScalarField>(muName_);

        apply(AU, rho, mu, U);
    }
    else
    {
        if (mesh_.foundObject<volScalarField>(nuName_))
        {
            const volScalarField& nu =
                mesh_.lookupObject<volScalarField>(nuName_);

            apply(AU, geometricOneField(), nu, U);
        }
        else
        {
            const volScalarField& rho =
                mesh_.lookupObject<volScalarField>(rhoName_);
            const volScalarField& mu =
                mesh_.lookupObject<volScalarField>(muName_);

            apply(AU, geometricOneField(), mu/rho, U);
        }
    }
}


bool CML::porosityModels::DarcyForchheimer::writeData(Ostream& os) const
{
    os  << indent << name_ << endl;
    dict_.write(os);

    return true;
}


// ************************************************************************* //
