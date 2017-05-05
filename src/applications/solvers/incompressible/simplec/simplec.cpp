/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
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

Application
    simplec

Description
    Steady-state solver for incompressible, turbulent flow based on 
    SIMPLEC algorithm.

Author
    Aleksandar Jemcov

\*---------------------------------------------------------------------------*/

#include "fvCFD.hpp"
#include "singlePhaseTransportModel.hpp"
#include "RASModel.hpp"
#include "simplecControl.hpp"
#include "fvIOoptionList.hpp"


int main(int argc, char *argv[])
{
    #include "setRootCase.hpp"
    #include "createTime.hpp"
    #include "createMesh.hpp"
    #include "createFields.hpp"
    #include "createFvOptions.hpp"
    #include "initContinuityErrs.hpp"

    simplecControl simplec(mesh);

    Info<< "\nStarting time loop\n" << endl;

    while (simplec.loop())
    {
        Info<< "Time = " << runTime.timeName() << nl << endl;

        {
            #include "UEqn.hpp"
            #include "pEqn.hpp"
        }

        turbulence->correct();

        runTime.write();

        #include "reportTimeStats.hpp"            
    }

    Info<< "End\n" << endl;

    return 0;
}



