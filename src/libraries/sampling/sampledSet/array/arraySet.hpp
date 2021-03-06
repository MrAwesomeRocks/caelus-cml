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
    CML::arraySet

Description

SourceFiles
    arraySet.cpp

\*---------------------------------------------------------------------------*/

#ifndef arraySet_H
#define arraySet_H

#include "sampledSet.hpp"
#include "DynamicList.hpp"
#include "coordinateSystem.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes
class passiveParticle;
template<class Type> class particle;

/*---------------------------------------------------------------------------*\
                           Class arraySet Declaration
\*---------------------------------------------------------------------------*/

class arraySet
:
    public sampledSet
{
    // Private data

        //- Coordinate syste
        coordinateSystem coordSys_;

        //- Point density vector
        Vector<label> pointsDensity_;

        //- Span box
        Vector<scalar> spanBox_;


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
    TypeName("array");


    // Constructors

        //- Construct from components
        arraySet
        (
            const word& name,
            const polyMesh& mesh,
            const meshSearch& searchEngine,
            const word& axis,
            const coordinateSystem& coordSys,
            const Vector<label>& pointsDensity,
            const Vector<scalar>& spanBox
        );

        //- Construct from dictionary
        arraySet
        (
            const word& name,
            const polyMesh& mesh,
            const meshSearch& searchEngine,
            const dictionary& dict
        );


    //- Destructor
    virtual ~arraySet();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
