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
    2011-2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#include "executeIfExecutableFitsFunctionObject.hpp"
#include "addToRunTimeSelectionTable.hpp"

#include "polyMesh.hpp"
#include "IOmanip.hpp"
#include "Time.hpp"
#include "argList.hpp"

#ifdef darwin
#include "mach-o/dyld.h"
#endif
#ifdef __linux__
#include <unistd.h>
#endif
#ifdef windows
#undef DebugInfo
#include <shlwapi.h>
#endif
// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(executeIfExecutableFitsFunctionObject, 0);

    addToRunTimeSelectionTable
    (
        functionObject,
        executeIfExecutableFitsFunctionObject,
        dictionary
    );

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

executeIfExecutableFitsFunctionObject::executeIfExecutableFitsFunctionObject
(
    const word& name,
    const Time& t,
    const dictionary& dict
)
:
    conditionalFunctionObjectListProxy(
        name,
        t,
        dict
    )
{
    // do it here to avoid the superclass-read being read twice
    readRegexp(dict);

    fileName exePath;
    
#ifdef darwin
    {
        char path[1024];
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0) {
            exePath=string(path);
        }
    }
#elif defined(__linux__)
    {
        const int bufSize=1024;
        char path[bufSize];
        label length=readlink("/proc/self/exe",path,bufSize-1);
        path[length]='\0';
        exePath=string(path);
    }
#else
    {
		const int bufSize=1024;
		char path[bufSize];
		GetModuleFileName(nullptr, path, bufSize);
		exePath=string(path);
    }
#endif

    executable_=exePath.name();

    if(debug) {
        Info << "Executable: " << executable_ << " "<< exePath << endl;
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool executeIfExecutableFitsFunctionObject::condition()
{
    return executableNameRegexp_.match(executable_);
}

void executeIfExecutableFitsFunctionObject::readRegexp(const dictionary& dict)
{
    executableNameRegexp_.set(
        string(dict.lookup("executableNameRegexp")),
        dict.lookupOrDefault<bool>("ignoreCase",false)
    );
}

bool executeIfExecutableFitsFunctionObject::read(const dictionary& dict)
{
    readRegexp(dict);
    return conditionalFunctionObjectListProxy::read(dict);
}

} // namespace CML

// ************************************************************************* //
