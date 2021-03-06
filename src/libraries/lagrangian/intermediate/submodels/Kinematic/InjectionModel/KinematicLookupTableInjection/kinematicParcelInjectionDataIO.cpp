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

#include "kinematicParcelInjectionData.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

CML::kinematicParcelInjectionData::kinematicParcelInjectionData(Istream& is)
{
    is.check("reading (Px Py Pz)");
    is >> x_;

    is.check("reading (Ux Uy Uz)");
    is >> U_;

    is.check("reading d");
    is >> d_;

    is.check("reading rho");
    is >> rho_;

    is.check("reading mDot");
    is >> mDot_;

    is.check("kinematicParcelInjectionData(Istream& is)");
}


// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

CML::Ostream& CML::operator<<
(
    Ostream& os,
    const kinematicParcelInjectionData& data
)
{
    os << data.x_ << data.U_ << data.d_ << data.rho_ << data.mDot_;

    return os;
}


CML::Istream& CML::operator>>(Istream& is, kinematicParcelInjectionData& data)
{
    is.check("reading (Px Py Pz)");
    is >> data.x_;

    is.check("reading (Ux Uy Uz)");
    is >> data.U_;

    is.check("reading d");
    is >> data.d_;

    is.check("reading rho");
    is >> data.rho_;

    is.check("reading mDot");
    is >> data.mDot_;

    is.check("operator(Istream&, kinematicParcelInjectionData&)");

    return is;
}


// ************************************************************************* //
