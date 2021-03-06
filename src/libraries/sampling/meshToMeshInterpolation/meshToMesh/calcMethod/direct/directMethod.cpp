/*---------------------------------------------------------------------------*\
Copyright (C) 2013-2014 OpenFOAM Foundation
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

#include "directMethod.hpp"
#include "indexedOctree.hpp"
#include "treeDataCell.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(directMethod, 0);
    addToRunTimeSelectionTable(meshToMeshMethod, directMethod, components);
}

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool CML::directMethod::intersect
(
    const label srcCellI,
    const label tgtCellI
) const
{
    return tgt_.pointInCell
    (
        src_.cellCentres()[srcCellI],
        tgtCellI,
        polyMesh::FACEPLANES
    );
}


bool CML::directMethod::findInitialSeeds
(
    const labelList& srcCellIDs,
    const boolList& mapFlag,
    const label startSeedI,
    label& srcSeedI,
    label& tgtSeedI
) const
{
    const cellList& srcCells = src_.cells();
    const faceList& srcFaces = src_.faces();
    const pointField& srcPts = src_.points();

    for (label i = startSeedI; i < srcCellIDs.size(); i++)
    {
        label srcI = srcCellIDs[i];

        if (mapFlag[srcI])
        {
            const pointField
                pts(srcCells[srcI].points(srcFaces, srcPts).xfer());

            forAll(pts, ptI)
            {
                const point& pt = pts[ptI];
                label tgtI = tgt_.cellTree().findInside(pt);

                if (tgtI != -1 && intersect(srcI, tgtI))
                {
                    srcSeedI = srcI;
                    tgtSeedI = tgtI;

                    return true;
                }
            }
        }
    }

    if (debug)
    {
        Pout<< "could not find starting seed" << endl;
    }

    return false;
}


void CML::directMethod::calculateAddressing
(
    labelListList& srcToTgtCellAddr,
    scalarListList& srcToTgtCellWght,
    labelListList& tgtToSrcCellAddr,
    scalarListList& tgtToSrcCellWght,
    const label srcSeedI,
    const label tgtSeedI,
    const labelList& srcCellIDs, // not used
    boolList& mapFlag,
    label& startSeedI
)
{
    // store a list of src cells already mapped
    labelList srcTgtSeed(src_.nCells(), -1);

    List<DynamicList<label> > srcToTgt(src_.nCells());
    List<DynamicList<label> > tgtToSrc(tgt_.nCells());

    DynamicList<label> srcSeeds(10);

    const scalarField& srcVc = src_.cellVolumes();
    const scalarField& tgtVc = tgt_.cellVolumes();

    label srcCellI = srcSeedI;
    label tgtCellI = tgtSeedI;

    do
    {
        // store src/tgt cell pair
        srcToTgt[srcCellI].append(tgtCellI);
        tgtToSrc[tgtCellI].append(srcCellI);

        // mark source cell srcSeedI as matched
        mapFlag[srcCellI] = false;

        // accumulate intersection volume
        V_ += srcVc[srcCellI];

        // find new source seed cell
        appendToDirectSeeds
        (
            mapFlag,
            srcTgtSeed,
            srcSeeds,
            srcCellI,
            tgtCellI
        );
    }
    while (srcCellI >= 0);

    // transfer addressing into persistent storage
    forAll(srcToTgtCellAddr, i)
    {
        srcToTgtCellWght[i] = scalarList(srcToTgt[i].size(), srcVc[i]);
        srcToTgtCellAddr[i].transfer(srcToTgt[i]);
    }

    forAll(tgtToSrcCellAddr, i)
    {
        tgtToSrcCellWght[i] = scalarList(tgtToSrc[i].size(), tgtVc[i]);
        tgtToSrcCellAddr[i].transfer(tgtToSrc[i]);
    }
}


void CML::directMethod::appendToDirectSeeds
(
    boolList& mapFlag,
    labelList& srcTgtSeed,
    DynamicList<label>& srcSeeds,
    label& srcSeedI,
    label& tgtSeedI
) const
{
    const labelList& srcNbr = src_.cellCells()[srcSeedI];
    const labelList& tgtNbr = tgt_.cellCells()[tgtSeedI];

    const vectorField& srcCentre = src_.cellCentres();

    forAll(srcNbr, i)
    {
        label srcI = srcNbr[i];

        if (mapFlag[srcI] && (srcTgtSeed[srcI] == -1))
        {
            // source cell srcI not yet mapped

            // identfy if target cell exists for source cell srcI
            bool found = false;
            forAll(tgtNbr, j)
            {
                label tgtI = tgtNbr[j];

                if
                (
                    tgt_.pointInCell
                    (
                        srcCentre[srcI],
                        tgtI,
                        polyMesh::FACEPLANES
                    )
                )
                {
                    // new match - append to lists
                    found = true;

                    srcTgtSeed[srcI] = tgtI;
                    srcSeeds.append(srcI);

                    break;
                }
            }

            if (!found)
            {
                // no match available for source cell srcI
                mapFlag[srcI] = false;
            }
        }
    }

    if (srcSeeds.size())
    {
        srcSeedI = srcSeeds.remove();
        tgtSeedI = srcTgtSeed[srcSeedI];
    }
    else
    {
        srcSeedI = -1;
        tgtSeedI = -1;
    }
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::directMethod::directMethod
(
    const polyMesh& src,
    const polyMesh& tgt
)
:
    meshToMeshMethod(src, tgt)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::directMethod::~directMethod()
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::directMethod::calculate
(
    labelListList& srcToTgtAddr,
    scalarListList& srcToTgtWght,
    labelListList& tgtToSrcAddr,
    scalarListList& tgtToSrcWght
)
{
    bool ok = initialise
    (
        srcToTgtAddr,
        srcToTgtWght,
        tgtToSrcAddr,
        tgtToSrcWght
    );

    if (!ok)
    {
        return;
    }

    // (potentially) participating source mesh cells
    const labelList srcCellIDs(maskCells());

    // list to keep track of whether src cell can be mapped
    boolList mapFlag(src_.nCells(), false);
    UIndirectList<bool>(mapFlag, srcCellIDs) = true;

    // find initial point in tgt mesh
    label srcSeedI = -1;
    label tgtSeedI = -1;
    label startSeedI = 0;

    bool startWalk =
        findInitialSeeds
        (
            srcCellIDs,
            mapFlag,
            startSeedI,
            srcSeedI,
            tgtSeedI
        );

    if (startWalk)
    {
        calculateAddressing
        (
            srcToTgtAddr,
            srcToTgtWght,
            tgtToSrcAddr,
            tgtToSrcWght,
            srcSeedI,
            tgtSeedI,
            srcCellIDs,
            mapFlag,
            startSeedI
        );
    }
    else
    {
        // if meshes are collocated, after inflating the source mesh bounding
        // box tgt mesh cells may be transferred, but may still not overlap
        // with the source mesh
        return;
    }
}


// ************************************************************************* //
