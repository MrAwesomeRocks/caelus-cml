/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright (C) 1991-2008 OpenCFD Ltd.
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
    2014 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#include "vectorEntryToExpression.hpp"

#include "addToRunTimeSelectionTable.hpp"

#include "vector.hpp"

#include "primitiveEntry.hpp"

namespace CML {

defineTypeNameAndDebug(vectorEntryToExpression,0);

addNamedToRunTimeSelectionTable(entryToExpression,vectorEntryToExpression, nothing,vector);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

vectorEntryToExpression::vectorEntryToExpression()
    :
    entryToExpression()
{
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

vectorEntryToExpression::~vectorEntryToExpression()
{
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

string vectorEntryToExpression::toExpr(const entry &e)
{
    const primitiveEntry &pe=dynamicCast<const primitiveEntry&>(e);

    return toStr(vector(pe.stream()),"vector");
}

// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //

} // namespace

// ************************************************************************* //
