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

#include "yPlusLES.hpp"
#include "volFields.hpp"

#include "incompressible/LES/LESModel/LESModel.hpp"
#include "compressible/LES/LESModel/compressibleLESModel.hpp"
#include "wallFvPatch.hpp"
#include "nearWallDist.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(yPlusLES, 0);
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::yPlusLES::writeFileHeader(const label i)
{
    writeHeader(file(), "y+ (LES)");

    writeCommented(file(), "Time");
    writeTabbed(file(), "patch");
    writeTabbed(file(), "min");
    writeTabbed(file(), "max");
    writeTabbed(file(), "average");
    file() << endl;
}


void CML::yPlusLES::calcIncompressibleYPlus
(
    const fvMesh& mesh,
    const volVectorField& U,
    volScalarField& yPlus
)
{
    const incompressible::LESModel& model =
        mesh.lookupObject<incompressible::LESModel>("LESProperties");

    volScalarField::GeometricBoundaryField d = nearWallDist(mesh).y();
    volScalarField nuEff(model.nuEff());

    const fvPatchList& patches = mesh.boundary();

    const volScalarField nuLam(model.nu());

    bool foundPatch = false;
    forAll(patches, patchi)
    {
        const fvPatch& currPatch = patches[patchi];

        if (isA<wallFvPatch>(currPatch))
        {
            foundPatch = true;
            yPlus.boundaryField()[patchi] =
                d[patchi]
               *sqrt
                (
                    nuEff.boundaryField()[patchi]
                   *mag(U.boundaryField()[patchi].snGrad())
                )
               /nuLam.boundaryField()[patchi];

            const scalarField& Yp = yPlus.boundaryField()[patchi];

            scalar minYp = gMin(Yp);
            scalar maxYp = gMax(Yp);
            scalar avgYp = gAverage(Yp);

            if (Pstream::master())
            {
                Info(log_)<< "    patch " << currPatch.name()
                    << " y+ : min = " << minYp << ", max = " << maxYp
                    << ", average = " << avgYp << nl;

                file() << obr_.time().value()
                    << token::TAB << currPatch.name()
                    << token::TAB << minYp
                    << token::TAB << maxYp
                    << token::TAB << avgYp
                    << endl;
            }
        }
    }

    if (log_ && !foundPatch)
    {
        Info<< "    no " << wallFvPatch::typeName << " patches" << endl;
    }
}


void CML::yPlusLES::calcCompressibleYPlus
(
    const fvMesh& mesh,
    const volVectorField& U,
    volScalarField& yPlus
)
{
    const compressible::LESModel& model =
        mesh.lookupObject<compressible::LESModel>("LESProperties");

    volScalarField::GeometricBoundaryField d = nearWallDist(mesh).y();
    volScalarField muEff(model.muEff());
    const volScalarField& rho(model.rho());

    const fvPatchList& patches = mesh.boundary();

    const volScalarField muLam(model.mu());

    Info<< type() << " output:" << nl;

    bool foundPatch = false;
    forAll(patches, patchi)
    {
        const fvPatch& currPatch = patches[patchi];

        if (isA<wallFvPatch>(currPatch))
        {
            foundPatch = true;
            yPlus.boundaryField()[patchi] =
                d[patchi]
               *sqrt
                (
                    (muEff.boundaryField()[patchi]/rho.boundaryField()[patchi])
                   *mag(U.boundaryField()[patchi].snGrad())
                )
               /(muLam.boundaryField()[patchi]/rho.boundaryField()[patchi]);

            const scalarField& Yp = yPlus.boundaryField()[patchi];

            scalar minYp = gMin(Yp);
            scalar maxYp = gMax(Yp);
            scalar avgYp = gAverage(Yp);

            if (Pstream::master())
            {
                Info(log_)<< "    patch " << currPatch.name()
                    << " y+ : min = " << minYp << ", max = " << maxYp
                    << ", average = " << avgYp << nl;

                file() << obr_.time().value()
                    << token::TAB << currPatch.name()
                    << token::TAB << minYp
                    << token::TAB << maxYp
                    << token::TAB << avgYp
                    << endl;
            }
         }
    }

    if (log_ && !foundPatch)
    {
        Info<< "    no " << wallFvPatch::typeName << " patches" << endl;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::yPlusLES::yPlusLES
(
    const word& name,
    const objectRegistry& obr,
    const dictionary& dict,
    const bool loadFromFiles
)
:
    functionObjectFile(obr, name, typeName),
    name_(name),
    obr_(obr),
    active_(true),
    log_(true),
    phiName_("phi"),
    UName_("U")
{
    // Check if the available mesh is an fvMesh, otherwise deactivate
    if (!isA<fvMesh>(obr_))
    {
        active_ = false;
        WarningInFunction
            << "No fvMesh available, deactivating " << name_ << nl
            << endl;
    }

    if (active_)
    {
        const fvMesh& mesh = refCast<const fvMesh>(obr_);

        volScalarField* yPlusLESPtr
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
                dimensionedScalar("0", dimless, 0.0)
            )
        );

        mesh.objectRegistry::store(yPlusLESPtr);
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::yPlusLES::~yPlusLES()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::yPlusLES::read(const dictionary& dict)
{
    if (active_)
    {
        log_ = dict.lookupOrDefault<Switch>("log", true);
        phiName_ = dict.lookupOrDefault<word>("phiName", "phi");
    }
}


void CML::yPlusLES::execute()
{
    if (active_)
    {
        functionObjectFile::write();

        const surfaceScalarField& phi =
            obr_.lookupObject<surfaceScalarField>(phiName_);

        const volVectorField& U = obr_.lookupObject<volVectorField>(UName_);

        const fvMesh& mesh = refCast<const fvMesh>(obr_);

        volScalarField& yPlusLES =
            const_cast<volScalarField&>
            (
                mesh.lookupObject<volScalarField>(type())
            );

        Info(log_)<< type() << " " << name_ << " output:" << nl;

        if (phi.dimensions() == dimMass/dimTime)
        {
            calcCompressibleYPlus(mesh, U, yPlusLES);
        }
        else
        {
            calcIncompressibleYPlus(mesh, U, yPlusLES);
        }
    }
}


void CML::yPlusLES::end()
{
    if (active_)
    {
        execute();
    }
}


void CML::yPlusLES::timeSet()
{
    // Do nothing
}


void CML::yPlusLES::write()
{
    if (active_)
    {
        functionObjectFile::write();

        const volScalarField& yPlusLES =
            obr_.lookupObject<volScalarField>(type());

        Info(log_)<< "    writing field " << yPlusLES.name() << nl << endl;

        yPlusLES.write();
    }
}


// ************************************************************************* //
