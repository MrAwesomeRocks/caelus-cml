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
    CML::sigQuit

Description
    Signal handler for QUIT interrupt.

    The standard interrupt handler is overridden to ensure that the
    runningJob file is removed.

See Also
    CML::JobInfo

SourceFiles
    sigQuit.cpp

\*---------------------------------------------------------------------------*/

#ifndef sigQuit_H
#define sigQuit_H

#include <signal.h>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class sigQuit Declaration
\*---------------------------------------------------------------------------*/

class sigQuit
{
    // Private data

        //- Saved old signal trapping setting
        static struct sigaction oldAction_;


    // Private Member Functions

        //- Handler for caught signals
        static void sigHandler(int);


public:

    // Constructors

        //- Construct null
        sigQuit();


    //- Destructor
    ~sigQuit();


    // Member functions

        //- Activate SIGQUIT signal handler
        void set(const bool verbose);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
