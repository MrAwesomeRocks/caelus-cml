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
    CML::functionEntries::removeEntry

Description
    Remove a dictionary entry.

    The \c \#remove directive takes a list or a single wordRe.
    For example,
    \verbatim
        #remove entry0
        #remove ( entry1 entry2 entry3 otherEntry )
        #remove "entry[1-3]"
        #remove ( "entry[1-3]" otherEntry )
    \endverbatim

    The removal only occurs in the current context.
    Removing sub-entries or parent entries is not supported.

SourceFiles
    removeEntry.cpp

\*---------------------------------------------------------------------------*/

#ifndef removeEntry_H
#define removeEntry_H

#include "functionEntry.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace functionEntries
{

/*---------------------------------------------------------------------------*\
                           Class removeEntry Declaration
\*---------------------------------------------------------------------------*/

class removeEntry
:
    public functionEntry
{
    // Private Member Functions

        //- Disallow default bitwise copy construct
        removeEntry(const removeEntry&);

        //- Disallow default bitwise assignment
        void operator=(const removeEntry&);


public:

    //- Runtime type information
    ClassName("remove");


    // Member Functions

        //- Execute the functionEntry in a sub-dict context
        static bool execute(dictionary& parentDict, Istream&);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace functionEntries
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
