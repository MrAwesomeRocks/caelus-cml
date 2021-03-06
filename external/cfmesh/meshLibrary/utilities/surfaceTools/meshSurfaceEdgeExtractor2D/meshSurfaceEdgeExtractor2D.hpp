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

Class
    meshSurfaceEdgeExtractor2D

Description
    Stores boundary faces into patches and captures edges and corners

Author: Franjo Juretic (franjo.juretic@c-fields.com)

SourceFiles
    meshSurfaceEdgeExtractor2D.cpp

\*---------------------------------------------------------------------------*/

#ifndef meshSurfaceEdgeExtractor2D_HPP
#define meshSurfaceEdgeExtractor2D_HPP

#include "polyMeshGenModifier.hpp"
#include "boolList.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declarations
class meshOctree;
class meshSurfaceEngine;

/*---------------------------------------------------------------------------*\
                Class meshSurfaceEdgeExtractor2D Declaration
\*---------------------------------------------------------------------------*/

class meshSurfaceEdgeExtractor2D
{
    // Private data
        //- mesh
        polyMeshGen& mesh_;

        //- octree
        const meshOctree& meshOctree_;

    // Private member functions

        //- Disallow default bitwise copy construct
        meshSurfaceEdgeExtractor2D(const meshSurfaceEdgeExtractor2D&);

        //- Disallow default bitwise assignment
        void operator=(const meshSurfaceEdgeExtractor2D&);

public:

    // Constructors

        //- Construct from mesh data
        meshSurfaceEdgeExtractor2D
        (
            polyMeshGen& mesh,
            const meshOctree& octree
        );

    // Destructor

        ~meshSurfaceEdgeExtractor2D();

    // Member Functions
        //- distribute boundary faces into patches
        void distributeBoundaryFaces();

        //- re-map points after edges have been extracted
        void remapBoundaryPoints();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
