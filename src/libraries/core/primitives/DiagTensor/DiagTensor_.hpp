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
    CML::DiagTensor

Description
    Templated 3D DiagTensor derived from VectorSpace.

    Adding construction from 3 components, element access using xx(), yy()
    and zz() member functions and the inner-product (dot-product) and
    outer-product operators.

SourceFiles
    DiagTensorI.hpp

\*---------------------------------------------------------------------------*/

#ifndef DiagTensor__H
#define DiagTensor__H

#include "Tensor_.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class DiagTensor Declaration
\*---------------------------------------------------------------------------*/

template<class Cmpt>
class DiagTensor
:
    public VectorSpace<DiagTensor<Cmpt>, Cmpt, 3>
{

public:

    // Member constants

        enum
        {
            rank = 2 // Rank of DiagTensor is 2
        };


    // Static data members

        static const char* const typeName;
        static const char* componentNames[];
        static const DiagTensor zero;
        static const DiagTensor one;
        static const DiagTensor max;
        static const DiagTensor min;


    //- Component labeling enumeration
    enum components { XX, YY, ZZ };


    // Constructors

        //- Construct null
        inline DiagTensor();

        //- Construct given VectorSpace
        template<class Cmpt2>
        inline DiagTensor(const VectorSpace<DiagTensor<Cmpt2>, Cmpt2, 3>&);

        //- Construct given three components
        inline DiagTensor(const Cmpt& txx, const Cmpt& tyy, const Cmpt& tzz);

        //- Construct from Istream
        inline DiagTensor(Istream&);


    // Member Functions

        // Access

            inline const Cmpt& xx() const;
            inline const Cmpt& yy() const;
            inline const Cmpt& zz() const;

            inline Cmpt& xx();
            inline Cmpt& yy();
            inline Cmpt& zz();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Include inline implementations
#include "DiagTensorI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
