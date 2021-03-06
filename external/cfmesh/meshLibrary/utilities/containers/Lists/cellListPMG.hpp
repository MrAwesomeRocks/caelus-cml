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
    cellListPMG

Description
    This is a container with additional size to prevent re-allocation
    every time it is resized

Author: Franjo Juretic (franjo.juretic@c-fields.com)

SourceFiles
    

\*---------------------------------------------------------------------------*/

#ifndef cellListPMG_HPP
#define cellListPMG_HPP

#include "cellList.hpp"
#include "Istream.hpp"
#include "Ostream.hpp"
#include "SubList.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

class cellListPMG
    : public cellList
{
    // Private data
        //- number of used elements
        label nElmts_;
    
    // Disallow bitwise assignment
        void operator=(const cellListPMG&);
    
        cellListPMG(const cellListPMG&);
    
    // Disallow transfer from cellList
        void transfer(cellList&);
    
public:
    
    // Constructors
        //- null construct
        inline cellListPMG();

    // Destructor
        inline ~cellListPMG();
            
    // Member functions
        //- return the number of used elements
        inline label size() const;

        //- set the number of used elements
        inline void setSize(const label nElmts);

        //- set the size to zero
        inline void clear();

        //- add a cell at the end of the list
        inline void append(const cell&);
        
        //- return an element with bound checking
        inline cell& newElmt(const label);

    // Member operators
        inline void operator=(const cellList&);
    
        friend inline Ostream& operator<<(Ostream&, const cellListPMG&);
        
        friend inline Istream& operator>>(Istream&, cellListPMG&);
};


// Constructors

inline cellListPMG::cellListPMG()
:
    cellList(),
    nElmts_(0)
{
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
// Destructor

inline cellListPMG::~cellListPMG()
{
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
            
inline label cellListPMG::size() const
{
    return nElmts_;
}

inline void cellListPMG::setSize(const label nElmts)
{
    if( nElmts >= cellList::size() )
    {
        if( cellList::size() != 0 )
        {
            Info << "Resizing cells!" << endl;
            cellList copy(label(1.5*nElmts));
            for(label i=0;i<nElmts_;++i)
                copy[i].transfer(this->operator[](i));
            
            cellList::transfer(copy);
        }
        else
        {
            cellList::setSize(label(1.5*nElmts));
        }
    }

    nElmts_ = nElmts;
}

inline void cellListPMG::clear()
{
    nElmts_ = 0;
}

inline void cellListPMG::append(const cell& c)
{
    const label i = nElmts_;
    setSize(i+1);
    this->operator[](i) = c;
}

inline cell& cellListPMG::newElmt(const label cI)
{
    setSize(cI+1);
    return this->operator[](cI);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

inline void cellListPMG::operator=(const cellList& cls)
{
    setSize(cls.size());
    forAll(cls, cI)
        this->operator[](cI) = cls[cI];
}
    
inline Ostream& operator<<(Ostream& os, const cellListPMG& cls)
{
    SubList<cell> c(cls, cls.nElmts_, 0);
    
    os << c;
    return os;
}

inline Istream& operator>>(Istream& is, cellListPMG& cls)
{
    cellList& cells = static_cast<cellList&>(cls);
    is >> cells;
    cls.nElmts_  = cells.size();
    
    return is;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML            

#endif

// ************************************************************************* //
