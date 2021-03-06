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
    CML::RBD::joints::floating

Description
    Prismatic joint for translation along the specified arbitrary axis.

    Reference:
    \verbatim
        Featherstone, R. (2008).
        Rigid body dynamics algorithms.
        Springer.
        Chapter 4.
    \endverbatim

SourceFiles
    floating.cpp

\*---------------------------------------------------------------------------*/

#ifndef RBD_joints_floating_HPP
#define RBD_joints_floating_HPP

#include "compositeJoint.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace RBD
{
namespace joints
{

/*---------------------------------------------------------------------------*\
                         Class floating Declaration
\*---------------------------------------------------------------------------*/

class floating
:
    public composite
{
    // Private member functions

        //- Return a list of joints needed to emulate a floating body
        static autoPtr<composite> sixDoF();


public:

    //- Runtime type information
    TypeName("floating");


    // Constructors

        //- Construct
        floating();

        //- Construct for given model from dictionary
        floating(const dictionary& dict);

        //- Clone this joint
        virtual autoPtr<joint> clone() const;


    //- Destructor
    virtual ~floating();


    // Member Functions

        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace joints
} // End namespace RBD
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
