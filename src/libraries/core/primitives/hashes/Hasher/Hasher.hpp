/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
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

InNamespace
    CML

Description
    Misc. hashing functions, mostly from Bob Jenkins.

    The Jenkins hashing function(s) is similar in speed to Paul Hsieh's
    SuperFast hash, but is public domain, supports incremental hashing
    and has been reported to have better characteristics.
    It is also what postgresql seems to be using.

SeeAlso
    http://burtleburtle.net/bob/c/lookup3.c
    and HasherInt.H for a specialized version

SourceFiles
    Hasher.cpp

\*---------------------------------------------------------------------------*/

#ifndef Hasher_H
#define Hasher_H

#include <cstddef>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    //- Bob Jenkins's 96-bit mixer hashing function (lookup3)
    //  \param[in] data - a character stream
    //  \param[in] len  - the number of bytes
    //  \param[in] seed - the previous hash, or an arbitrary value
    unsigned Hasher(const void* data, size_t len, unsigned seed = 0);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
