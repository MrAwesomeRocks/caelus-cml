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

Class
    CML::Tensor

Description
    Templated 3D tensor derived from VectorSpace adding construction from
    9 components, element access using xx(), xy() etc. member functions and
    the inner-product (dot-product) and outer-product of two Vectors
    (tensor-product) operators.

SourceFiles
    TensorI.hpp

\*---------------------------------------------------------------------------*/

#ifndef Tensor__H
#define Tensor__H

#include "Vector_.hpp"
#include "SphericalTensor_.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

template<class Cmpt>
class SymmTensor;

/*---------------------------------------------------------------------------*\
                           Class Tensor Declaration
\*---------------------------------------------------------------------------*/

template<class Cmpt>
class Tensor
:
    public VectorSpace<Tensor<Cmpt>, Cmpt, 9>
{

public:

    //- Equivalent type of labels used for valid component indexing
    typedef Tensor<label> labelType;


    // Member constants

        enum
        {
            rank = 2 // Rank of Tensor is 2
        };


    // Static data members

        static const char* const typeName;
        static const char* componentNames[];

        static const Tensor zero;
        static const Tensor one;
        static const Tensor max;
        static const Tensor min;
        static const Tensor I;


    //- Component labeling enumeration
    enum components { XX, XY, XZ, YX, YY, YZ, ZX, ZY, ZZ };


    // Constructors

        //- Construct null
        inline Tensor();

        //- Construct given VectorSpace of the same rank
        template<class Cmpt2>
        inline Tensor(const VectorSpace<Tensor<Cmpt2>, Cmpt2, 9>&);

        //- Construct given SphericalTensor
        inline Tensor(const SphericalTensor<Cmpt>&);

        //- Construct given SymmTensor
        inline Tensor(const SymmTensor<Cmpt>&);

        //- Construct given triad
        inline Tensor(const Vector<Vector<Cmpt> >&);

        //- Construct given the three vector components
        inline Tensor
        (
            const Vector<Cmpt>& x,
            const Vector<Cmpt>& y,
            const Vector<Cmpt>& z
        );

        //- Construct given the nine components
        inline Tensor
        (
            const Cmpt txx, const Cmpt txy, const Cmpt txz,
            const Cmpt tyx, const Cmpt tyy, const Cmpt tyz,
            const Cmpt tzx, const Cmpt tzy, const Cmpt tzz
        );

        //- Construct from Istream
        Tensor(Istream&);


    // Member Functions

        // Access

            inline const Cmpt& xx() const;
            inline const Cmpt& xy() const;
            inline const Cmpt& xz() const;
            inline const Cmpt& yx() const;
            inline const Cmpt& yy() const;
            inline const Cmpt& yz() const;
            inline const Cmpt& zx() const;
            inline const Cmpt& zy() const;
            inline const Cmpt& zz() const;

            inline Cmpt& xx();
            inline Cmpt& xy();
            inline Cmpt& xz();
            inline Cmpt& yx();
            inline Cmpt& yy();
            inline Cmpt& yz();
            inline Cmpt& zx();
            inline Cmpt& zy();
            inline Cmpt& zz();

            // Access vector components.

            inline Vector<Cmpt> x() const;
            inline Vector<Cmpt> y() const;
            inline Vector<Cmpt> z() const;
            inline Vector<Cmpt> vectorComponent(const direction) const;

        //- Transpose
        inline Tensor<Cmpt> T() const;


    // Member Operators

        //- Assign to a SphericalTensor
        inline void operator=(const SphericalTensor<Cmpt>&);

        //- Assign to a SymmTensor
        inline void operator=(const SymmTensor<Cmpt>&);

        //- Assign to a triad
        inline void operator=(const Vector<Vector<Cmpt> >&);
};


template<class Cmpt>
class typeOfRank<Cmpt, 2>
{
public:

    typedef Tensor<Cmpt> type;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Include inline implementations
#include "TensorI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
