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
    CML::midPointSet

Description

SourceFiles
    midPointSet.cpp

\*---------------------------------------------------------------------------*/

#ifndef midPointSet_H
#define midPointSet_H

#include "faceOnlySet.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes
class meshSearch;

/*---------------------------------------------------------------------------*\
                           Class midPointSet Declaration
\*---------------------------------------------------------------------------*/

class midPointSet
:
    public faceOnlySet
{
    // Private Member Functions

        void genSamples();


public:

    //- Runtime type information
    TypeName("midPoint");


    // Constructors

        //- Construct from components
        midPointSet
        (
            const word& name,
            const polyMesh& mesh,
            const meshSearch& searchEngine,
            const word& axis,
            const point& start,
            const point& end
        );

        //- Construct from dictionary
        midPointSet
        (
            const word& name,
            const polyMesh& mesh,
            const meshSearch& searchEngine,
            const dictionary& dict
        );


    //- Destructor
    virtual ~midPointSet();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
