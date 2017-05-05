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

InClass
    domainDecomposition

Description
    Private member of domainDecomposition.
    Decomposes the mesh into bits

\*---------------------------------------------------------------------------*/

#include "domainDecomposition.hpp"
#include "IOstreams.hpp"
#include "boolList.hpp"
#include "cyclicPolyPatch.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::domainDecomposition::append(labelList& lst, const label elem)
{
    label sz = lst.size();
    lst.setSize(sz+1);
    lst[sz] = elem;
}


void CML::domainDecomposition::addInterProcFace
(
    const label facei,
    const label ownerProc,
    const label nbrProc,

    List<Map<label> >& nbrToInterPatch,
    List<DynamicList<DynamicList<label> > >& interPatchFaces
) const
{
    Map<label>::iterator patchIter = nbrToInterPatch[ownerProc].find(nbrProc);

    // Introduce turning index only for internal faces (are duplicated).
    label ownerIndex = facei+1;
    label nbrIndex = -(facei+1);

    if (patchIter != nbrToInterPatch[ownerProc].end())
    {
        // Existing interproc patch. Add to both sides.
        label toNbrProcPatchI = patchIter();
        interPatchFaces[ownerProc][toNbrProcPatchI].append(ownerIndex);

        if (isInternalFace(facei))
        {
            label toOwnerProcPatchI = nbrToInterPatch[nbrProc][ownerProc];
            interPatchFaces[nbrProc][toOwnerProcPatchI].append(nbrIndex);
        }
    }
    else
    {
        // Create new interproc patches.
        label toNbrProcPatchI = nbrToInterPatch[ownerProc].size();
        nbrToInterPatch[ownerProc].insert(nbrProc, toNbrProcPatchI);
        DynamicList<label> oneFace;
        oneFace.append(ownerIndex);
        interPatchFaces[ownerProc].append(oneFace);

        if (isInternalFace(facei))
        {
            label toOwnerProcPatchI = nbrToInterPatch[nbrProc].size();
            nbrToInterPatch[nbrProc].insert(ownerProc, toOwnerProcPatchI);
            oneFace.clear();
            oneFace.append(nbrIndex);
            interPatchFaces[nbrProc].append(oneFace);
        }
    }
}


void CML::domainDecomposition::decomposeMesh()
{
    // Decide which cell goes to which processor
    distributeCells();

    // Distribute the cells according to the given processor label

    // calculate the addressing information for the original mesh
    Info<< "\nCalculating original mesh data" << endl;

    // set references to the original mesh
    const polyBoundaryMesh& patches = boundaryMesh();
    const faceList& fcs = faces();
    const labelList& owner = faceOwner();
    const labelList& neighbour = faceNeighbour();

    // loop through the list of processor labels for the cell and add the
    // cell shape to the list of cells for the appropriate processor

    Info<< "\nDistributing cells to processors" << endl;

    // Cells per processor
    procCellAddressing_ = invertOneToMany(nProcs_, cellToProc_);

    Info<< "\nDistributing faces to processors" << endl;

    // Loop through all internal faces and decide which processor they belong to
    // First visit all internal faces. If cells at both sides belong to the
    // same processor, the face is an internal face. If they are different,
    // it belongs to both processors.

    procFaceAddressing_.setSize(nProcs_);

    // Internal faces
    forAll(neighbour, facei)
    {
        if (cellToProc_[owner[facei]] == cellToProc_[neighbour[facei]])
        {
            // Face internal to processor. Notice no turning index.
            procFaceAddressing_[cellToProc_[owner[facei]]].append(facei+1);
        }
    }

    // for all processors, set the size of start index and patch size
    // lists to the number of patches in the mesh
    forAll(procPatchSize_, procI)
    {
        procPatchSize_[procI].setSize(patches.size());
        procPatchStartIndex_[procI].setSize(patches.size());
    }

    forAll(patches, patchi)
    {
        // Reset size and start index for all processors
        forAll(procPatchSize_, procI)
        {
            procPatchSize_[procI][patchi] = 0;
            procPatchStartIndex_[procI][patchi] =
                procFaceAddressing_[procI].size();
        }

        const label patchStart = patches[patchi].start();

        if (!isA<cyclicPolyPatch>(patches[patchi]))
        {
            // Normal patch. Add faces to processor where the cell
            // next to the face lives

            const labelUList& patchFaceCells =
                patches[patchi].faceCells();

            forAll(patchFaceCells, facei)
            {
                const label curProc = cellToProc_[patchFaceCells[facei]];

                // add the face without turning index
                procFaceAddressing_[curProc].append(patchStart+facei+1);

                // increment the number of faces for this patch
                procPatchSize_[curProc][patchi]++;
            }
        }
        else
        {
            const cyclicPolyPatch& pp = refCast<const cyclicPolyPatch>
            (
                patches[patchi]
            );
            // cyclic: check opposite side on this processor
            const labelUList& patchFaceCells = pp.faceCells();

            const labelUList& nbrPatchFaceCells =
                pp.neighbPatch().faceCells();

            forAll(patchFaceCells, facei)
            {
                const label curProc = cellToProc_[patchFaceCells[facei]];
                const label nbrProc = cellToProc_[nbrPatchFaceCells[facei]];
                if (curProc == nbrProc)
                {
                    // add the face without turning index
                    procFaceAddressing_[curProc].append(patchStart+facei+1);
                    // increment the number of faces for this patch
                    procPatchSize_[curProc][patchi]++;
                }
            }
        }
    }


    // Done internal bits of the new mesh and the ordinary patches.


    // Per processor, from neighbour processor to the inter-processor patch
    // that communicates with that neighbour
    List<Map<label> > procNbrToInterPatch(nProcs_);

    // Per processor the faces per inter-processor patch
    List<DynamicList<DynamicList<label> > > interPatchFaces(nProcs_);

    // Processor boundaries from internal faces
    forAll(neighbour, facei)
    {
        label ownerProc = cellToProc_[owner[facei]];
        label nbrProc = cellToProc_[neighbour[facei]];

        if (ownerProc != nbrProc)
        {
            // inter - processor patch face found.
            addInterProcFace
            (
                facei,
                ownerProc,
                nbrProc,

                procNbrToInterPatch,
                interPatchFaces
            );
        }
    }

    // Add the proper processor faces to the sub information. For faces
    // originating from internal faces this is always -1.
    List<labelListList> subPatchIDs(nProcs_);
    List<labelListList> subPatchStarts(nProcs_);
    forAll(interPatchFaces, procI)
    {
        label nInterfaces = interPatchFaces[procI].size();

        subPatchIDs[procI].setSize(nInterfaces, labelList(1, label(-1)));
        subPatchStarts[procI].setSize(nInterfaces, labelList(1, label(0)));
    }


    // Special handling needed for the case that multiple processor cyclic
    // patches are created on each local processor domain, e.g. if a 3x3 case
    // is decomposed using the decomposition:
    //
    //              | 1 | 0 | 2 |
    //  cyclic left | 2 | 0 | 1 | cyclic right
    //              | 2 | 0 | 1 |
    //
    // - processors 1 and 2 will both have pieces of both cyclic left- and
    //   right sub-patches present
    // - the interface patch faces are stored in a single list, where each
    //   sub-patch is referenced into the list using a patch start index and
    //   size
    // - if the patches are in order (in the boundary file) of left, right
    //   - processor 1 will send: left, right
    //   - processor 1 will need to receive in reverse order: right, left
    //   - similarly for processor 2
    // - the sub-patches are therefore generated in 4 passes of the patch lists
    //   1. add faces from owner patch where local proc i < nbr proc i
    //   2. add faces from nbr patch where local proc i < nbr proc i
    //   3. add faces from owner patch where local proc i > nbr proc i
    //   4. add faces from nbr patch where local proc i > nbr proc i

    processInterCyclics
    (
        patches,
        interPatchFaces,
        procNbrToInterPatch,
        subPatchIDs,
        subPatchStarts,
        true,
        lessOp<label>()
    );

    processInterCyclics
    (
        patches,
        interPatchFaces,
        procNbrToInterPatch,
        subPatchIDs,
        subPatchStarts,
        false,
        lessOp<label>()
    );

    processInterCyclics
    (
        patches,
        interPatchFaces,
        procNbrToInterPatch,
        subPatchIDs,
        subPatchStarts,
        false,
        greaterOp<label>()
    );

    processInterCyclics
    (
        patches,
        interPatchFaces,
        procNbrToInterPatch,
        subPatchIDs,
        subPatchStarts,
        true,
        greaterOp<label>()
    );


    // Sort inter-proc patch by neighbour
    labelList order;
    forAll(procNbrToInterPatch, procI)
    {
        label nInterfaces = procNbrToInterPatch[procI].size();

        procNeighbourProcessors_[procI].setSize(nInterfaces);
        procProcessorPatchSize_[procI].setSize(nInterfaces);
        procProcessorPatchStartIndex_[procI].setSize(nInterfaces);
        procProcessorPatchSubPatchIDs_[procI].setSize(nInterfaces);
        procProcessorPatchSubPatchStarts_[procI].setSize(nInterfaces);

        //Info<< "Processor " << procI << endl;

        // Get sorted neighbour processors
        const Map<label>& curNbrToInterPatch = procNbrToInterPatch[procI];
        labelList nbrs = curNbrToInterPatch.toc();

        sortedOrder(nbrs, order);

        DynamicList<DynamicList<label> >& curInterPatchFaces =
            interPatchFaces[procI];

        forAll(nbrs, i)
        {
            const label nbrProc = nbrs[i];
            const label interPatch = curNbrToInterPatch[nbrProc];

            procNeighbourProcessors_[procI][i] = nbrProc;
            procProcessorPatchSize_[procI][i] =
                curInterPatchFaces[interPatch].size();
            procProcessorPatchStartIndex_[procI][i] =
                procFaceAddressing_[procI].size();

            // Add size as last element to substarts and transfer
            append
            (
                subPatchStarts[procI][interPatch],
                curInterPatchFaces[interPatch].size()
            );
            procProcessorPatchSubPatchIDs_[procI][i].transfer
            (
                subPatchIDs[procI][interPatch]
            );
            procProcessorPatchSubPatchStarts_[procI][i].transfer
            (
                subPatchStarts[procI][interPatch]
            );

            //Info<< "    nbr:" << nbrProc << endl;
            //Info<< "    interpatch:" << interPatch << endl;
            //Info<< "    size:" << procProcessorPatchSize_[procI][i] << endl;
            //Info<< "    start:" << procProcessorPatchStartIndex_[procI][i]
            //    << endl;
            //Info<< "    subPatches:"
            //    << procProcessorPatchSubPatchIDs_[procI][i]
            //    << endl;
            //Info<< "    subStarts:"
            //    << procProcessorPatchSubPatchStarts_[procI][i] << endl;

            // And add all the face labels for interPatch
            DynamicList<label>& interPatchFaces =
                curInterPatchFaces[interPatch];

            forAll(interPatchFaces, j)
            {
                procFaceAddressing_[procI].append(interPatchFaces[j]);
            }
            interPatchFaces.clearStorage();
        }
        curInterPatchFaces.clearStorage();
        procFaceAddressing_[procI].shrink();
    }


////XXXXXXX
//// Print a bit
//    forAll(procPatchStartIndex_, procI)
//    {
//        Info<< "Processor:" << procI << endl;
//
//        Info<< "    total faces:" << procFaceAddressing_[procI].size()
//            << endl;
//
//        const labelList& curProcPatchStartIndex = procPatchStartIndex_[procI];
//
//        forAll(curProcPatchStartIndex, patchI)
//        {
//            Info<< "    patch:" << patchI
//                << "\tstart:" << curProcPatchStartIndex[patchI]
//                << "\tsize:" << procPatchSize_[procI][patchI]
//                << endl;
//        }
//    }
//    Info<< endl;
//
//    forAll(procNeighbourProcessors_, procI)
//    {
//        Info<< "Processor " << procI << endl;
//
//        forAll(procNeighbourProcessors_[procI], i)
//        {
//            Info<< "    nbr:" << procNeighbourProcessors_[procI][i] << endl;
//            Info<< "    size:" << procProcessorPatchSize_[procI][i] << endl;
//            Info<< "    start:" << procProcessorPatchStartIndex_[procI][i]
//                << endl;
//        }
//    }
//    Info<< endl;
//
//    forAll(procFaceAddressing_, procI)
//    {
//        Info<< "Processor:" << procI << endl;
//
//        Info<< "    faces:" << procFaceAddressing_[procI] << endl;
//    }



    Info<< "\nDistributing points to processors" << endl;
    // For every processor, loop through the list of faces for the processor.
    // For every face, loop through the list of points and mark the point as
    // used for the processor. Collect the list of used points for the
    // processor.

    forAll(procPointAddressing_, procI)
    {
        boolList pointLabels(nPoints(), false);

        // Get reference to list of used faces
        const labelList& procFaceLabels = procFaceAddressing_[procI];

        forAll(procFaceLabels, facei)
        {
            // Because of the turning index, some labels may be negative
            const labelList& facePoints = fcs[mag(procFaceLabels[facei]) - 1];

            forAll(facePoints, pointi)
            {
                // Mark the point as used
                pointLabels[facePoints[pointi]] = true;
            }
        }

        // Collect the used points
        labelList& procPointLabels = procPointAddressing_[procI];

        procPointLabels.setSize(pointLabels.size());

        label nUsedPoints = 0;

        forAll(pointLabels, pointi)
        {
            if (pointLabels[pointi])
            {
                procPointLabels[nUsedPoints] = pointi;

                nUsedPoints++;
            }
        }

        // Reset the size of used points
        procPointLabels.setSize(nUsedPoints);
    }
}

// ************************************************************************* //
