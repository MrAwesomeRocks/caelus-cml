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

#include "faceZoneToCell.hpp"
#include "polyMesh.hpp"

#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(faceZoneToCell, 0);
    addToRunTimeSelectionTable(topoSetSource, faceZoneToCell, word);
    addToRunTimeSelectionTable(topoSetSource, faceZoneToCell, istream);

    template<>
    const char* CML::NamedEnum
    <
        CML::faceZoneToCell::faceAction,
        2
    >::names[] =
    {
        "master",
        "slave"
    };
}


CML::topoSetSource::addToUsageTable CML::faceZoneToCell::usage_
(
    faceZoneToCell::typeName,
    "\n    Usage: faceZoneToCell zone master|slave\n\n"
    "    Select master or slave side of the faceZone."
    " Note:accepts wildcards for zone.\n\n"
);


const CML::NamedEnum<CML::faceZoneToCell::faceAction, 2>
    CML::faceZoneToCell::faceActionNames_;


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::faceZoneToCell::combine(topoSet& set, const bool add) const
{
    bool hasMatched = false;

    forAll(mesh_.faceZones(), i)
    {
        const faceZone& zone = mesh_.faceZones()[i];

        if (zoneName_.match(zone.name()))
        {
            const labelList& cellLabels =
            (
                option_ == MASTER
              ? zone.masterCells()
              : zone.slaveCells()
            );

            Info<< "    Found matching zone " << zone.name()
                << " with " << cellLabels.size() << " cells on selected side."
                << endl;

            hasMatched = true;

            forAll(cellLabels, i)
            {
                // Only do active cells
                if (cellLabels[i] < mesh_.nCells())
                {
                    addOrDelete(set, cellLabels[i], add);
                }
            }
        }
    }

    if (!hasMatched)
    {
        WarningInFunction
            << "Cannot find any faceZone named " << zoneName_ << endl
            << "Valid names are " << mesh_.faceZones().names() << endl;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from components
CML::faceZoneToCell::faceZoneToCell
(
    const polyMesh& mesh,
    const word& zoneName,
    const faceAction option
)
:
    topoSetSource(mesh),
    zoneName_(zoneName),
    option_(option)
{}


// Construct from dictionary
CML::faceZoneToCell::faceZoneToCell
(
    const polyMesh& mesh,
    const dictionary& dict
)
:
    topoSetSource(mesh),
    zoneName_(dict.lookup("name")),
    option_(faceActionNames_.read(dict.lookup("option")))
{}


// Construct from Istream
CML::faceZoneToCell::faceZoneToCell
(
    const polyMesh& mesh,
    Istream& is
)
:
    topoSetSource(mesh),
    zoneName_(checkIs(is)),
    option_(faceActionNames_.read(checkIs(is)))
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::faceZoneToCell::~faceZoneToCell()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::faceZoneToCell::applyToSet
(
    const topoSetSource::setAction action,
    topoSet& set
) const
{
    if ((action == topoSetSource::NEW) || (action == topoSetSource::ADD))
    {
        Info<< "    Adding all " << faceActionNames_[option_]
            << " cells of faceZone " << zoneName_ << " ..." << endl;

        combine(set, true);
    }
    else if (action == topoSetSource::DELETE)
    {
        Info<< "    Removing all " << faceActionNames_[option_]
            << " cells of faceZone " << zoneName_ << " ..." << endl;

        combine(set, false);
    }
}


// ************************************************************************* //
