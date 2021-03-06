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

InClass
    CML::writer

SourceFiles
    writers.cpp

\*---------------------------------------------------------------------------*/

#ifndef writers_H
#define writers_H

#include "writer.hpp"
#include "fieldTypes.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
// Only used internally
#define makeTypeSetWritersTypeName(typeWriter, dataType)                      \
                                                                              \
    defineNamedTemplateTypeNameAndDebug(typeWriter< dataType >, 0)

// Sometimes used externally
#define makeSetWritersTypeName(typeWriter)                                    \
                                                                              \
    makeTypeSetWritersTypeName(typeWriter, scalar);                           \
    makeTypeSetWritersTypeName(typeWriter, vector);                           \
    makeTypeSetWritersTypeName(typeWriter, sphericalTensor);                  \
    makeTypeSetWritersTypeName(typeWriter, symmTensor);                       \
    makeTypeSetWritersTypeName(typeWriter, tensor)

// Define type info for single dataType template instantiation (eg, vector)
#define makeSetWriterType(typeWriter, dataType)                               \
                                                                              \
    defineNamedTemplateTypeNameAndDebug(typeWriter< dataType >, 0);           \
    addTemplatedToRunTimeSelectionTable                                       \
    (                                                                         \
        writer, typeWriter, dataType, word                                    \
    )


// Define type info for scalar, vector etc. instantiations
#define makeSetWriters(typeWriter)                                            \
                                                                              \
    makeSetWriterType(typeWriter, scalar);                                    \
    makeSetWriterType(typeWriter, vector);                                    \
    makeSetWriterType(typeWriter, sphericalTensor);                           \
    makeSetWriterType(typeWriter, symmTensor);                                \
    makeSetWriterType(typeWriter, tensor)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
