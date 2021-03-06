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
    CML::injectionModel

Description
    Base class for film injection models, handling mass transfer from the
    film.

SourceFiles
    injectionModel.C
    injectionModelNew.C

\*---------------------------------------------------------------------------*/

#ifndef injectionModel_H
#define injectionModel_H

#include "filmSubModelBase.hpp"
#include "runTimeSelectionTables.hpp"
#include "scalarField.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace regionModels
{
namespace surfaceFilmModels
{

/*---------------------------------------------------------------------------*\
                      Class injectionModel Declaration
\*---------------------------------------------------------------------------*/

class injectionModel
:
    public filmSubModelBase
{
    // Private data

        //- Injected mass
        scalar injectedMass_;


    // Private Member Functions

        //- Disallow default bitwise copy construct
        injectionModel(const injectionModel&);

        //- Disallow default bitwise assignment
        void operator=(const injectionModel&);


protected:

    // Protected Member Functions

        //- Add to injected mass
        void addToInjectedMass(const scalar dMass);

        //- Correct
        void correct();


public:

    //- Runtime type information
    TypeName("injectionModel");


    // Declare runtime constructor selection table

         declareRunTimeSelectionTable
         (
             autoPtr,
             injectionModel,
             dictionary,
             (
                surfaceFilmRegionModel& film,
                const dictionary& dict
             ),
             (film, dict)
         );


    // Constructors

        //- Construct null
        injectionModel(surfaceFilmRegionModel& film);

        //- Construct from type name, dictionary and surface film model
        injectionModel
        (
            const word& modelType,
            surfaceFilmRegionModel& film,
            const dictionary& dict
        );


    // Selectors

        //- Return a reference to the selected injection model
        static autoPtr<injectionModel> New
        (
            surfaceFilmRegionModel& film,
            const dictionary& dict,
            const word& mdoelType
        );


    //- Destructor
    virtual ~injectionModel();


    // Member Functions

        //- Correct
        virtual void correct
        (
            scalarField& availableMass,
            scalarField& massToInject,
            scalarField& diameterToInject
        ) = 0;

        //- Return the total mass injected
        virtual scalar injectedMassTotal() const;

        //- Accumulate the total mass injected for the patches into the
        //  scalarField provided
        virtual void patchInjectedMassTotals(scalarField& patchMasses) const
        {}
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmModels
} // End namespace regionModels
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
