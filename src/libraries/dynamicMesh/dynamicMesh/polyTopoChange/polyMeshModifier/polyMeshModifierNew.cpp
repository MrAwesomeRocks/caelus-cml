/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2015 OpenFOAM Foundation
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

#include "polyMeshModifier.hpp"
#include "dictionary.hpp"

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::autoPtr<CML::polyMeshModifier> CML::polyMeshModifier::New
(
    const word& name,
    const dictionary& dict,
    const label index,
    const polyTopoChanger& mme
)
{
    if (debug)
    {
        Info<< "polyMeshModifier::New(const word&, const dictionary&, "
            << "const label, const polyTopoChanger&) : "
            << "constructing polyMeshModifier"
            << endl;
    }

    const word modifierType(dict.lookup("type"));

    dictionaryConstructorTable::iterator cstrIter =
        dictionaryConstructorTablePtr_->find(modifierType);

    if (cstrIter == dictionaryConstructorTablePtr_->end())
    {
        FatalIOErrorInFunction(dict)
            << "Unknown polyMeshModifier type "
            << modifierType << nl << nl
            << "Valid polyMeshModifier types are :" << endl
            << dictionaryConstructorTablePtr_->sortedToc()
            << exit(FatalIOError);
    }

    return autoPtr<polyMeshModifier>(cstrIter()(name, dict, index, mme));
}


// ************************************************************************* //
