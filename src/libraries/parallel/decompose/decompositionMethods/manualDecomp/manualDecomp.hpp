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
    CML::manualDecomp

Description
    Decomposition given a cell-to-processor association in a file

SourceFiles
    manualDecomp.cpp

\*---------------------------------------------------------------------------*/

#ifndef manualDecomp_H
#define manualDecomp_H

#include "decompositionMethod.hpp"

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class manualDecomp Declaration
\*---------------------------------------------------------------------------*/

class manualDecomp
:
    public decompositionMethod
{
    // Private data

        fileName decompDataFile_;


    // Private Member Functions

        //- Disallow default bitwise copy construct and assignment
        void operator=(const manualDecomp&);
        manualDecomp(const manualDecomp&);


public:

    //- Runtime type information
    TypeName("manual");


    // Constructors

        //- Construct given the decomposition dictionary
        manualDecomp(const dictionary& decompositionDict);

    //- Destructor
    virtual ~manualDecomp()
    {}


    // Member Functions

        //- manual decompose does not care about proc boundaries - is all
        //  up to the user.
        virtual bool parallelAware() const
        {
            return true;
        }

        //- Return for every coordinate the wanted processor number. Use the
        //  mesh connectivity (if needed)
        virtual labelList decompose
        (
            const polyMesh& mesh,
            const pointField& cc,
            const scalarField& cWeights
        );

        //- Return for every coordinate the wanted processor number. Explicitly
        //  provided connectivity - does not use mesh_.
        //  The connectivity is equal to mesh.cellCells() except for
        //  - in parallel the cell numbers are global cell numbers (starting
        //    from 0 at processor0 and then incrementing all through the
        //    processors)
        //  - the connections are across coupled patches
        virtual labelList decompose
        (
            const labelListList& globalCellCells,
            const pointField& cc,
            const scalarField& cWeights
        )
        {
            NotImplemented;
            return labelList(0);
        }

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
