/*---------------------------------------------------------------------------*\
Copyright (C) 2016 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Caelus is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

Class
    CML::RBD::rigidBodySolvers::Newmark

Description
    Newmark 2nd-order time-integrator for 6DoF solid-body motion.

    Reference:
    \verbatim
        Newmark, N. M. (1959).
        A method of computation for structural dynamics.
        Journal of the Engineering Mechanics Division, 85(3), 67-94.
    \endverbatim

    Example specification in dynamicMeshDict:
    \verbatim
    solver
    {
        type    Newmark;
        gamma   0.5;    // Velocity integration coefficient
        beta    0.25;   // Position integration coefficient
    }
    \endverbatim

SourceFiles
    Newmark.cpp

\*---------------------------------------------------------------------------*/

#ifndef Newmark_HPP
#define Newmark_HPP

#include "rigidBodySolver.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace RBD
{
namespace rigidBodySolvers
{

/*---------------------------------------------------------------------------*\
                           Class Newmark Declaration
\*---------------------------------------------------------------------------*/

class Newmark
:
    public rigidBodySolver
{
    // Private data

        //- Coefficient for velocity integration (default: 0.5)
        const scalar gamma_;

        //- Coefficient for position and orientation integration (default: 0.25)
        const scalar beta_;


public:

    //- Runtime type information
    TypeName("Newmark");


    // Constructors

        //- Construct for the given body from dictionary
        Newmark
        (
            rigidBodyMotion& body,
            const dictionary& dict
        );


    //- Destructor
    virtual ~Newmark();


    // Member Functions

        //- Integrate the rigid-body motion for one time-step
        virtual void solve
        (
            const scalarField& tau,
            const Field<spatialVector>& fx
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace rigidBodySolvers
} // End namespace RBD
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
