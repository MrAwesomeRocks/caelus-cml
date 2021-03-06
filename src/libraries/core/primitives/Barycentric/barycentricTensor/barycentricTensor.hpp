/*---------------------------------------------------------------------------*\
Copyright (C) 2016-2018 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of CML::.

    CML:: is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CML:: is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with CML::.  If not, see <http://www.gnu.org/licenses/>.

Typedef
    CML::barycentricTensor

Description
    A scalar version of the templated BarycentricTensor

\*---------------------------------------------------------------------------*/

#ifndef barycentricTensor_HPP
#define barycentricTensor_HPP

#include "scalar.hpp"
#include "BarycentricTensor_.hpp"
#include "contiguous.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

typedef BarycentricTensor<scalar> barycentricTensor;


template<>
inline bool contiguous<barycentricTensor>()
{
    return true;
}


} // End namespace CML


#endif
