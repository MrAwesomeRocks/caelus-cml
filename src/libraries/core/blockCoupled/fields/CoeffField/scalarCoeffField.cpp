/*---------------------------------------------------------------------------*\
Copyright (C) 2004-2013 H. Jasak
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    Caelus is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

Class
    scalarCoeffField

Description
    This file was obtained from Hrvoje Jasak at the NUMAP-FOAM summer school
    in Zagreb, Croatia, 2010.

    Please report any omissions of authorship and/or copyright to
    info@appliedccm.com.au. Corrections will be made upon provision of proof.

\*---------------------------------------------------------------------------*/

#include "scalarCoeffField.hpp"


// * * * * * * * * * * * * * * * Static Members  * * * * * * * * * * * * * * //

const char* const CML::CoeffField<CML::scalar>::typeName("CoeffField");

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::blockCoeffBase::activeLevel
CML::CoeffField<CML::scalar>::activeType() const
{
    return blockCoeffBase::SCALAR;
}


CML::tmp<CML::CoeffField<CML::scalar> >
CML::CoeffField<CML::scalar>::transpose() const
{
    return tmp<CoeffField<scalar> >(new CoeffField<scalar>(*this));
}


const CML::scalarField&
CML::CoeffField<CML::scalar>::asScalar() const
{
    return *this;
}


CML::scalarField&
CML::CoeffField<CML::scalar>::asScalar()
{
    return *this;
}


const CML::scalarField&
CML::CoeffField<CML::scalar>::asLinear() const
{
    return *this;
}


CML::scalarField&
CML::CoeffField<CML::scalar>::asLinear()
{
    return *this;
}


const CML::scalarField&
CML::CoeffField<CML::scalar>::asSquare() const
{
    return *this;
}


CML::scalarField&
CML::CoeffField<CML::scalar>::asSquare()
{
    return *this;
}


CML::BlockCoeff<CML::scalar>
CML::CoeffField<CML::scalar>::getCoeff(const label index) const
{
    BlockCoeff<scalar> result;

    result.asScalar() = (operator[](index));

    return result;
}


void CML::CoeffField<CML::scalar>::setCoeff
(
    const label index,
    const BlockCoeff<scalar>& coeff
)
{
    operator[](index) = coeff.asScalar();
}


void CML::CoeffField<CML::scalar>::getSubset
(
    CoeffField<scalar>& f,
    const label start,
    const label size
) const
{
    // Check sizes
    if (f.size() != size)
    {
        FatalErrorIn
        (
            "void CML::CoeffField<CML::scalar>::getSubset\n"
            "(\n"
            "    CoeffField<scalar>& f,\n"
            "    const label start,\n"
            "    const label size\n"
            ") const"
        )   << "Incompatible sizes: " << f.size() << " and " << size
            << abort(FatalError);
    }

    const scalarField& localF = *this;

    forAll (f, fI)
    {
        f[fI] = localF[start + fI];
    }
}


void CML::CoeffField<CML::scalar>::getSubset
(
    CoeffField<scalar>& f,
    const labelList& addr
) const
{
    // Check sizes
    if (f.size() != addr.size())
    {
        FatalErrorIn
        (
            "void CML::CoeffField<CML::scalar>::getSubset\n"
            "(\n"
            "    CoeffField<scalar>& f,\n"
            "    const labelList addr\n"
            ") const"
        )   << "Incompatible sizes: " << f.size() << " and " << addr.size()
            << abort(FatalError);
    }

    const scalarField& localF = *this;

    forAll (f, fI)
    {
        f[fI] = localF[addr[fI]];
    }
}


void CML::CoeffField<CML::scalar>::setSubset
(
    const CoeffField<scalar>& f,
    const label start,
    const label size
)
{
    // Check sizes
    if (f.size() != size)
    {
        FatalErrorIn
        (
            "void CML::CoeffField<CML::scalar>::setSubset\n"
            "(\n"
            "     const CoeffField<scalar>& f,\n"
            "    const label start,\n"
            "    const label size\n"
            ")"
        )   << "Incompatible sizes: " << f.size() << " and " << size
            << abort(FatalError);
    }

    scalarField& localF = *this;

    forAll (f, fI)
    {
        localF[start + fI] = f[fI];
    }
}


void CML::CoeffField<CML::scalar>::setSubset
(
    const CoeffField<scalar>& f,
    const labelList& addr
)
{
    // Check sizes
    if (f.size() != addr.size())
    {
        FatalErrorIn
        (
            "void CML::CoeffField<CML::scalar>::setSubset\n"
            "(\n"
            "    const CoeffField<scalar>& f,\n"
            "    const labelList addr\n"
            ")"
        )   << "Incompatible sizes: " << f.size() << " and " << addr.size()
            << abort(FatalError);
    }

    scalarField& localF = this->asScalar();

    forAll (f, fI)
    {
        localF[addr[fI]] = f[fI];
    }
}


void CML::CoeffField<CML::scalar>::zeroOutSubset
(
    const label start,
    const label size
)
{
    scalarField& localF = *this;

    for (label ffI = 0; ffI < size; ffI++)
    {
        localF[start + ffI] = pTraits<scalar>::zero;
    }
}


void CML::CoeffField<CML::scalar>::zeroOutSubset
(
    const labelList& addr
)
{
    scalarField& localF = *this;

    forAll (addr, ffI)
    {
        localF[addr[ffI]] = pTraits<scalar>::zero;
    }
}


void CML::CoeffField<CML::scalar>::addSubset
(
    const CoeffField<scalar>& f,
    const labelList& addr
)
{
    // Check sizes
    if (f.size() != addr.size())
    {
        FatalErrorIn
        (
            "void CML::CoeffField<CML::scalar>::addSubset\n"
            "(\n"
            "    const CoeffField<scalar>& f,\n"
            "    const labelList addr\n"
            ")"
        )   << "Incompatible sizes: " << f.size() << " and " << addr.size()
            << abort(FatalError);
    }

    scalarField& localF = this->asScalar();

    forAll (f, fI)
    {
        localF[addr[fI]] += f[fI];
    }
}

// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

void CML::CoeffField<CML::scalar>::operator=(const scalarField& f)
{
    scalarField::operator=(f);
}


void CML::CoeffField<CML::scalar>::operator=(const tmp<scalarField>& tf)
{
    scalarField::operator=(tf);
}

// * * * * * * * * * * * * * * * Ostream Operator  * * * * * * * * * * * * * //

CML::Ostream& CML::operator<<(Ostream& os, const CoeffField<scalar>& f)
{
    os << static_cast<const scalarField&>(f);

    return os;
}


CML::Ostream& CML::operator<<
(
    Ostream& os,
    const tmp<CoeffField<scalar> >& tf
)
{
    os << tf();
    tf.clear();
    return os;
}

/* * * * * * * * * * * * * * * * Global functions  * * * * * * * * * * * * * */

template<>
CML::tmp<CML::CoeffField<CML::scalar> >
CML::inv(const CoeffField<scalar>& f)
{
    tmp<CoeffField<scalar> > tresult(new CoeffField<scalar>(f.size()));
    scalarField& result = tresult();
    result = 1.0/f;

    return tresult;
}


template<>
void CML::negate
(
    CoeffField<scalar>& f,
    const CoeffField<scalar>& f1
)
{
    f = f1;
    f.negate();
}


template<>
void CML::multiply
(
    scalarField& f,
    const CoeffField<scalar>& f1,
    const scalar& f2
)
{
    const scalarField& sf = f1;
    f = sf*f2;
}


template<>
void CML::multiply
(
    scalarField& f,
    const CoeffField<scalar>& f1,
    const scalarField& f2
)
{
    const scalarField& sf = f1;
    f = sf*f2;
}


template<>
void CML::multiply
(
    scalarField& f,
    const scalarField& f1,
    const CoeffField<scalar>& f2
)
{
    const scalarField& sf = f2;
    f = f1*sf;
}
