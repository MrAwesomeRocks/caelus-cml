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
    CML::subBody

Description
    This specialized rigidBody holds the original body after it has been merged
    into a master.

SourceFiles
    subBodyI.hpp
    subBody.cpp

\*---------------------------------------------------------------------------*/

#ifndef RBD_subBody_HPP
#define RBD_subBody_HPP

#include "rigidBody.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace RBD
{

/*---------------------------------------------------------------------------*\
                         Class subBody Declaration
\*---------------------------------------------------------------------------*/

class subBody
{
    // Private data

        //- Original body from which this sub-body was constructed
        autoPtr<rigidBody> body_;

        //- Master body name
        word masterName_;

        //- Master body ID
        label masterID_;

        //- Transform with respect to master body
        spatialTransform masterXT_;


public:

    // Constructors

        //- Construct a merged version of the given rigidBody
        //  providing the ID of the master body to which this will be merged
        //  and the transform relative to the master
        inline subBody
        (
            const autoPtr<rigidBody>& bodyPtr,
            const word& masterName,
            const label masterID,
            const spatialTransform& masterXT
        );

        //- Return clone of this subBody
        inline autoPtr<subBody> clone() const;


    // Member Functions

        //- Return the original body from which this sub-body was constructed
        inline const rigidBody& body() const;

        //- Return the body name
        inline const word& name() const;

        //- Return the master body name
        inline const word& masterName() const;

        //- Return the master body Id
        inline label masterID() const;

        //- Return the transform with respect to the master body
        inline const spatialTransform& masterXT() const;

        //- Write
        void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace RBD
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "subBodyI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
