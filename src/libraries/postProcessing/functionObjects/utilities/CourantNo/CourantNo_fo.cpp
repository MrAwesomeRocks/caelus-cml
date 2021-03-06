/*---------------------------------------------------------------------------*\
Copyright (C) 2013-2015 OpenFOAM Foundation
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

#include "CourantNo_fo.hpp"
#include "volFields.hpp"
#include "surfaceFields.hpp"
#include "dictionary.hpp"
#include "zeroGradientFvPatchFields.hpp"
#include "fvcSurfaceIntegrate.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(CourantNo, 0);
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

CML::tmp<CML::volScalarField> CML::CourantNo::rho
(
    const surfaceScalarField& phi
) const
{
    if (phi.dimensions() == dimMass/dimTime)
    {
        return (obr_.lookupObject<volScalarField>(rhoName_));
    }
    else
    {
        const fvMesh& mesh = refCast<const fvMesh>(obr_);

        return tmp<volScalarField>
        (
            new volScalarField
            (
                IOobject
                (
                    "rho",
                    mesh.time().timeName(),
                    mesh,
                    IOobject::NO_READ,
                    IOobject::NO_WRITE
                ),
                mesh,
                dimensionedScalar("rho", dimless, 1.0)
            )
        );
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::CourantNo::CourantNo
(
    const word& name,
    const objectRegistry& obr,
    const dictionary& dict,
    const bool loadFromFiles
)
:
    name_(name),
    obr_(obr),
    active_(true),
    phiName_("phi"),
    rhoName_("rho")
{
    // Check if the available mesh is an fvMesh, otherwise deactivate
    if (!isA<fvMesh>(obr_))
    {
        active_ = false;
        WarningInFunction
            << "No fvMesh available, deactivating " << name_ << nl
            << endl;
    }

    read(dict);

    if (active_)
    {
        const fvMesh& mesh = refCast<const fvMesh>(obr_);

        volScalarField* CourantNoPtr
        (
            new volScalarField
            (
                IOobject
                (
                    type(),
                    mesh.time().timeName(),
                    mesh,
                    IOobject::NO_READ,
                    IOobject::NO_WRITE
                ),
                mesh,
                dimensionedScalar("0", dimless, 0.0),
                zeroGradientFvPatchScalarField::typeName
            )
        );

        mesh.objectRegistry::store(CourantNoPtr);
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::CourantNo::~CourantNo()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::CourantNo::read(const dictionary& dict)
{
    if (active_)
    {
        phiName_ = dict.lookupOrDefault<word>("phiName", "phi");
        rhoName_ = dict.lookupOrDefault<word>("rhoName", "rho");
    }
}


void CML::CourantNo::execute()
{
    if (active_)
    {
        const fvMesh& mesh = refCast<const fvMesh>(obr_);

        const surfaceScalarField& phi =
            mesh.lookupObject<surfaceScalarField>(phiName_);

        volScalarField& CourantNo =
            const_cast<volScalarField&>
            (
                mesh.lookupObject<volScalarField>(type())
            );

        scalarField& iField = CourantNo.internalField();

        const scalarField sumPhi
        (
            fvc::surfaceSum(mag(phi))().internalField()
           /rho(phi)().internalField()
        );

        iField = 0.5*sumPhi/mesh.V().field()*mesh.time().deltaTValue();

        CourantNo.correctBoundaryConditions();
    }
}


void CML::CourantNo::end()
{
    if (active_)
    {
        execute();
    }
}


void CML::CourantNo::timeSet()
{
    // Do nothing
}


void CML::CourantNo::write()
{
    if (active_)
    {
        const volScalarField& CourantNo =
            obr_.lookupObject<volScalarField>(type());

        Info<< type() << " " << name_ << " output:" << nl
            << "    writing field " << CourantNo.name() << nl
            << endl;

        CourantNo.write();
    }
}


// ************************************************************************* //
