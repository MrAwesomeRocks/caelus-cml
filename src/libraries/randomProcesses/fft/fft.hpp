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
    CML::fft

Description
    Fast fourier transform derived from the Numerical
    Recipes in C routine.

    The complex transform field is returned in the field supplied.  The
    direction of transform is supplied as an argument (1 = forward, -1 =
    reverse).  The dimensionality and organisation of the array of values
    in space is supplied in the nn indexing array.

SourceFiles
    fft.cpp

\*---------------------------------------------------------------------------*/

#ifndef fft_H
#define fft_H

#include "complexFields.hpp"
#include "labelList.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

class fft
{

public:

    enum transformDirection
    {
        FORWARD_TRANSFORM = 1,
        REVERSE_TRANSFORM = -1
    };


    static void transform
    (
        complexField& field,
        const labelList& nn,
        transformDirection fftDirection
    );


    static tmp<complexField> forwardTransform
    (
        const tmp<complexField>& field,
        const labelList& nn
    );


    static tmp<complexField> reverseTransform
    (
        const tmp<complexField>& field,
        const labelList& nn
    );


    static tmp<complexVectorField> forwardTransform
    (
        const tmp<complexVectorField>& field,
        const labelList& nn
    );


    static tmp<complexVectorField> reverseTransform
    (
        const tmp<complexVectorField>& field,
        const labelList& nn
    );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
