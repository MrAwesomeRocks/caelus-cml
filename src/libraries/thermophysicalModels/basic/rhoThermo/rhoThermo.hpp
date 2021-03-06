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
    CML::basicRhoThermo

Description
    Basic thermodynamic properties based on density

SourceFiles
    rhoThermo.cpp

\*---------------------------------------------------------------------------*/

#ifndef rhoThermo_HPP
#define rhoThermo_HPP

#include "fluidThermo.hpp"
#include "runTimeSelectionTables.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                       Class rhoThermo Declaration
\*---------------------------------------------------------------------------*/

class rhoThermo
:
    public fluidThermo
{

protected:

    //- Density field [kg/m^3]
    //  Named 'rhoThermo' to avoid (potential) conflict with solver density
    volScalarField rho_;

    //- Compressibility [s^2/m^2]
    volScalarField psi_;

    //- Dynamic viscosity [kg/m/s]
    volScalarField mu_;


    //- Construct as copy (not implemented)
    rhoThermo(const rhoThermo&);


public:

    //- Runtime type information
    TypeName("rhoThermo");


    //- Declare run-time constructor selection table
    declareRunTimeSelectionTable
    (
        autoPtr,
        rhoThermo,
        fvMesh,
        (const fvMesh& mesh, const word& phaseName),
        (mesh, phaseName)
    );


    //- Construct from mesh and phase name
    rhoThermo
    (
        const fvMesh&,
        const word& phaseName
    );

    //- Construct from mesh, dictionary and phase name
    rhoThermo
    (
        const fvMesh&,
        const dictionary&,
        const word& phaseName
    );


    //- Selector
    static autoPtr<rhoThermo> New
    (
        const fvMesh&,
        const word& phaseName=word::null
    );


    //- Destructor
    virtual ~rhoThermo()
    {}


    // Member functions

    // Fields derived from thermodynamic state variables

    //- Density [kg/m^3]
    virtual tmp<volScalarField> rho() const;

    //- Density for patch [kg/m^3]
    virtual tmp<scalarField> rho(const label patchi) const;

    //- Return non-const access to the local density field [kg/m^3]
    virtual volScalarField& rho();

    //- Add the given density correction to the density field.
    //  Used to update the density field following pressure solution
    virtual void correctRho(const volScalarField& deltaRho);

    //- Compressibility [s^2/m^2]
    virtual const volScalarField& psi() const;


    // Access to transport state variables

    //- Dynamic viscosity of mixture [kg/m/s]
    virtual const volScalarField& mu() const;

    //- Dynamic viscosity of mixture for patch [kg/m/s]
    virtual const scalarField& mu(const label patchi) const;

};


} // End namespace CML


#endif
