/*---------------------------------------------------------------------------*\
Copyright Niels G. Jacobsen, Deltares.
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

\*---------------------------------------------------------------------------*/

#include "jensenJacobsenChristensen2014.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(jensenJacobsenChristensen2014, 0);
addToRunTimeSelectionTable
(
    wavesPorosityModel,
    jensenJacobsenChristensen2014,
    wavesPorosityModel
);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //


jensenJacobsenChristensen2014::jensenJacobsenChristensen2014
(
    const fvMesh& mesh
)
:
    wavesPorosityModel(mesh),

    pZones_(mesh)
{
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

tmp<fvMatrix<vector> > jensenJacobsenChristensen2014::ddt
(
    GeometricField<vector, fvPatchField, volMesh>& U
)
{
    return pZones_.ddt(U);
}


tmp<fvMatrix<vector> > jensenJacobsenChristensen2014::ddt
(
    const geometricOneField& rho,
    GeometricField<vector, fvPatchField, volMesh>& U
)
{
    return pZones_.ddt(rho, U);
}


tmp<fvMatrix<vector> > jensenJacobsenChristensen2014::ddt
(
    const dimensionedScalar& rho,
    GeometricField<vector, fvPatchField, volMesh>& U
)
{
	return pZones_.ddt(rho, U);
}


tmp<fvMatrix<vector> > jensenJacobsenChristensen2014::ddt
(
    const volScalarField& rho,
    GeometricField<vector, fvPatchField, volMesh>& U
)
{
    return pZones_.ddt(rho, U);
}

void jensenJacobsenChristensen2014::updatePorosity()
{
	// Store the porosity from the last time step. Needed for moving porosity
	// fields
	porosity_.storeOldTime();

	// Obtain the new porosity field from the pZones as a tmp<volScalarField>
	tmp<volScalarField> tporosity = pZones_.porosity();
	const volScalarField& poro = tporosity();

    // Set the internal field values
	porosity_.internalField() = poro.internalField();

	// Update boundary conditions
	porosity_.correctBoundaryConditions();
}


const volScalarField& jensenJacobsenChristensen2014::porosity() const
{
    return porosity_;
}


void jensenJacobsenChristensen2014::addResistance(fvVectorMatrix& UEqn) const
{
    pZones_.addResistance(UEqn);
}


void jensenJacobsenChristensen2014::addResistance
(
    const fvVectorMatrix& UEqn,
    volTensorField& AU
) const
{
    pZones_.addResistance(UEqn, AU);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
