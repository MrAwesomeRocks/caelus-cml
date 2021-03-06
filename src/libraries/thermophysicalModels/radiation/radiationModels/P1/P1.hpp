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
    CML::radiation::P1

Description
    Works well for combustion applications where optical thickness, tau is
    large, i.e. tau = a*L > 3 (L = distance between objects)

    Assumes
     - all surfaces are diffuse
     - tends to over predict radiative fluxes from sources/sinks
       *** SOURCES NOT CURRENTLY INCLUDED ***

SourceFiles
    P1.cpp

\*---------------------------------------------------------------------------*/

#ifndef radiationModelP1_HPP
#define radiationModelP1_HPP

#include "radiationModel.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace radiation
{

/*---------------------------------------------------------------------------*\
                           Class P1 Declaration
\*---------------------------------------------------------------------------*/

class P1
:
    public radiationModel
{

    //- Incident radiation / [W/m2]
    volScalarField G_;

    //- Total radiative heat flux [W/m2]
    volScalarField qr_;

    //- Absorption coefficient
    volScalarField a_;

    //- Emission coefficient
    volScalarField e_;

    //- Emission contribution
    volScalarField E_;


    //- Disallow default bitwise copy construct
    P1(const P1&);

    //- Disallow default bitwise assignment
    void operator=(const P1&);


public:

    //- Runtime type information
    TypeName("P1");


    //- Construct from components
    P1(const volScalarField& T);

    //- Construct from components
    P1(const dictionary& dict, const volScalarField& T);


    //- Destructor
    virtual ~P1()
    {}


    // Member functions

    // Edit

    //- Solve radiation equation(s)
    void calculate();

    //- Read radiation properties dictionary
    bool read();


    // Access

    //- Source term component (for power of T^4)
    virtual tmp<volScalarField> Rp() const;

    //- Source term component (constant)
    virtual tmp<DimensionedField<scalar, volMesh> > Ru() const;

};


} // End namespace radiation
} // End namespace CML


#endif
