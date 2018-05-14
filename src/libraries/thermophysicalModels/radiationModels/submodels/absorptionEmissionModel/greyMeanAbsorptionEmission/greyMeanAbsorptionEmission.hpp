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
    CML::radiation::greyMeanAbsorptionEmission

Description
    greyMeanAbsorptionEmission radiation absorption and emission coefficients
    for continuous phase

    The coefficients for the species in the Look up table have to be specified
    for use in moles x P [atm], i.e. (k[i] = species[i]*p*9.869231e-6).

    The coefficients for CO and soot or any other added are multiplied by the
    respective mass fraction being solved

    All the species in the dictionary need either to be in the look-up table or
    being solved. Conversely, all the species solved do not need to be included
    in the calculation of the absorption coefficient

    The names of the species in the absorption dictionary must match exactly the
    name in the look-up table or the name of the field being solved

    The look-up table ("speciesTable") file should be in constant

    i.e. dictionary
    \verbatim
        LookUpTableFileName     "speciesTable";

        EhrrCoeff       0.0;

        CO2
        {
            Tcommon     300.;   // Common Temp
            invTemp     true;   // Is the polynomial using inverse temperature?
            Tlow        300.;   // Low Temp
            Thigh       2500.;  // High Temp

            loTcoeffs           // coeffs for T < Tcommon
            (
                0               //  a0            +
                0               //  a1*T          +
                0               //  a2*T^(+/-)2   +
                0               //  a3*T^(+/-)3   +
                0               //  a4*T^(+/-)4   +
                0               //  a5*T^(+/-)5   +
            );
            hiTcoeffs           // coeffs for T > Tcommon
            (
                18.741
                -121.31e3
                273.5e6
                -194.05e9
                56.31e12
                -5.8169e15
            );
        }
    \endverbatim

SourceFiles
    greyMeanAbsorptionEmission.cpp

\*---------------------------------------------------------------------------*/

#ifndef greyMeanAbsorptionEmission_H
#define greyMeanAbsorptionEmission_H

#include "interpolationLookUpTable.hpp"
#include "absorptionEmissionModel.hpp"
#include "HashTable.hpp"
#include "absorptionCoeffs.hpp"
#include "basicThermo.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace radiation
{

/*---------------------------------------------------------------------------*\
                 Class greyMeanAbsorptionEmission Declaration
\*---------------------------------------------------------------------------*/

class greyMeanAbsorptionEmission
:
    public absorptionEmissionModel
{
public:

    // Public data

        // Maximum number of species considered for absorptivity
        static const int nSpecies_ = 5;

        // Absorption Coefficients
        absorptionCoeffs coeffs_[nSpecies_];


private:

    // Private data

        //- Absorption model dictionary
        dictionary coeffsDict_;

        //- Hash table of species names
        HashTable<label> speciesNames_;

        //- Indices of species in the look-up table
        FixedList<label, nSpecies_> specieIndex_;

        //- Look-up table of species related to ft
        mutable autoPtr<interpolationLookUpTable<scalar> > lookUpTablePtr_;

        //- SLG thermo package
        const basicThermo& thermo_;

        //- Emission constant coefficient
        const scalar EhrrCoeff_;

        //- Pointer list of species in the registry involved in the absorption
        UPtrList<volScalarField> Yj_;


public:

    //- Runtime type information
    TypeName("greyMeanAbsorptionEmission");


    // Constructors

        //- Construct from components
        greyMeanAbsorptionEmission(const dictionary& dict, const fvMesh& mesh);


    //- Destructor
    virtual ~greyMeanAbsorptionEmission();


    // Member Functions

        // Access

            // Absorption coefficient

                //- Absorption coefficient for continuous phase
                tmp<volScalarField> aCont(const label bandI = 0) const;


            // Emission coefficient

                //- Emission coefficient for continuous phase
                tmp<volScalarField> eCont(const label bandI = 0) const;


            // Emission contribution

                //- Emission contribution for continuous phase
                tmp<volScalarField> ECont(const label bandI = 0) const;


    // Member Functions

        inline bool isGrey() const
        {
            return true;
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace radiation
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
