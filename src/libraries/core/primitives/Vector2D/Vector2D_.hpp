/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2016 OpenFOAM Foundation
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
    CML::Vector2D

Description
    Templated 2D Vector derived from VectorSpace adding construction from
    2 components, element access using x() and y() member functions and
    the inner-product (dot-product).

SourceFiles
    Vector2DI.hpp

\*---------------------------------------------------------------------------*/

#ifndef Vector2D__H
#define Vector2D__H

#include "VectorSpace.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class Vector2D Declaration
\*---------------------------------------------------------------------------*/

template<class Cmpt>
class Vector2D
:
    public VectorSpace<Vector2D<Cmpt>, Cmpt, 2>
{

public:

    //- Equivalent type of labels used for valid component indexing
    typedef Vector2D<label> labelType;


    // Member constants

        //- Rank of Vector2D is 1
        static const direction rank = 1;


    //- Component labeling enumeration
    enum components { X, Y };


    // Constructors

        //- Construct null
        inline Vector2D();

        //- Construct initialized to zero
        inline Vector2D(const CML::zero);

        //- Construct given VectorSpace
        inline Vector2D(const VectorSpace<Vector2D<Cmpt>, Cmpt, 2>&);

        //- Construct given two components
        inline Vector2D(const Cmpt& vx, const Cmpt& vy);

        //- Construct from Istream
        inline Vector2D(Istream&);


    // Member Functions

        // Access

            inline const Cmpt& x() const;
            inline const Cmpt& y() const;

            inline Cmpt& x();
            inline Cmpt& y();


        // Operators

            //- Perp dot product (dot product with perpendicular vector)
            inline scalar perp(const Vector2D<Cmpt>& b) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Include inline implementations
#include "Vector2DI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
