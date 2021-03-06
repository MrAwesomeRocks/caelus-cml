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
    renameBoundaryPatches

Description
    Renames boundary patches based on user intput and sets correct patch types

Author: Franjo Juretic (franjo.juretic@c-fields.com)

SourceFiles
    renameBoundaryPatches.cpp

\*---------------------------------------------------------------------------*/

#ifndef renameBoundaryPatches_HPP
#define renameBoundaryPatches_HPP

#include "polyMeshGenModifier.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declarations
class IOdictionary;

/*---------------------------------------------------------------------------*\
                           Class renameBoundaryPatches Declaration
\*---------------------------------------------------------------------------*/

class renameBoundaryPatches
{
    // Private data
        //- mesh
        polyMeshGen& mesh_;
    
        //- dictionary containing relevant information
        const IOdictionary& meshDict_;

    // Private member functions
        //- calculate new boundary and replace the existing one
        void calculateNewBoundary();
        
        //- Disallow default bitwise copy construct
        renameBoundaryPatches(const renameBoundaryPatches&);

        //- Disallow default bitwise assignment
        void operator=(const renameBoundaryPatches&);

public:

    // Constructors

        //- Construct from mesh and IOdictionary
        renameBoundaryPatches
        (
            polyMeshGen& mesh,
            const IOdictionary& meshDict
        );

    // Destructor

        ~renameBoundaryPatches();

    // Member Functions
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
