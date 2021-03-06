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

#include "meshSurfaceEdgeExtractorFUN.hpp"
#include "meshSurfaceEngine.hpp"
#include "demandDrivenData.hpp"

# ifdef USE_OMP
#include <omp.h>
# endif

// #define DEBUGSearch

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

meshSurfaceEngine& meshSurfaceEdgeExtractorFUN::surfaceEngine()
{
    # ifdef USE_OMP
    if( omp_in_parallel() )
        FatalErrorInFunction
            << "Cannot create surface engine with a parallel region"
            << exit(FatalError);
    # endif

    if( !surfaceEnginePtr_ )
        surfaceEnginePtr_ = new meshSurfaceEngine(mesh_);

    return *surfaceEnginePtr_;
}

void meshSurfaceEdgeExtractorFUN::clearOut()
{
    deleteDemandDrivenData(surfaceEnginePtr_);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from mesh and octree
meshSurfaceEdgeExtractorFUN::meshSurfaceEdgeExtractorFUN
(
    polyMeshGen& mesh,
    const meshOctree& octree,
    const bool createWrapperSheet
)
:
    mesh_(mesh),
    meshOctree_(octree),
    surfaceEnginePtr_(nullptr),
    createWrapperSheet_(createWrapperSheet)
{
    if( Pstream::parRun() )
        FatalErrorInFunction
          << "Cannot run in parallel!" << exit(FatalError);

    createBasicFundamentalSheets();

    smoothMeshSurface();

    remapBoundaryPoints();
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

meshSurfaceEdgeExtractorFUN::~meshSurfaceEdgeExtractorFUN()
{
    deleteDemandDrivenData(surfaceEnginePtr_);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
