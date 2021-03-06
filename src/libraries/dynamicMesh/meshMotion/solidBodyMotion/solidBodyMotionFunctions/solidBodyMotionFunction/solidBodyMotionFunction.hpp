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

Namespace
    CML::solidBodyMotionFunctions

Description
    Namespace for solid-body motions


Class
    CML::solidBodyMotionFunction

Description
    Base class for defining solid-body motions

SourceFiles
    solidBodyMotionFunction.cpp
    dynamicFvMeshNew.cpp

\*---------------------------------------------------------------------------*/

#ifndef solidBodyMotionFunction_H
#define solidBodyMotionFunction_H

#include "Time.hpp"
#include "dictionary.hpp"
#include "septernion.hpp"
#include "autoPtr.hpp"
#include "runTimeSelectionTables.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class solidBodyMotionFunction Declaration
\*---------------------------------------------------------------------------*/

class solidBodyMotionFunction
{
protected:

    // Protected data

        dictionary SBMFCoeffs_;
        const Time& time_;
        // Does the motion function calculate incremental motion
        bool incremental_;


private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        solidBodyMotionFunction(const solidBodyMotionFunction&);

        //- Disallow default bitwise assignment
        void operator=(const solidBodyMotionFunction&);


public:

    //- Runtime type information
    TypeName("solidBodyMotionFunction");


    // Declare run-time constructor selection table

        declareRunTimeSelectionTable
        (
            autoPtr,
            solidBodyMotionFunction,
            dictionary,
            (const dictionary& SBMFCoeffs, const Time& runTime, const bool incremental),
            (SBMFCoeffs, runTime, incremental)
        );


    // Constructors

        //- Construct from the SBMFCoeffs dictionary and Time
        solidBodyMotionFunction
        (
            const dictionary& SBMFCoeffs,
            const Time& runTime,
            const bool incremental
        );


    // Selectors

        //- Select constructed from the SBMFCoeffs dictionary and Time
        static autoPtr<solidBodyMotionFunction> New
        (
            const dictionary& SBMFCoeffs,
            const Time& runTime,
            const bool incremental = false
        );


    //- Destructor
    virtual ~solidBodyMotionFunction();


    // Member Functions

        //- Return the solid-body motion transformation septernion
        virtual septernion transformation() const = 0;

        //- Update properties from given dictionary
        virtual bool read(const dictionary& SBMFCoeffs) = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
