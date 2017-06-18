/*---------------------------------------------------------------------------*\
    Copyright            : (C) 2011 Symscape
    Website              : www.symscape.com
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
    clockTime

Description
    Starts timing (using rtc) and returns elapsed time from start. Better
    resolution (2uSec instead of ~20mSec) than cpuTime.

SourceFiles
    clockTime.C

\*---------------------------------------------------------------------------*/

#ifndef clockTime_H
#define clockTime_H

#include <sys/types.h>
#include <sys/time.h>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class clockTime Declaration
\*---------------------------------------------------------------------------*/

class clockTime
{
    // Private data

        struct timeval startTime_;
        mutable struct timeval lastTime_;
        mutable struct timeval newTime_;

        static void getTime(struct timeval& t);

        static double timeDifference
        (
            const struct timeval& start,
            const struct timeval& end
        );


public:

    // Constructors

        //- Construct from components
        clockTime();


    // Member Functions

        // Access

            //- Returns CPU time from start of run
            double elapsedTime() const;

            //- Returns CPU time from last call of clockTimeIncrement()
            double timeIncrement() const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
