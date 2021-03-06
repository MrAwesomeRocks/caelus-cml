/*---------------------------------------------------------------------------*\
Copyright (C) 2012-2018 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Caelus is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

Class
    CML::inclinedFilmNusseltInletVelocityFvPatchVectorField

Description
    Film velocity boundary condition for inclined films that imposes a
    sinusoidal perturbation on top of a mean flow rate, where the velocity is
    calculated using the Nusselt solution.

SourceFiles
    inclinedFilmNusseltInletVelocityFvPatchVectorField.cpp

\*---------------------------------------------------------------------------*/

#ifndef inclinedFilmNusseltInletVelocityFvPatchVectorField_HPP
#define inclinedFilmNusseltInletVelocityFvPatchVectorField_HPP

#include "fvPatchFields.hpp"
#include "fixedValueFvPatchFields.hpp"
#include "DataEntry.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
     Class inclinedFilmNusseltInletVelocityFvPatchVectorField Declaration
\*---------------------------------------------------------------------------*/

class inclinedFilmNusseltInletVelocityFvPatchVectorField
:
    public fixedValueFvPatchVectorField
{
    // Private data

        //- Mean mass flow rate per unit length [kg/s/m]
        autoPtr<DataEntry<scalar> > GammaMean_;

        //- Perturbation amplitude [m]
        autoPtr<DataEntry<scalar> > a_;

        //- Perturbation frequency [rad/s/m]
        autoPtr<DataEntry<scalar> > omega_;


public:

    //- Runtime type information
    TypeName("inclinedFilmNusseltInletVelocity");


    // Constructors

        //- Construct from patch and internal field
        inclinedFilmNusseltInletVelocityFvPatchVectorField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        inclinedFilmNusseltInletVelocityFvPatchVectorField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given
        // inclinedFilmNusseltInletVelocityFvPatchVectorField onto a new patch
        inclinedFilmNusseltInletVelocityFvPatchVectorField
        (
            const inclinedFilmNusseltInletVelocityFvPatchVectorField&,
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        inclinedFilmNusseltInletVelocityFvPatchVectorField
        (
            const inclinedFilmNusseltInletVelocityFvPatchVectorField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchVectorField> clone() const
        {
            return tmp<fvPatchVectorField>
            (
                new inclinedFilmNusseltInletVelocityFvPatchVectorField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        inclinedFilmNusseltInletVelocityFvPatchVectorField
        (
            const inclinedFilmNusseltInletVelocityFvPatchVectorField&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchVectorField> clone
        (
            const DimensionedField<vector, volMesh>& iF
        ) const
        {
            return tmp<fvPatchVectorField>
            (
                new inclinedFilmNusseltInletVelocityFvPatchVectorField
                (
                    *this, iF
                )
            );
        }


    // Member functions

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
