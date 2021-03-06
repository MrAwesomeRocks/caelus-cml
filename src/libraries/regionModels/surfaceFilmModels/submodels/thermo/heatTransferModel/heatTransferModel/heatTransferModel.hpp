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
    CML::regionModels::surfaceFilmModels::heatTransferModel

Description
    Base class for film heat transfer models

SourceFiles
    heatTransferModel.C
    heatTransferModelNew.C

\*---------------------------------------------------------------------------*/

#ifndef heatTransferModel_H
#define heatTransferModel_H

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
                      Class heatTransferModel Declaration
\*---------------------------------------------------------------------------*/

class heatTransferModel
:
    public filmSubModelBase
{
private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        heatTransferModel(const heatTransferModel&);

        //- Disallow default bitwise assignment
        void operator=(const heatTransferModel&);


public:

    //- Runtime type information
    TypeName("heatTransferModel");


    // Declare runtime constructor selection table

         declareRunTimeSelectionTable
         (
             autoPtr,
             heatTransferModel,
             dictionary,
             (
                 surfaceFilmRegionModel& film,
                 const dictionary& dict
             ),
             (film, dict)
         );

    // Constructors

        //- Construct null
        heatTransferModel(surfaceFilmRegionModel& film);

        //- Construct from type name, dictionary and surface film model
        heatTransferModel
        (
            const word& modelType,
            surfaceFilmRegionModel& film,
            const dictionary& dict
        );


    // Selectors

        //- Return a reference to the selected phase change model
        static autoPtr<heatTransferModel> New
        (
            surfaceFilmRegionModel& film,
            const dictionary& dict
        );


    //- Destructor
    virtual ~heatTransferModel();


    // Member Functions

        // Evolution

            //- Correct
            virtual void correct() = 0;

            //- Return the heat transfer coefficient [W/m2/K]
            virtual tmp<volScalarField> h() const = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmModels
} // End namespace regionModels
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
