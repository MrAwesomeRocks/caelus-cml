/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2015 OpenFOAM Foundation
-------------------------------------------------------------------------------
License

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

#include "reactionRateFlameArea.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

CML::autoPtr<CML::reactionRateFlameArea> CML::reactionRateFlameArea::New
(
    const dictionary& dict,
    const fvMesh& mesh,
    const combustionModel& combModel
)
{
    word reactionRateFlameAreaType
    (
        dict.lookup("reactionRateFlameArea")
    );

    Info<< "Selecting reaction rate flame area correlation "
        << reactionRateFlameAreaType << endl;

    dictionaryConstructorTable::iterator cstrIter =
        dictionaryConstructorTablePtr_->find(reactionRateFlameAreaType);

    if (cstrIter == dictionaryConstructorTablePtr_->end())
    {
        FatalIOErrorInFunction
        (
            dict
        )   << "Unknown reactionRateFlameArea type "
            << reactionRateFlameAreaType << endl << endl
            << "Valid reaction rate flame area types are :" << endl
            << dictionaryConstructorTablePtr_->toc()
            << exit(FatalIOError);
    }

    const label tempOpen = reactionRateFlameAreaType.find('<');

    const word className = reactionRateFlameAreaType(0, tempOpen);

    return autoPtr<reactionRateFlameArea>
        (cstrIter()(className, dict, mesh, combModel));
}
