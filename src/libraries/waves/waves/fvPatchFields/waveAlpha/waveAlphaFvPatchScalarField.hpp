/*---------------------------------------------------------------------------*\
Copyright Niels Gjøl Jacobsen, Technical University of Denmark.
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

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
    CML::waveAlphaFvPatchScalarField

Description
    Boundary condition for the VOF-fraction in a multiphase solver. To be used
    for wave boundary conditions. For more details see:

    @article { jacobsenFuhrmanFredsoe2011,
        Author = {Jacobsen, N G and Fuhrman, D R and Freds\o{}e, J},
        title = {{A Wave Generation Toolbox for the Open-Source CFD Library: OpenFoam\textregistered{}}},
        Journal = {{Int. J. for Numer. Meth. Fluids}},
        Year = {2012},
        Volume = {70},
        Number = {9},
        Pages = {1073-1088},
        DOI = {{10.1002/fld.2726}},
    }

    The boundary condition is derived from mixedFvPatchField and it is generic
    in the sense, that it loads the abstract class waveTheory.

SourceFiles
    waveAlphaFvPatchScalarField.cpp

Author
    Niels Gjøl Jacobsen, Technical University of Denmark.


\*---------------------------------------------------------------------------*/

#ifndef waveAlphaFvPatchScalarField_HPP
#define waveAlphaFvPatchScalarField_HPP

#include "mixedFvPatchField.hpp"
#include "convexPolyhedral.hpp"
#include "waveTheory.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                 Class waveAlphaFvPatchScalarField Declaration
\*---------------------------------------------------------------------------*/

class waveAlphaFvPatchScalarField
:
    public mixedFvPatchField<scalar>,
    public convexPolyhedral
{

private:

    //- Private member functions
        autoPtr<waveTheories::waveTheory> waveProps_;

    // Private member functions

    //- Returns a scalarField of the signed distance to an arbitrary surface
        virtual void signedPointToSurfaceDistance
        (
            const pointField&,
            scalarField&
        );


    //- Returns a scalar of the signed distance to an arbitrary surface
        virtual scalar signedPointToSurfaceDistance
        (
            const point&
        ) const;

public:

    //- Runtime type information
    TypeName("waveAlpha");


    // Constructors

        //- Construct from patch and internal field
        waveAlphaFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        waveAlphaFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given surfaceWaveVelocityFvPatchVectorField
        //  onto a new patch
        waveAlphaFvPatchScalarField
        (
            const waveAlphaFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchField<scalar> > clone() const
        {
            return tmp<fvPatchField<scalar> >
            (
                new waveAlphaFvPatchScalarField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        waveAlphaFvPatchScalarField
        (
            const waveAlphaFvPatchScalarField&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchField<scalar> > clone
        (
            const DimensionedField<scalar,
            volMesh>& iF
        ) const
        {
            return tmp<fvPatchField<scalar> >
            (
                new waveAlphaFvPatchScalarField(*this, iF)
            );
        }


    // Member functions

        //- Update the coefficients associated with the patch field
        virtual void updateCoeffs();

        //- Evaluate the patch field
        virtual void evaluate();

        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML


#endif

// ************************************************************************* //
