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
    CML::turbulentHeatFluxTemperatureFvPatchScalarField

Description
    Fixed heat boundary condition to specify temperature gradient. Input
    heat source either specified in terms of an absolute power [W], or as a
    flux [W/m^2].

    \heading Patch usage

    \table
        Property     | Description             | Required    | Default value
        heatSource   | heat source type: \c flux [W/m^2] or \c power [W]| yes |
        q            | heat source value       | yes          |
        alphaEff     | turbulent thermal diffusivity field name | yes |
    \endtable

    Example of the boundary condition specification:
    \verbatim
    myPatch
    {
        type            turbulentHeatFluxTemperature;
        heatSource      flux;
        q               uniform 10;
        alphaEff        alphaEff;
        value           uniform 300; // place holder
    }
    \endverbatim

Note
    - The units of \c alphaEff are [kg/m/s]
    - The density*specific heat capacity \c rhoCp0 (units [kg/m^3]*[J/kg/K])
      is read from the transport dictionary entry

SourceFiles
    turbulentHeatFluxTemperatureFvPatchScalarField.cpp

\*---------------------------------------------------------------------------*/

#ifndef turbulentHeatFluxTemperatureFvPatchScalarFields_H
#define turbulentHeatFluxTemperatureFvPatchScalarFields_H

#include "fvPatchFields.hpp"
#include "fixedGradientFvPatchFields.hpp"
#include "NamedEnum.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace incompressible
{

/*---------------------------------------------------------------------------*\
       Class turbulentHeatFluxTemperatureFvPatchScalarField Declaration
\*---------------------------------------------------------------------------*/

class turbulentHeatFluxTemperatureFvPatchScalarField
:
    public fixedGradientFvPatchScalarField
{
public:

    // Data types

        //- Enumeration listing the possible hest source input modes
        enum heatSourceType
        {
            hsPower,
            hsFlux
        };


private:

    // Private data

        //- Heat source type names
        static const NamedEnum<heatSourceType, 2> heatSourceTypeNames_;

        //- Heat source type
        heatSourceType heatSource_;

        //- Heat power [W] or flux [W/m2]
        //  NOTE: to be divided by density, rho, if used in kinematic form
        scalarField q_;

        //- Name of effective thermal diffusivity field
        word alphaEffName_;


public:

    //- Runtime type information
    TypeName("turbulentHeatFluxTemperature");


    // Constructors

        //- Construct from patch and internal field
        turbulentHeatFluxTemperatureFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        turbulentHeatFluxTemperatureFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given
        //  turbulentHeatFluxTemperatureFvPatchScalarField onto
        //  a new patch
        turbulentHeatFluxTemperatureFvPatchScalarField
        (
            const turbulentHeatFluxTemperatureFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        turbulentHeatFluxTemperatureFvPatchScalarField
        (
            const turbulentHeatFluxTemperatureFvPatchScalarField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchScalarField> clone() const
        {
            return tmp<fvPatchScalarField>
            (
                new turbulentHeatFluxTemperatureFvPatchScalarField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        turbulentHeatFluxTemperatureFvPatchScalarField
        (
            const turbulentHeatFluxTemperatureFvPatchScalarField&,
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
                new turbulentHeatFluxTemperatureFvPatchScalarField
                (
                    *this,
                    iF
                )
            );
        }


    // Member functions

        // Mapping functions

            //- Map (and resize as needed) from self given a mapping object
            virtual void autoMap(const fvPatchFieldMapper&);

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


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace incompressible
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
