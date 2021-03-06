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
    CML::triSurfaceMeshPointSet

Description
    sampleSet from all points of a triSurfaceMesh.

SourceFiles
    triSurfaceMeshPointSet.cpp

\*---------------------------------------------------------------------------*/

#ifndef triSurfaceMeshPointSet_H
#define triSurfaceMeshPointSet_H

#include "sampledSet.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes

/*---------------------------------------------------------------------------*\
                           Class triSurfaceMeshPointSet Declaration
\*---------------------------------------------------------------------------*/

class triSurfaceMeshPointSet
:
    public sampledSet
{
    // Private data

        //- Name of triSurfaceMesh
        const word surface_;

        //- Sampling points
        List<point> sampleCoords_;


    // Private Member Functions

        //- Samples all points in sampleCoords.
        void calcSamples
        (
            DynamicList<point>& samplingPts,
            DynamicList<label>& samplingCells,
            DynamicList<label>& samplingFaces,
            DynamicList<label>& samplingSegments,
            DynamicList<scalar>& samplingCurveDist
        ) const;

        //- Uses calcSamples to obtain samples. Copies them into *this.
        void genSamples();


public:

    //- Runtime type information
    TypeName("triSurfaceMeshPointSet");


    // Constructors

        //- Construct from dictionary
        triSurfaceMeshPointSet
        (
            const word& name,
            const polyMesh& mesh,
            const meshSearch& searchEngine,
            const dictionary& dict
        );


    //- Destructor
    virtual ~triSurfaceMeshPointSet();


    // Member Functions

        //- Get reference point
        virtual point getRefPoint(const List<point>&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
