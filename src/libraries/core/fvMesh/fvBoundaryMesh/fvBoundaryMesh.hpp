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
    CML::fvBoundaryMesh

Description
    CML::fvBoundaryMesh

SourceFiles
    fvBoundaryMesh.cpp

\*---------------------------------------------------------------------------*/

#ifndef fvBoundaryMesh_H
#define fvBoundaryMesh_H

#include "fvPatchList.hpp"
#include "lduInterfacePtrsList.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

class fvMesh;
class polyBoundaryMesh;

/*---------------------------------------------------------------------------*\
                       Class fvBoundaryMesh Declaration
\*---------------------------------------------------------------------------*/

class fvBoundaryMesh
:
    public fvPatchList
{
    // Private data

        //- Reference to mesh
        const fvMesh& mesh_;


    // Private Member Functions

        //- Disable default copy construct
        fvBoundaryMesh(const fvBoundaryMesh&);

        //- Disallow assignment
        void operator=(const fvBoundaryMesh&);

        //- Add fvPatches corresponding to the given polyBoundaryMesh
        void addPatches(const polyBoundaryMesh&);


protected:

        //- Update boundary based on new polyBoundaryMesh
        void readUpdate(const polyBoundaryMesh&);


public:

    friend class fvMesh;


    // Constructors

        //- Construct with zero size
        fvBoundaryMesh(const fvMesh&);

        //- Construct from polyBoundaryMesh
        fvBoundaryMesh(const fvMesh&, const polyBoundaryMesh&);


    // Member Functions

        // Access

            //- Return the mesh reference
            const fvMesh& mesh() const
            {
                return mesh_;
            }

            //- Return a list of pointers for each patch
            //  with only those pointing to interfaces being set
            lduInterfacePtrsList interfaces() const;

            //- Find patch index given a name
            label findPatchID(const word& patchName) const;


        //- Correct patches after moving points
        void movePoints();


    // Member Operators

        //- Return const and non-const reference to fvPatch by index.
        using fvPatchList::operator[];

        //- Return const reference to fvPatch by name.
        const fvPatch& operator[](const word&) const;

        //- Return reference to fvPatch by name.
        fvPatch& operator[](const word&);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
