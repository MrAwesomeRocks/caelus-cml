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

#include "regIOobject.hpp"
#include "IFstream.hpp"
#include "Time.hpp"
#include "Pstream.hpp"


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::Istream& CML::regIOobject::readStream()
{
    if (IFstream::debug)
    {
        Info<< "regIOobject::readStream() : "
            << "reading object " << name()
            << " from file " << objectPath()
            << endl;
    }

    if (readOpt() == NO_READ)
    {
        FatalErrorInFunction
            << "NO_READ specified for read-constructor of object " << name()
            << " of class " << headerClassName()
            << abort(FatalError);
    }

    // Construct object stream and read header if not already constructed
    if (!isPtr_)
    {

        fileName objPath;
        if (watchIndex_ != -1)
        {
            // File is being watched. Read exact file that is being watched.
            objPath = time().getFile(watchIndex_);
        }
        else
        {
            // Search intelligently for file
            objPath = filePath();

            if (!objPath.size())
            {
                FatalIOError
                (
                    "regIOobject::readStream()",
                    __FILE__,
                    __LINE__,
                    objectPath(),
                    0
                )   << "cannot find file"
                    << exit(FatalIOError);
            }
        }

        if (!(isPtr_ = objectStream(objPath)))
        {
            FatalIOError
            (
                "regIOobject::readStream()",
                __FILE__,
                __LINE__,
                objPath,
                0
            )   << "cannot open file"
                << exit(FatalIOError);
        }
        else if (!readHeader(*isPtr_))
        {
            FatalIOErrorInFunction(*isPtr_)
                << "problem while reading header for object " << name()
                << exit(FatalIOError);
        }
    }

    // Mark as uptodate if read successfully
    if (watchIndex_ != -1)
    {
        time().setUnmodified(watchIndex_);
    }

    return *isPtr_;
}


CML::Istream& CML::regIOobject::readStream(const word& expectName)
{
    if (IFstream::debug)
    {
        Info<< "regIOobject::readStream(const word&) : "
            << "reading object " << name()
            << " from file " << objectPath()
            << endl;
    }

    // Construct IFstream if not already constructed
    if (!isPtr_)
    {
        readStream();

        // Check the className of the regIOobject
        // dictionary is an allowable name in case the actual class
        // instantiated is a dictionary
        if
        (
            expectName.size()
         && headerClassName() != expectName
         && headerClassName() != "dictionary"
        )
        {
            FatalIOErrorInFunction(*isPtr_)
                << "unexpected class name " << headerClassName()
                << " expected " << expectName << endl
                << "    while reading object " << name()
                << exit(FatalIOError);
        }
    }

    return *isPtr_;
}


void CML::regIOobject::close()
{
    if (IFstream::debug)
    {
        Info<< "regIOobject::close() : "
            << "finished reading " << filePath()
            << endl;
    }

    if (isPtr_)
    {
        delete isPtr_;
        isPtr_ = nullptr;
    }
}


bool CML::regIOobject::readData(Istream&)
{
    return false;
}


bool CML::regIOobject::read()
{
    // Note: cannot do anything in readStream itself since this is used by
    // e.g. GeometricField.

    bool masterOnly =
        regIOobject::fileModificationChecking == timeStampMaster
     || regIOobject::fileModificationChecking == inotifyMaster;

    bool ok = true;
    if (Pstream::master() || !masterOnly)
    {
        if (IFstream::debug)
        {
            Pout<< "regIOobject::read() : "
                << "reading object " << name()
                << " from file " << endl;
        }

        // Set flag
        bool oldFlag = regIOobject::masterOnlyReading;
        regIOobject::masterOnlyReading = masterOnly;

        // Read file
        ok = readData(readStream(type()));
        close();

        regIOobject::masterOnlyReading = oldFlag;
    }

    if (masterOnly && Pstream::parRun())
    {
        // Scatter master data using communication scheme

        const List<Pstream::commsStruct>& comms =
        (
            (Pstream::nProcs() < Pstream::nProcsSimpleSum)
          ? Pstream::linearCommunication()
          : Pstream::treeCommunication()
        );

        // Master reads headerclassname from file. Make sure this gets
        // transferred as well as contents.
        Pstream::scatter
        (
            comms,
            const_cast<word&>(headerClassName()),
            Pstream::msgType()
        );
        Pstream::scatter(comms, note(), Pstream::msgType());


        // Get my communication order
        const Pstream::commsStruct& myComm = comms[Pstream::myProcNo()];

        // Reveive from up
        if (myComm.above() != -1)
        {
            if (IFstream::debug)
            {
                Pout<< "regIOobject::read() : "
                    << "reading object " << name()
                    << " from processor " << myComm.above()
                    << endl;
            }

            // Note: use ASCII for now - binary IO of dictionaries is
            // not currently supported
            IPstream fromAbove
            (
                Pstream::scheduled,
                myComm.above(),
                0,
                Pstream::msgType(),
                IOstream::ASCII
            );
            ok = readData(fromAbove);
        }

        // Send to my downstairs neighbours
        forAll(myComm.below(), belowI)
        {
            OPstream toBelow
            (
                Pstream::scheduled,
                myComm.below()[belowI],
                0,
                Pstream::msgType(),
                IOstream::ASCII
            );
            writeData(toBelow);
        }
    }
    return ok;
}


bool CML::regIOobject::modified() const
{
    if (watchIndex_ != -1)
    {
        return time().getState(watchIndex_) != fileMonitor::UNMODIFIED;
    }
    else
    {
        return false;
    }
}


bool CML::regIOobject::readIfModified()
{
    if (watchIndex_ != -1)
    {
        if (modified())
        {
            const fileName& fName = time().getFile(watchIndex_);
            Info<< "regIOobject::readIfModified() : " << nl
                << "    Re-reading object " << name()
                << " from file " << fName << endl;
            return read();
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}


// ************************************************************************* //
