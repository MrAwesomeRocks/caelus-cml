/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2016 OpenFOAM Foundation
Copyright (C) 2017 OpenCFD Ltd.
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

\*---------------------------------------------------------------------------*/

#include "label.hpp"
#include "autoPtr.hpp"
#include "writer.hpp"

namespace CML
{
    class polyMesh;
    class surfaceWriter;

    label checkTopology
    (
        const polyMesh&,
        const bool,
        const bool,
        const autoPtr<surfaceWriter>&,
        const autoPtr<writer<scalar> >&
    );
}
