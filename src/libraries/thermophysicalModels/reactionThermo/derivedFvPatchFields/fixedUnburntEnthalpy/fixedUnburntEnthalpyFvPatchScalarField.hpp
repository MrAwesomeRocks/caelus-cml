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
    CML::fixedUnburntEnthalpyFvPatchScalarField

Description
    Fixed boundary condition for unburnt

SourceFiles
    fixedUnburntEnthalpyFvPatchScalarField.cpp

\*---------------------------------------------------------------------------*/

#ifndef fixedUnburntEnthalpyFvPatchScalarField_HPP
#define fixedUnburntEnthalpyFvPatchScalarField_HPP

#include "fixedValueFvPatchFields.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
          Class fixedUnburntEnthalpyFvPatchScalarField Declaration
\*---------------------------------------------------------------------------*/

class fixedUnburntEnthalpyFvPatchScalarField
:
    public fixedValueFvPatchScalarField
{

public:

    //- Runtime type information
    TypeName("fixedUnburntEnthalpy");


    //- Construct from patch and internal field
    fixedUnburntEnthalpyFvPatchScalarField
    (
        const fvPatch&,
        const DimensionedField<scalar, volMesh>&
    );

    //- Construct from patch, internal field and dictionary
    fixedUnburntEnthalpyFvPatchScalarField
    (
        const fvPatch&,
        const DimensionedField<scalar, volMesh>&,
        const dictionary&
    );

    //- Construct by mapping given fixedUnburntEnthalpyFvPatchScalarField
    // onto a new patch
    fixedUnburntEnthalpyFvPatchScalarField
    (
        const fixedUnburntEnthalpyFvPatchScalarField&,
        const fvPatch&,
        const DimensionedField<scalar, volMesh>&,
        const fvPatchFieldMapper&
    );

    //- Construct as copy
    fixedUnburntEnthalpyFvPatchScalarField
    (
        const fixedUnburntEnthalpyFvPatchScalarField&
    );

    //- Construct and return a clone
    virtual tmp<fvPatchScalarField> clone() const
    {
        return tmp<fvPatchScalarField>
        (
            new fixedUnburntEnthalpyFvPatchScalarField(*this)
        );
    }

    //- Construct as copy setting internal field reference
    fixedUnburntEnthalpyFvPatchScalarField
    (
        const fixedUnburntEnthalpyFvPatchScalarField&,
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
            new fixedUnburntEnthalpyFvPatchScalarField(*this, iF)
        );
    }


    // Member functions

    // Evaluation functions

    //- Update the coefficients associated with the patch field
    virtual void updateCoeffs();

};


} // End namespace CML


#endif
