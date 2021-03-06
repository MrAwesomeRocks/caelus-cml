/*---------------------------------------------------------------------------*\
Copyright (C) 2017-2018 OpenFOAM Foundation
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
    CML::regionModels::surfaceFilmModels::waxSolventEvaporation

Description
    Wax solvent mixture evaporation model.

SourceFiles
    waxSolventEvaporation.cpp

\*---------------------------------------------------------------------------*/

#ifndef waxSolventEvaporation_HPP
#define waxSolventEvaporation_HPP

#include "phaseChangeModel.hpp"
#include "uniformDimensionedFields.hpp"
#include "DataEntry.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace regionModels
{
namespace surfaceFilmModels
{

/*---------------------------------------------------------------------------*\
                    Class waxSolventEvaporation Declaration
\*---------------------------------------------------------------------------*/

class waxSolventEvaporation
:
    public phaseChangeModel
{
    // Private member functions

        //- Disallow default bitwise copy construct
        waxSolventEvaporation(const waxSolventEvaporation&);

        //- Disallow default bitwise assignment
        void operator=(const waxSolventEvaporation&);


protected:

    // Protected data

        //- Molecular weight of wax [kg/kmol]
        uniformDimensionedScalarField Wwax_;

        //- Molecular weight of liquid [kg/kmol]
        uniformDimensionedScalarField Wsolvent_;

        //- Initial solvent mass-fraction
        uniformDimensionedScalarField Ysolvent0_;

        //- Solvent mass-fraction
        volScalarField Ysolvent_;

        //- Minimum film height for model to be active
        const scalar deltaMin_;

        //- Length scale [m]
        const scalar L_;

        //- Boiling temperature factor
        //  Used to set max limit on temperature to Tb*TbFactor
        const scalar TbFactor_;

        //- Switch to treat YInf as zero
        Switch YInfZero_;

        //- Activity coefficient as a function of solvent mole fraction
        autoPtr<DataEntry<scalar> > activityCoeff_;


    // Protected member functions

        //- Return Sherwood number as a function of Reynolds and Schmidt numbers
        scalar Sh(const scalar Re, const scalar Sc) const;

        template<class YInfType>
        void correctModel
        (
            const scalar dt,
            scalarField& availableMass,
            scalarField& dMass,
            scalarField& dEnergy,
            YInfType YInf
        );


public:

    //- Runtime type information
    TypeName("waxSolventEvaporation");


    // Constructors

        //- Construct from surface film model
        waxSolventEvaporation
        (
            surfaceFilmRegionModel& film,
            const dictionary& dict
        );


    //- Destructor
    virtual ~waxSolventEvaporation();


    // Member Functions

        //- Correct
        virtual void correctModel
        (
            const scalar dt,
            scalarField& availableMass,
            scalarField& dMass,
            scalarField& dEnergy
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmModels
} // End namespace regionModels
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
