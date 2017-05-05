/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2015 OpenFOAM Foundation
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
    CML::pointConstraint

Description
    Accumulates point constraints through successive applications of the
    applyConstraint function.

    After all the constraints have been entered the resulting
    transformation tensor is returned by the constraintTransformation
    function.

SourceFiles
    pointConstraintI.hpp

\*---------------------------------------------------------------------------*/

#ifndef pointConstraint_H
#define pointConstraint_H

#include "label.hpp"
#include "uLabel.hpp"
#include "vector.hpp"
#include "tensor.hpp"
#include "Tuple2.hpp"
#include "transform.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class pointConstraint Declaration
\*---------------------------------------------------------------------------*/

class pointConstraint
:
    public Tuple2<label, vector>
{

public:

    // Constructors

        //- Construct null
        inline pointConstraint();

        //- Construct from components
        inline pointConstraint(const Tuple2<label, vector>&);

        //- Construct from Istream
        inline pointConstraint(Istream&);


    // Member Functions

        //- Apply and accumulate the effect of the given constraint direction
        inline void applyConstraint(const vector& cd);

        //- Combine constraints
        inline void combine(const pointConstraint&);

        //- Return the accumulated constraint transformation tensor
        inline tensor constraintTransformation() const;

        //- Return the accumulated unconstrained directions. Directions
        //  coded as first n rows of tensor.
        inline void unconstrainedDirections(label& n, tensor& vecs) const;
};


//- Reduce operator
class combineConstraintsEqOp
{
public:
    inline void operator()(pointConstraint&, const pointConstraint&) const;
};

//- Transformation function
inline pointConstraint transform(const tensor& tt, const pointConstraint& v);

//- contiguous
template<class T> bool contiguous();
template<>
inline bool contiguous<pointConstraint>()                {return true;}



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "pointConstraintI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
