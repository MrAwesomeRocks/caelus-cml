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
    polyMeshGen

Description
    Mesh with selections

Author: Franjo Juretic (franjo.juretic@c-fields.com)

SourceFiles

\*---------------------------------------------------------------------------*/

#ifndef polyMeshGen_HPP
#define polyMeshGen_HPP

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "polyMeshGenCells.hpp"
#include "dictionary.hpp"

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

class polyMeshGen
: public polyMeshGenCells
{
    // Private data
        //- meta data about the meshing process
        IOdictionary metaDict_;

public:

    // Constructors
        //- Null constructor
        polyMeshGen(const Time& t);

        //- Construct from components without the boundary
        polyMeshGen
        (
            const Time& t,
            const pointField& points,
            const faceList& faces,
            const cellList& cells
        );

        //- Construct from components with the boundary
        polyMeshGen
        (
            const Time& t,
            const pointField& points,
            const faceList& faces,
            const cellList& cells,
            const wordList& patchNames,
            const labelList& patchStart,
            const labelList& nFacesInPatch
        );

    // Destructor

        ~polyMeshGen();

    // Public member functions
        //- return a constant reference to metaDict
        inline const dictionary& metaData() const
        {
            return metaDict_;
        }

        //- return a reference to metaDict
        inline dictionary& metaData()
        {
            return metaDict_;
        }

    // Read mesh
        void read();

    // Write mesh
        void write() const;
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
