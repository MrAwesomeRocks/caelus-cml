/*---------------------------------------------------------------------------*\
Copyright Niels G. Jacobsen, Deltares.
Copyright (C) 2018 Applied CCM Pty Ltd
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

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

\*----------------------------------------------------------------------------*/

#include "jjc2014Zone.hpp"
#include "fvMesh.hpp"
#include "fvMatrices.hpp"
#include "geometricOneField.hpp"

// * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * * //

// adjust negative resistance values to be multiplier of max value
void CML::jjc2014Zone::checkNegativeResistance(dimensionedVector& resist)
{
    scalar minCmpt = min(0, cmptMin(resist.value()));

    if (minCmpt < 0)
    {
        FatalErrorInFunction
            << "negative resistances! " << resist
            << exit(FatalError);
    }
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::jjc2014Zone::jjc2014Zone
(
    const word& name,
    const fvMesh& mesh,
    const dictionary& dict
)
:
    name_(name),
    mesh_(mesh),
    dict_(dict),
    cellZoneID_(mesh_.cellZones().findZoneID(name)),
    coordSys_(mesh, dict),
    porosity_(readScalar(dict_.lookup("porosity"))),
    addedMassCoeff_(readScalar(dict_.lookup("gammaAddedMass"))),
    D_("D", dimensionSet(0, -2, 0, 0, 0), tensor::zero),
    F_("F", dimensionSet(0, -1, 0, 0, 0), tensor::zero)
{
    Info<< "Creating porous zone: " << name_ << endl;

    autoPtr<CML::porosityCoefficient> pcPtr(CML::porosityCoefficient::New(dict));

    bool foundZone = (cellZoneID_ != -1);
    reduce(foundZone, orOp<bool>());

    if (!foundZone && Pstream::master())
    {
        FatalErrorInFunction
            << "cannot find porous cellZone " << name_
            << exit(FatalError);
    }

    // porosity
    if (porosity_ <= 0.0 || porosity_ > 1.0)
    {
        FatalIOErrorInFunction(dict_)
            << "out-of-range porosity value " << porosity_
            << exit(FatalIOError);
    }

    // local-to-global transformation tensor
    const tensor E = coordSys_.R().R();

    dimensionedVector d(pcPtr->linearCoefficient());

    if (D_.dimensions() != d.dimensions())
    {
        FatalIOErrorInFunction(dict_)
            << "incorrect dimensions for d: " << d.dimensions()
            << " should be " << D_.dimensions()
            << exit(FatalIOError);
    }

    checkNegativeResistance(d);

    D_.value().xx() = d.value().x();
    D_.value().yy() = d.value().y();
    D_.value().zz() = d.value().z();
    D_.value() = (E & D_ & E.T()).value();


    dimensionedVector f(pcPtr->quadraticCoefficient());

    if (F_.dimensions() != f.dimensions())
    {
        FatalIOErrorInFunction(dict_)
            << "incorrect dimensions for f: " << f.dimensions()
            << " should be " << F_.dimensions()
            << exit(FatalIOError);
    }

    checkNegativeResistance(f);

    F_.value().xx() = 0.5*f.value().x();
    F_.value().yy() = 0.5*f.value().y();
    F_.value().zz() = 0.5*f.value().z();
    F_.value() = (E & F_ & E.T()).value();

    // it is an error not to define anything
    if
    (
        magSqr(D_.value()) <= VSMALL
     && magSqr(F_.value()) <= VSMALL
    )
    {
        FatalIOErrorInFunction(dict_)
            << "neither powerLaw (C0/C1) "
               "nor Darcy-Forchheimer law (d/f) specified"
            << exit(FatalIOError);
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::jjc2014Zone::porosity
(
    volScalarField & poro
) const
{
    if (cellZoneID_ == -1)
    {
        return;
    }

    const labelList& cells = mesh_.cellZones()[cellZoneID_];

    forAll(cells, celli)
    {
        poro[cells[celli]] = porosity_;
    }
}


void CML::jjc2014Zone::addResistance(fvVectorMatrix& UEqn) const
{
    if (cellZoneID_ == -1)
    {
        return;
    }

    bool compressible = false;
    if (UEqn.dimensions() == dimensionSet(1, 1, -2, 0, 0))
    {
        compressible = true;
    }

    const labelList& cells = mesh_.cellZones()[cellZoneID_];
    const scalarField& V = mesh_.V();
    scalarField& Udiag = UEqn.diag();
    vectorField& Usource = UEqn.source();
    const vectorField& U = UEqn.psi();

    const tensor& D = D_.value();
    const tensor& F = F_.value();

    if (magSqr(D) > VSMALL || magSqr(F) > VSMALL)
    {
        if (compressible)
        {
            addViscousInertialResistance
            (
                Udiag,
                Usource,
                cells,
                V,
                mesh_.lookupObject<volScalarField>("rho"),
                mesh_.lookupObject<volScalarField>("mu"),
                U
            );
        }
        else
        {
            addViscousInertialResistance
            (
                Udiag,
                Usource,
                cells,
                V,
                geometricOneField(),
                mesh_.lookupObject<volScalarField>("nu"),
                U
            );
        }
    }
}


void CML::jjc2014Zone::addResistance
(
    const fvVectorMatrix& UEqn,
    volTensorField& AU,
    bool correctAUprocBC
) const
{
    if (cellZoneID_ == -1)
    {
        return;
    }

    bool compressible = false;
    if (UEqn.dimensions() == dimensionSet(1, 1, -2, 0, 0))
    {
        compressible = true;
    }

    const labelList& cells = mesh_.cellZones()[cellZoneID_];
    const vectorField& U = UEqn.psi();

    const tensor& D = D_.value();
    const tensor& F = F_.value();

    if (magSqr(D) > VSMALL || magSqr(F) > VSMALL)
    {
        if (compressible)
        {
            addViscousInertialResistance
            (
                AU,
                cells,
                mesh_.lookupObject<volScalarField>("rho"),
                mesh_.lookupObject<volScalarField>("mu"),
                U
            );
        }
        else
        {
            addViscousInertialResistance
            (
                AU,
                cells,
                geometricOneField(),
                mesh_.lookupObject<volScalarField>("nu"),
                U
            );
        }
    }

    if (correctAUprocBC)
    {
        // Correct the boundary conditions of the tensorial diagonal to ensure
        // processor boundaries are correctly handled when AU^-1 is interpolated
        // for the pressure equation.
        AU.correctBoundaryConditions();
    }
}


void CML::jjc2014Zone::writeDict(Ostream& os, bool subDict) const
{
    if (subDict)
    {
        os  << indent << token::BEGIN_BLOCK << incrIndent << nl;
        os.writeKeyword("name") << zoneName() << token::END_STATEMENT << nl;
    }
    else
    {
        os  << indent << zoneName() << nl
            << indent << token::BEGIN_BLOCK << incrIndent << nl;
    }

    if (dict_.found("note"))
    {
        os.writeKeyword("note") << string(dict_.lookup("note"))
            << token::END_STATEMENT << nl;
    }

    coordSys_.writeDict(os, true);

    if (dict_.found("porosity"))
    {
        os.writeKeyword("porosity") << porosity() << token::END_STATEMENT << nl;
    }

    // powerLaw coefficients
    if (const dictionary* dictPtr = dict_.subDictPtr("powerLaw"))
    {
        os << indent << "powerLaw";
        dictPtr->write(os);
    }

    // Darcy-Forchheimer coefficients
    if (const dictionary* dictPtr = dict_.subDictPtr("Darcy"))
    {
        os << indent << "Darcy";
        dictPtr->write(os);
    }

    os << decrIndent << indent << token::END_BLOCK << endl;
}


// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

CML::Ostream& CML::operator<<(Ostream& os, const jjc2014Zone& pZone)
{
    pZone.writeDict(os);
    return os;
}

// ************************************************************************* //
