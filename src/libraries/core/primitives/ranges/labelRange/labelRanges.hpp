/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2017 OpenFOAM Foundation
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
    CML::labelRanges

Description
    A list of labelRange.

SourceFiles
    labelRanges.cpp

\*---------------------------------------------------------------------------*/

#ifndef labelRanges_H
#define labelRanges_H

#include "labelRange.hpp"
#include "DynamicList.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes
class Istream;
class Ostream;

// Forward declaration of friend functions and operators
class labelRanges;
Istream& operator>>(Istream&, labelRanges&);
Ostream& operator<<(Ostream&, const labelRanges&);

/*---------------------------------------------------------------------------*\
                         Class labelRanges Declaration
\*---------------------------------------------------------------------------*/

class labelRanges
:
    private DynamicList<labelRange>
{
    // Private typedefs for convenience

        typedef DynamicList<labelRange> ParentType;


    // Private Member Functions

        //- Insert range before specified insertion index, by copying up
        void insertBefore(const label, const labelRange&);

        //- Purge empty ranges, by copying down
        void purgeEmpty();

        //- Print the range for debugging purposes
        Ostream& printRange(Ostream&, const labelRange&) const;


public:

    // Constructors

        //- Construct null
        inline labelRanges();

        //- Construct given size
        inline explicit labelRanges(const label);

        //- Construct from Istream.
        labelRanges(Istream&);


    // Member Functions

        //- Clear the addressed list
        using DynamicList<labelRange>::clear;

        //- Return true if the list is empty
        using DynamicList<labelRange>::empty;

        //- Return true if the value is within any of the ranges
        inline bool contains(const label) const;

        //- Add the range to the list
        bool add(const labelRange&);

        //- Remove the range from the list
        bool remove(const labelRange&);

    // STL iterator

        //- An STL const_iterator
        class const_iterator
        {
            // Private data

                //- Reference to the list for which this is an iterator
                const labelRanges& list_;

                //- Current list index
                label index_;

                //- Index of current element at listIndex
                label subIndex_;

        public:

            // Constructors

                //- Construct null - equivalent to an 'end' position
                inline const_iterator();

                //- Construct from list, moving to its 'begin' position
                inline explicit const_iterator(const labelRanges&);


            // Member operators

                inline bool operator==(const const_iterator&) const;

                inline bool operator!=(const const_iterator&) const;

                inline label operator*();
                inline label operator()();

                inline const_iterator& operator++();
                inline const_iterator operator++(int);
        };


        //- const_iterator set to the beginning of the list
        inline const_iterator cbegin() const;

        //- const_iterator set to beyond the end of the list
        inline const const_iterator& cend() const;

        //- const_iterator set to the beginning of the list
        inline const_iterator begin() const;

        //- const_iterator set to beyond the end of the list
        inline const const_iterator& end() const;


    // IOstream Operators

        friend Istream& operator>>(Istream&, labelRanges&);
        friend Ostream& operator<<(Ostream&, const labelRanges&);


private:
    //- const labelRanges held by endIter_
        static const labelRanges endLabelRanges_;
    
    //- const_iterator returned by end(), cend()
        static const const_iterator endIter_;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "labelRangesI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
