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
    meshSubset

Description
    A class containing labels of elements

Author: Franjo Juretic (franjo.juretic@c-fields.com)


\*---------------------------------------------------------------------------*/

#ifndef meshSubset_HPP
#define meshSubset_HPP

#include "label.hpp"
#include "HashSet.hpp"
#include "VRWGraph.hpp"

#include "labelList.hpp"
#include "IOstreams.hpp"

#include <set>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

class Ostream;
class VRWGraph;

/*---------------------------------------------------------------------------*\
                        class meshSubset Declaration
\*---------------------------------------------------------------------------*/

class meshSubset
{
    // Private data
        //- name of the given subset
        word name_;

        //- type of subset
        label type_;

        //- labels of elements
        //HashSet<label> data_;
        std::set<label> data_;

public:

    // Enumerators

        enum subsetType_
        {
            UNKNOWN = 0,
            CELLSUBSET = 1,
            FACESUBSET = 2,
            POINTSUBSET = 4,
            FEATUREEDGESUBSET = 8
        };

    // Constructors
        //- Null constructor
        inline meshSubset();

        //- Construct from name. Creates an empty subset
        inline meshSubset(const word& name, const subsetType_&);

        //- Construct from name and element labels
        template<class ListType>
        inline meshSubset
        (
            const word& name,
            const subsetType_&,
            const ListType& elements
        );

        //- Copy construct
        inline meshSubset(const meshSubset&);

        //- Construct from Istream
        inline meshSubset(Istream&);

    // Destructor
        ~meshSubset();


    // Member Functions

        // Access

            //- Return name
            inline const word& name() const;

            //- Type of subset
            inline label type() const;

            //- elements contained in the subset
            template<class ListType>
            inline void containedElements(ListType&) const;

        // Edit

            //- add element label to subset
            inline void addElement(const label);

            //- remove element from subset
            inline void removeElement(const label);

            //- update subset after mesh modification
            //- the list contains new labels of mesh elements
            //- removed elements have a negative label
            template<class ListType>
            inline void updateSubset(const ListType&);

            //- update subset after modification
            //- this modifier is used in case when elements are decomposed
            inline void updateSubset(const VRWGraph&);

        // Searches
            //- find if the element exists in the subset
            inline bool contains(const label) const;

    // operators

            inline void operator=(const meshSubset&);
            inline bool operator==(const meshSubset&) const;
            inline bool operator!=(const meshSubset&) const;

    // IOstream operators

            friend inline Ostream& operator<<(Ostream&, const meshSubset&);
            friend inline Istream& operator>>(Istream&, meshSubset&);
};


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

inline meshSubset::meshSubset()
:
    name_(),
    type_(UNKNOWN),
    data_()
{}

inline meshSubset::meshSubset
(
    const word& name,
    const meshSubset::subsetType_& t
)
:
    name_(name),
    type_(t),
    data_()
{}

template<class ListType>
inline meshSubset::meshSubset
(
    const word& name,
    const meshSubset::subsetType_& type,
    const ListType& elements
)
:
    name_(name),
    type_(type),
    data_()
{
    forAll(elements, i)
        data_.insert(elements[i]);
}

inline meshSubset::meshSubset(const meshSubset& ms)
:
    name_(ms.name_),
    type_(ms.type_),
    data_()
{
    data_ = ms.data_;
}

inline meshSubset::meshSubset(Istream& is)
:
    name_(),
    type_(UNKNOWN),
    data_()
{
    is >> *this;
}

inline meshSubset::~meshSubset()
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

inline const word& meshSubset::name() const
{
    return name_;
}

inline label meshSubset::type() const
{
    return type_;
}

template<class ListType>
inline void meshSubset::containedElements(ListType& l) const
{
    l.setSize(data_.size());

    label counter(0);
    forAllConstIter(std::set<label>, data_, it)
        l[counter++] = *it;
}

inline void meshSubset::addElement(const label elmt)
{
    data_.insert(elmt);
}

inline void meshSubset::removeElement(const label elmt)
{
    data_.erase(elmt);
}

template<class ListType>
inline void meshSubset::updateSubset(const ListType& newLabels)
{
    std::set<label> newData;

    forAllConstIter(std::set<label>, data_, it)
    {
        if( newLabels[*it] < 0 )
            continue;

        newData.insert(newLabels[*it]);
    }

    data_ = newData;
}

inline void meshSubset::updateSubset(const VRWGraph& newLabels)
{
    std::set<label> newData;

    forAllConstIter(std::set<label>, data_, it)
    {
        forAllRow(newLabels, *it, i)
            newData.insert(newLabels(*it, i));
    }

    data_ = newData;
}

inline bool meshSubset::contains(const label elmt) const
{
    return (data_.find(elmt) != data_.end());
}

inline void meshSubset::operator=(const meshSubset& ms)
{
    name_ = ms.name_;
    type_ = ms.type_;

    data_.clear();
    data_ = ms.data_;
}

inline bool meshSubset::operator==(const meshSubset& ms) const
{
    if( ms.name_ != name_ )
        return false;
    if( ms.type_ != type_ )
        return false;

    forAllConstIter(std::set<label>, data_, it)
        if( ms.data_.find(*it) == ms.data_.end() )
            return false;

    return true;
}

inline bool meshSubset::operator!=(const meshSubset& ms) const
{
    return !operator==(ms);
}

// * * * * * * * * * * * * * * * Ostream Operator  * * * * * * * * * * * * * //

inline Ostream& operator<<(Ostream& os, const meshSubset& sel)
{
    os.check("inline Ostream& operator<<(Ostream&, const meshSubset&)");

    os << sel.name_ << nl << sel.type_;

    labelList data(sel.data_.size());
    label counter(0);
    forAllConstIter(std::set<label>, sel.data_, it)
        data[counter++] = *it;

    os << nl << data;

    return os;
}

inline Istream& operator>>(Istream& is, meshSubset& sel)
{
    is.check("friend Istream& operator>>(Istream&, meshSubset&)");

    labelList data;
    is >> sel.name_ >> sel.type_ >> data;

    sel.data_.clear();
    forAll(data, i)
        sel.data_.insert(data[i]);

    is.check("friend Istream& operator>>(Istream&, meshSubset&)");

    return is;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
