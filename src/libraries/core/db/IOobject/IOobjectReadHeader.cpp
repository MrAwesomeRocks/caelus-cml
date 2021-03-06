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

\*---------------------------------------------------------------------------*/

#include "IOobject.hpp"
#include "dictionary.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

bool CML::IOobject::readHeader(Istream& is)
{
    if (IOobject::debug)
    {
        Info<< "IOobject::readHeader(Istream&) : reading header for file "
            << is.name() << endl;
    }

    // Check Istream not already bad
    if (!is.good())
    {
        if (rOpt_ == MUST_READ || rOpt_ == MUST_READ_IF_MODIFIED)
        {
            FatalIOErrorInFunction(is)
                << " stream not open for reading essential object from file "
                << is.name()
                << exit(FatalIOError);
        }

        if (IOobject::debug)
        {
            SeriousIOErrorInFunction(is)
                << " stream not open for reading from file "
                << is.name() << endl;
        }

        return false;
    }

    token firstToken(is);

    if
    (
        is.good()
     && firstToken.isWord()
     && firstToken.wordToken() == "FoamFile"
    )
    {
        dictionary headerDict(is);

        is.version(headerDict.lookup("version"));
        is.format(headerDict.lookup("format"));
        headerClassName_ = word(headerDict.lookup("class"));

        const word headerObject(headerDict.lookup("object"));
        if (IOobject::debug && headerObject != name())
        {
            IOWarningInFunction(is)
                << " object renamed from "
                << name() << " to " << headerObject
                << " for file " << is.name() << endl;
        }

        // The note entry is optional
        headerDict.readIfPresent("note", note_);
    }
    else
    {
        SeriousIOErrorInFunction(is)
            << "First token could not be read or is not the keyword 'FoamFile'"
            << nl << nl << "Check header is of the form:" << nl << endl;

        writeHeader(Info);

        return false;
    }

    // Check stream is still OK
    if (is.good())
    {
        objState_ = GOOD;
    }
    else
    {
        if (rOpt_ == MUST_READ || rOpt_ == MUST_READ_IF_MODIFIED)
        {
            FatalIOErrorInFunction(is)
                << " stream failure while reading header"
                << " on line " << is.lineNumber()
                << " of file " << is.name()
                << " for essential object" << name()
                << exit(FatalIOError);
        }

        if (IOobject::debug)
        {
            Info<< "IOobject::readHeader(Istream&) :"
                << " stream failure while reading header"
                << " on line " << is.lineNumber()
                << " of file " << is.name() << endl;
        }

        objState_ = BAD;

        return false;
    }

    if (IOobject::debug)
    {
        Info<< " .... read" << endl;
    }

    return true;
}


// ************************************************************************* //
