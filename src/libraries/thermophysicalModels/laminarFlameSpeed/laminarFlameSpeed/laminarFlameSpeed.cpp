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

#include "laminarFlameSpeed.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(laminarFlameSpeed, 0);

    defineRunTimeSelectionTable(laminarFlameSpeed, dictionary);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::laminarFlameSpeed::laminarFlameSpeed
(
    const dictionary& dict,
    const psiuReactionThermo& ct
)
:
    psiuReactionThermo_(ct),
    fuel_(dict.lookup("fuel")),
    equivalenceRatio_(0)
{
    if (!psiuReactionThermo_.composition().contains("ft"))
    {
        equivalenceRatio_ =
            dimensionedScalar(dict.lookup("equivalenceRatio")).value();
    }
}
