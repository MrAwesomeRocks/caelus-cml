/*---------------------------------------------------------------------------*\
Copyright (C) 2015-2018 OpenFOAM Foundation
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
    CML::regionModels::surfaceFilmModels::patchInjection

Description
    Remove and inject the mass in the film as it passes over the selected
    patches.

SourceFiles
    patchInjection.cpp

\*---------------------------------------------------------------------------*/

#ifndef patchInjection_HPP
#define patchInjection_HPP

#include "injectionModel.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace regionModels
{
namespace surfaceFilmModels
{

/*---------------------------------------------------------------------------*\
                      Class patchInjection Declaration
\*---------------------------------------------------------------------------*/

class patchInjection
:
    public injectionModel
{
    // Private member functions

        //- Disallow default bitwise copy construct
        patchInjection(const patchInjection&);

        //- Disallow default bitwise assignment
        void operator=(const patchInjection&);


protected:

        //- Stable film thickness - mass only removed if thickness exceeds
        //  this threshold value
        scalar deltaStable_;

        //- List of patch IDs at which the film is removed
        labelList patchIDs_;

        //- Injected mass for each patch at which the film is removed
        scalarField patchInjectedMasses_;


public:

    //- Runtime type information
    TypeName("patchInjection");


    // Constructors

        //- Construct from surface film model
        patchInjection(surfaceFilmRegionModel& film, const dictionary& dict);


    //- Destructor
    virtual ~patchInjection();


    // Member Functions

        //- Correct
        virtual void correct
        (
            scalarField& availableMass,
            scalarField& massToInject,
            scalarField& diameterToInject
        );

        //- Accumulate the total mass injected for the patches into the
        //  scalarField provided
        virtual void patchInjectedMassTotals
        (
            scalarField& patchMasses
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmModels
} // End namespace regionModels
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
