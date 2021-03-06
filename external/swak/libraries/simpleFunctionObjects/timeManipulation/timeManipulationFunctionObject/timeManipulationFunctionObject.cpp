/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright  held by original author
-------------------------------------------------------------------------------
License
    This file is based on CAELUS.

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

Contributors/Copyright:
    2008-2011, 2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#include "timeManipulationFunctionObject.hpp"
#include "addToRunTimeSelectionTable.hpp"

#include "polyMesh.hpp"
#include "IOmanip.hpp"
#include "Time.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(timeManipulationFunctionObject, 0);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

timeManipulationFunctionObject::timeManipulationFunctionObject
(
    const word &name,
    const Time& t,
    const dictionary& dict
)
:
    simpleFunctionObject(name,t,dict),
    tolerateAdaptiveTimestep_(false),
    myEndTime_(-1)
{
}

bool timeManipulationFunctionObject::start()
{
    simpleFunctionObject::start();

    tolerateAdaptiveTimestep_=dict_.lookupOrDefault<bool>(
        "tolerateAdaptiveTimestep",false
    );

    if(
        !tolerateAdaptiveTimestep_
        &&
        time().controlDict().lookupOrDefault<bool>("adjustTimeStep",false)
    ) {
        FatalErrorInFunction
            << "'adjustTimeStep' is set. Function object " << name()
                << " manipulates time. This may lead to strange behaviour. "
                << " If you think that is OK set 'tolerateAdaptiveTimestep'"
                << " in " << name()
                << endl
                << exit(FatalError);
    }

    return true;
}

void timeManipulationFunctionObject::write()
{
    scalar newDeltaT=this->deltaT();
    scalar newEndTime=this->endTime();

    scalar minDeltaT=newDeltaT;
    scalar maxDeltaT=newDeltaT;
    reduce(minDeltaT,minOp<scalar>());
    reduce(maxDeltaT,maxOp<scalar>());
    if(minDeltaT!=maxDeltaT) {
        FatalErrorInFunction
            << "Across the processors the minimum " << minDeltaT
                << " and the maximum " << maxDeltaT << " of the new deltaT"
                << " differ by " << maxDeltaT-minDeltaT
                << endl
                << exit(FatalError);

    }
    if(newDeltaT!=time().deltaT().value()) {
        Info << name() << " sets deltaT from " <<
            time().deltaT().value() << " to " << newDeltaT << endl;

        const_cast<Time&>(time()).setDeltaT(newDeltaT);
    }

    scalar minEndTime=newEndTime;
    scalar maxEndTime=newEndTime;
    reduce(minEndTime,minOp<scalar>());
    reduce(maxEndTime,maxOp<scalar>());
    if(minEndTime!=maxEndTime) {
        FatalErrorInFunction
            << "Across the processors the minimum " << minEndTime
                << " and the maximum " << maxEndTime << " of the new endTime"
                << " differ by " << maxEndTime-minEndTime
                << endl
                << exit(FatalError);

    }
    if(newEndTime!=time().endTime().value()) {
        Info << name() << " sets endTime from " <<
            time().endTime().value() << " to " << newEndTime << endl;

        const_cast<Time&>(time()).setEndTime(newEndTime);

        myEndTime_=newEndTime;
    }

    if(
        myEndTime_>-1
        &&
        time().value()>=time().endTime().value()
        &&
        !time().outputTime()
    ) {
        WarningInFunction
            << "Forcing write because we (" << name()
                << ") changed the endTime to "
                << myEndTime_ << " and this is not a write-time"
                << endl;

        const_cast<Time&>(time()).writeNow();
    }
}

scalar timeManipulationFunctionObject::deltaT()
{
    return time().deltaT().value();
}

scalar timeManipulationFunctionObject::endTime()
{
    return time().endTime().value();
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

} // namespace CML

// ************************************************************************* //
