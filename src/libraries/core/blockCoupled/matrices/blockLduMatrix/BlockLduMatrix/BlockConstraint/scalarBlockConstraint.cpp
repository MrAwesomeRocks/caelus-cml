/*---------------------------------------------------------------------------*\
Copyright (C) 2004-6 H. Jasak
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    Caelus is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

Description
    This file was obtained from Hrvoje Jasak at the NUMAP-FOAM summer school
    in Zagreb, Croatia, 2010.

    Please report any omissions of authorship and/or copyright to
    info@appliedccm.com.au. Corrections will be made upon provision of proof.

\*---------------------------------------------------------------------------*/

#include "error.hpp"
#include "scalarBlockConstraint.hpp"


namespace CML
{

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<>
void BlockConstraint<scalar>::combine
(
    const BlockConstraint<scalar>& e
)
{
    if (e.fixedComponents_ > fixedComponents_)
    {
        fixedComponents_ = e.fixedComponents_;

        value_ = e.value_;
    }
}


} // End namespace CML
