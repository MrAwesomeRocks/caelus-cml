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

#include "radiationModel.hpp"
#include "volFields.hpp"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::autoPtr<CML::radiation::radiationModel>
CML::radiation::radiationModel::New
(
    const volScalarField& T
)
{
    IOobject radIO
    (
        "radiationProperties",
        T.time().constant(),
        T.mesh(),
        IOobject::MUST_READ_IF_MODIFIED,
        IOobject::NO_WRITE,
        false
    );

    word modelType("none");
    if (radIO.headerOk())
    {
        IOdictionary(radIO).lookup("radiationModel") >> modelType;
    }
    else
    {
        Info<< "Radiation model not active: radiationProperties not found"
            << endl;
    }

    Info<< "Selecting radiationModel " << modelType << endl;

    TConstructorTable::iterator cstrIter =
        TConstructorTablePtr_->find(modelType);

    if (cstrIter == TConstructorTablePtr_->end())
    {
        FatalErrorIn
        (
            "radiationModel::New(const volScalarField&)"
        )   << "Unknown radiationModel type "
            << modelType << nl << nl
            << "Valid radiationModel types are:" << nl
            << TConstructorTablePtr_->sortedToc()
            << exit(FatalError);
    }

    return autoPtr<radiationModel>(cstrIter()(T));
}


CML::autoPtr<CML::radiation::radiationModel>
CML::radiation::radiationModel::New
(
    const dictionary& dict,
    const volScalarField& T
)
{
    const word modelType(dict.lookup("radiationModel"));

    Info<< "Selecting radiationModel " << modelType << endl;

    dictionaryConstructorTable::iterator cstrIter =
        dictionaryConstructorTablePtr_->find(modelType);

    if (cstrIter == dictionaryConstructorTablePtr_->end())
    {
        FatalErrorIn
        (
            "radiationModel::New(const volScalarField&)"
        )   << "Unknown radiationModel type "
            << modelType << nl << nl
            << "Valid radiationModel types are:" << nl
            << dictionaryConstructorTablePtr_->sortedToc()
            << exit(FatalError);
    }

    return autoPtr<radiationModel>(cstrIter()(dict, T));
}


// ************************************************************************* //
