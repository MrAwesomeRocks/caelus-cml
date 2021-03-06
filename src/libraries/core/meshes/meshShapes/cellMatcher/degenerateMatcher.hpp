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
    CML::degenerateMatcher

Description
    Collection of all hex degenerate matchers (hex, wedge, prism etc.)
    Has static member function to match a shape.

See Also
    cellMatcher

SourceFiles
    degenerateMatcher.cpp

\*---------------------------------------------------------------------------*/

#ifndef degenerateMatcher_H
#define degenerateMatcher_H

#include "hexMatcher.hpp"
#include "wedgeMatcher.hpp"
#include "prismMatcher.hpp"
#include "tetWedgeMatcher.hpp"
#include "pyrMatcher.hpp"
#include "tetMatcher.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class degenerateMatcher Declaration
\*---------------------------------------------------------------------------*/

class degenerateMatcher
{

    // Static data members

        //- Matchers for all degenerate hex shapes
        static hexMatcher hex;
        static wedgeMatcher wedge;
        static prismMatcher prism;
        static tetWedgeMatcher tetWedge;
        static pyrMatcher pyr;
        static tetMatcher tet;

    // Static functions

        //- Recognize basic shape
        static cellShape match
        (
            const faceList& faces,
            const labelList& faceOwner,
            const label cellI,
            const labelList& cellFaces
        );

public:

    // Static data members

        //- Recognize shape given faces of a cell
        static cellShape match(const faceList& faces);

        //- Recognize given uncollapsed shape (usually hex) with duplicate
        //  vertices. cellShape just used to extract faces.
        static cellShape match(const cellShape& shape);

        //- Recognize shape given mesh and cellI
        static cellShape match(const primitiveMesh& mesh, const label cellI);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
