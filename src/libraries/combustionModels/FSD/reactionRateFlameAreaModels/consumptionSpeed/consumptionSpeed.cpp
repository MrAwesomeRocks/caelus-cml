/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2018 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of CAELUS.

    CAELUS is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    CAELUS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with CAELUS; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\*---------------------------------------------------------------------------*/

#include "consumptionSpeed.hpp"

/* * * * * * * * * * * * * * * private static data * * * * * * * * * * * * * */

namespace CML
{
    defineTypeNameAndDebug(consumptionSpeed, 0);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::consumptionSpeed::consumptionSpeed
(
    const dictionary& dict
)
:   omega0_(readScalar(dict.lookup("omega0"))),
    eta_(readScalar(dict.lookup("eta"))),
    sigmaExt_(readScalar(dict.lookup("sigmaExt"))),
    omegaMin_(readScalar(dict.lookup("omegaMin")))
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

CML::scalar CML::consumptionSpeed::omega0Sigma
(
    scalar sigma,
    scalar a
) const
{
    scalar omega0 = 0.0;

    if (sigma < sigmaExt_)
    {
        omega0 = max
        (
            a*omega0_*(1.0 - exp(eta_*max(sigma, 0.0))),
            omegaMin_
        ) ;
    }

    return omega0;
}


CML::tmp<CML::volScalarField> CML::consumptionSpeed::omega0Sigma
(
    const volScalarField& sigma
)
{
    tmp<volScalarField> tomega0
    (
        new volScalarField
        (
            IOobject
            (
                "omega0",
                sigma.time().timeName(),
                sigma.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            sigma.mesh(),
            dimensionedScalar
            (
                "omega0",
                dimensionSet(1, -2, -1, 0, 0, 0, 0),
                0
            )
        )
    );

    volScalarField& omega0 = tomega0();

    volScalarField::InternalField& iomega0 = omega0.internalField();

    forAll(iomega0, celli)
    {
        iomega0[celli] = omega0Sigma(sigma[celli], 1.0);
    }

    volScalarField::GeometricBoundaryField& bomega0 = omega0.boundaryField();

    forAll(bomega0, patchi)
    {
        forAll(bomega0[patchi], facei)
        {
            bomega0[patchi][facei] =
                omega0Sigma
                (
                    sigma.boundaryField()[patchi][facei],
                    1.0
                );
        }
    }

    return tomega0;
}


void  CML::consumptionSpeed::read(const dictionary& dict)
{
    dict.lookup("omega0") >> omega0_ ;
    dict.lookup("eta") >> eta_ ;
    dict.lookup("sigmaExt") >> sigmaExt_;
    dict.lookup("omegaMin") >> omegaMin_;
}
