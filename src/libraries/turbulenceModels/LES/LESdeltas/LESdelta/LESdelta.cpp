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

#include "LESdelta.hpp"
#include "calculatedFvPatchFields.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(LESdelta, 0);
    defineRunTimeSelectionTable(LESdelta, dictionary);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::LESdelta::LESdelta(const word& name, const fvMesh& mesh)
:
    mesh_(mesh),
    delta_
    (
        IOobject
        (
            name,
            mesh.time().timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh,
        dimensionedScalar(name, dimLength, SMALL),
        calculatedFvPatchScalarField::typeName
    )
{}


// * * * * * * * * * * * * * * * * Selectors * * * * * * * * * * * * * * * * //

CML::autoPtr<CML::LESdelta> CML::LESdelta::New
(
    const word& name,
    const fvMesh& mesh,
    const dictionary& dict
)
{
    const word deltaType(dict.lookup("delta"));

    Info<< "Selecting LES delta type " << deltaType << endl;

    dictionaryConstructorTable::iterator cstrIter =
        dictionaryConstructorTablePtr_->find(deltaType);

    if (cstrIter == dictionaryConstructorTablePtr_->end())
    {
        FatalErrorInFunction
            << "Unknown LESdelta type "
            << deltaType << nl << nl
            << "Valid LESdelta types are :" << endl
            << dictionaryConstructorTablePtr_->sortedToc()
            << exit(FatalError);
    }

    return autoPtr<LESdelta>(cstrIter()(name, mesh, dict));
}


CML::autoPtr<CML::LESdelta> CML::LESdelta::New
(
    const word& name,
    const fvMesh& mesh,
    const dictionary& dict,
    const dictionaryConstructorTable& additionalConstructors
)
{
    const word deltaType(dict.lookup("delta"));

    Info<< "Selecting LES delta type " << deltaType << endl;

    // First on additional ones
    dictionaryConstructorTable::const_iterator cstrIter =
        additionalConstructors.find(deltaType);

    if (cstrIter != additionalConstructors.end())
    {
        return autoPtr<LESdelta>(cstrIter()(name, mesh, dict));
    }
    else
    {
        dictionaryConstructorTable::const_iterator cstrIter =
            dictionaryConstructorTablePtr_->find(deltaType);

        if (cstrIter == dictionaryConstructorTablePtr_->end())
        {
            FatalErrorInFunction
                << "Unknown LESdelta type "
                << deltaType << nl << nl
                << "Valid LESdelta types are :" << endl
                << additionalConstructors.sortedToc()
                << " and "
                << dictionaryConstructorTablePtr_->sortedToc()
                << exit(FatalError);
            return autoPtr<LESdelta>();
        }
        else
        {
            return autoPtr<LESdelta>(cstrIter()(name, mesh, dict));
        }
    }
}


// ************************************************************************* //
