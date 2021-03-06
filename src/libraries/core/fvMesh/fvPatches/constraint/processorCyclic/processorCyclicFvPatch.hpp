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
    CML::processorCyclicFvPatch

Description
    Processor patch.

SourceFiles
    processorCyclicFvPatch.cpp

\*---------------------------------------------------------------------------*/

#ifndef processorCyclicFvPatch_H
#define processorCyclicFvPatch_H

#include "processorCyclicPolyPatch.hpp"
#include "processorFvPatch.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class processorCyclicFvPatch Declaration
\*---------------------------------------------------------------------------*/

class processorCyclicFvPatch
:
    public processorFvPatch
{
    // Private Data

        const processorCyclicPolyPatch& procPolyPatch_;

public:

    //- Runtime type information
    TypeName(processorCyclicPolyPatch::typeName_());


    // Constructors

        //- Construct from components
        processorCyclicFvPatch(const polyPatch& patch, const fvBoundaryMesh& bm)
        :
            processorFvPatch(patch, bm),
            procPolyPatch_(refCast<const processorCyclicPolyPatch>(patch))
        {}


    // Member functions

        //- Return message tag used for sending
        virtual int tag() const
        {
            // Allocate from Pstream?
            return procPolyPatch_.tag();
        }

        const processorCyclicPolyPatch& procPolyPatch() const
        {
            return procPolyPatch_;
        }

        //- Are the cyclic planes parallel
        virtual bool parallel() const
        {
            return procPolyPatch_.parallel();
        }

        //- Return face transformation tensor
        virtual const tensorField& forwardT() const
        {
            return procPolyPatch_.forwardT();
        }

        //- Return neighbour-cell transformation tensor
        virtual const tensorField& reverseT() const
        {
            return procPolyPatch_.reverseT();
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
