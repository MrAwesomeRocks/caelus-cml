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

#include "meshOctreeCreator.hpp"
#include "triSurf.hpp"
#include "IOdictionary.hpp"

//#define DEBUGBalancing

# ifdef DEBUGBalancing
#include <sstream>
# endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
// Private member functions

void meshOctreeCreator::loadDistribution(const bool distributeUsed)
{
    if( octree_.neiProcs().size() == 0 || !meshDictPtr_ )
        return;

    direction usedType(0);
    if( distributeUsed )
    {
        usedType |= meshOctreeCubeBasic::INSIDE;
        if( meshDictPtr_->found("keepCellsIntersectingBoundary") )
            usedType |= meshOctreeCubeBasic::DATA;
    }

    meshOctreeModifier(octree_).loadDistribution(usedType);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
