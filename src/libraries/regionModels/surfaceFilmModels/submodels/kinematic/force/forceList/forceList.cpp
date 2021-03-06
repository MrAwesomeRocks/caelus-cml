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

\*---------------------------------------------------------------------------*/

#include "forceList.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace regionModels
{
namespace surfaceFilmModels
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

forceList::forceList(surfaceFilmRegionModel& film)
:
    PtrList<force>()
{}


forceList::forceList
(
    surfaceFilmRegionModel& film,
    const dictionary& dict
)
:
    PtrList<force>()
{
    const wordList models(dict.lookup("forces"));

    Info<< "    Selecting film force models" << endl;
    if (models.size() > 0)
    {
        this->setSize(models.size());

        forAll(models, i)
        {
            set(i, force::New(film, dict, models[i]));
        }
    }
    else
    {
        Info<< "        none" << endl;
    }
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

forceList::~forceList()
{}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

tmp<fvVectorMatrix> forceList::correct(volVectorField& U)
{
    tmp<fvVectorMatrix> tResult
    (
        new fvVectorMatrix(U, dimForce/dimArea*dimVolume)
    );
    fvVectorMatrix& result = tResult();

    forAll(*this, i)
    {
        result += this->operator[](i).correct(U);
    }

    return tResult;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmModels
} // End namespace regionModels
} // End namespace CML

// ************************************************************************* //
