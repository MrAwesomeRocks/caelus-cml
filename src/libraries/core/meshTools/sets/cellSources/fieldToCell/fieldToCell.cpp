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

#include "fieldToCell.hpp"
#include "polyMesh.hpp"
#include "cellSet.hpp"
#include "Time.hpp"
#include "IFstream.hpp"
#include "fieldDictionary.hpp"

#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{

defineTypeNameAndDebug(fieldToCell, 0);

addToRunTimeSelectionTable(topoSetSource, fieldToCell, word);

addToRunTimeSelectionTable(topoSetSource, fieldToCell, istream);

}


CML::topoSetSource::addToUsageTable CML::fieldToCell::usage_
(
    fieldToCell::typeName,
    "\n    Usage: fieldToCell field min max\n\n"
    "    Select all cells with field value >= min and <= max\n\n"
);


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::fieldToCell::applyToSet
(
    const topoSetSource::setAction action,
    const scalarField& field,
    topoSet& set
) const
{
    Info<< "    Field min:" << min(field)
        << " max:" << max(field) << endl;

    if ((action == topoSetSource::NEW) || (action == topoSetSource::ADD))
    {
        Info<< "    Adding all cells with value of field " << fieldName_
            << " within range " << min_ << ".." << max_ << endl;

        forAll(field, cellI)
        {
            if (field[cellI] >= min_ && field[cellI] <= max_)
            {
                set.insert(cellI);
            }
        }
    }
    else if (action == topoSetSource::DELETE)
    {
        Info<< "    Removing all cells with value of field " << fieldName_
            << " within range " << min_ << ".." << max_ << endl;

        forAll(field, cellI)
        {
            if (field[cellI] >= min_ && field[cellI] <= max_)
            {
                set.erase(cellI);
            }
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from components
CML::fieldToCell::fieldToCell
(
    const polyMesh& mesh,
    const word& fieldName,
    const scalar min,
    const scalar max
)
:
    topoSetSource(mesh),
    fieldName_(fieldName),
    min_(min),
    max_(max)
{}


// Construct from dictionary
CML::fieldToCell::fieldToCell
(
    const polyMesh& mesh,
    const dictionary& dict
)
:
    topoSetSource(mesh),
    fieldName_(dict.lookup("fieldName")),
    min_(readScalar(dict.lookup("min"))),
    max_(readScalar(dict.lookup("max")))
{}


// Construct from Istream
CML::fieldToCell::fieldToCell
(
    const polyMesh& mesh,
    Istream& is
)
:
    topoSetSource(mesh),
    fieldName_(checkIs(is)),
    min_(readScalar(checkIs(is))),
    max_(readScalar(checkIs(is)))
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::fieldToCell::~fieldToCell()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::fieldToCell::applyToSet
(
    const topoSetSource::setAction action,
    topoSet& set
) const
{

//    // Construct temporary fvMesh from polyMesh
//    fvMesh fMesh
//    (
//        mesh(), // IOobject
//        mesh().points(),
//        mesh().faces(),
//        mesh().cells()
//    );
//
//    const polyBoundaryMesh& patches = mesh().boundaryMesh();
//
//    List<polyPatch*> newPatches(patches.size());
//    forAll(patches, patchI)
//    {
//        const polyPatch& pp = patches[patchI];
//
//        newPatches[patchI] =
//            patches[patchI].clone
//            (
//                fMesh.boundaryMesh(),
//                patchI,
//                pp.size(),
//                pp.start()
//            ).ptr();
//    }
//    fMesh.addFvPatches(newPatches);

    // Try to load field
    IOobject fieldObject
    (
        fieldName_,
        mesh().time().timeName(),
        mesh(),
        IOobject::MUST_READ,
        IOobject::AUTO_WRITE,
        false
    );

    if (!fieldObject.headerOk())
    {
        WarningInFunction
            << "Cannot read field " << fieldName_
            << " from time " << mesh().time().timeName() << endl;
    }
    else if (fieldObject.headerClassName() == "volScalarField")
    {
        IFstream str(fieldObject.filePath());

        // Read dictionary
        fieldDictionary fieldDict(fieldObject, fieldObject.headerClassName());

        scalarField internalVals("internalField", fieldDict, mesh().nCells());

        applyToSet(action, internalVals, set);
    }
    else if (fieldObject.headerClassName() == "volVectorField")
    {
        IFstream str(fieldObject.filePath());

        // Read dictionary
        fieldDictionary fieldDict(fieldObject, fieldObject.headerClassName());

        vectorField internalVals("internalField", fieldDict, mesh().nCells());

        applyToSet(action, mag(internalVals), set);
    }
    else
    {
        WarningInFunction
            << "Cannot handle fields of type " << fieldObject.headerClassName()
            << endl;
    }
}


// ************************************************************************* //
