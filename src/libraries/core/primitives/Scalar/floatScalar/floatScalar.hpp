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

Typedef
    CML::floatScalar

Description
    Float precision floating point scalar type.

SourceFiles
    floatScalar.cpp

\*---------------------------------------------------------------------------*/

#ifndef floatScalar_H
#define floatScalar_H

#include "doubleFloat.hpp"
#include "direction.hpp"
#include "word.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

typedef float floatScalar;

// Largest and smallest scalar values allowed in certain parts of the code
// (6 is the number of significant figures in an
//  IEEE single precision number.  See limits.h or float.h)
static const floatScalar floatScalarGREAT = 1.0e+6;
static const floatScalar floatScalarVGREAT = 1.0e+37;
static const floatScalar floatScalarROOTVGREAT = 1.0e+18;
static const floatScalar floatScalarSMALL = 1.0e-6;
static const floatScalar floatScalarROOTSMALL = 1.0e-3;
static const floatScalar floatScalarVSMALL = 1.0e-37;
static const floatScalar floatScalarROOTVSMALL = 1.0e-18;

//- Read whole of buf as a scalar. Return true if successful.
inline bool readScalar(const char* buf, floatScalar& s)
{
    char* endPtr;
    s = strtof(buf, &endPtr);

    return (*endPtr == '\0');
}

#define Scalar floatScalar
#define ScalarVGREAT floatScalarVGREAT
#define ScalarVSMALL floatScalarVSMALL
#define ScalarROOTVGREAT floatScalarROOTVGREAT
#define ScalarROOTVSMALL floatScalarROOTVSMALL
#define readScalar readFloatScalar

inline Scalar mag(const Scalar s)
{
    return ::fabsf(s);
}

#define transFunc(func)            \
inline Scalar func(const Scalar s) \
{                                  \
    return ::func##f(s);           \
}

#include "Scalar_.hpp"

inline Scalar hypot(const Scalar x, const Scalar y)
{
    return ::hypotf(x, y);
}

inline Scalar atan2(const Scalar y, const Scalar x)
{
    return ::atan2f(y, x);
}

inline Scalar jn(const int n, const Scalar s)
{
    return ::jnf(n, s);
}

inline Scalar yn(const int n, const Scalar s)
{
    return ::ynf(n, s);
}

#undef Scalar
#undef ScalarVGREAT
#undef ScalarVSMALL
#undef ScalarROOTVGREAT
#undef ScalarROOTVSMALL
#undef readScalar
#undef transFunc


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
