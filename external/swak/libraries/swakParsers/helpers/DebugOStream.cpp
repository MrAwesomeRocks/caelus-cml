/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright (C) 1991-2008 OpenCFD Ltd.
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
    2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#include "DebugOStream.hpp"
#include "Pstream.hpp"
#include <sstream>

namespace CML {

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

    // Move to separate module later
DebugOStream::DebugOStream(
    ostream &o,
    const word &typeName,
    const void *object,
    const bool parallel
)
    :
    prefixOSstream(
        o,
        typeName
    )
{
    prefix()=typeName+"("+getHex(object)+") ";
    if(Pstream::parRun() && parallel) {
        std::ostringstream proc;
        proc << "[" << Pstream::myProcNo() << "]";
        prefix()=proc.str()+prefix();
    }
}

word getHex(const void *ptr)
{
    std::ostringstream makeHex;
    makeHex << std::hex << (void*)ptr;
    return word(makeHex.str());
}

// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * Friend Functions  * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //

} // namespace

// ************************************************************************* //
