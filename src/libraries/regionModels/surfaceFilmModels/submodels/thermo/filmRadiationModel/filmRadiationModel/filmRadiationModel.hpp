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
    CML::regionModels::surfaceFilmModels::filmRadiationModel

Description
    Base class for film radiation models

SourceFiles
    filmRadiationModel.cpp
    filmRadiationModelNew.cpp

\*---------------------------------------------------------------------------*/

#ifndef filmRadiationModel_H
#define filmRadiationModel_H

#include "filmSubModelBase.hpp"
#include "runTimeSelectionTables.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace regionModels
{
namespace surfaceFilmModels
{

/*---------------------------------------------------------------------------*\
                    Class filmRadiationModel Declaration
\*---------------------------------------------------------------------------*/

class filmRadiationModel
:
    public filmSubModelBase
{
private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        filmRadiationModel(const filmRadiationModel&);

        //- Disallow default bitwise assignment
        void operator=(const filmRadiationModel&);


public:

    //- Runtime type information
    TypeName("radiationModel");


    // Declare runtime constructor selection table

         declareRunTimeSelectionTable
         (
             autoPtr,
             filmRadiationModel,
             dictionary,
             (
                 surfaceFilmRegionModel& film,
                 const dictionary& dict
             ),
             (film, dict)
         );

    // Constructors

        //- Construct null
        filmRadiationModel(surfaceFilmRegionModel& film);

        //- Construct from type name, dictionary and surface film model
        filmRadiationModel
        (
            const word& modelType,
            surfaceFilmRegionModel& film,
            const dictionary& dict
        );


    // Selectors

        //- Return a reference to the selected phase change model
        static autoPtr<filmRadiationModel> New
        (
            surfaceFilmRegionModel& film,
            const dictionary& dict
        );


    //- Destructor
    virtual ~filmRadiationModel();


    // Member Functions

        // Evolution

            //- Correct
            virtual void correct() = 0;

            //- Return the radiation sensible enthalpy source
            virtual tmp<volScalarField> Shs() = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmModels
} // End namespace regionModels
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
