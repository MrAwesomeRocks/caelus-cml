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

#include "NSRDSfunc14.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(NSRDSfunc14, 0);
    addToRunTimeSelectionTable
    (
        thermophysicalFunction,
        NSRDSfunc14,
        dictionary
    );
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::NSRDSfunc14::NSRDSfunc14
(
    const scalar Tc,
    const scalar a,
    const scalar b,
    const scalar c,
    const scalar d
)
:
    Tc_(Tc),
    a_(a),
    b_(b),
    c_(c),
    d_(d)
{}


CML::NSRDSfunc14::NSRDSfunc14(const dictionary& dict)
:
    Tc_(readScalar(dict.lookup("Tc"))),
    a_(readScalar(dict.lookup("a"))),
    b_(readScalar(dict.lookup("b"))),
    c_(readScalar(dict.lookup("c"))),
    d_(readScalar(dict.lookup("d")))
{}
