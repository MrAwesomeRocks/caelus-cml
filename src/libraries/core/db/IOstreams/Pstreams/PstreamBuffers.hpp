/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
Copyright (C) 2011 OpenFOAM Foundation
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
    CML::PstreamBuffers

Description
    Buffers for inter-processor communications streams (UOPstream, UIPstream).

    Use UOPstream to stream data into buffers, call finishedSends() to
    notify that data is in buffers and then use IUPstream to get data out
    of received buffers. Works with both blocking and nonBlocking. Does
    not make much sense with scheduled since there you would not need these
    explicit buffers.

    Example usage:

        PstreamBuffers pBuffers(Pstream::nonBlocking);

        for (label proci = 0; proci < Pstream::nProcs(); proci++)
        {
            if (proci != Pstream::myProcNo())
            {
                someObject vals;

                UOPstream str(proci, pBuffers);
                str << vals;
            }
        }

        pBuffers.finishedSends();   // no-op for blocking

        for (label proci = 0; proci < Pstream::nProcs(); proci++)
        {
            if (proci != Pstream::myProcNo())
            {
                UIPstream str(proci, pBuffers);
                someObject vals(str);
            }
        }


SourceFiles
    PstreamBuffers.cpp

\*---------------------------------------------------------------------------*/

#include "Pstream.hpp"

#ifndef PstreamBuffers_H
#define PstreamBuffers_H

#include "DynamicList.hpp"
#include "UPstream.hpp"
#include "IOstream.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                          Class PstreamBuffers Declaration
\*---------------------------------------------------------------------------*/

class PstreamBuffers
{
    friend class UOPstream;
    friend class UIPstream;

    // Private data

        //- Communications type of this stream
        const UPstream::commsTypes commsType_;

        const int tag_;

        const IOstream::streamFormat format_;

        const IOstream::versionNumber version_;

        //- Send buffer
        List<DynamicList<char> > sendBuf_;

        //- Receive buffer
        List<DynamicList<char> > recvBuf_;

        //- Read position in recvBuf_
        labelList recvBufPos_;

        bool finishedSendsCalled_;

public:

    // Static data

        static DynamicList<char> nullBuf;


    // Constructors

        //- Construct given comms type,
        //  write format and IO version
        PstreamBuffers
        (
            const UPstream::commsTypes commsType,
            const int tag = UPstream::msgType(),
            IOstream::streamFormat format=IOstream::BINARY,
            IOstream::versionNumber version=IOstream::currentVersion
        );

    //- Destructor
    ~PstreamBuffers();


    // Member functions

        int tag() const
        {
            return tag_;
        }

        //- Mark all sends as having been done. This will start receives
        //  in non-blocking mode. If block will wait for all transfers to
        //  finish (only relevant for nonBlocking mode)
        void finishedSends(const bool block = true);

        //- Mark all sends as having been done. Same as above but also returns
        //  sizes (bytes) received. Note:currently only valid for
        //  non-blocking.
        void finishedSends(labelListList& sizes, const bool block = true);

        //- Clear storage and reset
        void clear();

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
