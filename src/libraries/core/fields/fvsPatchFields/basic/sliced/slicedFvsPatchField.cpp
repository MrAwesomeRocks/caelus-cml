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

#include "slicedFvsPatchField.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
slicedFvsPatchField<Type>::slicedFvsPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, surfaceMesh>& iF,
    const Field<Type>& completeField
)
:
    fvsPatchField<Type>(p, iF, Field<Type>())
{
    // Set the fvsPatchField to a slice of the given complete field
    UList<Type>::operator=(p.patchSlice(completeField));
}


template<class Type>
slicedFvsPatchField<Type>::slicedFvsPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, surfaceMesh>& iF
)
:
    fvsPatchField<Type>(p, iF)
{}


template<class Type>
slicedFvsPatchField<Type>::slicedFvsPatchField
(
    const slicedFvsPatchField<Type>& ptf,
    const fvPatch& p,
    const DimensionedField<Type, surfaceMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fvsPatchField<Type>(ptf, p, iF, mapper)
{
    NotImplemented;
}


template<class Type>
slicedFvsPatchField<Type>::slicedFvsPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, surfaceMesh>& iF,
    const dictionary& dict
)
:
    fvsPatchField<Type>(p, iF, Field<Type>("value", dict, p.size()))
{
    NotImplemented;
}


template<class Type>
slicedFvsPatchField<Type>::slicedFvsPatchField
(
    const slicedFvsPatchField<Type>& ptf,
    const DimensionedField<Type, surfaceMesh>& iF
)
:
    fvsPatchField<Type>(ptf.patch(), iF, Field<Type>())
{
    // Transfer the slice from the argument
    UList<Type>::operator=(ptf);
}

template<class Type>
tmp<fvsPatchField<Type> > slicedFvsPatchField<Type>::clone() const
{
    return tmp<fvsPatchField<Type> >
    (
        new slicedFvsPatchField<Type>(*this)
    );
}


template<class Type>
slicedFvsPatchField<Type>::slicedFvsPatchField
(
    const slicedFvsPatchField<Type>& ptf
)
:
    fvsPatchField<Type>
    (
        ptf.patch(),
        ptf.dimensionedInternalField(),
        Field<Type>()
    )
{
    // Transfer the slice from the argument
    UList<Type>::operator=(ptf);
}


template<class Type>
tmp<fvsPatchField<Type> > slicedFvsPatchField<Type>::clone
(
    const DimensionedField<Type, surfaceMesh>& iF
) const
{
    return tmp<fvsPatchField<Type> >
    (
        new slicedFvsPatchField<Type>(*this, iF)
    );
}


template<class Type>
slicedFvsPatchField<Type>::~slicedFvsPatchField<Type>()
{
    // Set the fvsPatchField storage pointer to nullptr before its destruction
    // to protect the field it a slice of.
    UList<Type>::operator=(UList<Type>(nullptr, 0));
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
