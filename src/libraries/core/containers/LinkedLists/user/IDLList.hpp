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
    CML::IDLList

Description
    Intrusive doubly-linked list.

\*---------------------------------------------------------------------------*/

#ifndef IDLList_H
#define IDLList_H

#include "ILList.hpp"
#include "DLListBase.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class IDLList Declaration
\*---------------------------------------------------------------------------*/

template<class T>
class IDLList
:
    public ILList<DLListBase, T>
{

public:

    // Constructors

        //- Null construct
        IDLList()
        {}

        //- Construct given initial T
        IDLList(const T& a)
        :
            ILList<DLListBase, T>(a)
        {}

        //- Construct from Istream using given Istream constructor class
        template<class INew>
        IDLList(Istream& is, const INew& inewt)
        :
            ILList<DLListBase, T>(is, inewt)
        {}

        //- Construct from Istream
        IDLList(Istream& is)
        :
            ILList<DLListBase, T>(is)
        {}

        //- Copy constructor with additional argument for clone
        template<class CloneArg>
        IDLList(const IDLList<T>& idll, const CloneArg& cloneArg)
        :
            ILList<DLListBase, T>(idll, cloneArg)
        {}
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
