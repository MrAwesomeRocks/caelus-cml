/*---------------------------------------------------------------------------*\
Copyright (C) 2010 I. Clifford
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    Caelus is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

Class
    BlockLduInterfaceFieldPtrsList

Description
    List of coupled interface fields to be used in coupling.
    Templated typedef workaround.

    This file was obtained from Ivor Clifford at the OpenFOAM Workshop
    in PennState, USA, 2011.

    Please report any omissions of authorship and/or copyright to
    info@appliedccm.com.au. Corrections will be made upon provision of proof.

Author
    Ivor Clifford, ivor.clifford@gmail.com

\*---------------------------------------------------------------------------*/

#ifndef BlockLduInterfaceFieldPtrsList_HPP
#define BlockLduInterfaceFieldPtrsList_HPP

#include "BlockLduInterfaceField.hpp"
#include "UPtrList.hpp"


namespace CML
{

template<class T>
struct BlockLduInterfaceFieldPtrsList
{
    typedef UPtrList<const BlockLduInterfaceField<T> > Type;
};

template<class T>
struct BlockLduInterfaceFieldPtrsListList
{
    typedef List<typename BlockLduInterfaceFieldPtrsList<T>::Type> Type;
};

}
#endif
