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

#ifndef makeChemistrySolverTypes_HPP
#define makeChemistrySolverTypes_HPP

#include "chemistrySolver.hpp"

#include "StandardChemistryModel.hpp"
#include "TDACChemistryModel.hpp"

#include "noChemistrySolver.hpp"
#include "EulerImplicit.hpp"
#include "ode_.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define makeChemistrySolverType(SS, Comp, Thermo)                              \
                                                                               \
    typedef SS<StandardChemistryModel<Comp, Thermo>> SS##Comp##Thermo;         \
                                                                               \
    defineTemplateTypeNameAndDebugWithName                                     \
    (                                                                          \
        SS##Comp##Thermo,                                                      \
        (#SS"<" + word(StandardChemistryModel<Comp, Thermo>::typeName_()) + "<"\
        + word(Comp::typeName_()) + "," + Thermo::typeName() + ">>").c_str(),  \
        0                                                                      \
    );                                                                         \
                                                                               \
    BasicChemistryModel<Comp>::                                                \
        add##thermo##ConstructorToTable<SS##Comp##Thermo>                      \
        add##SS##Comp##Thermo##thermo##ConstructorTo##BasicChemistryModel##Comp\
##Table_; \
                                                                               \
    typedef SS<TDACChemistryModel<Comp, Thermo>> TDAC##SS##Comp##Thermo;       \
                                                                               \
    defineTemplateTypeNameAndDebugWithName                                     \
    (                                                                          \
        TDAC##SS##Comp##Thermo,                                                \
        (#SS"<" + word(TDACChemistryModel<Comp, Thermo>::typeName_()) + "<"    \
        + word(Comp::typeName_()) + "," + Thermo::typeName() + ">>").c_str(),  \
        0                                                                      \
    );                                                                         \
                                                                               \
    BasicChemistryModel<Comp>::                                                \
        add##thermo##ConstructorToTable<TDAC##SS##Comp##Thermo>                \
        add##TDAC##SS##Comp##Thermo##thermo##ConstructorTo##BasicChemistryModel\
##Comp##Table_;


#define makeChemistrySolverTypes(Comp, Thermo)                                 \
                                                                               \
    makeChemistrySolverType                                                    \
    (                                                                          \
        noChemistrySolver,                                                     \
        Comp,                                                                  \
        Thermo                                                                 \
    );                                                                         \
                                                                               \
    makeChemistrySolverType                                                    \
    (                                                                          \
        EulerImplicit,                                                         \
        Comp,                                                                  \
        Thermo                                                                 \
    );                                                                         \
                                                                               \
    makeChemistrySolverType                                                    \
    (                                                                          \
        ode,                                                                   \
        Comp,                                                                  \
        Thermo                                                                 \
    );                                                                         \


#endif
