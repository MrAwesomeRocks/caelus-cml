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

InNamespace
    CML

Description
    Find the reference cell nearest (in index) to the given cell but which
    is not on a cyclic, symmetry or processor patch.

SourceFiles
    findRefCell.cpp

\*---------------------------------------------------------------------------*/

#ifndef findRefCell_H
#define findRefCell_H

#include "label.hpp"
#include "volFields.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

//- If the field fieldRef needs referencing find the reference cell nearest
//  (in index) to the given cell looked-up for field, but which is not on a
//  cyclic, symmetry or processor patch.
void setRefCell
(
    const volScalarField& field,
    const volScalarField& fieldRef,
    const dictionary& dict,
    label& refCelli,
    scalar& refValue,
    const bool forceReference = false
);

//- If the field needs referencing find the reference cell nearest
//  (in index) to the given cell looked-up for field, but which is not on a
//  cyclic, symmetry or processor patch.
void setRefCell
(
    const volScalarField& field,
    const dictionary& dict,
    label& refCelli,
    scalar& refValue,
    const bool forceReference = false
);

//- Return the current value of field in the reference cell
scalar getRefCellValue
(
    const volScalarField& field,
    const label refCelli
);

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
