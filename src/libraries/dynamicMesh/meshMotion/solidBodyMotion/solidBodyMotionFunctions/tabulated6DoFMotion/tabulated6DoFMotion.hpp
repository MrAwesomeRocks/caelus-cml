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
    CML::solidBodyMotionFunctions::tabulated6DoFMotion

Description
    Tabulated 6DoF motion function.

    Obtained by interpolating tabulated data for surge (x-translation),
    sway (y-translation), heave (z-translation), roll (rotation about x),
    pitch (rotation about y) and yaw (rotation about z).

SourceFiles
    tabulated6DoFMotion.cpp

\*---------------------------------------------------------------------------*/

#ifndef tabulated6DoFMotion_H
#define tabulated6DoFMotion_H

#include "solidBodyMotionFunction.hpp"
#include "primitiveFields.hpp"
#include "Vector2D_.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace solidBodyMotionFunctions
{

/*---------------------------------------------------------------------------*\
                          Class tabulated6DoFMotion Declaration
\*---------------------------------------------------------------------------*/

class tabulated6DoFMotion
:
    public solidBodyMotionFunction
{
    // Private data

        //- Time data file name read from dictionary
        fileName timeDataFileName_;

        //- Center of gravity read from dictionary
        vector CofG_;

        //- Type used to read in the translation and rotation "vectors"
        typedef Vector2D<vector> translationRotationVectors;

        //- Field of times
        scalarField times_;

        //- Field of translation and rotation "vectors"
        Field<translationRotationVectors> values_;


    // Private Member Functions

        //- Disallow copy construct
        tabulated6DoFMotion(const tabulated6DoFMotion&);

        //- Disallow default bitwise assignment
        void operator=(const tabulated6DoFMotion&);


public:

    //- Runtime type information
    TypeName("tabulated6DoFMotion");


    // Constructors

        //- Construct from components
        tabulated6DoFMotion
        (
            const dictionary& SBMFCoeffs,
            const Time& runTime,
            const bool incremental
        );


    //- Destructor
    virtual ~tabulated6DoFMotion();


    // Member Functions

        //- Return the solid-body motion transformation septernion
        virtual septernion transformation() const;

        //- Update properties from given dictionary
        virtual bool read(const dictionary& SBMFCoeffs);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace solidBodyMotionFunctions
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
