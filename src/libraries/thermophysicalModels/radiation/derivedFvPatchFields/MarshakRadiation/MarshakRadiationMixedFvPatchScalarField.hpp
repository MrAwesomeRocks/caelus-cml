/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2018 OpenFOAM Foundation
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

Class
    CML::MarshakRadiationFvPatchScalarField

Description
    A 'mixed' boundary condition that implements a Marshak condition for the
    incident radiation field (usually written as G)

    The radiation temperature is retrieved from the mesh database, using a
    user specified temperature field name.

Usage
    \table
        Property     | Description             | Required    | Default value
        T            | temperature field name  | no          | T
    \endtable

    Example of the boundary condition specification:
    \verbatim
    <patchName>
    {
        type            MarshakRadiation;
        T               T;
        value           uniform 0;
    }
    \endverbatim

See also
    CML::radiationCoupledBase
    CML::mixedFvPatchField

SourceFiles
    MarshakRadiationFvPatchScalarField.cpp

\*---------------------------------------------------------------------------*/

#ifndef MarshakRadiationMixedFvPatchField_HPP
#define MarshakRadiationMixedFvPatchField_HPP

#include "mixedFvPatchFields.hpp"
#include "radiationCoupledBase.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
             Class MarshakRadiationFvPatchScalarField Declaration
\*---------------------------------------------------------------------------*/

class MarshakRadiationFvPatchScalarField
:
    public mixedFvPatchScalarField,
    public radiationCoupledBase
{

    //- Name of temperature field
    word TName_;


public:

    //- Runtime type information
    TypeName("MarshakRadiation");


    //- Construct from patch and internal field
    MarshakRadiationFvPatchScalarField
    (
        const fvPatch&,
        const DimensionedField<scalar, volMesh>&
    );

    //- Construct from patch, internal field and dictionary
    MarshakRadiationFvPatchScalarField
    (
        const fvPatch&,
        const DimensionedField<scalar, volMesh>&,
        const dictionary&
    );

    //- Construct by mapping given MarshakRadiationFvPatchField onto a new
    //  patch
    MarshakRadiationFvPatchScalarField
    (
        const MarshakRadiationFvPatchScalarField&,
        const fvPatch&,
        const DimensionedField<scalar, volMesh>&,
        const fvPatchFieldMapper&
    );

    //- Construct as copy
    MarshakRadiationFvPatchScalarField
    (
        const MarshakRadiationFvPatchScalarField&
    );

    //- Construct and return a clone
    virtual tmp<fvPatchScalarField> clone() const
    {
        return tmp<fvPatchScalarField>
        (
            new MarshakRadiationFvPatchScalarField(*this)
        );
    }

    //- Construct as copy setting internal field reference
    MarshakRadiationFvPatchScalarField
    (
        const MarshakRadiationFvPatchScalarField&,
        const DimensionedField<scalar, volMesh>&
    );

    //- Construct and return a clone setting internal field reference
    virtual tmp<fvPatchScalarField> clone
    (
        const DimensionedField<scalar, volMesh>& iF
    ) const
    {
        return tmp<fvPatchScalarField>
        (
            new MarshakRadiationFvPatchScalarField(*this, iF)
        );
    }


    // Member functions

    // Access

    //- Return the temperature field name
    const word& TName() const
    {
        return TName_;
    }

    //- Return reference to the temperature field name to allow
    //  adjustment
    word& TName()
    {
        return TName_;
    }


    // Mapping functions

    //- Map (and resize as needed) from self given a mapping object
    virtual void autoMap
    (
        const fvPatchFieldMapper&
    );

    //- Reverse map the given fvPatchField onto this fvPatchField
    virtual void rmap
    (
        const fvPatchScalarField&,
        const labelList&
    );


    // Evaluation functions

    //- Update the coefficients associated with the patch field
    virtual void updateCoeffs();


    // I-O

    //- Write
    virtual void write(Ostream&) const;

};


} // End namespace CML


#endif
