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

\*---------------------------------------------------------------------------*/

#include "jumpCyclicFvPatchFields.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "volFields.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    makePatchFieldsTypeName(jumpCyclic);
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<>
void CML::jumpCyclicFvPatchField<CML::scalar>::updateInterfaceMatrix
(
    const scalarField& psiInternal,
    scalarField& result,
    const lduMatrix&,
    const scalarField& coeffs,
    const direction cmpt,
    const Pstream::commsTypes
) const
{
    scalarField pnf(this->size());

    const labelUList& nbrFaceCells =
        this->cyclicPatch().neighbFvPatch().faceCells();

    // Only apply jump to original field
    if (&psiInternal == &this->internalField())
    {
        Field<scalar> jf(this->jump());

        if (!this->cyclicPatch().owner())
        {
            jf *= -1.0;
        }

        forAll(*this, facei)
        {
            pnf[facei] = psiInternal[nbrFaceCells[facei]] - jf[facei];
        }
    }
    else
    {
        forAll(*this, facei)
        {
            pnf[facei] = psiInternal[nbrFaceCells[facei]];
        }
    }

    // Transform according to the transformation tensors
    this->transformCoupleField(pnf, cmpt);

    // Multiply the field by coefficients and add into the result
    const labelUList& faceCells = this->cyclicPatch().faceCells();
    forAll(faceCells, elemI)
    {
        result[faceCells[elemI]] -= coeffs[elemI]*pnf[elemI];
    }
}
