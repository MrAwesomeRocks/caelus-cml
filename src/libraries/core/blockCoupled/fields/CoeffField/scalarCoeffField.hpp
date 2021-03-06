/*---------------------------------------------------------------------------*\
Copyright (C) 2004-2013 H. Jasak
Copyright (C) 2018 Applied CCM Pty Ltd
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
    CoeffField<T>

Description
    Template specialisation for scalar coefficients

    This file was obtained from Hrvoje Jasak at the NUMAP-FOAM summer school
    in Zagreb, Croatia, 2010.

    Enhancements from FOAM-Extend-4.0 have been back ported.

    Please report any omissions of authorship and/or copyright to
    info@appliedccm.com.au. Corrections will be made upon provision of proof.

Author
    Hrvoje Jasak, Wikki Ltd.  All rights reserved.

SourceFiles
    scalarCoeffField.cpp

\*---------------------------------------------------------------------------*/

#ifndef scalarCoeffField_HPP
#define scalarCoeffField_HPP

#include "CoeffField.hpp"


namespace CML
{

// Forward declaration of friend functions and operators
Ostream& operator<<
(
    Ostream&,
    const CoeffField<scalar>&
);

Ostream& operator<<
(
    Ostream&,
    const tmp<CoeffField<scalar> >&
);


template<>
class CoeffField<scalar>
:
    public scalarField
{
public:

    //- Component type
    typedef scalar scalarType;
    typedef scalar linearType;
    typedef scalar squareType;

    //- Field type
    typedef scalarField scalarTypeField;
    typedef scalarField linearTypeField;
    typedef scalarField squareTypeField;

    static const char* const typeName;

    //- Construct given size
    explicit CoeffField<scalar>(const label size)
    :
        scalarField(size, pTraits<scalar>::zero)
    {}

    //- Construct as copy
    CoeffField(const CoeffField<scalar>& f)
    :
        scalarField(f)
    {}

    //- Construct from Istream
    CoeffField(Istream& is)
    :
        scalarField(is)
    {}

    // Member functions
    //- Return active type
    blockCoeffBase::activeLevel activeType() const;

    //- Return the field transpose
    tmp<CoeffField<scalar> > transpose() const;

    // Return as typed.  Fails when asked for the incorrect type
    //- Return as scalar field
    const scalarField& asScalar() const;
    scalarField& asScalar();

    //- Return as linear field.  Equal to scalar field
    const scalarField& asLinear() const;
    scalarField& asLinear();

    //- Return as square field.  Equal to scalar field
    const scalarField& asSquare() const;
    scalarField& asSquare();

    //- Return coefficient as a block
    BlockCoeff<scalar> getCoeff(const label index) const;

    //- Set coefficient from a block
    void setCoeff(const label index, const BlockCoeff<scalar>& coeff);

    // Subset operations
    //- Get subset with offset and size and store in given field
    void getSubset
    (
        CoeffField<scalar>& f,
        const label start,
        const label size
    ) const;

    //- Get subset with addressing and store in given field
    void getSubset
    (
        CoeffField<scalar>& f,
        const labelList& addr
    ) const;

    //- Set subset with offset and size from given field
    void setSubset
    (
        const CoeffField<scalar>& f,
        const label start,
        const label size
    );

    //- Get subset with addressing and store in target field
    void setSubset
    (
        const CoeffField<scalar>& f,
        const labelList& addr
    );

    //- Zero out subset with offset and size
    void zeroOutSubset
    (
        const label start,
        const label size
    );

    //- Zero out subset with addressing
    void zeroOutSubset
    (
        const labelList& addr
    );

    //- Add subset with addressing to field
    void addSubset
    (
        const CoeffField<scalar>& f,
        const labelList& addr
    );

    // Member operators
    void operator=(const scalarField&);
    void operator=(const tmp<scalarField>&);

    // IOstream operators
    friend Ostream& operator<<
    (
        Ostream&,
        const CoeffField<scalar>&
    );

    friend Ostream& operator<<
    (
        Ostream&,
        const tmp<CoeffField<scalar> >&
    );
};

/* * * * * * * * * * * * * * * * Global operators  * * * * * * * * * * * * * */

template<>
tmp<CoeffField<scalar> > inv(const CoeffField<scalar>& f);


template<>
void negate
(
    CoeffField<scalar>& f,
    const CoeffField<scalar>& f1
);


template<>
void multiply
(
    scalarField& f,
    const CoeffField<scalar>& f1,
    const scalar& f2
);


template<>
void multiply
(
    scalarField& f,
    const CoeffField<scalar>& f1,
    const scalarField& f2
);


template<>
void multiply
(
    scalarField& f,
    const scalarField& f1,
    const CoeffField<scalar>& f2
);


} // End namespace CML
#endif
