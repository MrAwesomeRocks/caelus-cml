/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2015 OpenFOAM Foundation
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
    write function for regIOobjects

\*---------------------------------------------------------------------------*/

#include "regIOobject.hpp"
#include "Time.hpp"
#include "OSspecific.hpp"
#include "OFstream.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

bool CML::regIOobject::writeObject
(
    IOstream::streamFormat fmt,
    IOstream::versionNumber ver,
    IOstream::compressionType cmp
) const
{
    if (!good())
    {
        SeriousErrorInFunction
            << "bad object " << name()
            << endl;

        return false;
    }

    if (instance().empty())
    {
        SeriousErrorInFunction
            << "instance undefined for object " << name()
            << endl;

        return false;
    }

    if
    (
        instance() != time().timeName()
     && instance() != time().system()
     && instance() != time().caseSystem()
     && instance() != time().constant()
     && instance() != time().caseConstant()
    )
    {
        const_cast<regIOobject&>(*this).instance() = time().timeName();
    }

    mkDir(path());

    if (OFstream::debug)
    {
        Info<< "regIOobject::write() : "
            << "writing file " << objectPath();
    }


    bool osGood = false;

    {
        // Try opening an OFstream for object
        OFstream os(objectPath(), fmt, ver, cmp);

        // If any of these fail, return (leave error handling to Ostream class)
        if (!os.good())
        {
            return false;
        }

        if (!writeHeader(os))
        {
            return false;
        }

        // Write the data to the Ostream
        if (!writeData(os))
        {
            return false;
        }

        writeEndDivider(os);

        osGood = os.good();
    }

    if (OFstream::debug)
    {
        Info<< " .... written" << endl;
    }

    // Only update the lastModified_ time if this object is re-readable,
    // i.e. lastModified_ is already set
    if (watchIndex_ != -1)
    {
        time().setUnmodified(watchIndex_);
    }

    return osGood;
}


bool CML::regIOobject::write() const
{
    return writeObject
    (
        time().writeFormat(),
        IOstream::currentVersion,
        time().writeCompression()
    );
}

// ************************************************************************* //
