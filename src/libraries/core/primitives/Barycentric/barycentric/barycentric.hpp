/*---------------------------------------------------------------------------*\
Copyright (C) 2016-2018 OpenFOAM Foundation
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

Typedef
    CML::barycentric

Description
    A scalar version of the templated Barycentric

\*---------------------------------------------------------------------------*/

#ifndef barycentric_HPP
#define barycentric_HPP

#include "scalar.hpp"
#include "Barycentric_.hpp"
#include "contiguous.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes
class Random;


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

typedef Barycentric<scalar> barycentric;


//- Generate a random barycentric coordinate within the unit tetrahedron
barycentric barycentric01(Random& rndGen);


template<>
inline bool contiguous<barycentric>()
{
    return true;
}


} // End namespace CML


#endif
