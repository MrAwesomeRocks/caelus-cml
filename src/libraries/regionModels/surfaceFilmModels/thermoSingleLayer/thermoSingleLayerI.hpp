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

#include "thermoSingleLayer.hpp"
#include "filmRadiationModel.hpp"
#include "heatTransferModel.hpp"
#include "phaseChangeModel.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace regionModels
{
namespace surfaceFilmModels
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

inline const SLGThermo& thermoSingleLayer::thermo() const
{
    return thermo_;
}


inline tmp<scalarField> thermoSingleLayer::hs
(
    const scalarField& T,
    const label patchi
) const
{
    const scalarField& Cp = Cp_.boundaryField()[patchi];
    return Cp*(T - Tref.value());
}


inline tmp<volScalarField> thermoSingleLayer::hs
(
    const volScalarField& T
) const
{
    return tmp<volScalarField>
    (
        new volScalarField
        (
            IOobject
            (
                "hs(" + T.name() + ")",
                time().timeName(),
                regionMesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            Cp_*(T - Tref)
        )
    );
}


inline tmp<volScalarField> thermoSingleLayer::T
(
    const volScalarField& hs
) const
{
    tmp<volScalarField> tT
    (
        new volScalarField
        (
            IOobject
            (
                "T(" + hs.name() + ")",
                time().timeName(),
                regionMesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            hs/Cp_ + Tref
        )
    );

    tT().min(Tmax_);
    tT().max(Tmin_);

    return tT;
}


inline const volScalarField& thermoSingleLayer::hsSp() const
{
    return hsSp_;
}


inline const volScalarField& thermoSingleLayer::hsSpPrimary() const
{
    return hsSpPrimary_;
}


inline const volScalarField& thermoSingleLayer::TPrimary() const
{
    return TPrimary_;
}


inline const PtrList<volScalarField>& thermoSingleLayer::YPrimary() const
{
    return YPrimary_;
}


inline const heatTransferModel& thermoSingleLayer::htcs() const
{
    return htcs_();
}


inline const heatTransferModel& thermoSingleLayer::htcw() const
{
    return htcw_();
}


inline const phaseChangeModel& thermoSingleLayer::phaseChange() const
{
    return phaseChange_();
}


inline const filmRadiationModel& thermoSingleLayer::radiation() const
{
    return radiation_();
}


inline tmp<scalarField> thermoSingleLayer::Qconvw(const label patchi) const
{
    const scalarField htc(htcw_->h()().boundaryField()[patchi]);
    const scalarField& Tp = T_.boundaryField()[patchi];
    const scalarField& Twp = Tw_.boundaryField()[patchi];

    return htc*(Tp - Twp);
}


inline tmp<scalarField> thermoSingleLayer::Qconvp(const label patchi) const
{
    const scalarField htc(htcs_->h()().boundaryField()[patchi]);
    const scalarField& Tp = T_.boundaryField()[patchi];
    const scalarField& Tpp = TPrimary_.boundaryField()[patchi];

    return htc*(Tp - Tpp);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmModels
} // End namespace regionModels
} // End namespace CML

// ************************************************************************* //
