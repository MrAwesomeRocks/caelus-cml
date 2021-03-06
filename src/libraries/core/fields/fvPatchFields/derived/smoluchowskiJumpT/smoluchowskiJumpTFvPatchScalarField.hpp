/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
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

Class
    CML::smoluchowskiJumpTFvPatchScalarField

Description
    Smoluchowski temperature jump boundary condition

SourceFiles
    smoluchowskiJumpTFvPatchScalarField.cpp

\*---------------------------------------------------------------------------*/

#ifndef smoluchowskiJumpTFvPatchScalarFields_H
#define smoluchowskiJumpTFvPatchScalarFields_H

#include "mixedFvPatchFields.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                 Class smoluchowskiJumpTFvPatch Declaration
\*---------------------------------------------------------------------------*/

class smoluchowskiJumpTFvPatchScalarField
:
    public mixedFvPatchScalarField
{

    // Private data

        // Accommodation coefficient
        scalar accommodationCoeff_;

        // Wall surface temperature
        scalarField Twall_;

        // Heat capacity ratio (default 1.4)
        scalar gamma_;

public:

    //- Runtime type information
    TypeName("smoluchowskiJumpT");


    // Constructors

        //- Construct from patch and internal field
        smoluchowskiJumpTFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        smoluchowskiJumpTFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given smoluchowskiJumpTFvPatchScalarField
        //  onto a new patch
        smoluchowskiJumpTFvPatchScalarField
        (
            const smoluchowskiJumpTFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchScalarField> clone() const
        {
            return tmp<fvPatchScalarField>
            (
                new smoluchowskiJumpTFvPatchScalarField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        smoluchowskiJumpTFvPatchScalarField
        (
            const smoluchowskiJumpTFvPatchScalarField&,
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
                new smoluchowskiJumpTFvPatchScalarField(*this, iF)
            );
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
                const fvPatchField<scalar>&,
                const labelList&
            );


        // Evaluation functions

            //- Update the coefficients associated with the patch field
            virtual void updateCoeffs();


        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
