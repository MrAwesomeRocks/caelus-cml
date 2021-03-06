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
    CML::StaticAssertionFailed

Description
    Macros and classes to provide static (compile-time) assertions.

    Ideas from various sources
    (http://www.ddj.com/cpp/184401547, http://www.boost.org)

\*---------------------------------------------------------------------------*/

#ifndef StaticAssert_H
#define StaticAssert_H

namespace CML
{

//- Forward declaration of StaticAssertionFailed.
//  Leave as an incomplete class so that sizeof(..) fails
template<bool Truth> class StaticAssertionFailed;

/*---------------------------------------------------------------------------*\
                    Class StaticAssertionFailed Declaration
\*---------------------------------------------------------------------------*/

//- Specialization for successful assertions
template<>
class StaticAssertionFailed<true>
{};


//- Helper class for handling static assertions
template<unsigned Test>
class StaticAssertionTest {};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// internal use:
// ~~~~~~~~~~~~~
// paste together strings, even if an argument is itself a macro
#define StaticAssertMacro(X,Y)  StaticAssertMacro1(X,Y)
#define StaticAssertMacro1(X,Y) StaticAssertMacro2(X,Y)
#define StaticAssertMacro2(X,Y) X##Y

#ifdef __GNUC__
    #define StaticAssertUnusedTypedef __attribute__((unused))
#else
    #define StaticAssertUnusedTypedef
#endif

//- Assert that some test is true at compile-time
#define StaticAssert(Test)                                                     \
    typedef ::CML::StaticAssertionTest                                        \
    <                                                                          \
        sizeof( ::CML::StaticAssertionFailed<((Test) ? true : false)>)        \
    > StaticAssertMacro(StaticAssertionTest, __LINE__) StaticAssertUnusedTypedef

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
