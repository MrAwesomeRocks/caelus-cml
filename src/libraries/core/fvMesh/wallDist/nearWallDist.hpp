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
    CML::nearWallDist

Description
    Distance calculation for cells with face on a wall.
    Searches pointNeighbours to find closest.

SourceFiles
    nearWallDist.cpp

\*---------------------------------------------------------------------------*/

#ifndef nearWallDist_H
#define nearWallDist_H

#include "volFields.hpp"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

class fvMesh;

/*---------------------------------------------------------------------------*\
                           Class nearWallDist Declaration
\*---------------------------------------------------------------------------*/

class nearWallDist
:
    public volScalarField::GeometricBoundaryField
{
    // Private data

        //- Reference to mesh
        const fvMesh& mesh_;


    // Private Member Functions

        //- Do all calculations
        void calculate();

        //- Disallow default bitwise copy construct
        nearWallDist(const nearWallDist&);

        //- Disallow default bitwise assignment
        void operator=(const nearWallDist&);


public:

    // Constructors

        //- Construct from components
        nearWallDist(const fvMesh& mesh);


    //- Destructor
    virtual ~nearWallDist();


    // Member Functions

        const volScalarField::GeometricBoundaryField& y() const
        {
            return *this;
        }

        //- Correct for mesh geom/topo changes
        virtual void correct();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
