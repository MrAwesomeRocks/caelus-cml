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

#include "mixedPointPatchField.hpp"
#include "pointPatchFieldMapper.hpp"


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class Type>
void CML::mixedPointPatchField<Type>::checkFieldSize() const
{
    if
    (
        this->size() != this->patch().size()
     || refValue_.size() != this->patch().size()
     || valueFraction_.size() != this->patch().size()
    )
    {
        FatalErrorInFunction
            << "field does not correspond to patch. " << endl
            << "Field size: " << this->size() << " value size: "
            << refValue_.size()
            << " valueFraction size: " << valueFraction_.size()
            << " patch size: " << this->patch().size()
            << abort(FatalError);
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
CML::mixedPointPatchField<Type>::mixedPointPatchField
(
    const pointPatch& p,
    const DimensionedField<Type, pointMesh>& iF
)
:
    valuePointPatchField<Type>(p, iF),
    refValue_(p.size()),
    valueFraction_(p.size())
{}


template<class Type>
CML::mixedPointPatchField<Type>::mixedPointPatchField
(
    const pointPatch& p,
    const DimensionedField<Type, pointMesh>& iF,
    const dictionary& dict
)
:
    valuePointPatchField<Type>(p, iF, dict, false),
    refValue_("refValue", dict, p.size()),
    valueFraction_("valueFraction", dict, p.size())
{}


template<class Type>
CML::mixedPointPatchField<Type>::mixedPointPatchField
(
    const mixedPointPatchField<Type>& ptf,
    const pointPatch& p,
    const DimensionedField<Type, pointMesh>& iF,
    const pointPatchFieldMapper& mapper
)
:
    valuePointPatchField<Type>
    (
        ptf,
        p,
        iF,
        mapper
    ),
    refValue_(ptf.refValue_, mapper),
    valueFraction_(ptf.valueFraction_, mapper)

{}


template<class Type>
CML::mixedPointPatchField<Type>::mixedPointPatchField
(
    const mixedPointPatchField<Type>& ptf,
    const DimensionedField<Type, pointMesh>& iF
)
:
    valuePointPatchField<Type>(ptf, iF),
    refValue_(ptf.refValue_),
    valueFraction_(ptf.valueFraction_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// Map and resize from self given a mapper
template<class Type>
void CML::mixedPointPatchField<Type>::autoMap
(
    const pointPatchFieldMapper& m
)
{
    Field<Type>::autoMap(m);
    refValue_.autoMap(m);
    valueFraction_.autoMap(m);
}


// Grab the values using rmap
template<class Type>
void CML::mixedPointPatchField<Type>::rmap
(
    const pointPatchField<Type>& ptf,
    const labelList& addr
)
{
    const mixedPointPatchField<Type>& mptf =
        refCast<const mixedPointPatchField<Type> >(ptf);

    Field<Type>::rmap(mptf, addr);
    refValue_.rmap(mptf.refValue_, addr);
    valueFraction_.rmap(mptf.valueFraction_, addr);
}


// Evaluate patch field
template<class Type>
void CML::mixedPointPatchField<Type>::evaluate(const Pstream::commsTypes)
{
    Field<Type>::operator=
    (
        valueFraction_*refValue_
      + (1.0 - valueFraction_)*this->patchInternalField()
    );

    // Get internal field to insert values into
    Field<Type>& iF = const_cast<Field<Type>&>(this->internalField());

    this->setInInternalField(iF, *this);
}


// Write
template<class Type>
void CML::mixedPointPatchField<Type>::write(Ostream& os) const
{
    pointPatchField<Type>::write(os);
    refValue_.writeEntry("refValue", os);
    valueFraction_.writeEntry("valueFraction", os);
}


// ************************************************************************* //
