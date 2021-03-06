/*---------------------------------------------------------------------------*\
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

Description
    Read from UIPstream

\*---------------------------------------------------------------------------*/

#include "UIPstream.hpp"

// * * * * * * * * * * * * * * * * Constructor * * * * * * * * * * * * * * * //

CML::UIPstream::UIPstream
(
    const commsTypes commsType,
    const int fromProcNo,
    DynamicList<char>& externalBuf,
    label& externalBufPosition,
    const int tag,
    const bool clearAtEnd,
    streamFormat format,
    versionNumber version
)
:
    UPstream(commsType),
    Istream(format, version),
    fromProcNo_(fromProcNo),
    externalBuf_(externalBuf),
    externalBufPosition_(externalBufPosition),
    tag_(tag),
    clearAtEnd_(clearAtEnd),
    messageSize_(0)
{
    NotImplemented;
}


CML::UIPstream::UIPstream
(
    const int fromProcNo,
    PstreamBuffers& buffers
)
:
    UPstream(buffers.commsType_),
    Istream(buffers.format_, buffers.version_),
    fromProcNo_(fromProcNo),
    externalBuf_(buffers.recvBuf_[fromProcNo]),
    externalBufPosition_(buffers.recvBufPos_[fromProcNo]),
    tag_(buffers.tag_),
    clearAtEnd_(true),
    messageSize_(0)
{
    NotImplemented;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::label CML::UIPstream::read
(
    const commsTypes commsType,
    const int fromProcNo,
    char* buf,
    const std::streamsize bufSize,
    const int tag
)
{
    NotImplemented;

     return 0;
}


// ************************************************************************* //
