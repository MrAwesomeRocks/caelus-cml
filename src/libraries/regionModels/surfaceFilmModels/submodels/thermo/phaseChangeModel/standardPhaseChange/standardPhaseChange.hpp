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
    CML::regionModels::surfaceFilmModels::standardPhaseChange

Description
    Standard phase change model with modification for boiling

SourceFiles
    standardPhaseChange.C

\*---------------------------------------------------------------------------*/

#ifndef standardPhaseChange_H
#define standardPhaseChange_H

#include "phaseChangeModel.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace regionModels
{
namespace surfaceFilmModels
{

/*---------------------------------------------------------------------------*\
                    Class standardPhaseChange Declaration
\*---------------------------------------------------------------------------*/

class standardPhaseChange
:
    public phaseChangeModel
{
private:

    // Private member functions

        //- Disallow default bitwise copy construct
        standardPhaseChange(const standardPhaseChange&);

        //- Disallow default bitwise assignment
        void operator=(const standardPhaseChange&);


protected:

    // Protected data

        //- Minimum film height for model to be active
        const scalar deltaMin_;

        //- Length scale / [m]
        const scalar L_;

        //- Boiling temperature factor / []
        //  Used to set max limit on temperature to Tb*TbFactor
        const scalar TbFactor_;

        //- Switch to treat YInf as zero
        Switch YInfZero_;


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
    TypeName("standardPhaseChange");


    // Constructors

        //- Construct from surface film model
        standardPhaseChange
        (
            surfaceFilmRegionModel& film,
            const dictionary& dict
        );


    //- Destructor
    virtual ~standardPhaseChange();


    // Member Functions

        // Evolution

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
