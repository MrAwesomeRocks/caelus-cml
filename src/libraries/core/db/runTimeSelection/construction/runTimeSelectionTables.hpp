/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
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

Description
    Macros to ease declaration of run-time selection tables.

    declareRunTimeSelectionTable is used to create a run-time selection table
    for a base-class which holds constructor pointers on the table.

    declareRunTimeNewSelectionTable is used to create a run-time selection
    table for a derived-class which holds "New" pointers on the table.

\*---------------------------------------------------------------------------*/

#include "token.hpp"

#ifndef runTimeSelectionTables_H
#define runTimeSelectionTables_H

#include "autoPtr.hpp"
#include "HashTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


// external use:
// ~~~~~~~~~~~~~
// declare a run-time selection:
#define declareRunTimeSelectionTable\
(autoPtr,baseType,argNames,argList,parList)                                   \
                                                                              \
    /* Construct from argList function pointer type */                        \
    typedef autoPtr< baseType > (*argNames##ConstructorPtr)argList;           \
                                                                              \
    /* Construct from argList function table type */                          \
    typedef HashTable< argNames##ConstructorPtr, word, string::hash >         \
        argNames##ConstructorTable;                                           \
                                                                              \
    /* Construct from argList function pointer table pointer */               \
    static argNames##ConstructorTable* argNames##ConstructorTablePtr_;        \
                                                                              \
    /* Table constructor called from the table add function */                \
    static void construct##argNames##ConstructorTables();                     \
                                                                              \
    /* Table destructor called from the table add function destructor */      \
    static void destroy##argNames##ConstructorTables();                       \
                                                                              \
    /* Class to add constructor from argList to table */                      \
    template< class baseType##Type >                                          \
    class add##argNames##ConstructorToTable                                   \
    {                                                                         \
    public:                                                                   \
                                                                              \
        static autoPtr< baseType > New argList                                \
        {                                                                     \
            return autoPtr< baseType >(new baseType##Type parList);           \
        }                                                                     \
                                                                              \
        add##argNames##ConstructorToTable                                     \
        (                                                                     \
            const word& lookup = baseType##Type::typeName                     \
        )                                                                     \
        {                                                                     \
            construct##argNames##ConstructorTables();                         \
            argNames##ConstructorTablePtr_->insert(lookup, New);              \
        }                                                                     \
                                                                              \
        ~add##argNames##ConstructorToTable()                                  \
        {                                                                     \
            destroy##argNames##ConstructorTables();                           \
        }                                                                     \
    };                                                                        \
                                                                              \
    /* Class to add constructor from argList to table */                      \
    /* Remove only the entry (not the table) upon destruction */              \
    template< class baseType##Type >                                          \
    class addRemovable##argNames##ConstructorToTable                          \
    {                                                                         \
        /* retain lookup name for later removal */                            \
        const word& lookup_;                                                  \
                                                                              \
    public:                                                                   \
                                                                              \
        static autoPtr< baseType > New argList                                \
        {                                                                     \
            return autoPtr< baseType >(new baseType##Type parList);           \
        }                                                                     \
                                                                              \
        addRemovable##argNames##ConstructorToTable                            \
        (                                                                     \
            const word& lookup = baseType##Type::typeName                     \
        )                                                                     \
        :                                                                     \
            lookup_(lookup)                                                   \
        {                                                                     \
            construct##argNames##ConstructorTables();                         \
            argNames##ConstructorTablePtr_->set(lookup, New);                 \
        }                                                                     \
                                                                              \
        ~addRemovable##argNames##ConstructorToTable()                         \
        {                                                                     \
            if (argNames##ConstructorTablePtr_)                               \
            {                                                                 \
                argNames##ConstructorTablePtr_->erase(lookup_);               \
            }                                                                 \
        }                                                                     \
    };



// external use:
// ~~~~~~~~~~~~~
// declare a run-time selection for derived classes:
#define declareRunTimeNewSelectionTable\
(autoPtr,baseType,argNames,argList,parList)                                   \
                                                                              \
    /* Construct from argList function pointer type */                        \
    typedef autoPtr< baseType > (*argNames##ConstructorPtr)argList;           \
                                                                              \
    /* Construct from argList function table type */                          \
    typedef HashTable< argNames##ConstructorPtr, word, string::hash >         \
        argNames##ConstructorTable;                                           \
                                                                              \
    /* Construct from argList function pointer table pointer */               \
    static argNames##ConstructorTable* argNames##ConstructorTablePtr_;        \
                                                                              \
    /* Table constructor called from the table add function */                \
    static void construct##argNames##ConstructorTables();                     \
                                                                              \
    /* Table destructor called from the table add function destructor */      \
    static void destroy##argNames##ConstructorTables();                       \
                                                                              \
    /* Class to add constructor from argList to table */                      \
    template< class baseType##Type >                                          \
    class add##argNames##ConstructorToTable                                   \
    {                                                                         \
    public:                                                                   \
                                                                              \
        static autoPtr< baseType > New##baseType argList                      \
        {                                                                     \
            return autoPtr< baseType >(baseType##Type::New parList.ptr());    \
        }                                                                     \
                                                                              \
        add##argNames##ConstructorToTable                                     \
        (                                                                     \
            const word& lookup = baseType##Type::typeName                     \
        )                                                                     \
        {                                                                     \
            construct##argNames##ConstructorTables();                         \
            argNames##ConstructorTablePtr_->insert                            \
            (                                                                 \
                lookup,                                                       \
                New##baseType                                                 \
            );                                                                \
        }                                                                     \
                                                                              \
        ~add##argNames##ConstructorToTable()                                  \
        {                                                                     \
            destroy##argNames##ConstructorTables();                           \
        }                                                                     \
    };                                                                        \
                                                                              \
    /* Class to add constructor from argList to table */                      \
    template< class baseType##Type >                                          \
    class addRemovable##argNames##ConstructorToTable                          \
    {                                                                         \
        /* retain lookup name for later removal */                            \
        const word& lookup_;                                                  \
                                                                              \
    public:                                                                   \
                                                                              \
        static autoPtr< baseType > New##baseType argList                      \
        {                                                                     \
            return autoPtr< baseType >(baseType##Type::New parList.ptr());    \
        }                                                                     \
                                                                              \
        addRemovable##argNames##ConstructorToTable                            \
        (                                                                     \
            const word& lookup = baseType##Type::typeName                     \
        )                                                                     \
        :                                                                     \
            lookup_(lookup)                                                   \
        {                                                                     \
            construct##argNames##ConstructorTables();                         \
            argNames##ConstructorTablePtr_->set                               \
            (                                                                 \
                lookup,                                                       \
                New##baseType                                                 \
            );                                                                \
        }                                                                     \
                                                                              \
        ~addRemovable##argNames##ConstructorToTable()                         \
        {                                                                     \
            if (argNames##ConstructorTablePtr_)                               \
            {                                                                 \
                argNames##ConstructorTablePtr_->erase(lookup_);               \
            }                                                                 \
        }                                                                     \
    };


// internal use:
// constructor aid
#define defineRunTimeSelectionTableConstructor\
(baseType,argNames)                                                           \
                                                                              \
    /* Table constructor called from the table add function */                \
    void baseType::construct##argNames##ConstructorTables()                   \
    {                                                                         \
        static bool constructed = false;                                      \
        if (!constructed)                                                     \
        {                                                                     \
            constructed = true;                                               \
            baseType::argNames##ConstructorTablePtr_                          \
                = new baseType::argNames##ConstructorTable;                   \
        }                                                                     \
    }


// internal use:
// destructor aid
#define defineRunTimeSelectionTableDestructor\
(baseType,argNames)                                                           \
                                                                              \
    /* Table destructor called from the table add function destructor */      \
    void baseType::destroy##argNames##ConstructorTables()                     \
    {                                                                         \
        if (baseType::argNames##ConstructorTablePtr_)                         \
        {                                                                     \
            delete baseType::argNames##ConstructorTablePtr_;                  \
            baseType::argNames##ConstructorTablePtr_ = nullptr;                  \
        }                                                                     \
    }


// internal use:
// create pointer to hash-table of functions
#define defineRunTimeSelectionTablePtr\
(baseType,argNames)                                                           \
                                                                              \
    /* Define the constructor function table */                               \
    baseType::argNames##ConstructorTable*                                     \
        baseType::argNames##ConstructorTablePtr_ = nullptr


// not much in use:
#define defineTemplateRunTimeSelectionTablePtr(baseType,argNames)             \
                                                                              \
    /* Define the constructor function table */                               \
    typename baseType::argNames##ConstructorTable*                            \
        baseType::argNames##ConstructorTablePtr_ = nullptr


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


// external use:
// ~~~~~~~~~~~~~
// define run-time selection table
#define defineRunTimeSelectionTable\
(baseType,argNames)                                                           \
                                                                              \
    defineRunTimeSelectionTablePtr(baseType,argNames);                        \
    defineRunTimeSelectionTableConstructor(baseType,argNames);                \
    defineRunTimeSelectionTableDestructor(baseType,argNames)


// external use:
// ~~~~~~~~~~~~~
// define run-time selection table for template classes
// use when baseType doesn't need a template argument (eg, is a typedef)
#define defineTemplateRunTimeSelectionTable\
(baseType,argNames)                                                           \
                                                                              \
    template<>                                                                \
    defineRunTimeSelectionTablePtr(baseType,argNames);                        \
    template<>                                                                \
    defineRunTimeSelectionTableConstructor(baseType,argNames);                \
    template<>                                                                \
    defineRunTimeSelectionTableDestructor(baseType,argNames)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


// internal use:
// constructor aid
// use when baseType requires the Targ template argument
#define defineTemplatedRunTimeSelectionTableConstructor\
(baseType,argNames,Targ)                                                      \
                                                                              \
    /* Table constructor called from the table add function */                \
    void baseType< Targ >::construct##argNames##ConstructorTables()           \
    {                                                                         \
        static bool constructed = false;                                      \
        if (!constructed)                                                     \
        {                                                                     \
            constructed = true;                                               \
            baseType< Targ >::argNames##ConstructorTablePtr_                  \
                = new baseType< Targ >::argNames##ConstructorTable;           \
        }                                                                     \
    }


// internal use:
// destructor aid
// use when baseType requires the Targ template argument
#define defineTemplatedRunTimeSelectionTableDestructor\
(baseType,argNames,Targ)                                                      \
                                                                              \
    /* Table destructor called from the table add function destructor */      \
    void baseType< Targ >::destroy##argNames##ConstructorTables()             \
    {                                                                         \
        if (baseType< Targ >::argNames##ConstructorTablePtr_)                 \
        {                                                                     \
            delete baseType< Targ >::argNames##ConstructorTablePtr_;          \
            baseType< Targ >::argNames##ConstructorTablePtr_ = nullptr;          \
        }                                                                     \
    }


// internal use:
// create pointer to hash-table of functions
// use when baseType requires the Targ template argument
#define defineTemplatedRunTimeSelectionTablePtr\
(baseType,argNames,Targ)                                                      \
                                                                              \
    /* Define the constructor function table */                               \
    baseType< Targ >::argNames##ConstructorTable*                             \
        baseType< Targ >::argNames##ConstructorTablePtr_ = nullptr


// external use:
// ~~~~~~~~~~~~~
// define run-time selection table for template classes
// use when baseType requires the Targ template argument
#define defineTemplatedRunTimeSelectionTable\
(baseType,argNames,Targ)                                                      \
                                                                              \
    template<>                                                                \
    defineTemplatedRunTimeSelectionTablePtr(baseType,argNames,Targ);          \
    template<>                                                                \
    defineTemplatedRunTimeSelectionTableConstructor(baseType,argNames,Targ);  \
    template<>                                                                \
    defineTemplatedRunTimeSelectionTableDestructor(baseType,argNames,Targ)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
