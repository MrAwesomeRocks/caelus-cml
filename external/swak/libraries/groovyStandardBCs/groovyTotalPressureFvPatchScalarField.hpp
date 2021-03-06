/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright (C) 1991-2010 OpenCFD Ltd.
-------------------------------------------------------------------------------
License
    This file is based on CAELUS.

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
    CML::groovyTotalPressureFvPatchScalarField

Description
    CML::groovyTotalPressureFvPatchScalarField

SourceFiles
    groovyTotalPressureFvPatchScalarField.C

Contributors/Copyright:
    2011, 2013-2014 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef groovyTotalPressureFvPatchScalarField_H
#define groovyTotalPressureFvPatchScalarField_H

#include "totalPressureFvPatchScalarField.hpp"

#include "PatchValueExpressionDriver.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class groovyTotalPressureFvPatch Declaration
\*---------------------------------------------------------------------------*/

class groovyTotalPressureFvPatchScalarField
:
    public totalPressureFvPatchScalarField
{
    // Private data

    exprString p0Expression_;

    PatchValueExpressionDriver driver_;

public:

    //- Runtime type information
    TypeName("groovyTotalPressure");


    // Constructors

        //- Construct from patch and internal field
        groovyTotalPressureFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        groovyTotalPressureFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given groovyTotalPressureFvPatchScalarField
        //  onto a new patch
        groovyTotalPressureFvPatchScalarField
        (
            const groovyTotalPressureFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        groovyTotalPressureFvPatchScalarField
        (
            const groovyTotalPressureFvPatchScalarField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchScalarField> clone() const
        {
            return tmp<fvPatchScalarField>
            (
                new groovyTotalPressureFvPatchScalarField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        groovyTotalPressureFvPatchScalarField
        (
            const groovyTotalPressureFvPatchScalarField&,
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
                new groovyTotalPressureFvPatchScalarField(*this, iF)
            );
        }

        // Mapping functions

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
