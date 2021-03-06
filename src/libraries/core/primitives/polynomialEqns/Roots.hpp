/*---------------------------------------------------------------------------*\
Copyright (C) 2017-2018 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Caelus is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

Class
    CML::Roots

Description
    Templated storage for the roots of polynomial equations, plus flags to
    indicate the nature of the roots.

\*---------------------------------------------------------------------------*/

#ifndef Roots_HPP
#define Roots_HPP

#include "VectorSpace.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//- Types of root
enum class rootType
{
    real = 0,
    complex,
    posInf,
    negInf,
    nan
};


/*---------------------------------------------------------------------------*\
                         Class Roots Declaration
\*---------------------------------------------------------------------------*/

template<direction N>
class Roots
:
    public VectorSpace<Roots<N>, scalar, N>
{
    // Private data

        //- Root types, encoded into a single integer
        label types_;

public:

    // Constructors

        //- Construct null
        inline Roots();

        //- Construct with a uniform value
        inline Roots(const rootType t, const scalar x);

        //- Construct by concatenation
        inline Roots
        (
            const rootType t,
            const scalar x,
            const Roots<N - 1>& xs
        );

        //- Construct by concatenation
        inline Roots
        (
            const Roots<N - 1>& xs,
            const rootType t,
            const scalar x
        );

        //- Construct by concatenation
        template <direction M>
        inline Roots(const Roots<M>& xs, const Roots<N - M>& ys);


    // Member Functions

        //- Set the type of the i-th root
        inline void type(const direction i, const rootType t);

        //- Return the type of the i-th root
        inline rootType type(const direction i) const;
};


} // End namespace CML


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template <CML::direction N>
inline CML::Roots<N>::Roots()
:
    types_(0)
{
    forAll(*this, i)
    {
        type(i, rootType::nan);
    }
}


template <CML::direction N>
inline CML::Roots<N>::Roots(const rootType t, const scalar x)
:
    types_(0)
{
    forAll(*this, i)
    {
        this->v_[i] = x;
        type(i, t);
    }
}


template <CML::direction N>
inline CML::Roots<N>::Roots
(
    const rootType t,
    const scalar x,
    const Roots<N - 1>& xs
)
:
    types_(0)
{
    this->v_[0] = x;
    type(0, t);
    forAll(xs, i)
    {
        this->v_[i+1] = xs[i];
        type(i + 1, xs.type(i));
    }
}


template <CML::direction N>
inline CML::Roots<N>::Roots
(
    const Roots<N - 1>& xs,
    const rootType t,
    const scalar x
)
:
    types_(0)
{
    forAll(xs, i)
    {
        this->v_[i] = xs[i];
        type(i, xs.type(i));
    }
    this->v_[N-1] = x;
    type(N - 1, t);
}


template <CML::direction N>
template <CML::direction M>
inline CML::Roots<N>::Roots
(
    const Roots<M>& xs,
    const Roots<N - M>& ys
)
:
    types_(0)
{
    forAll(xs, i)
    {
        this->v_[i] = xs[i];
        type(i, xs.type(i));
    }
    forAll(ys, i)
    {
        this->v_[i + M] = ys[i];
        type(i + M, ys.type(i));
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template <CML::direction N>
inline void CML::Roots<N>::type
(
    const direction i,
    const rootType t
)
{
    types_ += (static_cast<int>(t) - static_cast<int>(type(i))) << 3*i;
}


template <CML::direction N>
inline CML::rootType CML::Roots<N>::type(const direction i) const
{
    return static_cast<rootType>((types_ >> 3*i) & 7);
}


#endif
