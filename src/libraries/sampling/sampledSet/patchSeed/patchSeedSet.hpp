/*---------------------------------------------------------------------------*\
Copyright (C) 2012 OpenFOAM Foundation
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
    CML::patchSeedSet

Description
    Initialises points on or just off patch

SourceFiles
    patchSeedSet.cpp

\*---------------------------------------------------------------------------*/

#ifndef patchSeedSet_H
#define patchSeedSet_H

#include "sampledSet.hpp"
#include "DynamicList.hpp"
#include "HashSet.hpp"
#include "Random.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class patchSeedSet Declaration
\*---------------------------------------------------------------------------*/

class patchSeedSet
:
    public sampledSet
{
    // Private data

        //- Patches to sample
        const labelHashSet patchSet_;

//        //- Maximum distance to look for nearest
//        const scalar searchDist_;
//
//        //- Offset distance
//        const scalar offsetDist_;

        //- Maximum number of patch faces to seed
        const label maxPoints_;

        //- Random number generator (if maxPoints < num patch faces)
        autoPtr<Random> rndGenPtr_;


    // Private Member Functions

        //- Samples all points in sampleCoords.
        void calcSamples
        (
            DynamicList<point>& samplingPts,
            DynamicList<label>& samplingCells,
            DynamicList<label>& samplingFaces,
            DynamicList<label>& samplingSegments,
            DynamicList<scalar>& samplingCurveDist
        );

        //- Uses calcSamples to obtain samples. Copies them into *this.
        void genSamples();


public:

    //- Runtime type information
    TypeName("patchSeed");


    // Constructors

        //- Construct from dictionary
        patchSeedSet
        (
            const word& name,
            const polyMesh& mesh,
            const meshSearch& searchEngine,
            const dictionary& dict
        );


    //- Destructor
    virtual ~patchSeedSet();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
