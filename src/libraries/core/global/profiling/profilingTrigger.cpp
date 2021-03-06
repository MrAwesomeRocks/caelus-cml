/*---------------------------------------------------------------------------*\
Copyright (C) 2009-2016 Bernhard Gschaider
Copyright (C) 2016-2017 OpenCFD Ltd.
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

\*---------------------------------------------------------------------------*/

#include "profiling.hpp"
#include "profilingInformation.hpp"
#include "profilingTrigger.hpp"


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::profilingTrigger::profilingTrigger(const char* name)
:
    clock_(),
    ptr_(profiling::New(name, clock_))
{}


CML::profilingTrigger::profilingTrigger(const string& name)
:
    clock_(),
    ptr_(profiling::New(name, clock_))
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::profilingTrigger::~profilingTrigger()
{
    stop();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool CML::profilingTrigger::running() const
{
    return ptr_;
}


void CML::profilingTrigger::stop()
{
    if (ptr_)
    {
        ptr_->update(clock_.elapsedTime());
        profiling::unstack(ptr_);
        // pointer is managed by pool storage -> thus no delete here
    }
    ptr_ = 0;
}


// ************************************************************************* //
