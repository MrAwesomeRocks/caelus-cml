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

\*---------------------------------------------------------------------------*/

#ifndef doubleFloat_H
#define doubleFloat_H

#include "label.hpp"
#include "products.hpp"

#ifdef windows 
#if defined(__STRICT_ANSI__)
#undef __STRICT_ANSI__
#define REDEFINE_SA__
#endif
#endif

#include <cmath>

#ifdef darwin
    #ifndef DUMMY_SCALAR_FUNCTIONS
    #define DUMMY_SCALAR_FUNCTIONS
    inline float j0f(float x) { return float(j0(double(x)));}
    inline float j1f(float x) { return float(j1(double(x)));}
    inline float y0f(float x) { return float(y0(double(x)));}
    inline float y1f(float x) { return float(y1(double(x)));}
    inline float jnf(const int n, const float s) { return float(jn(n, double(s))); }
    inline float ynf(const int n, const float s) { return float(yn(n, double(s))); }
    #endif
#endif 

#ifdef windows
    #ifndef DUMMY_SCALAR_FUNCTIONS
    #define DUMMY_SCALAR_FUNCTIONS
    inline float j0f(float x) { return float(_j0(double(x)));}
    inline float j1f(float x) { return float(_j1(double(x)));}
    inline float y0f(float x) { return float(_y0(double(x)));}
    inline float y1f(float x) { return float(_y1(double(x)));}
    inline float jnf(const int n, const float s) { return float(jn(n, double(s))); }
    inline float ynf(const int n, const float s) { return float(yn(n, double(s))); }	
    #endif
#endif 


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Cmpt>
class typeOfRank<Cmpt, 0>
{
public:

    typedef Cmpt type;
};


template<class Cmpt>
class symmTypeOfRank<Cmpt, 0>
{
public:

    typedef Cmpt type;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class T>
inline bool equal(const T& s1, const T& s2)
{
    return s1 == s2;
}


#define MAXMINPOW(retType, type1, type2)          \
                                                  \
MAXMIN(retType, type1, type2)                     \
                                                  \
inline double pow(const type1 s, const type2 e)   \
{                                                 \
    return ::pow(double(s), double(e));           \
}


MAXMINPOW(double, double, double)
MAXMINPOW(double, double, float)
MAXMINPOW(double, float, double)
MAXMINPOW(float, float, float)
MAXMINPOW(double, double, int)
MAXMINPOW(double, int, double)
MAXMINPOW(double, double, long)
MAXMINPOW(double, long, double)
MAXMINPOW(float, float, int)
MAXMINPOW(float, int, float)
MAXMINPOW(float, float, long)
MAXMINPOW(float, long, float)

#undef MAXMINPOW


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
