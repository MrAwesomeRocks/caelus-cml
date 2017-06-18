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

Namespace
    CML::RBD::restraints

Description
    Namespace for rigid-body dynamics restraints

Class
    CML::RBD::restraint

Description
    Base class for defining restraints for rigid-body dynamics

SourceFiles
    rigidBodyRestraint.cpp
    rigidBodyRestraintNew.cpp

\*---------------------------------------------------------------------------*/

#ifndef RBD_rigidBodyRestraint_HPP
#define RBD_rigidBodyRestraint_HPP

#include "dictionary.hpp"
#include "autoPtr.hpp"
#include "spatialVector.hpp"
#include "point.hpp"
#include "scalarField.hpp"
#include "runTimeSelectionTables.hpp"
#include "OFstream.hpp"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace RBD
{

// Forward declaration of classes
class rigidBodyModel;

/*---------------------------------------------------------------------------*\
                Class restraint Declaration
\*---------------------------------------------------------------------------*/

class restraint
{

protected:

    // Protected data

        //- Name of the restraint
        word name_;

        //- ID of the body the restraint is applied to
        label bodyID_;

        //- Index of the body the force is applied to
        label bodyIndex_;

        //- Restraint model specific coefficient dictionary
        dictionary coeffs_;

        //- Reference to the model
        const rigidBodyModel& model_;

        //- Transform the given point on the restrained body to the global frame
        inline point bodyPoint(const point& p) const;

        //- Transform the given force spatialVector on the body to the global frame
        inline spatialVector bodyForce(const vector& m, const vector& f) const;

        //- Transform the velocity of the given point on the restrained body
        //  to the global frame
        inline spatialVector bodyPointVelocity(const point& p) const;


public:

    //- Runtime type information
    TypeName("restraint");


    // Declare run-time constructor selection table

        declareRunTimeSelectionTable
        (
            autoPtr,
            restraint,
            dictionary,
            (
                const word& name,
                const dictionary& dict,
                const rigidBodyModel& model
            ),
            (name, dict, model)
        );


    // Constructors

        //- Construct from the dict dictionary and Time
        restraint
        (
            const word& name,
            const dictionary& dict,
            const rigidBodyModel& model
        );

        //- Construct and return a clone
        virtual autoPtr<restraint> clone() const = 0;


    // Selectors

        //- Select constructed from the dict dictionary and Time
        static autoPtr<restraint> New
        (
            const word& name,
            const dictionary& dict,
            const rigidBodyModel& model
        );


    //- Destructor
    virtual ~restraint();


    // Member Functions

        //- Return the name
        const word& name() const
        {
            return name_;
        }

        label bodyID() const
        {
            return bodyID_;
        }

        //- Accumulate the retraint internal joint forces into the tau field and
        //  external forces into the fx field
        virtual void restrain
        (
            scalarField& tau,
            Field<spatialVector>& fx
        ) const = 0;

        //- Update properties from given dictionary
        virtual bool read(const dictionary& dict);

        //- Return access to coeffs
        const dictionary& coeffDict() const;

        //- Write
        virtual void write(Ostream&) const = 0;
        
        virtual void writeState(autoPtr<OFstream> &stateFilePtr) const = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace RBD
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "rigidBodyRestraintI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //