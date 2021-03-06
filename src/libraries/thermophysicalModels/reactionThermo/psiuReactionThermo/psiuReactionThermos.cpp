/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2018 OpenFOAM Foundation
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

#include "psiuReactionThermo.hpp"
#include "heheuPsiThermo.hpp"

#include "makeReactionThermo.hpp"
#include "addToRunTimeSelectionTable.hpp"

#include "specie.hpp"
#include "perfectGas.hpp"
#include "hConstThermo.hpp"
#include "janafThermo.hpp"
#include "specieThermo.hpp"
#include "constTransport.hpp"
#include "sutherlandTransport.hpp"

#include "absoluteEnthalpy.hpp"
#include "absoluteInternalEnergy.hpp"

#include "homogeneousMixture.hpp"
#include "inhomogeneousMixture.hpp"
#include "veryInhomogeneousMixture.hpp"
#include "multiComponentMixture.hpp"
#include "egrMixture.hpp"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * h-hu-Thermos * * * * * * * * * * * * * * * //

makeReactionThermos
(
    psiThermo,
    psiuReactionThermo,
    heheuPsiThermo,
    homogeneousMixture,
    constTransport,
    absoluteEnthalpy,
    hConstThermo,
    perfectGas,
    specie
);

makeReactionThermos
(
    psiThermo,
    psiuReactionThermo,
    heheuPsiThermo,
    inhomogeneousMixture,
    constTransport,
    absoluteEnthalpy,
    hConstThermo,
    perfectGas,
    specie
);

makeReactionThermos
(
    psiThermo,
    psiuReactionThermo,
    heheuPsiThermo,
    veryInhomogeneousMixture,
    constTransport,
    absoluteEnthalpy,
    hConstThermo,
    perfectGas,
    specie
);

makeReactionThermos
(
    psiThermo,
    psiuReactionThermo,
    heheuPsiThermo,
    egrMixture,
    constTransport,
    absoluteEnthalpy,
    hConstThermo,
    perfectGas,
    specie
);


makeReactionThermos
(
    psiThermo,
    psiuReactionThermo,
    heheuPsiThermo,
    homogeneousMixture,
    constTransport,
    absoluteEnthalpy,
    janafThermo,
    perfectGas,
    specie
);

makeReactionThermos
(
    psiThermo,
    psiuReactionThermo,
    heheuPsiThermo,
    homogeneousMixture,
    sutherlandTransport,
    absoluteEnthalpy,
    janafThermo,
    perfectGas,
    specie
);

makeReactionThermos
(
    psiThermo,
    psiuReactionThermo,
    heheuPsiThermo,
    inhomogeneousMixture,
    sutherlandTransport,
    absoluteEnthalpy,
    janafThermo,
    perfectGas,
    specie
);

makeReactionThermos
(
    psiThermo,
    psiuReactionThermo,
    heheuPsiThermo,
    veryInhomogeneousMixture,
    sutherlandTransport,
    absoluteEnthalpy,
    janafThermo,
    perfectGas,
    specie
);

makeReactionThermos
(
    psiThermo,
    psiuReactionThermo,
    heheuPsiThermo,
    egrMixture,
    sutherlandTransport,
    absoluteEnthalpy,
    janafThermo,
    perfectGas,
    specie
);


makeReactionThermos
(
    psiThermo,
    psiuReactionThermo,
    heheuPsiThermo,
    homogeneousMixture,
    sutherlandTransport,
    absoluteInternalEnergy,
    janafThermo,
    perfectGas,
    specie
);

makeReactionThermos
(
    psiThermo,
    psiuReactionThermo,
    heheuPsiThermo,
    inhomogeneousMixture,
    sutherlandTransport,
    absoluteInternalEnergy,
    janafThermo,
    perfectGas,
    specie
);

makeReactionThermos
(
    psiThermo,
    psiuReactionThermo,
    heheuPsiThermo,
    veryInhomogeneousMixture,
    sutherlandTransport,
    absoluteInternalEnergy,
    janafThermo,
    perfectGas,
    specie
);

makeReactionThermos
(
    psiThermo,
    psiuReactionThermo,
    heheuPsiThermo,
    egrMixture,
    sutherlandTransport,
    absoluteInternalEnergy,
    janafThermo,
    perfectGas,
    specie
);


} // End namespace CML
