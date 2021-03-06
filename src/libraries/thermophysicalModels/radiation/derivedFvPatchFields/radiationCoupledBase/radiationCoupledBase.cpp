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

#include "radiationCoupledBase.hpp"
#include "volFields.hpp"
#include "mappedPatchBase.hpp"
#include "fvPatchFieldMapper.hpp"
#include "radiationModel.hpp"
#include "opaqueSolid.hpp"
#include "absorptionEmissionModel.hpp"

// * * * * * * * * * * * * * Static Member Data  * * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(radiationCoupledBase, 0);

    template<>
    const char* CML::NamedEnum
    <
        CML::radiationCoupledBase::emissivityMethodType,
        2
    >::names[] =
    {
        "solidRadiation",
        "lookup"
    };
}

const CML::NamedEnum<CML::radiationCoupledBase::emissivityMethodType, 2>
    CML::radiationCoupledBase::emissivityMethodTypeNames_;


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::radiationCoupledBase::radiationCoupledBase
(
    const fvPatch& patch,
    const word& calculationType,
    const scalarField& emissivity
)
:
    patch_(patch),
    method_(emissivityMethodTypeNames_[calculationType]),
    emissivity_(emissivity)
{}


CML::radiationCoupledBase::radiationCoupledBase
(
    const fvPatch& patch,
    const word& calculationType,
    const scalarField& emissivity,
    const fvPatchFieldMapper& mapper
)
:
    patch_(patch),
    method_(emissivityMethodTypeNames_[calculationType]),
    emissivity_(emissivity, mapper)
{}


CML::radiationCoupledBase::radiationCoupledBase
(
    const fvPatch& patch,
    const dictionary& dict
)
:
    patch_(patch),
    method_(emissivityMethodTypeNames_.read(dict.lookup("emissivityMode")))
{
    switch (method_)
    {
        case SOLIDRADIATION:
        {
            if (!isA<mappedPatchBase>(patch_.patch()))
            {
                FatalIOErrorInFunction
                (
                    dict
                )   << "\n    patch type '" << patch_.type()
                    << "' not type '" << mappedPatchBase::typeName << "'"
                    << "\n    for patch " << patch_.name()
                    << exit(FatalIOError);
            }

            emissivity_ = scalarField(patch_.size(), 0.0);
        }
        break;

        case LOOKUP:
        {
            if (!dict.found("emissivity"))
            {
                FatalIOErrorInFunction
                (
                    dict
                )   << "\n    emissivity key does not exist for patch "
                    << patch_.name()
                    << exit(FatalIOError);
            }
            else
            {
                emissivity_ = scalarField("emissivity", dict, patch_.size());
            }
        }
        break;
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::scalarField CML::radiationCoupledBase::emissivity() const
{
    switch (method_)
    {
        case SOLIDRADIATION:
        {
            // Get the coupling information from the mappedPatchBase
            const mappedPatchBase& mpp =
                refCast<const mappedPatchBase>(patch_.patch());

            const polyMesh& nbrMesh = mpp.sampleMesh();

            const radiation::opaqueSolid& radiation =
                nbrMesh.lookupObject<radiation::opaqueSolid>
                (
                    "radiationProperties"
                );

            const fvMesh& nbrFvMesh = refCast<const fvMesh>(nbrMesh);

            const fvPatch& nbrPatch =
                nbrFvMesh.boundary()[mpp.samplePolyPatch().index()];

            // NOTE: for an opaqueSolid the absorptionEmission model returns the
            // emissivity of the surface rather than the emission coefficient
            // and the input specification MUST correspond to this.
            scalarField emissivity
            (
                radiation.absorptionEmission().e()().boundaryField()
                [
                    nbrPatch.index()
                ]
            );
            mpp.distribute(emissivity);

            return emissivity;

        }
        break;

        case LOOKUP:
        {
            // return local value
            return emissivity_;
        }

        default:
        {
            FatalErrorInFunction
                << "Unimplemented method " << method_ << endl
                << "Please set 'emissivity' to one of "
                << emissivityMethodTypeNames_.toc()
                << exit(FatalError);
        }
        break;
    }

    return scalarField(0);
}


void CML::radiationCoupledBase::autoMap
(
    const fvPatchFieldMapper& m
)
{
    emissivity_.autoMap(m);
}


void CML::radiationCoupledBase::rmap
(
    const fvPatchScalarField& ptf,
    const labelList& addr
)
{
    const radiationCoupledBase& mrptf =
        refCast<const radiationCoupledBase>(ptf);

    emissivity_.rmap(mrptf.emissivity_, addr);
}


void CML::radiationCoupledBase::write(Ostream& os) const
{
    os.writeKeyword("emissivityMode") << emissivityMethodTypeNames_[method_]
        << token::END_STATEMENT << nl;
    emissivity_.writeEntry("emissivity", os);
}
