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

Global
    CML::addToGlobalFunctionSelectionTable

Description
    Macros for easy insertion into global function selection tables

\*---------------------------------------------------------------------------*/

#ifndef addToGlobalFunctionSelectionTable_H
#define addToGlobalFunctionSelectionTable_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// add to hash-table of functions with 'lookup' as the key
#define addNamedToGlobalFunctionSelectionTable\
(memberFunction,argNames,lookup,functionPtr)                                  \
                                                                              \
    /* Add to the table, find by lookup name */                               \
    add##memberFunction##argNames##GlobalMemberFunctionToTable                \
    add_##lookup##_##memberFunction##argNames##GlobalMemberFunctionTo##Table_ \
    (#lookup, functionPtr)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
