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
    CML::RBD::rigidBodyMotion

Description
    Six degree of freedom motion for a rigid body.

    Angular momentum stored in body fixed reference frame.  Reference
    orientation of the body (where Q = I) must align with the cartesian axes
    such that the Inertia tensor is in principle component form.  Can add
    restraints (e.g. a spring) and constraints (e.g. motion may only be on a
    plane).

    The time-integrator for the motion is run-time selectable with options for
    symplectic (explicit), Crank-Nicolson and Newmark schemes.

SourceFiles
    rigidBodyMotionI.hpp
    rigidBodyMotion.cpp
    rigidBodyMotionIO.cpp

\*---------------------------------------------------------------------------*/

#ifndef rigidBodyMotion_HPP
#define rigidBodyMotion_HPP

#include "rigidBodyModel.hpp"
#include "rigidBodyModelState.hpp"
#include "pointField.hpp"
#include "Switch.hpp"
#include "OFstream.hpp"
#include "septernion.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace RBD
{

// Forward declarations
class rigidBodySolver;

/*---------------------------------------------------------------------------*\
                      Class rigidBodyMotion Declaration
\*---------------------------------------------------------------------------*/

class rigidBodyMotion
:
    public rigidBodyModel
{
    friend class rigidBodySolver;

    // Private data

        //- Motion state data object
        rigidBodyModelState motionState_;

        //- Motion state data object for previous time-step
        rigidBodyModelState motionState0_;

        //- Initial transform for external forces to the bodies reference frame
        List<spatialTransform> X00_;

        //- Acceleration relaxation coefficient
        scalar aRelax_;

        //- Acceleration damping coefficient (for steady-state simulations)
        scalar aDamp_;

        //- Switch to turn reporting of motion data on and off
        Switch report_;

        //- Motion solver
        autoPtr<rigidBodySolver> solver_;


    // Private Member Functions

        //- Initialize the body-state
        void initialize();

        //- Disallow copy construct
        rigidBodyMotion(const rigidBodyMotion&);

        //- Disallow default bitwise assignment
        void operator=(const rigidBodyMotion&);


public:

    // Constructors

        //- Construct null
        rigidBodyMotion(const Time&);

        //- Construct from dictionary
        rigidBodyMotion
        (
            const Time& time,
            const dictionary& dict
        );

        //- Construct from constant and state dictionaries
        rigidBodyMotion
        (
            const Time& time,
            const dictionary& dict,
            const dictionary& stateDict
        );


    //- Destructor
    ~rigidBodyMotion();


    // Member Functions

        // Access

            //- Return the report Switch
            inline bool report() const;

            //- Return the motion state
            inline const rigidBodyModelState& state() const;

            //- Return the motion state for modification
            inline rigidBodyModelState& state();

            //- Return the initial transform to the global frame for the
            //  given body
            spatialTransform X00(const label bodyId) const;


        // Edit

            //- Store the motion state at the beginning of the time-step
            inline void newTime();


        // Update state

            //- Calculate and optionally relax the joint acceleration qDdot from
            //  the joint state q, velocity qDot, internal force tau (in the
            //  joint frame) and external force fx (in the global frame)
            void forwardDynamics
            (
                rigidBodyModelState& state,
                const scalarField& tau,
                const Field<spatialVector>& fx
            ) const;

            //- Integrate velocities, orientation and position
            //  for the given time-step
            void solve
            (
                scalar deltaT,
                const scalarField& tau,
                const Field<spatialVector>& fx
            );

            //- Report the status of the motion of the given body
            void status(const label bodyID) const;
            //- Write the status of the motion of the given body
            void writeState(const label bodyID, autoPtr<OFstream> &stateFilePtr);
            //- Write the status of the Restraints
            void writeRestState(List<autoPtr<OFstream> > &restraintStateFilePtrs);


        // Transformations

            //- Transform the given initial pointField of the specified body
            //  to correspond to the current motion state
            tmp<pointField> transformPoints
            (
                const label bodyID,
                const pointField& initialPoints
            ) const;

            //- Transform the given initial pointField of the specified body
            //  to correspond to the current motion state scaled using
            //  'slerp' interpolation
            tmp<pointField> transformPoints
            (
                const label bodyID,
                const scalarField& weight,
                const pointField& initialPoints
            ) const;

            //- Transform the given initial pointField of the specified body
            //  to correspond to the current motion state scaled using
            //  'slerp' interpolation
            tmp<pointField> transformPoints
            (
                const labelList& bodyIDs,
                const List<const scalarField*>& weights,
                const pointField& initialPoints
            ) const;

            septernion transformation(const label) const;


        //- Write
        void write(Ostream&) const;

        //- Read coefficients dictionary and update system parameters,
        //  constraints and restraints but not the current state
        bool read(const dictionary& dict);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace RBD
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "rigidBodyMotionI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
