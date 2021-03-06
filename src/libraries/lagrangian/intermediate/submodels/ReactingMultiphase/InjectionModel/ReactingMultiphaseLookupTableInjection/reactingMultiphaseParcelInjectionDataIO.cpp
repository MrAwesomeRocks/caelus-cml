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

#include "reactingMultiphaseParcelInjectionData.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

CML::reactingMultiphaseParcelInjectionData::
reactingMultiphaseParcelInjectionData(Istream& is)
:
    reactingParcelInjectionData(is)
{
    is.check("reading YGas's");
    is >> YGas_;

    is.check("reading YLiquid's");
    is >> YLiquid_;

    is.check("reading YSolid's");
    is >> YSolid_;

    is.check("reactingMultiphaseParcelInjectionData(Istream& is)");
}


// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

CML::Ostream& CML::operator<<
(
    Ostream& os,
    const reactingMultiphaseParcelInjectionData& data
)
{
    os << static_cast<const reactingParcelInjectionData&>(data);

    os << data.YGas_ << data.YLiquid_ << data.YSolid_;

    return os;
}


CML::Istream& CML::operator>>
(
    Istream& is,
    reactingMultiphaseParcelInjectionData& data
)
{
    is >> static_cast<reactingParcelInjectionData&>(data);

    is.check("reading YGas's");
    is >> data.YGas_;

    is.check("reading YLiquid's");
    is >> data.YLiquid_;

    is.check("reading YSolid's");
    is >> data.YSolid_;

    is.check("operator(Istream&, reactingMultiphaseParcelInjectionData&)");

    return is;
}


// ************************************************************************* //
