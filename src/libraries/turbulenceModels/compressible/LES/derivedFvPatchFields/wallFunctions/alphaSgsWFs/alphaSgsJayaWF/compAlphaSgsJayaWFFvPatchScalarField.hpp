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

Class
    alphaSgsJayatillekeWallFunctionFvPatchScalarField

Description
    Thermal wall function for turbulent thermal diffusivity based on the
    Jayatilleke thermal wall function

SourceFiles
    compAlphaSgsJayaWFFvPatchScalarField.cpp

\*---------------------------------------------------------------------------*/

#ifndef compAlphaSgsJayatillekeWallFunctionFvPatchScalarField_H
#define compAlphaSgsJayatillekeWallFunctionFvPatchScalarField_H

#include "fixedValueFvPatchFields.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace compressible
{
namespace LESModels
{

/*---------------------------------------------------------------------------*\
     Class alphaSgsJayatillekeWallFunctionFvPatchScalarField Declaration
\*---------------------------------------------------------------------------*/

class alphaSgsJayatillekeWallFunctionFvPatchScalarField
:
    public fixedValueFvPatchScalarField
{
    // Private data

        //- Turbulent Prandtl number
        scalar Prt_;

        //- Cmu coefficient
        scalar Cmu_;

        //- Von Karman constant
        scalar kappa_;

        //- E coefficient
        scalar E_;


        // Solution parameters

            static scalar maxExp_;
            static scalar tolerance_;
            static label maxIters_;


    // Private Member Functions

        //- Check the type of the patch
        void checkType();

        //- `P' function
        scalar Psmooth(const scalar Prat) const;

        //- Calculate y+ at the edge of the thermal laminar sublayer
        scalar yPlusTherm
        (
            const scalar P,
            const scalar Prat
        ) const;


public:

    //- Runtime type information
    TypeName("alphaSgsJayatillekeWallFunction");


    // Constructors

        //- Construct from patch and internal field
        alphaSgsJayatillekeWallFunctionFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        alphaSgsJayatillekeWallFunctionFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given an
        //  alphaSgsJayatillekeWallFunctionFvPatchScalarField
        //  onto a new patch
        alphaSgsJayatillekeWallFunctionFvPatchScalarField
        (
            const alphaSgsJayatillekeWallFunctionFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        alphaSgsJayatillekeWallFunctionFvPatchScalarField
        (
            const alphaSgsJayatillekeWallFunctionFvPatchScalarField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchScalarField> clone() const
        {
            return tmp<fvPatchScalarField>
            (
                new alphaSgsJayatillekeWallFunctionFvPatchScalarField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        alphaSgsJayatillekeWallFunctionFvPatchScalarField
        (
            const alphaSgsJayatillekeWallFunctionFvPatchScalarField&,
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
                new alphaSgsJayatillekeWallFunctionFvPatchScalarField
                (
                    *this,
                    iF
                )
            );
        }


    // Member functions

        // Evaluation functions

            //- Evaluate the patchField
            virtual void evaluate
            (	
                const Pstream::commsTypes commsType=Pstream::Pstream::blocking
            );


        // I-O

            //- Write
            void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace LESModels
} // End namespace compressible
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
