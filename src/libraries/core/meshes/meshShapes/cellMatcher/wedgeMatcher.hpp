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
    CML::wedgeMatcher

Description
    A cellMatcher for wedge cells

See Also
    cellMatcher

SourceFiles
    wedgeMatcher.cpp

\*---------------------------------------------------------------------------*/

#ifndef wedgeMatcher_H
#define wedgeMatcher_H

#include "cellMatcher.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class wedgeMatcher Declaration
\*---------------------------------------------------------------------------*/

class wedgeMatcher
:
    public cellMatcher
{
    // Static data members

        //- constants for this shape
        static const label vertPerCell;
        static const label facePerCell;
        static const label maxVertPerFace;


    // Private Member Functions

        //- Disallow default bitwise copy construct
        wedgeMatcher(const wedgeMatcher&);

        //- Disallow default bitwise assignment
        void operator=(const wedgeMatcher&);


public:

    // Constructors

        //- Construct null
        wedgeMatcher();

    //- Destructor
    ~wedgeMatcher();


    // Member Functions

        virtual label nVertPerCell() const
        {
            return vertPerCell;
        }

        virtual label nFacePerCell() const
        {
            return facePerCell;
        }

        virtual label nMaxVertPerFace() const
        {
            return maxVertPerFace;
        }

        virtual label faceHashValue() const;

        virtual bool faceSizeMatch(const faceList&, const labelList&) const;

        virtual bool matchShape
        (
            const bool checkOnly,
            const faceList& faces,
            const labelList& faceOwner,
            const label cellI,
            const labelList& myFaces
        );

        virtual bool isA(const primitiveMesh& mesh, const label cellI);

        virtual bool isA(const faceList&);

        virtual bool matches
        (
            const primitiveMesh& mesh,
            const label cellI,
            cellShape& shape
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
