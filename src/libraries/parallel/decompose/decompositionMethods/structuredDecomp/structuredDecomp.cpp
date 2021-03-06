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

#include "structuredDecomp.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "FaceCellWave.hpp"
#include "topoDistanceData.hpp"
#include "fvMeshSubset.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(structuredDecomp, 0);

    addToRunTimeSelectionTable
    (
        decompositionMethod,
        structuredDecomp,
        dictionary
    );
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::structuredDecomp::structuredDecomp(const dictionary& decompositionDict)
:
    decompositionMethod(decompositionDict),
    methodDict_(decompositionDict_.subDict(typeName + "Coeffs"))
{
    methodDict_.set("numberOfSubdomains", nDomains());
    method_ = decompositionMethod::New(methodDict_);
    patches_ = wordList(methodDict_.lookup("patches"));
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool CML::structuredDecomp::parallelAware() const
{
    return method_().parallelAware();
}


CML::labelList CML::structuredDecomp::decompose
(
    const polyMesh& mesh,
    const pointField& cc,
    const scalarField& cWeights
)
{
    labelList patchIDs(patches_.size());
    const polyBoundaryMesh& pbm = mesh.boundaryMesh();

    label nFaces = 0;
    forAll(patches_, i)
    {
        patchIDs[i] = pbm.findPatchID(patches_[i]);

        if (patchIDs[i] == -1)
        {
            FatalErrorInFunction
                << "Cannot find patch " << patches_[i] << endl
                << "Valid patches are " << pbm.names()
                << exit(FatalError);
        }
        nFaces += pbm[patchIDs[i]].size();
    }

    // Extract a submesh.
    labelHashSet patchCells(2*nFaces);
    forAll(patchIDs, i)
    {
        const labelUList& fc = pbm[patchIDs[i]].faceCells();
        forAll(fc, i)
        {
            patchCells.insert(fc[i]);
        }
    }

    // Subset the layer of cells next to the patch
    fvMeshSubset subsetter(dynamic_cast<const fvMesh&>(mesh));
    subsetter.setLargeCellSubset(patchCells);
    const fvMesh& subMesh = subsetter.subMesh();
    pointField subCc(cc, subsetter.cellMap());
    scalarField subWeights(cWeights, subsetter.cellMap());

    // Decompose the layer of cells
    labelList subDecomp(method_().decompose(subMesh, subCc, subWeights));


    // Transfer to final decomposition
    labelList finalDecomp(cc.size(), -1);
    forAll(subDecomp, i)
    {
        finalDecomp[subsetter.cellMap()[i]] = subDecomp[i];
    }

    // Field on cells and faces.
    List<topoDistanceData> cellData(mesh.nCells());
    List<topoDistanceData> faceData(mesh.nFaces());

    // Start of changes
    labelList patchFaces(nFaces);
    List<topoDistanceData> patchData(nFaces);
    nFaces = 0;
    forAll(patchIDs, i)
    {
        const polyPatch& pp = pbm[patchIDs[i]];
        const labelUList& fc = pp.faceCells();
        forAll(fc, i)
        {
            patchFaces[nFaces] = pp.start()+i;
            patchData[nFaces] = topoDistanceData(finalDecomp[fc[i]], 0);
            nFaces++;
        }
    }

    // Propagate information inwards
    FaceCellWave<topoDistanceData> deltaCalc
    (
        mesh,
        patchFaces,
        patchData,
        faceData,
        cellData,
        mesh.globalData().nTotalCells()+1
    );

    // And extract
    bool haveWarned = false;
    forAll(finalDecomp, cellI)
    {
        if (!cellData[cellI].valid(deltaCalc.data()))
        {
            if (!haveWarned)
            {
                WarningInFunction
                    << "Did not visit some cells, e.g. cell " << cellI
                    << " at " << mesh.cellCentres()[cellI] << endl
                    << "Assigning  these cells to domain 0." << endl;
                haveWarned = true;
            }
            finalDecomp[cellI] = 0;
        }
        else
        {
            finalDecomp[cellI] = cellData[cellI].data();
        }
    }

    return finalDecomp;
}


CML::labelList CML::structuredDecomp::decompose
(
    const labelListList& globalPointPoints,
    const pointField& points,
    const scalarField& pointWeights
)
{
    NotImplemented;

    return labelList::null();
}


// ************************************************************************* //
