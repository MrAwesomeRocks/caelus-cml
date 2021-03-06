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

#include "multiLevelDecomp.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "IFstream.hpp"
#include "globalIndex.hpp"
#include "mapDistribute.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(multiLevelDecomp, 0);

    addToRunTimeSelectionTable
    (
        decompositionMethod,
        multiLevelDecomp,
        dictionary
    );
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// Given a subset of cells determine the new global indices. The problem
// is in the cells from neighbouring processors which need to be renumbered.
void CML::multiLevelDecomp::subsetGlobalCellCells
(
    const label nDomains,
    const label domainI,
    const labelList& dist,

    const labelListList& cellCells,
    const labelList& set,
    labelListList& subCellCells,
    labelList& cutConnections
) const
{
    // Determine new index for cells by inverting subset
    labelList oldToNew(invert(cellCells.size(), set));

    globalIndex globalCells(cellCells.size());

    // Subset locally the elements for which I have data
    subCellCells = UIndirectList<labelList>(cellCells, set);

    // Get new indices for neighbouring processors
    List<Map<label> > compactMap;
    mapDistribute map(globalCells, subCellCells, compactMap);
    map.distribute(oldToNew);
    labelList allDist(dist);
    map.distribute(allDist);

    // Now we have:
    // oldToNew : the locally-compact numbering of all our cellCells. -1 if
    //            cellCell is not in set.
    // allDist  : destination domain for all our cellCells
    // subCellCells : indexes into oldToNew and allDist

    // Globally compact numbering for cells in set.
    globalIndex globalSubCells(set.size());

    // Now subCellCells contains indices into oldToNew which are the
    // new locations of the neighbouring cells.

    cutConnections.setSize(nDomains);
    cutConnections = 0;

    forAll(subCellCells, subCellI)
    {
        labelList& cCells = subCellCells[subCellI];

        // Keep the connections to valid mapped cells
        label newI = 0;
        forAll(cCells, i)
        {
            // Get locally-compact cell index of neighbouring cell
            label nbrCellI = oldToNew[cCells[i]];
            if (nbrCellI == -1)
            {
                cutConnections[allDist[cCells[i]]]++;
            }
            else
            {
                // Reconvert local cell index into global one

                // Get original neighbour
                label cellI = set[subCellI];
                label oldNbrCellI = cellCells[cellI][i];
                // Get processor from original neighbour
                label procI = globalCells.whichProcID(oldNbrCellI);
                // Convert into global compact numbering
                cCells[newI++] = globalSubCells.toGlobal(procI, nbrCellI);
            }
        }
        cCells.setSize(newI);
    }
}


void CML::multiLevelDecomp::decompose
(
    const labelListList& pointPoints,
    const pointField& points,
    const scalarField& pointWeights,
    const labelList& pointMap,      // map back to original points
    const label levelI,

    labelField& finalDecomp
)
{
    labelList dist
    (
        methods_[levelI].decompose
        (
            pointPoints,
            points,
            pointWeights
        )
    );

    forAll(pointMap, i)
    {
        label orig = pointMap[i];
        finalDecomp[orig] += dist[i];
    }

    if (levelI != methods_.size()-1)
    {
        // Recurse

        // Determine points per domain
        label n = methods_[levelI].nDomains();
        labelListList domainToPoints(invertOneToMany(n, dist));

        // 'Make space' for new levels of decomposition
        finalDecomp *= methods_[levelI+1].nDomains();

        // Extract processor+local index from point-point addressing
        if (debug && Pstream::master())
        {
            Pout<< "Decomposition at level " << levelI << " :" << endl;
        }

        for (label domainI = 0; domainI < n; domainI++)
        {
            // Extract elements for current domain
            const labelList domainPoints(findIndices(dist, domainI));

            // Subset point-wise data.
            pointField subPoints(points, domainPoints);
            scalarField subWeights(pointWeights, domainPoints);
            labelList subPointMap(UIndirectList<label>(pointMap, domainPoints));
            // Subset point-point addressing (adapt global numbering)
            labelListList subPointPoints;
            labelList nOutsideConnections;
            subsetGlobalCellCells
            (
                n,
                domainI,
                dist,

                pointPoints,
                domainPoints,

                subPointPoints,
                nOutsideConnections
            );

            label nPoints = returnReduce(domainPoints.size(), plusOp<label>());
            Pstream::listCombineGather(nOutsideConnections, plusEqOp<label>());
            Pstream::listCombineScatter(nOutsideConnections);
            label nPatches = 0;
            label nFaces = 0;
            forAll(nOutsideConnections, i)
            {
                if (nOutsideConnections[i] > 0)
                {
                    nPatches++;
                    nFaces += nOutsideConnections[i];
                }
            }

            string oldPrefix;
            if (debug && Pstream::master())
            {
                Pout<< "    Domain " << domainI << nl
                    << "        Number of cells = " << nPoints << nl
                    << "        Number of inter-domain patches = " << nPatches
                    << nl
                    << "        Number of inter-domain faces = " << nFaces << nl
                    << endl;
                oldPrefix = Pout.prefix();
                Pout.prefix() = "  " + oldPrefix;
            }

            decompose
            (
                subPointPoints,
                subPoints,
                subWeights,
                subPointMap,
                levelI+1,

                finalDecomp
            );
            if (debug && Pstream::master())
            {
                Pout.prefix() = oldPrefix;
            }
        }


        if (debug)
        {
            // Do straight decompose of two levels
            label nNext = methods_[levelI+1].nDomains();
            label nTotal = n*nNext;

            // Retrieve original level0 dictionary and modify number of domains
            dictionary::const_iterator iter =
                decompositionDict_.subDict(typeName + "Coeffs").begin();
            dictionary myDict = iter().dict();
            myDict.set("numberOfSubdomains", nTotal);

            if (debug && Pstream::master())
            {
                Pout<< "Reference decomposition with " << myDict << " :"
                    << endl;
            }

            autoPtr<decompositionMethod> method0 = decompositionMethod::New
            (
                myDict
            );
            labelList dist
            (
                method0().decompose
                (
                    pointPoints,
                    points,
                    pointWeights
                )
            );

            for (label blockI = 0; blockI < n; blockI++)
            {
                // Count the number inbetween blocks of nNext size

                label nPoints = 0;
                labelList nOutsideConnections(n, 0);
                forAll(pointPoints, pointI)
                {
                    if ((dist[pointI] / nNext) == blockI)
                    {
                        nPoints++;

                        const labelList& pPoints = pointPoints[pointI];

                        forAll(pPoints, i)
                        {
                            label distBlockI = dist[pPoints[i]] / nNext;
                            if (distBlockI != blockI)
                            {
                                nOutsideConnections[distBlockI]++;
                            }
                        }
                    }
                }

                reduce(nPoints, plusOp<label>());
                Pstream::listCombineGather
                (
                    nOutsideConnections,
                    plusEqOp<label>()
                );
                Pstream::listCombineScatter(nOutsideConnections);
                label nPatches = 0;
                label nFaces = 0;
                forAll(nOutsideConnections, i)
                {
                    if (nOutsideConnections[i] > 0)
                    {
                        nPatches++;
                        nFaces += nOutsideConnections[i];
                    }
                }

                if (debug && Pstream::master())
                {
                    Pout<< "    Domain " << blockI << nl
                        << "        Number of cells = " << nPoints << nl
                        << "        Number of inter-domain patches = "
                        << nPatches << nl
                        << "        Number of inter-domain faces = " << nFaces
                        << nl << endl;
                }
            }
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::multiLevelDecomp::multiLevelDecomp(const dictionary& decompositionDict)
:
    decompositionMethod(decompositionDict),
    methodsDict_(decompositionDict_.subDict(typeName + "Coeffs"))
{
    methods_.setSize(methodsDict_.size());
    label i = 0;
    forAllConstIter(dictionary, methodsDict_, iter)
    {
        methods_.set(i++, decompositionMethod::New(iter().dict()));
    }

    label n = 1;
    Info<< "decompositionMethod " << type() << " :" << endl;
    forAll(methods_, i)
    {
        Info<< "    level " << i << " decomposing with " << methods_[i].type()
            << " into " << methods_[i].nDomains() << " subdomains." << endl;

        n *= methods_[i].nDomains();
    }

    if (n != nDomains())
    {
        FatalErrorInFunction
            << "Top level decomposition specifies " << nDomains()
            << " domains which is not equal to the product of"
            << " all sub domains " << n
            << exit(FatalError);
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool CML::multiLevelDecomp::parallelAware() const
{
    forAll(methods_, i)
    {
        if (!methods_[i].parallelAware())
        {
            return false;
        }
    }
    return true;
}


CML::labelList CML::multiLevelDecomp::decompose
(
    const polyMesh& mesh,
    const pointField& cc,
    const scalarField& cWeights
)
{
    CompactListList<label> cellCells;
    calcCellCells(mesh, identity(cc.size()), cc.size(), true, cellCells);

    labelField finalDecomp(cc.size(), 0);
    labelList cellMap(identity(cc.size()));

    decompose
    (
        cellCells(),
        cc,
        cWeights,
        cellMap,      // map back to original cells
        0,

        finalDecomp
    );

    return finalDecomp;
}


CML::labelList CML::multiLevelDecomp::decompose
(
    const labelListList& globalPointPoints,
    const pointField& points,
    const scalarField& pointWeights
)
{
    labelField finalDecomp(points.size(), 0);
    labelList pointMap(identity(points.size()));

    decompose
    (
        globalPointPoints,
        points,
        pointWeights,
        pointMap,       // map back to original points
        0,

        finalDecomp
    );

    return finalDecomp;
}


// ************************************************************************* //
