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

Description
    Starts timing CPU usage and return elapsed time from start.

\*---------------------------------------------------------------------------*/

#include "cpuTime.hpp"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

inline
void cpuTime::getTime(std::clock_t& t)
{
    t = std::clock();
}


inline
double cpuTime::timeDifference
(
    const std::clock_t& start,
    const std::clock_t& end
)
{
    const double difference = std::difftime(end, start)/CLOCKS_PER_SEC;
    return difference;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

cpuTime::cpuTime()
{
    getTime(startTime_);
    lastTime_ = startTime_;
    newTime_ = startTime_;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

double cpuTime::elapsedCpuTime() const
{
    getTime(newTime_);
    const double elapsed = timeDifference(startTime_, newTime_);
    return elapsed;
}


double cpuTime::cpuTimeIncrement() const
{
    lastTime_ = newTime_;
    getTime(newTime_);
    const double increment = timeDifference(lastTime_, newTime_);
    return increment;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
