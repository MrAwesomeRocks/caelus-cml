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

#include "valuePointPatchField.hpp"
#include "pointPatchFieldMapper.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class Type>
void CML::valuePointPatchField<Type>::checkFieldSize() const
{
    if (this->size() != this->patch().size())
    {
        FatalErrorInFunction
            << "field does not correspond to patch. " << endl
            << "Field size: " << size() << " patch size: "
            << this->patch().size()
            << abort(FatalError);
    }
}


// * * * * * * * * * * * * * * * * Constructors * * * * * * * * * * * * * * * //

template<class Type>
CML::valuePointPatchField<Type>::valuePointPatchField
(
    const pointPatch& p,
    const DimensionedField<Type, pointMesh>& iF
)
:
    pointPatchField<Type>(p, iF),
    Field<Type>(p.size())
{}


template<class Type>
CML::valuePointPatchField<Type>::valuePointPatchField
(
    const pointPatch& p,
    const DimensionedField<Type, pointMesh>& iF,
    const dictionary& dict,
    const bool valueRequired
)
:
    pointPatchField<Type>(p, iF, dict),
    Field<Type>(p.size())
{
    if (dict.found("value"))
    {
        Field<Type>::operator=
        (
            Field<Type>("value", dict, p.size())
        );
    }
    else if (!valueRequired)
    {
        Field<Type>::operator=(pTraits<Type>::zero);
    }
    else
    {
        FatalIOErrorInFunction(dict)
            << "Essential entry 'value' missing"
            << exit(FatalIOError);
    }
}


template<class Type>
CML::valuePointPatchField<Type>::valuePointPatchField
(
    const valuePointPatchField<Type>& ptf,
    const pointPatch& p,
    const DimensionedField<Type, pointMesh>& iF,
    const pointPatchFieldMapper& mapper
)
:
    pointPatchField<Type>(p, iF),
    Field<Type>(ptf, mapper)
{}


template<class Type>
CML::valuePointPatchField<Type>::valuePointPatchField
(
    const valuePointPatchField<Type>& ptf,
    const DimensionedField<Type, pointMesh>& iF
)
:
    pointPatchField<Type>(ptf, iF),
    Field<Type>(ptf)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void CML::valuePointPatchField<Type>::autoMap
(
    const pointPatchFieldMapper& m
)
{
    Field<Type>::autoMap(m);
}


template<class Type>
void CML::valuePointPatchField<Type>::rmap
(
    const pointPatchField<Type>& ptf,
    const labelList& addr
)
{
    Field<Type>::rmap
    (
        refCast<const valuePointPatchField<Type> >
        (
            ptf
        ),
        addr
    );
}


template<class Type>
void CML::valuePointPatchField<Type>::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    // Get internal field to insert values into
    Field<Type>& iF = const_cast<Field<Type>&>(this->internalField());

    this->setInInternalField(iF, *this);

    pointPatchField<Type>::updateCoeffs();
}


template<class Type>
void CML::valuePointPatchField<Type>::evaluate(const Pstream::commsTypes)
{
    // Get internal field to insert values into
    Field<Type>& iF = const_cast<Field<Type>&>(this->internalField());

    this->setInInternalField(iF, *this);

    pointPatchField<Type>::evaluate();
}


template<class Type>
void CML::valuePointPatchField<Type>::write(Ostream& os) const
{
    pointPatchField<Type>::write(os);
    this->writeEntry("value", os);
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

template<class Type>
void CML::valuePointPatchField<Type>::operator=
(
    const valuePointPatchField<Type>& ptf
)
{
    Field<Type>::operator=(ptf);
}


template<class Type>
void CML::valuePointPatchField<Type>::operator=
(
    const pointPatchField<Type>& ptf
)
{
    Field<Type>::operator=(ptf.patchInternalField());
}


template<class Type>
void CML::valuePointPatchField<Type>::operator=
(
    const Field<Type>& tf
)
{
    Field<Type>::operator=(tf);
}


template<class Type>
void CML::valuePointPatchField<Type>::operator=
(
    const Type& t
)
{
    Field<Type>::operator=(t);
}


// Force an assignment
template<class Type>
void CML::valuePointPatchField<Type>::operator==
(
    const valuePointPatchField<Type>& ptf
)
{
    Field<Type>::operator=(ptf);
}


template<class Type>
void CML::valuePointPatchField<Type>::operator==
(
    const pointPatchField<Type>& ptf
)
{
    Field<Type>::operator=(ptf.patchInternalField());
}


template<class Type>
void CML::valuePointPatchField<Type>::operator==
(
    const Field<Type>& tf
)
{
    Field<Type>::operator=(tf);
}


template<class Type>
void CML::valuePointPatchField<Type>::operator==
(
    const Type& t
)
{
    Field<Type>::operator=(t);
}


// ************************************************************************* //
