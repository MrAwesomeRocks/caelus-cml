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
    CML::velocityComponentLaplacianFvMotionSolver

Description
    Mesh motion solver for an fvMesh.  Based on solving the cell-centre
    Laplacian for the given component of the motion velocity.

SourceFiles
    velocityComponentLaplacianFvMotionSolver.cpp

\*---------------------------------------------------------------------------*/

#ifndef velocityComponentLaplacianFvMotionSolver_H
#define velocityComponentLaplacianFvMotionSolver_H

#include "componentVelocityMotionSolver.hpp"
#include "fvMotionSolverCore.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward class declarations
class motionDiffusivity;

/*---------------------------------------------------------------------------*\
          Class velocityComponentLaplacianFvMotionSolver Declaration
\*---------------------------------------------------------------------------*/

class velocityComponentLaplacianFvMotionSolver
:
    public componentVelocityMotionSolver,
    public fvMotionSolverCore
{
    // Private data

        //- Cell-centre motion field
        mutable volScalarField cellMotionU_;

        //- Diffusivity used to control the motion
        autoPtr<motionDiffusivity> diffusivityPtr_;


    // Private Member Functions

        //- Disallow default bitwise copy construct
        velocityComponentLaplacianFvMotionSolver
        (
            const velocityComponentLaplacianFvMotionSolver&
        );

        //- Disallow default bitwise assignment
        void operator=(const velocityComponentLaplacianFvMotionSolver&);


public:

    //- Runtime type information
    TypeName("velocityComponentLaplacian");


    // Constructors

        //- Construct from polyMesh and IOdictionary
        velocityComponentLaplacianFvMotionSolver
        (
            const polyMesh&,
            const IOdictionary&
        );


    //- Destructor
    ~velocityComponentLaplacianFvMotionSolver();


    // Member Functions

        //- Non-const access to the cellMotionU in order to allow changes
        //  to the boundary motion
        volScalarField& cellMotionU()
        {
            return cellMotionU_;
        }

        //- Return point location obtained from the current motion field
        virtual tmp<pointField> curPoints() const;

        //- Solve for motion
        virtual void solve();

        //- Update topology
        virtual void updateMesh(const mapPolyMesh&);

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
