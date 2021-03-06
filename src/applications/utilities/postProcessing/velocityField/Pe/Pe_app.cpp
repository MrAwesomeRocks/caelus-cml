/*---------------------------------------------------------------------------* \
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

Application
    Pe

Description
    Calculates the Peclet number Pe from the flux phi and writes the maximum
    value, the surfaceScalarField Pef and volScalarField Pe.

    The -noWrite option just outputs the max/min values without writing
    the field.

\*---------------------------------------------------------------------------*/

#include "calc.hpp"
#include "surfaceInterpolate.hpp"
#include "fvcAverage.hpp"

#include "incompressible/singlePhaseTransportModel/singlePhaseTransportModel.hpp"
#include "incompressible/RAS/RASModel/RASModel.hpp"
#include "incompressible/LES/LESModel/LESModel.hpp"
#include "fluidThermo.hpp"
#include "compressible/RAS/RASModel/compressibleRASModel.hpp"
#include "compressible/LES/LESModel/compressibleLESModel.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void CML::calc(const argList& args, const Time& runTime, const fvMesh& mesh)
{
    bool writeResults = !args.optionFound("noWrite");

    IOobject phiHeader
    (
        "phi",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ
    );

    if (phiHeader.headerOk())
    {
        autoPtr<surfaceScalarField> PePtr;

        Info<< "    Reading phi" << endl;
        surfaceScalarField phi(phiHeader, mesh);

        volVectorField U
        (
            IOobject
            (
                "U",
                runTime.timeName(),
                mesh,
                IOobject::MUST_READ
            ),
            mesh
        );

        IOobject RASPropertiesHeader
        (
            "RASProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE
        );

        IOobject LESPropertiesHeader
        (
            "LESProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE
        );

        Info<< "    Calculating Pe" << endl;

        if (phi.dimensions() == dimensionSet(0, 3, -1, 0, 0))
        {
            if (RASPropertiesHeader.headerOk())
            {
                IOdictionary RASProperties(RASPropertiesHeader);

                singlePhaseTransportModel laminarTransport(U, phi);

                autoPtr<incompressible::RASModel> RASModel
                (
                    incompressible::RASModel::New
                    (
                        U,
                        phi,
                        laminarTransport
                    )
                );

                PePtr.set
                (
                    new surfaceScalarField
                    (
                        IOobject
                        (
                            "Pef",
                            runTime.timeName(),
                            mesh
                        ),
                        mag(phi)
                       /(
                            mesh.magSf()
                          * mesh.surfaceInterpolation::deltaCoeffs()
                          * fvc::interpolate(RASModel->nuEff())
                        )
                    )
                );
            }
            else if (LESPropertiesHeader.headerOk())
            {
                IOdictionary LESProperties(LESPropertiesHeader);

                singlePhaseTransportModel laminarTransport(U, phi);

                autoPtr<incompressible::LESModel> sgsModel
                (
                    incompressible::LESModel::New(U, phi, laminarTransport)
                );

                PePtr.set
                (
                    new surfaceScalarField
                    (
                        IOobject
                        (
                            "Pef",
                            runTime.timeName(),
                            mesh
                        ),
                        mag(phi)
                       /(
                            mesh.magSf()
                          * mesh.surfaceInterpolation::deltaCoeffs()
                          * fvc::interpolate(sgsModel->nuEff())
                        )
                    )
                );
            }
            else
            {
                IOdictionary transportProperties
                (
                    IOobject
                    (
                        "transportProperties",
                        runTime.constant(),
                        mesh,
                        IOobject::MUST_READ_IF_MODIFIED,
                        IOobject::NO_WRITE
                    )
                );

                dimensionedScalar nu(transportProperties.lookup("nu"));

                PePtr.set
                (
                    new surfaceScalarField
                    (
                        IOobject
                        (
                            "Pef",
                            runTime.timeName(),
                            mesh
                        ),
                        mag(phi)
                       /(
                            mesh.magSf()
                          * mesh.surfaceInterpolation::deltaCoeffs()
                          * nu
                        )
                    )
                );
            }
        }
        else if (phi.dimensions() == dimensionSet(1, 0, -1, 0, 0))
        {
            if (RASPropertiesHeader.headerOk())
            {
                IOdictionary RASProperties(RASPropertiesHeader);

                autoPtr<fluidThermo> thermo(fluidThermo::New(mesh));

                volScalarField rho
                (
                    IOobject
                    (
                        "rho",
                        runTime.timeName(),
                        mesh
                    ),
                    thermo->rho()
                );

                autoPtr<compressible::RASModel> RASModel
                (
                    compressible::RASModel::New
                    (
                        rho,
                        U,
                        phi,
                        thermo()
                    )
                );

                PePtr.set
                (
                    new surfaceScalarField
                    (
                        IOobject
                        (
                            "Pef",
                            runTime.timeName(),
                            mesh
                        ),
                        mag(phi)
                       /(
                            mesh.magSf()
                          * mesh.surfaceInterpolation::deltaCoeffs()
                          * fvc::interpolate(RASModel->muEff())
                        )
                    )
                );
            }
            else if (LESPropertiesHeader.headerOk())
            {
                IOdictionary LESProperties(LESPropertiesHeader);

                autoPtr<fluidThermo> thermo(fluidThermo::New(mesh));

                volScalarField rho
                (
                    IOobject
                    (
                        "rho",
                        runTime.timeName(),
                        mesh
                    ),
                    thermo->rho()
                );

                autoPtr<compressible::LESModel> sgsModel
                (
                    compressible::LESModel::New(rho, U, phi, thermo())
                );

                PePtr.set
                (
                    new surfaceScalarField
                    (
                        IOobject
                        (
                            "Pef",
                            runTime.timeName(),
                            mesh
                        ),
                        mag(phi)
                       /(
                            mesh.magSf()
                          * mesh.surfaceInterpolation::deltaCoeffs()
                          * fvc::interpolate(sgsModel->muEff())
                        )
                    )
                );
            }
            else
            {
                IOdictionary transportProperties
                (
                    IOobject
                    (
                        "transportProperties",
                        runTime.constant(),
                        mesh,
                        IOobject::MUST_READ_IF_MODIFIED,
                        IOobject::NO_WRITE
                    )
                );

                dimensionedScalar mu(transportProperties.lookup("mu"));

                PePtr.set
                (
                    new surfaceScalarField
                    (
                        IOobject
                        (
                            "Pef",
                            runTime.timeName(),
                            mesh
                        ),
                        mag(phi)
                       /(
                            mesh.magSf()
                          * mesh.surfaceInterpolation::deltaCoeffs()
                          * mu
                        )
                    )
                );
            }
        }
        else
        {
            FatalErrorInFunction
                << "Incorrect dimensions of phi: " << phi.dimensions()
                    << abort(FatalError);
        }

        Info<< "Pe max : " << max(PePtr()).value() << endl;

        if (writeResults)
        {
            Info<< "    Writing surfaceScalarField : "
                << PePtr().name() << endl;
            PePtr().write();

            volScalarField Pe
            (
                IOobject
                (
                    "Pe",
                    runTime.timeName(),
                    mesh
                ),
                fvc::average(PePtr())
            );

            Info<< "    Writing volScalarField : "
                << Pe.name() << endl;
            Pe.write();
        }
    }
    else
    {
        Info<< "    No phi" << endl;
    }
}


// ************************************************************************* //
