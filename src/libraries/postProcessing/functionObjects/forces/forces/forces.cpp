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

#include "forces.hpp"
#include "volFields.hpp"
#include "dictionary.hpp"
#include "Time.hpp"
#include "wordReList.hpp"

#include "incompressible/singlePhaseTransportModel/singlePhaseTransportModel.hpp"
#include "incompressible/RAS/RASModel/RASModel.hpp"
#include "incompressible/LES/LESModel/LESModel.hpp"

#include "basicThermo.hpp"
#include "compressible/RAS/RASModel/RASModel.hpp"
#include "compressible/LES/LESModel/LESModel.hpp"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(CML::forces, 0);


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

CML::tmp<CML::volSymmTensorField> CML::forces::devRhoReff() const
{
    if (obr_.foundObject<compressible::RASModel>("RASProperties"))
    {
        const compressible::RASModel& ras
            = obr_.lookupObject<compressible::RASModel>("RASProperties");

        return ras.devRhoReff();
    }
    else if (obr_.foundObject<incompressible::RASModel>("RASProperties"))
    {
        const incompressible::RASModel& ras
            = obr_.lookupObject<incompressible::RASModel>("RASProperties");

        return rho()*ras.devReff();
    }
    else if (obr_.foundObject<compressible::LESModel>("LESProperties"))
    {
        const compressible::LESModel& les =
        obr_.lookupObject<compressible::LESModel>("LESProperties");

        return les.devRhoBeff();
    }
    else if (obr_.foundObject<incompressible::LESModel>("LESProperties"))
    {
        const incompressible::LESModel& les
            = obr_.lookupObject<incompressible::LESModel>("LESProperties");

        return rho()*les.devReff();
    }
    else if (obr_.foundObject<basicThermo>("thermophysicalProperties"))
    {
        const basicThermo& thermo =
             obr_.lookupObject<basicThermo>("thermophysicalProperties");

        const volVectorField& U = obr_.lookupObject<volVectorField>(UName_);

        return -thermo.mu()*dev(twoSymm(fvc::grad(U)));
    }
    else if
    (
        obr_.foundObject<singlePhaseTransportModel>("transportProperties")
    )
    {
        const singlePhaseTransportModel& laminarT =
            obr_.lookupObject<singlePhaseTransportModel>
            ("transportProperties");

        const volVectorField& U = obr_.lookupObject<volVectorField>(UName_);

        return -rho()*laminarT.nu()*dev(twoSymm(fvc::grad(U)));
    }
    else if (obr_.foundObject<dictionary>("transportProperties"))
    {
        const dictionary& transportProperties =
             obr_.lookupObject<dictionary>("transportProperties");

        dimensionedScalar nu(transportProperties.lookup("nu"));

        const volVectorField& U = obr_.lookupObject<volVectorField>(UName_);

        return -rho()*nu*dev(twoSymm(fvc::grad(U)));
    }
    else
    {
        FatalErrorIn("forces::devRhoReff()")
            << "No valid model for viscous stress calculation."
            << exit(FatalError);

        return volSymmTensorField::null();
    }
}


CML::tmp<CML::volScalarField> CML::forces::rho() const
{
    if (rhoName_ == "rhoInf")
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
                    mesh
                ),
                mesh,
                dimensionedScalar("rho", dimDensity, rhoRef_)
            )
        );
    }
    else
    {
        return(obr_.lookupObject<volScalarField>(rhoName_));
    }
}


CML::scalar CML::forces::rho(const volScalarField& p) const
{
    if (p.dimensions() == dimPressure)
    {
        return 1.0;
    }
    else
    {
        if (rhoName_ != "rhoInf")
        {
            FatalErrorIn("forces::rho(const volScalarField& p)")
                << "Dynamic pressure is expected but kinematic is provided."
                << exit(FatalError);
        }

        return rhoRef_;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::forces::forces
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
    log_(false),
    patchSet_(),
    pName_(word::null),
    UName_(word::null),
    rhoName_(word::null),
    directForceDensity_(false),
    fDName_(""),
    rhoRef_(VGREAT),
    pRef_(0),
    coordSys_(),
    localSystem_(false),
    forcesFilePtr_(NULL)
{
    // Check if the available mesh is an fvMesh otherise deactivate
    if (!isA<fvMesh>(obr_))
    {
        active_ = false;
        WarningIn
        (
            "CML::forces::forces"
            "("
                "const word&, "
                "const objectRegistry&, "
                "const dictionary&, "
                "const bool"
            ")"
        )   << "No fvMesh available, deactivating."
            << endl;
    }

    read(dict);
}


CML::forces::forces
(
    const word& name,
    const objectRegistry& obr,
    const labelHashSet& patchSet,
    const word& pName,
    const word& UName,
    const word& rhoName,
    const scalar rhoInf,
    const scalar pRef,
    const coordinateSystem& coordSys
)
:
    name_(name),
    obr_(obr),
    active_(true),
    log_(false),
    patchSet_(patchSet),
    pName_(pName),
    UName_(UName),
    rhoName_(rhoName),
    directForceDensity_(false),
    fDName_(""),
    rhoRef_(rhoInf),
    pRef_(pRef),
    coordSys_(coordSys),
    localSystem_(false),
    forcesFilePtr_(NULL)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::forces::~forces()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::forces::read(const dictionary& dict)
{
    if (active_)
    {
        log_ = dict.lookupOrDefault<Switch>("log", false);
        directForceDensity_ = dict.lookupOrDefault("directForceDensity", false);

        const fvMesh& mesh = refCast<const fvMesh>(obr_);

        patchSet_ = mesh.boundaryMesh().patchSet
        (
            wordReList(dict.lookup("patches"))
        );

        if (directForceDensity_)
        {
            // Optional entry for fDName
            fDName_ = dict.lookupOrDefault<word>("fDName", "fD");

            // Check whether fDName exists, if not deactivate forces
            if
            (
                !obr_.foundObject<volVectorField>(fDName_)
            )
            {
                active_ = false;
                WarningIn("void forces::read(const dictionary&)")
                    << "Could not find " << fDName_ << " in database." << nl
                    << "    De-activating forces."
                    << endl;
            }
        }
        else
        {
            // Optional entries U and p
            pName_ = dict.lookupOrDefault<word>("pName", "p");
            UName_ = dict.lookupOrDefault<word>("UName", "U");
            rhoName_ = dict.lookupOrDefault<word>("rhoName", "rho");

            // Check whether UName, pName and rhoName exists,
            // if not deactivate forces
            if
            (
                !obr_.foundObject<volVectorField>(UName_)
             || !obr_.foundObject<volScalarField>(pName_)
             || (
                    rhoName_ != "rhoInf"
                 && !obr_.foundObject<volScalarField>(rhoName_)
                )
            )
            {
                active_ = false;

                WarningIn("void forces::read(const dictionary&)")
                    << "Could not find " << UName_ << ", " << pName_;

                if (rhoName_ != "rhoInf")
                {
                    Info<< " or " << rhoName_;
                }

                Info<< " in database." << nl
                    << "    De-activating forces." << endl;
            }

            // Reference density needed for incompressible calculations
            rhoRef_ = readScalar(dict.lookup("rhoInf"));

            // Reference pressure, 0 by default
            pRef_ = dict.lookupOrDefault<scalar>("pRef", 0.0);
        }

        coordSys_.clear();

        // Centre of rotation for moment calculations
        // specified directly, from coordinate system, or implicitly (0 0 0)
        if (!dict.readIfPresent<point>("CofR", coordSys_.origin()))
        {
            coordSys_ = coordinateSystem(obr_, dict);
            localSystem_ = true;
        }
    }
}


void CML::forces::makeFile()
{
    // Create the forces file if not already created
    if (forcesFilePtr_.empty())
    {
        if (debug)
        {
            Info<< "Creating forces file." << endl;
        }

        // File update
        if (Pstream::master())
        {
            fileName forcesDir;
            word startTimeName =
                obr_.time().timeName(obr_.time().startTime().value());

            if (Pstream::parRun())
            {
                // Put in undecomposed case (Note: gives problems for
                // distributed data running)
                forcesDir = obr_.time().path()/".."/name_/startTimeName;
            }
            else
            {
                forcesDir = obr_.time().path()/name_/startTimeName;
            }

            // Create directory if does not exist.
            mkDir(forcesDir);

            // Open new file at start up
            forcesFilePtr_.reset(new OFstream(forcesDir/(type() + ".dat")));

            // Add headers to output data
            writeFileHeader();
        }
    }
}


void CML::forces::writeFileHeader()
{
    if (forcesFilePtr_.valid())
    {
        forcesFilePtr_()
            << "# Time" << tab
            << "forces(pressure, viscous) moment(pressure, viscous)";

        if (localSystem_)
        {
            forcesFilePtr_()
                << tab
                << "local forces(pressure, viscous) "
                << "local moment(pressure, viscous)";
        }

        forcesFilePtr_()<< endl;
    }
}


void CML::forces::execute()
{
    // Do nothing - only valid on write
}


void CML::forces::end()
{
    // Do nothing - only valid on write
}


void CML::forces::write()
{
    if (active_)
    {
        // Create the forces file if not already created
        makeFile();

        forcesMoments fm = calcForcesMoment();

        if (Pstream::master())
        {
            if (log_)
            {
                Info<< "forces output:" << nl
                    << "    forces(pressure, viscous)" << fm.first() << nl
                    << "    moment(pressure, viscous)" << fm.second() << nl;
            }

            forcesFilePtr_() << obr_.time().value() << tab << fm;

            if (localSystem_)
            {
                forcesMoments fmLocal;

                fmLocal.first().first() =
                    coordSys_.localVector(fm.first().first());

                fmLocal.first().second() =
                    coordSys_.localVector(fm.first().second());

                fmLocal.second().first() =
                    coordSys_.localVector(fm.second().first());

                fmLocal.second().second() =
                    coordSys_.localVector(fm.second().second());

                forcesFilePtr_() << tab << fmLocal;

                if (log_)
                {
                    Info<< "  local:" << nl
                        << "    forces(pressure, viscous)" << fmLocal.first()
                        << nl
                        << "    moment(pressure, viscous)" << fmLocal.second()
                        << nl;
                }
            }


            forcesFilePtr_() << endl;

            if (log_)
            {
                Info<< endl;
            }
        }
    }
}


CML::forces::forcesMoments CML::forces::calcForcesMoment() const
{
    forcesMoments fm
    (
        pressureViscous(vector::zero, vector::zero),
        pressureViscous(vector::zero, vector::zero)
    );

    if (directForceDensity_)
    {
        const volVectorField& fD = obr_.lookupObject<volVectorField>(fDName_);

        const fvMesh& mesh = fD.mesh();

        const surfaceVectorField::GeometricBoundaryField& Sfb =
            mesh.Sf().boundaryField();

        forAllConstIter(labelHashSet, patchSet_, iter)
        {
            label patchi = iter.key();

            vectorField Md
            (
                mesh.C().boundaryField()[patchi] - coordSys_.origin()
            );

            scalarField sA(mag(Sfb[patchi]));

            // Normal force = surfaceUnitNormal * (surfaceNormal & forceDensity)
            vectorField fN
            (
                Sfb[patchi]/sA
               *(
                    Sfb[patchi] & fD.boundaryField()[patchi]
                )
            );

            fm.first().first() += sum(fN);
            fm.second().first() += sum(Md ^ fN);

            // Tangential force (total force minus normal fN)
            vectorField fT(sA*fD.boundaryField()[patchi] - fN);

            fm.first().second() += sum(fT);
            fm.second().second() += sum(Md ^ fT);
        }
    }
    else
    {
        const volVectorField& U = obr_.lookupObject<volVectorField>(UName_);
        const volScalarField& p = obr_.lookupObject<volScalarField>(pName_);

        const fvMesh& mesh = U.mesh();

        const surfaceVectorField::GeometricBoundaryField& Sfb =
            mesh.Sf().boundaryField();

        tmp<volSymmTensorField> tdevRhoReff = devRhoReff();
        const volSymmTensorField::GeometricBoundaryField& devRhoReffb
            = tdevRhoReff().boundaryField();

        // Scale pRef by density for incompressible simulations
        scalar pRef = pRef_/rho(p);

        forAllConstIter(labelHashSet, patchSet_, iter)
        {
            label patchi = iter.key();

            vectorField Md
            (
                mesh.C().boundaryField()[patchi] - coordSys_.origin()
            );

            vectorField pf(Sfb[patchi]*(p.boundaryField()[patchi] - pRef));

            fm.first().first() += rho(p)*sum(pf);
            fm.second().first() += rho(p)*sum(Md ^ pf);

            vectorField vf(Sfb[patchi] & devRhoReffb[patchi]);

            fm.first().second() += sum(vf);
            fm.second().second() += sum(Md ^ vf);
        }
    }

    reduce(fm, sumOp());

    return fm;
}


// ************************************************************************* //
