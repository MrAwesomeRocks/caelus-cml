/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright  held by original author
-------------------------------------------------------------------------------
License
    This file is based on CAELUS.

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

Contributors/Copyright:
    2012-2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#include "EvolveReactingMultiphaseCloudFunctionObject.hpp"
#include "addToRunTimeSelectionTable.hpp"

#include "polyMesh.hpp"
#include "IOmanip.hpp"
#include "Time.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{

    defineTypeNameAndDebug(EvolveReactingMultiphaseCloudFunctionObject, 0);

    addNamedToRunTimeSelectionTable
    (
        functionObject,
        EvolveReactingMultiphaseCloudFunctionObject,
        dictionary,
        evolveReactingMultiphaseCloud
    );

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

EvolveReactingMultiphaseCloudFunctionObject
::EvolveReactingMultiphaseCloudFunctionObject
(
    const word& name,
    const Time& t,
    const dictionary& dict
)
:
    EvolveCloudFunctionObject<
    basicReactingMultiphaseCloud
    >(
        name,
        t,
        dict
    )
{
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

#define TEMPLATE
bool EvolveReactingMultiphaseCloudFunctionObject
::start()
{
    this->cloud().set(
        new basicReactingMultiphaseCloud(
            this->cloudName(),
            this->TEMPLATE getField<volScalarField>("rhoName"),
            this->TEMPLATE getField<volVectorField>("UName"),
            this->g(),
            const_cast<SLGThermo &>(
                this->getField<SLGThermo>("SLGThermoPhysicsName")
            )
        )
    );

    return true;
}


} // namespace CML

// ************************************************************************* //
