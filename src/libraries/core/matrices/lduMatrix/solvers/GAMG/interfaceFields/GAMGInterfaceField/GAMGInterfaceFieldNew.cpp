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

#include "GAMGInterfaceField.hpp"

// * * * * * * * * * * * * * * * * Selectors * * * * * * * * * * * * * * * * //

CML::autoPtr<CML::GAMGInterfaceField> CML::GAMGInterfaceField::New
(
    const GAMGInterface& GAMGCp,
    const lduInterfaceField& fineInterface
)
{
    const word coupleType(fineInterface.interfaceFieldType());

    lduInterfaceFieldConstructorTable::iterator cstrIter =
        lduInterfaceFieldConstructorTablePtr_->find(coupleType);

    if (cstrIter == lduInterfaceFieldConstructorTablePtr_->end())
    {
        FatalErrorInFunction
            << "Unknown GAMGInterfaceField type "
            << coupleType << nl
            << "Valid GAMGInterfaceField types are :"
            << lduInterfaceFieldConstructorTablePtr_->sortedToc()
            << exit(FatalError);
    }

    return autoPtr<GAMGInterfaceField>(cstrIter()(GAMGCp, fineInterface));
}


CML::autoPtr<CML::GAMGInterfaceField> CML::GAMGInterfaceField::New
(
    const GAMGInterface& GAMGCp,
    const bool doTransform,
    const int rank
)
{
    const word coupleType(GAMGCp.type());

    lduInterfaceConstructorTable::iterator cstrIter =
        lduInterfaceConstructorTablePtr_->find(coupleType);

    if (cstrIter == lduInterfaceConstructorTablePtr_->end())
    {
        FatalErrorInFunction
            << "Unknown GAMGInterfaceField type "
            << coupleType << nl
            << "Valid GAMGInterfaceField types are :"
            << lduInterfaceConstructorTablePtr_->sortedToc()
            << exit(FatalError);
    }

    return autoPtr<GAMGInterfaceField>(cstrIter()(GAMGCp, doTransform, rank));
}


// ************************************************************************* //
