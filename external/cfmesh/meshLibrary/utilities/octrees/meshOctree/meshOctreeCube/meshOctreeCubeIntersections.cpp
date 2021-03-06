/*---------------------------------------------------------------------------*\
Copyright (C) Creative Fields, Ltd.
-------------------------------------------------------------------------------
License
    This file is part of cfMesh.

    cfMesh is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 3 of the License, or (at your
    option) any later version.

    cfMesh is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with cfMesh.  If not, see <http://www.gnu.org/licenses/>.

Author: Franjo Juretic (franjo.juretic@c-fields.com)

\*---------------------------------------------------------------------------*/

#include "triSurf.hpp"
#include "meshOctreeCube.hpp"
#include "VRWGraph.hpp"
#include "helperFunctions.hpp"

//#define DEBUGSearch

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

bool meshOctreeCube::hasContainedTriangles
(
    const triSurf& surface,
    const boundBox& rootBox,
    const VRWGraph& containedElements
) const
{
    if( containedElementsLabel_ == -1 )
        return false;

    forAllRow(containedElements, containedElementsLabel_, tI)
        if(
            intersectsTriangleExact
            (
                surface,
                rootBox,
                containedElements(containedElementsLabel_, tI)
            )
        )
            return true;

    return false;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
