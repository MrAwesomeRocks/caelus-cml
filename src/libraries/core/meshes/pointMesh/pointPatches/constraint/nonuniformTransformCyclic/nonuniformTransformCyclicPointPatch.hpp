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
    CML::nonuniformTransformCyclicPointPatch

Description
    Cyclic patch with slip constraint

SourceFiles
    nonuniformTransformCyclicPointPatch.cpp

\*---------------------------------------------------------------------------*/

#ifndef nonuniformTransformCyclicPointPatch_H
#define nonuniformTransformCyclicPointPatch_H

#include "cyclicPointPatch.hpp"
#include "nonuniformTransformCyclicPolyPatch.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                      Class nonuniformTransformCyclicPointPatch Declaration
\*---------------------------------------------------------------------------*/

class nonuniformTransformCyclicPointPatch
:
    public cyclicPointPatch
{

public:

    //- Runtime type information
    TypeName(nonuniformTransformCyclicPolyPatch::typeName_());


    // Constructors

        //- Construct from components
        nonuniformTransformCyclicPointPatch
        (
            const polyPatch& patch,
            const pointBoundaryMesh& bm
        )
        :
            cyclicPointPatch(patch, bm)
        {}


    // Destructor

        virtual ~nonuniformTransformCyclicPointPatch()
        {}


    // Member Functions

        //- Return point unit normals.
        virtual const vectorField& pointNormals() const;

        //- Accumulate the effect of constraint direction of this patch
        virtual void applyConstraint
        (
            const label pointi,
            pointConstraint&
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
