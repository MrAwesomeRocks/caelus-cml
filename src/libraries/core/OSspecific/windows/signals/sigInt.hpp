/*---------------------------------------------------------------------------*\
Copyright (C) 2011 Symscape
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
    sigInt

Description
    Signal handler for INT interrupt.  The standard interrupt handler is
    overridden to ensure that the runningJob file is removed.

SourceFiles
    sigInt.cpp

\*---------------------------------------------------------------------------*/

#ifndef sigInt_H
#define sigInt_H

#include <signal.h>
typedef	void (*__p_sig_fn_t)(int);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class sigInt Declaration
\*---------------------------------------------------------------------------*/

class sigInt
{
    // Private data

        //- Saved old signal trapping setting
        static __p_sig_fn_t oldAction_;


    // Private Member Functions

        static void sigIntHandler(int);


public:

    // Constructors

        sigInt();


    // Destructor

        ~sigInt();


    // Member functions

        //- Activate SIGINT signal handler
        void set(const bool verbose);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
