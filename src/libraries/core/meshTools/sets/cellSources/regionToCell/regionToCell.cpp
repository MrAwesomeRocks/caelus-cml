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

#include "regionToCell.hpp"
#include "polyMesh.hpp"
#include "regionSplit.hpp"
#include "globalMeshData.hpp"
#include "cellSet.hpp"
#include "syncTools.hpp"

#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{

defineTypeNameAndDebug(regionToCell, 0);

addToRunTimeSelectionTable(topoSetSource, regionToCell, word);

addToRunTimeSelectionTable(topoSetSource, regionToCell, istream);

}


CML::topoSetSource::addToUsageTable CML::regionToCell::usage_
(
    regionToCell::typeName,
    "\n    Usage: regionToCell subCellSet (x y z)\n\n"
    "    Select all cells in the connected region containing point.\n"
    "    If started inside the subCellSet keeps to it;\n"
    "    if started outside stays outside.\n"
);


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::regionToCell::combine(topoSet& set, const bool add) const
{
    label cellI = mesh_.findCell(insidePoint_);

    // Load the subset of cells
    boolList blockedFace(mesh_.nFaces(), false);
    {
        Info<< "Loading subset " << setName_ << " to delimit search region."
            << endl;
        cellSet subSet(mesh_, setName_);

        boolList inSubset(mesh_.nCells(), false);
        forAllConstIter(cellSet, subSet, iter)
        {
            inSubset[iter.key()] = true;
        }

        if (cellI != -1 && inSubset[cellI])
        {
            Pout<< "Point " << insidePoint_ << " is inside cellSet "
                << setName_ << endl
                << "Collecting all cells connected to " << cellI
                << " and inside cellSet " << setName_ << endl;
        }
        else
        {
            Pout<< "Point " << insidePoint_ << " is outside cellSet "
                << setName_ << endl
                << "Collecting all cells connected to " << cellI
                << " and outside cellSet " << setName_ << endl;
        }

        // Get coupled cell status
        label nInt = mesh_.nInternalFaces();
        boolList neiSet(mesh_.nFaces()-nInt, false);
        for (label faceI = nInt; faceI < mesh_.nFaces(); faceI++)
        {
             neiSet[faceI-nInt] = inSubset[mesh_.faceOwner()[faceI]];
        }
        syncTools::swapBoundaryFaceList(mesh_, neiSet);

        // Find faces inbetween subSet and non-subset.
        for (label faceI = 0; faceI < nInt; faceI++)
        {
            bool ownInSet = inSubset[mesh_.faceOwner()[faceI]];
            bool neiInSet = inSubset[mesh_.faceNeighbour()[faceI]];
            blockedFace[faceI] = (ownInSet != neiInSet);
        }
        for (label faceI = nInt; faceI < mesh_.nFaces(); faceI++)
        {
            bool ownInSet = inSubset[mesh_.faceOwner()[faceI]];
            bool neiInSet = neiSet[faceI-nInt];
            blockedFace[faceI] = (ownInSet != neiInSet);
        }
    }

    // Find connected regions without crossing boundary of the cellset.
    regionSplit regions(mesh_, blockedFace);

    // Get the region containing the insidePoint
    label regionI = -1;

    if (cellI != -1)
    {
        // On processor that has found cell.
        regionI = regions[cellI];
    }

    reduce(regionI, maxOp<label>());

    if (regionI == -1)
    {
        WarningInFunction
            << "Point " << insidePoint_
            << " is not inside the mesh." << nl
            << "Bounding box of the mesh:" << mesh_.bounds()
            << endl;
        return;
    }


    // Pick up the cells of the region
    const labelList regionCells(findIndices(regions, regionI));

    forAll(regionCells, i)
    {
        addOrDelete(set, regionCells[i], add);
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from components
CML::regionToCell::regionToCell
(
    const polyMesh& mesh,
    const word& setName,
    const point& insidePoint
)
:
    topoSetSource(mesh),
    setName_(setName),
    insidePoint_(insidePoint)
{}


// Construct from dictionary
CML::regionToCell::regionToCell
(
    const polyMesh& mesh,
    const dictionary& dict
)
:
    topoSetSource(mesh),
    setName_(dict.lookup("set")),
    insidePoint_(dict.lookup("insidePoint"))
{}


// Construct from Istream
CML::regionToCell::regionToCell
(
    const polyMesh& mesh,
    Istream& is
)
:
    topoSetSource(mesh),
    setName_(checkIs(is)),
    insidePoint_(checkIs(is))
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::regionToCell::~regionToCell()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::regionToCell::applyToSet
(
    const topoSetSource::setAction action,
    topoSet& set
) const
{
    if ((action == topoSetSource::NEW) || (action == topoSetSource::ADD))
    {
        Info<< "    Adding all cells of connected region containing point "
            << insidePoint_ << " ..." << endl;

        combine(set, true);
    }
    else if (action == topoSetSource::DELETE)
    {
        Info<< "    Removing all cells of connected region containing point "
            << insidePoint_ << " ..." << endl;

        combine(set, false);
    }
}


// ************************************************************************* //
