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

\*---------------------------------------------------------------------------*/

#include "volFields.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTemplate2TypeNameAndDebug(volScalarField::DimensionedInternalField, 0);
defineTemplate2TypeNameAndDebug(volVectorField::DimensionedInternalField, 0);
defineTemplate2TypeNameAndDebug
(
    volSphericalTensorField::DimensionedInternalField,
    0
);
defineTemplate2TypeNameAndDebug
(
    volSymmTensorField::DimensionedInternalField,
    0
);
defineTemplate2TypeNameAndDebug(volTensorField::DimensionedInternalField, 0);

defineTemplateTypeNameAndDebug(volScalarField, 0);
defineTemplateTypeNameAndDebug(volVectorField, 0);
defineTemplateTypeNameAndDebug(volSphericalTensorField, 0);
defineTemplateTypeNameAndDebug(volSymmTensorField, 0);
defineTemplateTypeNameAndDebug(volTensorField, 0);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// specialization for scalar fields
template<>
tmp<GeometricField<scalar, fvPatchField, volMesh> >
GeometricField<scalar, fvPatchField, volMesh>::component
(
    const direction
) const
{
    return *this;
}


// specialization for scalar fields
template<>
void GeometricField<scalar, fvPatchField, volMesh>::replace
(
    const direction,
    const GeometricField<scalar, fvPatchField, volMesh>& gsf
)
{
    *this == gsf;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
