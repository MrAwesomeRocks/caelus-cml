/*---------------------------------------------------------------------------*\
Copyright (C) 2016-2018 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Caelus is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

Class
    CML::SortableListEFA

Description
    A list that is sorted upon construction or when explicitly requested
    with the sort() method.


\*---------------------------------------------------------------------------*/

#ifndef SortableListEFA_HPP
#define SortableListEFA_HPP

#include "labelList.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class SortableListEFA Declaration
\*---------------------------------------------------------------------------*/

template <class Type>
class SortableListEFA
:
    public List<Type>
{

    //- Original indices
    labelList indices_;


public:


    //- More function class used by the sort function
    class more
    {
        const UList<Type>& values_;

        public:

        more(const UList<Type>& values)
        :
            values_(values)
        {}

        bool operator()(const label a, const label b)
        {
            return values_[a] > values_[b];
        }
    };


    //- Construct from List, sorting the elements. Starts with indices set
    //  to index in argument
    explicit SortableListEFA(const List<Type>&);

    //- Construct given size. Sort later on.
    explicit SortableListEFA(const label size);

    //- Construct given size and initial value. Sort later on.
    SortableListEFA(const label size, const Type&);

    //- Construct as copy.
    SortableListEFA(const SortableListEFA<Type>&);


    // Member Functions

    //- Return the list of sorted indices. Updated every sort.
    const labelList& indices() const
    {
        return indices_;
    }

    //- Size the list. If grow can cause undefined indices (until next sort)
    void setSize(const label);

    //- Sort the list (if changed after construction time)
    void sort();

    //- Partial sort the list (if changed after construction time)
    void partialSort(int M, int start);

    //- Sort the list (if changed after construction time)
    void stableSort();


    // Member Operators

    void operator=(const SortableListEFA<Type>&);
};


} // End namespace CML

#include "OSspecific.hpp"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template <class Type>
CML::SortableListEFA<Type>::SortableListEFA(const List<Type>& values)
:
    List<Type>(values),
    indices_(values.size())
{
    sort();
}


template <class Type>
CML::SortableListEFA<Type>::SortableListEFA(const label size)
:
    List<Type>(size),
    indices_(size)
{
    forAll(indices_, i)
    {
        indices_[i] = i;
    }
}


template <class Type>
CML::SortableListEFA<Type>::SortableListEFA(const label size, const Type& val)
:
    List<Type>(size, val),
    indices_(size)
{
    forAll(indices_, i)
    {
        indices_[i] = i;
    }
}


template <class Type>
CML::SortableListEFA<Type>::SortableListEFA(const SortableListEFA<Type>& lst)
:
    List<Type>(lst),
    indices_(lst.indices())
{
    forAll(indices_, i)
    {
        indices_[i] = i;
    }

}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template <class Type>
void CML::SortableListEFA<Type>::setSize(const label newSize)
{
    List<Type>::setSize(newSize);
    indices_.setSize(newSize);
}


template <class Type>
void CML::SortableListEFA<Type>::sort()
{
    forAll(indices_, i)
    {
        indices_[i] = i;
    }

    CML::sort(indices_, less(*this));

    List<Type> tmpValues(this->size());

    forAll(indices_, i)
    {
        tmpValues[i] = this->operator[](indices_[i]);
    }

    List<Type>::transfer(tmpValues);
}


template <class Type>
void CML::SortableListEFA<Type>::partialSort(int M, int start)
{
    std::partial_sort
    (
        indices_.begin()+start,
        indices_.begin()+start+M,
        indices_.end(),
        more(*this)
    );
}


template <class Type>
void CML::SortableListEFA<Type>::stableSort()
{
    forAll(indices_, i)
    {
        indices_[i] = i;
    }

    CML::stableSort(indices_, less(*this));

    List<Type> tmpValues(this->size());

    forAll(indices_, i)
    {
        tmpValues[i] = this->operator[](indices_[i]);
    }

    List<Type>::transfer(tmpValues);
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

template <class Type>
void CML::SortableListEFA<Type>::operator=(const SortableListEFA<Type>& rhs)
{
    List<Type>::operator=(rhs);
    indices_ = rhs.indices();
}


#endif
