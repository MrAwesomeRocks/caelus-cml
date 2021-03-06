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

#include "APIdiffCoefFunc.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(APIdiffCoefFunc, 0);
    addToRunTimeSelectionTable
    (
        thermophysicalFunction,
        APIdiffCoefFunc,
        dictionary
    );
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::APIdiffCoefFunc::APIdiffCoefFunc
(
    const scalar a,
    const scalar b,
    const scalar wf,
    const scalar wa
)
:
    a_(a),
    b_(b),
    wf_(wf),
    wa_(wa),
    alpha_(sqrt(1/wf_ + 1/wa_)),
    beta_(sqr(cbrt(a_) + cbrt(b_)))
{}


CML::APIdiffCoefFunc::APIdiffCoefFunc(const dictionary& dict)
:
    a_(readScalar(dict.lookup("a"))),
    b_(readScalar(dict.lookup("b"))),
    wf_(readScalar(dict.lookup("wf"))),
    wa_(readScalar(dict.lookup("wa"))),
    alpha_(sqrt(1/wf_ + 1/wa_)),
    beta_(sqr((cbrt(a_) + cbrt(b_))))
{}
