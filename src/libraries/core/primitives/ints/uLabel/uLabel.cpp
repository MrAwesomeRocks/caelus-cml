/*---------------------------------------------------------------------------*\
Copyright (C) 2011 OpenFOAM Foundation
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

#include "uLabel.hpp"
#include "error.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

const char* const pTraits<uLabel>::typeName = "uLabel";
const uLabel pTraits<uLabel>::zero = 0;
const uLabel pTraits<uLabel>::one = 1;
const uLabel pTraits<uLabel>::min = uLabelMin;
const uLabel pTraits<uLabel>::max = uLabelMax;
const uLabel pTraits<uLabel>::rootMin = pTraits<uLabel>::min;
const uLabel pTraits<uLabel>::rootMax = pTraits<uLabel>::max;

const char* pTraits<uLabel>::componentNames[] = { "x" };

pTraits<uLabel>::pTraits(const uLabel& p)
:
    p_(p)
{}


pTraits<uLabel>::pTraits(Istream& is)
{
    is >> p_;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
