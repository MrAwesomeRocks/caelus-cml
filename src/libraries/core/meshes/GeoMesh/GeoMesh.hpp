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

Class
    CML::GeoMesh

Description
    Generic mesh wrapper used by volMesh, surfaceMesh, pointMesh etc.

\*---------------------------------------------------------------------------*/

#ifndef GeoMesh_H
#define GeoMesh_H

#include "objectRegistry.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class GeoMesh Declaration
\*---------------------------------------------------------------------------*/

template<class MESH>
class GeoMesh
{

protected:

    // Protected data

        //- Reference to Mesh
        const MESH& mesh_;


public:

    // Public typedefs

        typedef MESH Mesh;
        typedef typename MESH::BoundaryMesh BoundaryMesh;


    // Constructors

        //- Construct from MESH
        explicit GeoMesh(const MESH& mesh)
        :
            mesh_(mesh)
        {}


    // Member Functions

        //- Return the object registry
        const objectRegistry& thisDb() const
        {
            return mesh_;
        }


    // Member Operators

        //- Return reference to polyMesh
        const MESH& operator()() const
        {
            return mesh_;
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
