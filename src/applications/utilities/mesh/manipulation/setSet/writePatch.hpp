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
    CML::writePatch

Description
    Write faceSet to vtk polydata file. Only one data which is original
    faceID.

SourceFiles
    writePatch.C

\*---------------------------------------------------------------------------*/

#ifndef writePatch_H
#define writePatch_H

#include "primitiveMesh.hpp"
#include "primitiveFacePatch.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Write faceSet
void writePatch
(
    const bool binary,
    const word& setName,
    const primitiveFacePatch& fp,
    const word& fieldName,
    labelList& fieldValues,
    const fileName& fileName
);

} // End namespace CML


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
