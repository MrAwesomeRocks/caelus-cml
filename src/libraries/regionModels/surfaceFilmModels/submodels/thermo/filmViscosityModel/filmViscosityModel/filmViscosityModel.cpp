/*---------------------------------------------------------------------------*\
Copyright (C) 2013-2018 OpenFOAM Foundation
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

\*---------------------------------------------------------------------------*/

#include "filmViscosityModel.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace regionModels
{
namespace surfaceFilmModels
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(filmViscosityModel, 0);
defineRunTimeSelectionTable(filmViscosityModel, dictionary);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

filmViscosityModel::filmViscosityModel
(
    const word& modelType,
    surfaceFilmRegionModel& film,
    const dictionary& dict,
    volScalarField& mu
)
:
    filmSubModelBase(film, dict, typeName, modelType),
    mu_(mu)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

filmViscosityModel::~filmViscosityModel()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void filmViscosityModel::info(Ostream& os) const
{}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // end namespace surfaceFilmModels
} // end namespace regionModels
} // end namespace CML

// ************************************************************************* //
