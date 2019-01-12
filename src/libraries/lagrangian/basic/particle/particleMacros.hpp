/*---------------------------------------------------------------------------*\
Copyright (C) 2012 OpenFOAM Foundation
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

InClass
    CML::particle

Description
    Macros for adding to particle property lists

\*---------------------------------------------------------------------------*/

#ifndef particleMacros_H
#define particleMacros_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//- Define a static 'propertyList' for particle properties
//  The property list is space-delimited with brackets for vector groupings
// \sa AddToPropertyList
#define DefinePropertyList(str)                                                \
                                                                               \
    static string propertyList_;                                               \
                                                                               \
    static string propertyList()                                               \
    {                                                                          \
        return str;                                                            \
    }


//- Add to existing static 'propertyList' for particle properties
//  The property list is space-delimited with brackets for vector groupings
// \sa DefinePropertyList
#define AddToPropertyList(ParcelType, str)                                     \
                                                                               \
    static string propertyList_;                                               \
                                                                               \
    static string propertyList()                                               \
    {                                                                          \
        return ParcelType::propertyList() + str;                               \
    }


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

#endif

// ************************************************************************* //
