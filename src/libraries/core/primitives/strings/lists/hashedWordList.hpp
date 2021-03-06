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
    CML::hashedWordList

Description
    A wordList with hashed indices for faster lookup by name.

SourceFiles
    hashedWordListI.hpp
    hashedWordList.cpp

\*---------------------------------------------------------------------------*/

#ifndef hashedWordList_H
#define hashedWordList_H

#include "wordList.hpp"
#include "HashTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

class hashedWordList;

// Forward declaration of friend functions and operators
Istream& operator>>(Istream&, hashedWordList&);
Ostream& operator<<(Ostream&, const hashedWordList&);


/*---------------------------------------------------------------------------*\
                       Class hashedWordList Declaration
\*---------------------------------------------------------------------------*/

class hashedWordList
:
    public List<word>
{
    // Private data

        HashTable<label, word> indices_;


    // Private Member Functions

        //- Rebuild the hash of indices
        void rehash();

public:

    // Constructors

        //- Construct null
        hashedWordList();

        //- Copy constructor.
        hashedWordList(const hashedWordList&);

        //- Construct from list of names
        hashedWordList(const UList<word>& names);

        //- Construct by transferring the parameter contents
        hashedWordList(const Xfer< List<word> >& names);

        //- Construct from number and list of names
        hashedWordList(const label nNames, const char** names);

        //- Construct from a nullptr-terminated list of names
        hashedWordList(const char** names);

        //- Construct from Istream
        hashedWordList(Istream&);


    // Member Functions

        //- Clear the list, i.e. set size to zero.
        void clear();

        //- Append an element at the end of the list
        void append(const word&);

        //- Does the list contain the specified name
        inline bool contains(const word&) const;

        //- Transfer the contents of the argument List into this list
        //  and annul the argument list.
        void transfer(List<word>&);


    // Member Operators

        //- Assignment operator from list of names
        inline void operator=(const UList<word>& names);

        //- Assignment operator.
        inline void operator=(const hashedWordList&);

        //- Return name corresponding to specified index
        inline const word& operator[](const label index) const;

        //- Return index corresponding to specified name
        inline label operator[](const word&) const;


    // Istream operators

        friend Istream& operator>>(Istream&, hashedWordList&);
        friend Ostream& operator<<(Ostream&, const hashedWordList&);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "hashedWordListI.hpp"

#endif

// ************************************************************************* //
