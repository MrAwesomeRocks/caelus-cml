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

#include "STLsurfaceFormat.hpp"

#include "addToRunTimeSelectionTable.hpp"
#include "addToMemberFunctionSelectionTable.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
namespace fileFormats
{

// read MeshedSurface (ascii)
addNamedTemplatedToRunTimeSelectionTable
(
    MeshedSurface,
    STLsurfaceFormat,
    face,
    fileExtension,
    stl
);
addNamedTemplatedToRunTimeSelectionTable
(
    MeshedSurface,
    STLsurfaceFormat,
    triFace,
    fileExtension,
    stl
);

// read MeshedSurface (binary)
addNamedTemplatedToRunTimeSelectionTable
(
    MeshedSurface,
    STLsurfaceFormat,
    face,
    fileExtension,
    stlb
);
addNamedTemplatedToRunTimeSelectionTable
(
    MeshedSurface,
    STLsurfaceFormat,
    triFace,
    fileExtension,
    stlb
);


// write MeshedSurfaceProxy (ascii)
addNamedTemplatedToMemberFunctionSelectionTable
(
    MeshedSurfaceProxy,
    STLsurfaceFormat,
    face,
    write,
    fileExtension,
    stl
);
addNamedTemplatedToMemberFunctionSelectionTable
(
    MeshedSurfaceProxy,
    STLsurfaceFormat,
    triFace,
    write,
    fileExtension,
    stl
);

// write MeshedSurfaceProxy (binary)
addNamedTemplatedToMemberFunctionSelectionTable
(
    MeshedSurfaceProxy,
    STLsurfaceFormat,
    face,
    write,
    fileExtension,
    stlb
);
addNamedTemplatedToMemberFunctionSelectionTable
(
    MeshedSurfaceProxy,
    STLsurfaceFormat,
    triFace,
    write,
    fileExtension,
    stlb
);

// write UnsortedMeshedSurface (ascii)
addNamedTemplatedToMemberFunctionSelectionTable
(
    UnsortedMeshedSurface,
    STLsurfaceFormat,
    face,
    write,
    fileExtension,
    stl
);
addNamedTemplatedToMemberFunctionSelectionTable
(
    UnsortedMeshedSurface,
    STLsurfaceFormat,
    triFace,
    write,
    fileExtension,
    stl
);

// write UnsortedMeshedSurface (binary)
addNamedTemplatedToMemberFunctionSelectionTable
(
    UnsortedMeshedSurface,
    STLsurfaceFormat,
    face,
    write,
    fileExtension,
    stlb
);
addNamedTemplatedToMemberFunctionSelectionTable
(
    UnsortedMeshedSurface,
    STLsurfaceFormat,
    triFace,
    write,
    fileExtension,
    stlb
);

}
}

// ************************************************************************* //
