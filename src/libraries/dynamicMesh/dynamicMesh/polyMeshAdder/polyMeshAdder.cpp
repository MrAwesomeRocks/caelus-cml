/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2012 OpenFOAM Foundation
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

\*----------------------------------------------------------------------------*/

#include "polyMeshAdder.hpp"
#include "mapAddedPolyMesh.hpp"
#include "IOobject.hpp"
#include "faceCoupleInfo.hpp"
#include "processorPolyPatch.hpp"
#include "SortableList.hpp"
#include "Time.hpp"
#include "globalMeshData.hpp"
#include "mergePoints.hpp"
#include "polyModifyFace.hpp"
#include "polyRemovePoint.hpp"
#include "polyTopoChange.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// Append all mapped elements of a list to a DynamicList
void CML::polyMeshAdder::append
(
    const labelList& map,
    const labelList& lst,
    DynamicList<label>& dynLst
)
{
    dynLst.setCapacity(dynLst.size() + lst.size());

    forAll(lst, i)
    {
        const label newElem = map[lst[i]];

        if (newElem != -1)
        {
            dynLst.append(newElem);
        }
    }
}


// Append all mapped elements of a list to a DynamicList
void CML::polyMeshAdder::append
(
    const labelList& map,
    const labelList& lst,
    const SortableList<label>& sortedLst,
    DynamicList<label>& dynLst
)
{
    dynLst.setCapacity(dynLst.size() + lst.size());

    forAll(lst, i)
    {
        const label newElem = map[lst[i]];

        if (newElem != -1 && findSortedIndex(sortedLst, newElem) == -1)
        {
            dynLst.append(newElem);
        }
    }
}


// Get index of patch in new set of patchnames/types
CML::label CML::polyMeshAdder::patchIndex
(
    const polyPatch& p,
    DynamicList<word>& allPatchNames,
    DynamicList<word>& allPatchTypes
)
{
    // Find the patch name on the list.  If the patch is already there
    // and patch types match, return index
    const word& pType = p.type();
    const word& pName = p.name();

    label patchI = findIndex(allPatchNames, pName);

    if (patchI == -1)
    {
        // Patch not found. Append to the list
        allPatchNames.append(pName);
        allPatchTypes.append(pType);

        return allPatchNames.size() - 1;
    }
    else if (allPatchTypes[patchI] == pType)
    {
        // Found name and types match
        return patchI;
    }
    else
    {
        // Found the name, but type is different

        // Duplicate name is not allowed.  Create a composite name from the
        // patch name and case name
        const word& caseName = p.boundaryMesh().mesh().time().caseName();

        allPatchNames.append(pName + "_" + caseName);
        allPatchTypes.append(pType);

        Pout<< "label patchIndex(const polyPatch& p) : "
            << "Patch " << p.index() << " named "
            << pName << " in mesh " << caseName
            << " already exists, but patch types"
            << " do not match.\nCreating a composite name as "
            << allPatchNames.last() << endl;

        return allPatchNames.size() - 1;
    }
}


// Get index of zone in new set of zone names
CML::label CML::polyMeshAdder::zoneIndex
(
    const word& curName,
    DynamicList<word>& names
)
{
    label zoneI = findIndex(names, curName);

    if (zoneI != -1)
    {
        return zoneI;
    }
    else
    {
        // Not found.  Add new name to the list
        names.append(curName);

        return names.size() - 1;
    }
}


void CML::polyMeshAdder::mergePatchNames
(
    const polyBoundaryMesh& patches0,
    const polyBoundaryMesh& patches1,

    DynamicList<word>& allPatchNames,
    DynamicList<word>& allPatchTypes,

    labelList& from1ToAllPatches,
    labelList& fromAllTo1Patches
)
{
    // Insert the mesh0 patches and zones
    allPatchNames.append(patches0.names());
    allPatchTypes.append(patches0.types());


    // Patches
    // ~~~~~~~
    // Patches from 0 are taken over as is; those from 1 get either merged
    // (if they share name and type) or appended.
    // Empty patches are filtered out much much later on.

    // Add mesh1 patches and build map both ways.
    from1ToAllPatches.setSize(patches1.size());

    forAll(patches1, patchI)
    {
        from1ToAllPatches[patchI] = patchIndex
        (
            patches1[patchI],
            allPatchNames,
            allPatchTypes
        );
    }
    allPatchTypes.shrink();
    allPatchNames.shrink();

    // Invert 1 to all patch map
    fromAllTo1Patches.setSize(allPatchNames.size());
    fromAllTo1Patches = -1;

    forAll(from1ToAllPatches, i)
    {
        fromAllTo1Patches[from1ToAllPatches[i]] = i;
    }
}


CML::labelList CML::polyMeshAdder::getPatchStarts
(
    const polyBoundaryMesh& patches
)
{
    labelList patchStarts(patches.size());
    forAll(patches, patchI)
    {
        patchStarts[patchI] = patches[patchI].start();
    }
    return patchStarts;
}


CML::labelList CML::polyMeshAdder::getPatchSizes
(
    const polyBoundaryMesh& patches
)
{
    labelList patchSizes(patches.size());
    forAll(patches, patchI)
    {
        patchSizes[patchI] = patches[patchI].size();
    }
    return patchSizes;
}


CML::List<CML::polyPatch*> CML::polyMeshAdder::combinePatches
(
    const polyMesh& mesh0,
    const polyMesh& mesh1,
    const polyBoundaryMesh& allBoundaryMesh,
    const label nAllPatches,
    const labelList& fromAllTo1Patches,

    const label nInternalFaces,
    const labelList& nFaces,

    labelList& from0ToAllPatches,
    labelList& from1ToAllPatches
)
{
    const polyBoundaryMesh& patches0 = mesh0.boundaryMesh();
    const polyBoundaryMesh& patches1 = mesh1.boundaryMesh();


    // Compacted new patch list.
    DynamicList<polyPatch*> allPatches(nAllPatches);


    // Map from 0 to all patches (since gets compacted)
    from0ToAllPatches.setSize(patches0.size());
    from0ToAllPatches = -1;

    label startFaceI = nInternalFaces;

    // Copy patches0 with new sizes. First patches always come from
    // mesh0 and will always be present.
    forAll(patches0, patchI)
    {
        // Originates from mesh0. Clone with new size & filter out empty
        // patch.
        label filteredPatchI;

        if (nFaces[patchI] == 0 && isA<processorPolyPatch>(patches0[patchI]))
        {
            //Pout<< "Removing zero sized mesh0 patch "
            //    << patches0[patchI].name() << endl;
            filteredPatchI = -1;
        }
        else
        {
            filteredPatchI = allPatches.size();

            allPatches.append
            (
                patches0[patchI].clone
                (
                    allBoundaryMesh,
                    filteredPatchI,
                    nFaces[patchI],
                    startFaceI
                ).ptr()
            );
            startFaceI += nFaces[patchI];
        }

        // Record new index in allPatches
        from0ToAllPatches[patchI] = filteredPatchI;

        // Check if patch was also in mesh1 and update its addressing if so.
        if (fromAllTo1Patches[patchI] != -1)
        {
            from1ToAllPatches[fromAllTo1Patches[patchI]] = filteredPatchI;
        }
    }

    // Copy unique patches of mesh1.
    forAll(from1ToAllPatches, patchI)
    {
        label allPatchI = from1ToAllPatches[patchI];

        if (allPatchI >= patches0.size())
        {
            // Patch has not been merged with any mesh0 patch.

            label filteredPatchI;

            if
            (
                nFaces[allPatchI] == 0
             && isA<processorPolyPatch>(patches1[patchI])
            )
            {
                //Pout<< "Removing zero sized mesh1 patch "
                //    << patches1[patchI].name() << endl;
                filteredPatchI = -1;
            }
            else
            {
                filteredPatchI = allPatches.size();

                allPatches.append
                (
                    patches1[patchI].clone
                    (
                        allBoundaryMesh,
                        filteredPatchI,
                        nFaces[allPatchI],
                        startFaceI
                    ).ptr()
                );
                startFaceI += nFaces[allPatchI];
            }

            from1ToAllPatches[patchI] = filteredPatchI;
        }
    }

    allPatches.shrink();

    return allPatches;
}


CML::labelList CML::polyMeshAdder::getFaceOrder
(
    const cellList& cells,
    const label nInternalFaces,
    const labelList& owner,
    const labelList& neighbour
)
{
    labelList oldToNew(owner.size(), -1);

    // Leave boundary faces in order
    for (label faceI = nInternalFaces; faceI < owner.size(); ++faceI)
    {
        oldToNew[faceI] = faceI;
    }

    // First unassigned face
    label newFaceI = 0;

    forAll(cells, cellI)
    {
        const labelList& cFaces = cells[cellI];

        SortableList<label> nbr(cFaces.size());

        forAll(cFaces, i)
        {
            label faceI = cFaces[i];

            label nbrCellI = neighbour[faceI];

            if (nbrCellI != -1)
            {
                // Internal face. Get cell on other side.
                if (nbrCellI == cellI)
                {
                    nbrCellI = owner[faceI];
                }

                if (cellI < nbrCellI)
                {
                    // CellI is master
                    nbr[i] = nbrCellI;
                }
                else
                {
                    // nbrCell is master. Let it handle this face.
                    nbr[i] = -1;
                }
            }
            else
            {
                // External face. Do later.
                nbr[i] = -1;
            }
        }

        nbr.sort();

        forAll(nbr, i)
        {
            if (nbr[i] != -1)
            {
                oldToNew[cFaces[nbr.indices()[i]]] = newFaceI++;
            }
        }
    }


    // Check done all faces.
    forAll(oldToNew, faceI)
    {
        if (oldToNew[faceI] == -1)
        {
            FatalErrorInFunction
                << "Did not determine new position"
                << " for face " << faceI
                << abort(FatalError);
        }
    }

    return oldToNew;
}


// Extends face f with split points. cutEdgeToPoints gives for every
// edge the points introduced inbetween the endpoints.
void CML::polyMeshAdder::insertVertices
(
    const edgeLookup& cutEdgeToPoints,
    const Map<label>& meshToMaster,
    const labelList& masterToCutPoints,
    const face& masterF,

    DynamicList<label>& workFace,
    face& allF
)
{
    workFace.clear();

    // Check any edge for being cut (check on the cut so takes account
    // for any point merging on the cut)

    forAll(masterF, fp)
    {
        label v0 = masterF[fp];
        label v1 = masterF.nextLabel(fp);

        // Copy existing face point
        workFace.append(allF[fp]);

        // See if any edge between v0,v1

        Map<label>::const_iterator v0Fnd = meshToMaster.find(v0);
        if (v0Fnd != meshToMaster.end())
        {
            Map<label>::const_iterator v1Fnd = meshToMaster.find(v1);
            if (v1Fnd != meshToMaster.end())
            {
                // Get edge in cutPoint numbering
                edge cutEdge
                (
                    masterToCutPoints[v0Fnd()],
                    masterToCutPoints[v1Fnd()]
                );

                edgeLookup::const_iterator iter = cutEdgeToPoints.find(cutEdge);

                if (iter != cutEdgeToPoints.end())
                {
                    const edge& e = iter.key();
                    const labelList& addedPoints = iter();

                    // cutPoints first in allPoints so no need for renumbering
                    if (e[0] == cutEdge[0])
                    {
                        forAll(addedPoints, i)
                        {
                            workFace.append(addedPoints[i]);
                        }
                    }
                    else
                    {
                        forAllReverse(addedPoints, i)
                        {
                            workFace.append(addedPoints[i]);
                        }
                    }
                }
            }
        }
    }

    if (workFace.size() != allF.size())
    {
        allF.transfer(workFace);
    }
}


// Adds primitives (cells, faces, points)
// Cells:
//  - all of mesh0
//  - all of mesh1
// Faces:
//  - all uncoupled of mesh0
//  - all coupled faces
//  - all uncoupled of mesh1
// Points:
//  - all coupled
//  - all uncoupled of mesh0
//  - all uncoupled of mesh1
void CML::polyMeshAdder::mergePrimitives
(
    const polyMesh& mesh0,
    const polyMesh& mesh1,
    const faceCoupleInfo& coupleInfo,

    const label nAllPatches,                // number of patches in the new mesh
    const labelList& fromAllTo1Patches,
    const labelList& from1ToAllPatches,

    pointField& allPoints,
    labelList& from0ToAllPoints,
    labelList& from1ToAllPoints,

    faceList& allFaces,
    labelList& allOwner,
    labelList& allNeighbour,
    label& nInternalFaces,
    labelList& nFacesPerPatch,
    label& nCells,

    labelList& from0ToAllFaces,
    labelList& from1ToAllFaces,
    labelList& from1ToAllCells
)
{
    const polyBoundaryMesh& patches0 = mesh0.boundaryMesh();
    const polyBoundaryMesh& patches1 = mesh1.boundaryMesh();

    const primitiveFacePatch& cutFaces = coupleInfo.cutFaces();
    const indirectPrimitivePatch& masterPatch = coupleInfo.masterPatch();
    const indirectPrimitivePatch& slavePatch = coupleInfo.slavePatch();


    // Points
    // ~~~~~~

    // Storage for new points
    allPoints.setSize(mesh0.nPoints() + mesh1.nPoints());
    label allPointI = 0;

    from0ToAllPoints.setSize(mesh0.nPoints());
    from0ToAllPoints = -1;
    from1ToAllPoints.setSize(mesh1.nPoints());
    from1ToAllPoints = -1;

    // Copy coupled points (on cut)
    {
        const pointField& cutPoints = coupleInfo.cutPoints();

        //const labelListList& cutToMasterPoints =
        //   coupleInfo.cutToMasterPoints();
        labelListList cutToMasterPoints
        (
            invertOneToMany
            (
                cutPoints.size(),
                coupleInfo.masterToCutPoints()
            )
        );

        //const labelListList& cutToSlavePoints =
        //    coupleInfo.cutToSlavePoints();
        labelListList cutToSlavePoints
        (
            invertOneToMany
            (
                cutPoints.size(),
                coupleInfo.slaveToCutPoints()
            )
        );

        forAll(cutPoints, i)
        {
            allPoints[allPointI] = cutPoints[i];

            // Mark all master and slave points referring to this point.

            const labelList& masterPoints = cutToMasterPoints[i];

            forAll(masterPoints, j)
            {
                label mesh0PointI = masterPatch.meshPoints()[masterPoints[j]];
                from0ToAllPoints[mesh0PointI] = allPointI;
            }

            const labelList& slavePoints = cutToSlavePoints[i];

            forAll(slavePoints, j)
            {
                label mesh1PointI = slavePatch.meshPoints()[slavePoints[j]];
                from1ToAllPoints[mesh1PointI] = allPointI;
            }
            allPointI++;
        }
    }

    // Add uncoupled mesh0 points
    forAll(mesh0.points(), pointI)
    {
        if (from0ToAllPoints[pointI] == -1)
        {
            allPoints[allPointI] = mesh0.points()[pointI];
            from0ToAllPoints[pointI] = allPointI;
            allPointI++;
        }
    }

    // Add uncoupled mesh1 points
    forAll(mesh1.points(), pointI)
    {
        if (from1ToAllPoints[pointI] == -1)
        {
            allPoints[allPointI] = mesh1.points()[pointI];
            from1ToAllPoints[pointI] = allPointI;
            allPointI++;
        }
    }

    allPoints.setSize(allPointI);


    // Faces
    // ~~~~~

    // Sizes per patch
    nFacesPerPatch.setSize(nAllPatches);
    nFacesPerPatch = 0;

    // Storage for faces and owner/neighbour
    allFaces.setSize(mesh0.nFaces() + mesh1.nFaces());
    allOwner.setSize(allFaces.size());
    allOwner = -1;
    allNeighbour.setSize(allFaces.size());
    allNeighbour = -1;
    label allFaceI = 0;

    from0ToAllFaces.setSize(mesh0.nFaces());
    from0ToAllFaces = -1;
    from1ToAllFaces.setSize(mesh1.nFaces());
    from1ToAllFaces = -1;

    // Copy mesh0 internal faces (always uncoupled)
    for (label faceI = 0; faceI < mesh0.nInternalFaces(); faceI++)
    {
        allFaces[allFaceI] = renumber(from0ToAllPoints, mesh0.faces()[faceI]);
        allOwner[allFaceI] = mesh0.faceOwner()[faceI];
        allNeighbour[allFaceI] = mesh0.faceNeighbour()[faceI];
        from0ToAllFaces[faceI] = allFaceI++;
    }

    // Copy coupled faces. Every coupled face has an equivalent master and
    // slave. Also uncount as boundary faces all the newly coupled faces.
    const labelList& cutToMasterFaces = coupleInfo.cutToMasterFaces();
    const labelList& cutToSlaveFaces = coupleInfo.cutToSlaveFaces();

    forAll(cutFaces, i)
    {
        label masterFaceI = cutToMasterFaces[i];

        label mesh0FaceI = masterPatch.addressing()[masterFaceI];

        if (from0ToAllFaces[mesh0FaceI] == -1)
        {
            // First occurrence of face
            from0ToAllFaces[mesh0FaceI] = allFaceI;

            // External face becomes internal so uncount
            label patch0 = patches0.whichPatch(mesh0FaceI);
            nFacesPerPatch[patch0]--;
        }

        label slaveFaceI = cutToSlaveFaces[i];

        label mesh1FaceI = slavePatch.addressing()[slaveFaceI];

        if (from1ToAllFaces[mesh1FaceI] == -1)
        {
            from1ToAllFaces[mesh1FaceI] = allFaceI;

            label patch1 = patches1.whichPatch(mesh1FaceI);
            nFacesPerPatch[from1ToAllPatches[patch1]]--;
        }

        // Copy cut face (since cutPoints are copied first no renumbering
        // necessary)
        allFaces[allFaceI] = cutFaces[i];
        allOwner[allFaceI] = mesh0.faceOwner()[mesh0FaceI];
        allNeighbour[allFaceI] = mesh1.faceOwner()[mesh1FaceI] + mesh0.nCells();

        allFaceI++;
    }

    // Copy mesh1 internal faces (always uncoupled)
    for (label faceI = 0; faceI < mesh1.nInternalFaces(); faceI++)
    {
        allFaces[allFaceI] = renumber(from1ToAllPoints, mesh1.faces()[faceI]);
        allOwner[allFaceI] = mesh1.faceOwner()[faceI] + mesh0.nCells();
        allNeighbour[allFaceI] = mesh1.faceNeighbour()[faceI] + mesh0.nCells();
        from1ToAllFaces[faceI] = allFaceI++;
    }

    nInternalFaces = allFaceI;

    // Copy (unmarked/uncoupled) external faces in new order.
    for (label allPatchI = 0; allPatchI < nAllPatches; allPatchI++)
    {
        if (allPatchI < patches0.size())
        {
            // Patch is present in mesh0
            const polyPatch& pp = patches0[allPatchI];

            nFacesPerPatch[allPatchI] += pp.size();

            label faceI = pp.start();

            forAll(pp, i)
            {
                if (from0ToAllFaces[faceI] == -1)
                {
                    // Is uncoupled face since has not yet been dealt with
                    allFaces[allFaceI] = renumber
                    (
                        from0ToAllPoints,
                        mesh0.faces()[faceI]
                    );
                    allOwner[allFaceI] = mesh0.faceOwner()[faceI];
                    allNeighbour[allFaceI] = -1;

                    from0ToAllFaces[faceI] = allFaceI++;
                }
                faceI++;
            }
        }
        if (fromAllTo1Patches[allPatchI] != -1)
        {
            // Patch is present in mesh1
            const polyPatch& pp = patches1[fromAllTo1Patches[allPatchI]];

            nFacesPerPatch[allPatchI] += pp.size();

            label faceI = pp.start();

            forAll(pp, i)
            {
                if (from1ToAllFaces[faceI] == -1)
                {
                    // Is uncoupled face
                    allFaces[allFaceI] = renumber
                    (
                        from1ToAllPoints,
                        mesh1.faces()[faceI]
                    );
                    allOwner[allFaceI] =
                        mesh1.faceOwner()[faceI]
                      + mesh0.nCells();
                    allNeighbour[allFaceI] = -1;

                    from1ToAllFaces[faceI] = allFaceI++;
                }
                faceI++;
            }
        }
    }
    allFaces.setSize(allFaceI);
    allOwner.setSize(allFaceI);
    allNeighbour.setSize(allFaceI);


    // So now we have all ok for one-to-one mapping.
    // For split slace faces:
    // - mesh consistent with slave side
    // - mesh not consistent with owner side. It is not zipped up, the
    //   original faces need edges split.

    // Use brute force to prevent having to calculate addressing:
    // - get map from master edge to split edges.
    // - check all faces to find any edge that is split.
    {
        // From two cut-points to labels of cut-points inbetween.
        // (in order: from e[0] to e[1]
        const edgeLookup& cutEdgeToPoints = coupleInfo.cutEdgeToPoints();

        // Get map of master face (in mesh labels) that are in cut. These faces
        // do not need to be renumbered.
        labelHashSet masterCutFaces(cutToMasterFaces.size());
        forAll(cutToMasterFaces, i)
        {
            label meshFaceI = masterPatch.addressing()[cutToMasterFaces[i]];

            masterCutFaces.insert(meshFaceI);
        }

        DynamicList<label> workFace(100);

        forAll(from0ToAllFaces, face0)
        {
            if (!masterCutFaces.found(face0))
            {
                label allFaceI = from0ToAllFaces[face0];

                insertVertices
                (
                    cutEdgeToPoints,
                    masterPatch.meshPointMap(),
                    coupleInfo.masterToCutPoints(),
                    mesh0.faces()[face0],

                    workFace,
                    allFaces[allFaceI]
                );
            }
        }

        // Same for slave face

        labelHashSet slaveCutFaces(cutToSlaveFaces.size());
        forAll(cutToSlaveFaces, i)
        {
            label meshFaceI = slavePatch.addressing()[cutToSlaveFaces[i]];

            slaveCutFaces.insert(meshFaceI);
        }

        forAll(from1ToAllFaces, face1)
        {
            if (!slaveCutFaces.found(face1))
            {
                label allFaceI = from1ToAllFaces[face1];

                insertVertices
                (
                    cutEdgeToPoints,
                    slavePatch.meshPointMap(),
                    coupleInfo.slaveToCutPoints(),
                    mesh1.faces()[face1],

                    workFace,
                    allFaces[allFaceI]
                );
            }
        }
    }

    // Now we have a full facelist and owner/neighbour addressing.


    // Cells
    // ~~~~~

    from1ToAllCells.setSize(mesh1.nCells());
    from1ToAllCells = -1;

    forAll(mesh1.cells(), i)
    {
        from1ToAllCells[i] = i + mesh0.nCells();
    }

    // Make cells (= cell-face addressing)
    nCells = mesh0.nCells() + mesh1.nCells();
    cellList allCells(nCells);
    primitiveMesh::calcCells(allCells, allOwner, allNeighbour, nCells);

    // Reorder faces for upper-triangular order.
    labelList oldToNew
    (
        getFaceOrder
        (
            allCells,
            nInternalFaces,
            allOwner,
            allNeighbour
        )
    );

    inplaceReorder(oldToNew, allFaces);
    inplaceReorder(oldToNew, allOwner);
    inplaceReorder(oldToNew, allNeighbour);
    inplaceRenumber(oldToNew, from0ToAllFaces);
    inplaceRenumber(oldToNew, from1ToAllFaces);
}


void CML::polyMeshAdder::mergePointZones
(
    const pointZoneMesh& pz0,
    const pointZoneMesh& pz1,
    const labelList& from0ToAllPoints,
    const labelList& from1ToAllPoints,

    DynamicList<word>& zoneNames,
    labelList& from1ToAll,
    List<DynamicList<label> >& pzPoints
)
{
    zoneNames.setCapacity(pz0.size() + pz1.size());
    zoneNames.append(pz0.names());

    from1ToAll.setSize(pz1.size());

    forAll(pz1, zoneI)
    {
        from1ToAll[zoneI] = zoneIndex(pz1[zoneI].name(), zoneNames);
    }
    zoneNames.shrink();

    // Point labels per merged zone
    pzPoints.setSize(zoneNames.size());

    forAll(pz0, zoneI)
    {
        append(from0ToAllPoints, pz0[zoneI], pzPoints[zoneI]);
    }

    // Get sorted zone contents for duplicate element recognition
    PtrList<SortableList<label> > pzPointsSorted(pzPoints.size());
    forAll(pzPoints, zoneI)
    {
        pzPointsSorted.set
        (
            zoneI,
            new SortableList<label>(pzPoints[zoneI])
        );
    }

    // Now we have full addressing for points so do the pointZones of mesh1.
    forAll(pz1, zoneI)
    {
        // Relabel all points of zone and add to correct pzPoints.
        const label allZoneI = from1ToAll[zoneI];

        append
        (
            from1ToAllPoints,
            pz1[zoneI],
            pzPointsSorted[allZoneI],
            pzPoints[allZoneI]
        );
    }

    forAll(pzPoints, i)
    {
        pzPoints[i].shrink();
    }
}


void CML::polyMeshAdder::mergeFaceZones
(
    const faceZoneMesh& fz0,
    const faceZoneMesh& fz1,
    const labelList& from0ToAllFaces,
    const labelList& from1ToAllFaces,

    DynamicList<word>& zoneNames,
    labelList& from1ToAll,
    List<DynamicList<label> >& fzFaces,
    List<DynamicList<bool> >& fzFlips
)
{
    zoneNames.setCapacity(fz0.size() + fz1.size());
    zoneNames.append(fz0.names());

    from1ToAll.setSize(fz1.size());

    forAll(fz1, zoneI)
    {
        from1ToAll[zoneI] = zoneIndex(fz1[zoneI].name(), zoneNames);
    }
    zoneNames.shrink();


    // Create temporary lists for faceZones.
    fzFaces.setSize(zoneNames.size());
    fzFlips.setSize(zoneNames.size());
    forAll(fz0, zoneI)
    {
        DynamicList<label>& newZone = fzFaces[zoneI];
        DynamicList<bool>& newFlip = fzFlips[zoneI];

        newZone.setCapacity(fz0[zoneI].size());
        newFlip.setCapacity(newZone.size());

        const labelList& addressing = fz0[zoneI];
        const boolList& flipMap = fz0[zoneI].flipMap();

        forAll(addressing, i)
        {
            label faceI = addressing[i];

            if (from0ToAllFaces[faceI] != -1)
            {
                newZone.append(from0ToAllFaces[faceI]);
                newFlip.append(flipMap[i]);
            }
        }
    }

    // Get sorted zone contents for duplicate element recognition
    PtrList<SortableList<label> > fzFacesSorted(fzFaces.size());
    forAll(fzFaces, zoneI)
    {
        fzFacesSorted.set
        (
            zoneI,
            new SortableList<label>(fzFaces[zoneI])
        );
    }

    // Now we have full addressing for faces so do the faceZones of mesh1.
    forAll(fz1, zoneI)
    {
        label allZoneI = from1ToAll[zoneI];

        DynamicList<label>& newZone = fzFaces[allZoneI];
        const SortableList<label>& newZoneSorted = fzFacesSorted[allZoneI];
        DynamicList<bool>& newFlip = fzFlips[allZoneI];

        newZone.setCapacity(newZone.size() + fz1[zoneI].size());
        newFlip.setCapacity(newZone.size());

        const labelList& addressing = fz1[zoneI];
        const boolList& flipMap = fz1[zoneI].flipMap();

        forAll(addressing, i)
        {
            label faceI = addressing[i];
            label allFaceI = from1ToAllFaces[faceI];

            if
            (
                allFaceI != -1
             && findSortedIndex(newZoneSorted, allFaceI) == -1
            )
            {
                newZone.append(allFaceI);
                newFlip.append(flipMap[i]);
            }
        }
    }

    forAll(fzFaces, i)
    {
        fzFaces[i].shrink();
        fzFlips[i].shrink();
    }
}


void CML::polyMeshAdder::mergeCellZones
(
    const cellZoneMesh& cz0,
    const cellZoneMesh& cz1,
    const labelList& from1ToAllCells,

    DynamicList<word>& zoneNames,
    labelList& from1ToAll,
    List<DynamicList<label> >& czCells
)
{
    zoneNames.setCapacity(cz0.size() + cz1.size());
    zoneNames.append(cz0.names());

    from1ToAll.setSize(cz1.size());
    forAll(cz1, zoneI)
    {
        from1ToAll[zoneI] = zoneIndex(cz1[zoneI].name(), zoneNames);
    }
    zoneNames.shrink();


    // Create temporary lists for cellZones.
    czCells.setSize(zoneNames.size());
    forAll(cz0, zoneI)
    {
        // Insert mesh0 cells
        czCells[zoneI].append(cz0[zoneI]);
    }


    // Cell mapping is trivial.
    forAll(cz1, zoneI)
    {
        const label allZoneI = from1ToAll[zoneI];

        append(from1ToAllCells, cz1[zoneI], czCells[allZoneI]);
    }

    forAll(czCells, i)
    {
        czCells[i].shrink();
    }
}


void CML::polyMeshAdder::mergeZones
(
    const polyMesh& mesh0,
    const polyMesh& mesh1,
    const labelList& from0ToAllPoints,
    const labelList& from0ToAllFaces,

    const labelList& from1ToAllPoints,
    const labelList& from1ToAllFaces,
    const labelList& from1ToAllCells,

    DynamicList<word>& pointZoneNames,
    List<DynamicList<label> >& pzPoints,

    DynamicList<word>& faceZoneNames,
    List<DynamicList<label> >& fzFaces,
    List<DynamicList<bool> >& fzFlips,

    DynamicList<word>& cellZoneNames,
    List<DynamicList<label> >& czCells
)
{
    labelList from1ToAllPZones;
    mergePointZones
    (
        mesh0.pointZones(),
        mesh1.pointZones(),
        from0ToAllPoints,
        from1ToAllPoints,

        pointZoneNames,
        from1ToAllPZones,
        pzPoints
    );

    labelList from1ToAllFZones;
    mergeFaceZones
    (
        mesh0.faceZones(),
        mesh1.faceZones(),
        from0ToAllFaces,
        from1ToAllFaces,

        faceZoneNames,
        from1ToAllFZones,
        fzFaces,
        fzFlips
    );

    labelList from1ToAllCZones;
    mergeCellZones
    (
        mesh0.cellZones(),
        mesh1.cellZones(),
        from1ToAllCells,

        cellZoneNames,
        from1ToAllCZones,
        czCells
    );
}


void CML::polyMeshAdder::addZones
(
    const DynamicList<word>& pointZoneNames,
    const List<DynamicList<label> >& pzPoints,

    const DynamicList<word>& faceZoneNames,
    const List<DynamicList<label> >& fzFaces,
    const List<DynamicList<bool> >& fzFlips,

    const DynamicList<word>& cellZoneNames,
    const List<DynamicList<label> >& czCells,

    polyMesh& mesh
)
{
    List<pointZone*> pZones(pzPoints.size());
    forAll(pZones, i)
    {
        pZones[i] = new pointZone
        (
            pointZoneNames[i],
            pzPoints[i],
            i,
            mesh.pointZones()
        );
    }

    List<faceZone*> fZones(fzFaces.size());
    forAll(fZones, i)
    {
        fZones[i] = new faceZone
        (
            faceZoneNames[i],
            fzFaces[i],
            fzFlips[i],
            i,
            mesh.faceZones()
        );
    }

    List<cellZone*> cZones(czCells.size());
    forAll(cZones, i)
    {
        cZones[i] = new cellZone
        (
            cellZoneNames[i],
            czCells[i],
            i,
            mesh.cellZones()
        );
    }

    mesh.addZones
    (
        pZones,
        fZones,
        cZones
    );
}



// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// Returns new mesh and sets
// - map from new cell/face/point/patch to either mesh0 or mesh1
//
// mesh0Faces/mesh1Faces: corresponding faces on both meshes.
CML::autoPtr<CML::polyMesh> CML::polyMeshAdder::add
(
    const IOobject& io,
    const polyMesh& mesh0,
    const polyMesh& mesh1,
    const faceCoupleInfo& coupleInfo,
    autoPtr<mapAddedPolyMesh>& mapPtr
)
{
    const polyBoundaryMesh& patches0 = mesh0.boundaryMesh();
    const polyBoundaryMesh& patches1 = mesh1.boundaryMesh();


    DynamicList<word> allPatchNames(patches0.size() + patches1.size());
    DynamicList<word> allPatchTypes(allPatchNames.size());

    // Patch maps
    labelList from1ToAllPatches(patches1.size());
    labelList fromAllTo1Patches(allPatchNames.size(), -1);

    mergePatchNames
    (
        patches0,
        patches1,
        allPatchNames,
        allPatchTypes,
        from1ToAllPatches,
        fromAllTo1Patches
    );


    // New points
    pointField allPoints;

    // Map from mesh0/1 points to allPoints.
    labelList from0ToAllPoints(mesh0.nPoints(), -1);
    labelList from1ToAllPoints(mesh1.nPoints(), -1);

    // New faces
    faceList allFaces;
    label nInternalFaces;

    // New cells
    labelList allOwner;
    labelList allNeighbour;
    label nCells;

    // Sizes per patch
    labelList nFaces(allPatchNames.size(), 0);

    // Maps
    labelList from0ToAllFaces(mesh0.nFaces(), -1);
    labelList from1ToAllFaces(mesh1.nFaces(), -1);

    // Map
    labelList from1ToAllCells(mesh1.nCells(), -1);

    mergePrimitives
    (
        mesh0,
        mesh1,
        coupleInfo,

        allPatchNames.size(),
        fromAllTo1Patches,
        from1ToAllPatches,

        allPoints,
        from0ToAllPoints,
        from1ToAllPoints,

        allFaces,
        allOwner,
        allNeighbour,
        nInternalFaces,
        nFaces,
        nCells,

        from0ToAllFaces,
        from1ToAllFaces,
        from1ToAllCells
    );


    // Zones
    // ~~~~~

    DynamicList<word> pointZoneNames;
    List<DynamicList<label> > pzPoints;

    DynamicList<word> faceZoneNames;
    List<DynamicList<label> > fzFaces;
    List<DynamicList<bool> > fzFlips;

    DynamicList<word> cellZoneNames;
    List<DynamicList<label> > czCells;

    mergeZones
    (
        mesh0,
        mesh1,

        from0ToAllPoints,
        from0ToAllFaces,

        from1ToAllPoints,
        from1ToAllFaces,
        from1ToAllCells,

        pointZoneNames,
        pzPoints,

        faceZoneNames,
        fzFaces,
        fzFlips,

        cellZoneNames,
        czCells
    );


    // Patches
    // ~~~~~~~

    // Map from 0 to all patches (since gets compacted)
    labelList from0ToAllPatches(patches0.size(), -1);

    List<polyPatch*> allPatches
    (
        combinePatches
        (
            mesh0,
            mesh1,
            patches0,           // Should be boundaryMesh() on new mesh.
            allPatchNames.size(),
            fromAllTo1Patches,
            mesh0.nInternalFaces()
          + mesh1.nInternalFaces()
          + coupleInfo.cutFaces().size(),
            nFaces,

            from0ToAllPatches,
            from1ToAllPatches
        )
    );


    // Map information
    // ~~~~~~~~~~~~~~~

    mapPtr.reset
    (
        new mapAddedPolyMesh
        (
            mesh0.nPoints(),
            mesh0.nFaces(),
            mesh0.nCells(),

            mesh1.nPoints(),
            mesh1.nFaces(),
            mesh1.nCells(),

            from0ToAllPoints,
            from0ToAllFaces,
            identity(mesh0.nCells()),

            from1ToAllPoints,
            from1ToAllFaces,
            from1ToAllCells,

            from0ToAllPatches,
            from1ToAllPatches,
            getPatchSizes(patches0),
            getPatchStarts(patches0)
        )
    );



    // Now we have extracted all information from all meshes.
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // Construct mesh
    autoPtr<polyMesh> tmesh
    (
        new polyMesh
        (
            io,
            xferMove(allPoints),
            xferMove(allFaces),
            xferMove(allOwner),
            xferMove(allNeighbour)
        )
    );
    polyMesh& mesh = tmesh();

    // Add zones to new mesh.
    addZones
    (
        pointZoneNames,
        pzPoints,

        faceZoneNames,
        fzFaces,
        fzFlips,

        cellZoneNames,
        czCells,
        mesh
    );

    // Add patches to new mesh
    mesh.addPatches(allPatches);

    return tmesh;
}


// Inplace add mesh1 to mesh0
CML::autoPtr<CML::mapAddedPolyMesh> CML::polyMeshAdder::add
(
    polyMesh& mesh0,
    const polyMesh& mesh1,
    const faceCoupleInfo& coupleInfo,
    const bool validBoundary
)
{
    const polyBoundaryMesh& patches0 = mesh0.boundaryMesh();
    const polyBoundaryMesh& patches1 = mesh1.boundaryMesh();

    DynamicList<word> allPatchNames(patches0.size() + patches1.size());
    DynamicList<word> allPatchTypes(allPatchNames.size());

    // Patch maps
    labelList from1ToAllPatches(patches1.size());
    labelList fromAllTo1Patches(allPatchNames.size(), -1);

    mergePatchNames
    (
        patches0,
        patches1,
        allPatchNames,
        allPatchTypes,
        from1ToAllPatches,
        fromAllTo1Patches
    );


    // New points
    pointField allPoints;

    // Map from mesh0/1 points to allPoints.
    labelList from0ToAllPoints(mesh0.nPoints(), -1);
    labelList from1ToAllPoints(mesh1.nPoints(), -1);

    // New faces
    faceList allFaces;
    labelList allOwner;
    labelList allNeighbour;
    label nInternalFaces;
    // Sizes per patch
    labelList nFaces(allPatchNames.size(), 0);
    label nCells;

    // Maps
    labelList from0ToAllFaces(mesh0.nFaces(), -1);
    labelList from1ToAllFaces(mesh1.nFaces(), -1);
    // Map
    labelList from1ToAllCells(mesh1.nCells(), -1);

    mergePrimitives
    (
        mesh0,
        mesh1,
        coupleInfo,

        allPatchNames.size(),
        fromAllTo1Patches,
        from1ToAllPatches,

        allPoints,
        from0ToAllPoints,
        from1ToAllPoints,

        allFaces,
        allOwner,
        allNeighbour,
        nInternalFaces,
        nFaces,
        nCells,

        from0ToAllFaces,
        from1ToAllFaces,
        from1ToAllCells
    );


    // Zones
    // ~~~~~

    DynamicList<word> pointZoneNames;
    List<DynamicList<label> > pzPoints;

    DynamicList<word> faceZoneNames;
    List<DynamicList<label> > fzFaces;
    List<DynamicList<bool> > fzFlips;

    DynamicList<word> cellZoneNames;
    List<DynamicList<label> > czCells;

    mergeZones
    (
        mesh0,
        mesh1,

        from0ToAllPoints,
        from0ToAllFaces,

        from1ToAllPoints,
        from1ToAllFaces,
        from1ToAllCells,

        pointZoneNames,
        pzPoints,

        faceZoneNames,
        fzFaces,
        fzFlips,

        cellZoneNames,
        czCells
    );


    // Patches
    // ~~~~~~~


    // Store mesh0 patch info before modifying patches0.
    labelList mesh0PatchSizes(getPatchSizes(patches0));
    labelList mesh0PatchStarts(getPatchStarts(patches0));

    // Map from 0 to all patches (since gets compacted)
    labelList from0ToAllPatches(patches0.size(), -1);

    // Inplace extend mesh0 patches (note that patches0.size() now also
    // has changed)
    polyBoundaryMesh& allPatches =
        const_cast<polyBoundaryMesh&>(mesh0.boundaryMesh());
    allPatches.setSize(allPatchNames.size());
    labelList patchSizes(allPatches.size());
    labelList patchStarts(allPatches.size());

    label startFaceI = nInternalFaces;

    // Copy patches0 with new sizes. First patches always come from
    // mesh0 and will always be present.
    label allPatchI = 0;

    forAll(from0ToAllPatches, patch0)
    {
        // Originates from mesh0. Clone with new size & filter out empty
        // patch.

        if (nFaces[patch0] == 0 && isA<processorPolyPatch>(allPatches[patch0]))
        {
            //Pout<< "Removing zero sized mesh0 patch " << allPatchNames[patch0]
            //    << endl;
            from0ToAllPatches[patch0] = -1;
            // Check if patch was also in mesh1 and update its addressing if so.
            if (fromAllTo1Patches[patch0] != -1)
            {
                from1ToAllPatches[fromAllTo1Patches[patch0]] = -1;
            }
        }
        else
        {
            // Clone. Note dummy size and start. Gets overwritten later in
            // resetPrimitives. This avoids getting temporarily illegal
            // SubList construction in polyPatch.
            allPatches.set
            (
                allPatchI,
                allPatches[patch0].clone
                (
                    allPatches,
                    allPatchI,
                    0,          // dummy size
                    0           // dummy start
                )
            );
            patchSizes[allPatchI] = nFaces[patch0];
            patchStarts[allPatchI] = startFaceI;

            // Record new index in allPatches
            from0ToAllPatches[patch0] = allPatchI;

            // Check if patch was also in mesh1 and update its addressing if so.
            if (fromAllTo1Patches[patch0] != -1)
            {
                from1ToAllPatches[fromAllTo1Patches[patch0]] = allPatchI;
            }

            startFaceI += nFaces[patch0];

            allPatchI++;
        }
    }

    // Copy unique patches of mesh1.
    forAll(from1ToAllPatches, patch1)
    {
        label uncompactAllPatchI = from1ToAllPatches[patch1];

        if (uncompactAllPatchI >= from0ToAllPatches.size())
        {
            // Patch has not been merged with any mesh0 patch.

            if
            (
                nFaces[uncompactAllPatchI] == 0
             && isA<processorPolyPatch>(patches1[patch1])
            )
            {
                //Pout<< "Removing zero sized mesh1 patch "
                //    << allPatchNames[uncompactAllPatchI] << endl;
                from1ToAllPatches[patch1] = -1;
            }
            else
            {
                // Clone.
                allPatches.set
                (
                    allPatchI,
                    patches1[patch1].clone
                    (
                        allPatches,
                        allPatchI,
                        0,          // dummy size
                        0           // dummy start
                    )
                );
                patchSizes[allPatchI] = nFaces[uncompactAllPatchI];
                patchStarts[allPatchI] = startFaceI;

                // Record new index in allPatches
                from1ToAllPatches[patch1] = allPatchI;

                startFaceI += nFaces[uncompactAllPatchI];

                allPatchI++;
            }
        }
    }

    allPatches.setSize(allPatchI);
    patchSizes.setSize(allPatchI);
    patchStarts.setSize(allPatchI);


    // Construct map information before changing mesh0 primitives
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    autoPtr<mapAddedPolyMesh> mapPtr
    (
        new mapAddedPolyMesh
        (
            mesh0.nPoints(),
            mesh0.nFaces(),
            mesh0.nCells(),

            mesh1.nPoints(),
            mesh1.nFaces(),
            mesh1.nCells(),

            from0ToAllPoints,
            from0ToAllFaces,
            identity(mesh0.nCells()),

            from1ToAllPoints,
            from1ToAllFaces,
            from1ToAllCells,

            from0ToAllPatches,
            from1ToAllPatches,

            mesh0PatchSizes,
            mesh0PatchStarts
        )
    );



    // Now we have extracted all information from all meshes.
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    mesh0.resetMotion();    // delete any oldPoints.
    mesh0.resetPrimitives
    (
        xferMove(allPoints),
        xferMove(allFaces),
        xferMove(allOwner),
        xferMove(allNeighbour),
        patchSizes,     // size
        patchStarts,    // patchstarts
        validBoundary   // boundary valid?
    );

    // Add zones to new mesh.
    mesh0.pointZones().clear();
    mesh0.faceZones().clear();
    mesh0.cellZones().clear();
    addZones
    (
        pointZoneNames,
        pzPoints,

        faceZoneNames,
        fzFaces,
        fzFlips,

        cellZoneNames,
        czCells,
        mesh0
    );

    return mapPtr;
}


CML::Map<CML::label> CML::polyMeshAdder::findSharedPoints
(
    const polyMesh& mesh,
    const scalar mergeDist
)
{
    const labelList& sharedPointLabels = mesh.globalData().sharedPointLabels();
    const labelList& sharedPointAddr = mesh.globalData().sharedPointAddr();

    // Because of adding the missing pieces e.g. when redistributing a mesh
    // it can be that there are multiple points on the same processor that
    // refer to the same shared point.

    // Invert point-to-shared addressing
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Map<labelList> sharedToMesh(sharedPointLabels.size());

    label nMultiple = 0;

    forAll(sharedPointLabels, i)
    {
        label pointI = sharedPointLabels[i];

        label sharedI = sharedPointAddr[i];

        Map<labelList>::iterator iter = sharedToMesh.find(sharedI);

        if (iter != sharedToMesh.end())
        {
            // sharedI already used by other point. Add this one.

            nMultiple++;

            labelList& connectedPointLabels = iter();

            label sz = connectedPointLabels.size();

            // Check just to make sure.
            if (findIndex(connectedPointLabels, pointI) != -1)
            {
                FatalErrorInFunction
                    << "Duplicate point in sharedPoint addressing." << endl
                    << "When trying to add point " << pointI << " on shared "
                    << sharedI  << " with connected points "
                    << connectedPointLabels
                    << abort(FatalError);
            }

            connectedPointLabels.setSize(sz+1);
            connectedPointLabels[sz] = pointI;
        }
        else
        {
            sharedToMesh.insert(sharedI, labelList(1, pointI));
        }
    }


    // Assign single master for every shared with multiple geometric points
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Map<label> pointToMaster(nMultiple);

    forAllConstIter(Map<labelList>, sharedToMesh, iter)
    {
        const labelList& connectedPointLabels = iter();

        //Pout<< "For shared:" << iter.key()
        //    << " found points:" << connectedPointLabels
        //    << " at coords:"
        //    <<  pointField(mesh.points(), connectedPointLabels) << endl;

        if (connectedPointLabels.size() > 1)
        {
            const pointField connectedPoints
            (
                mesh.points(),
                connectedPointLabels
            );

            labelList toMergedPoints;
            pointField mergedPoints;
            bool hasMerged = CML::mergePoints
            (
                connectedPoints,
                mergeDist,
                false,
                toMergedPoints,
                mergedPoints
            );

            if (hasMerged)
            {
                // Invert toMergedPoints
                const labelListList mergeSets
                (
                    invertOneToMany
                    (
                        mergedPoints.size(),
                        toMergedPoints
                    )
                );

                // Find master for valid merges
                forAll(mergeSets, setI)
                {
                    const labelList& mergeSet = mergeSets[setI];

                    if (mergeSet.size() > 1)
                    {
                        // Pick lowest numbered point
                        label masterPointI = labelMax;

                        forAll(mergeSet, i)
                        {
                            label pointI = connectedPointLabels[mergeSet[i]];

                            masterPointI = min(masterPointI, pointI);
                        }

                        forAll(mergeSet, i)
                        {
                            label pointI = connectedPointLabels[mergeSet[i]];

                            //Pout<< "Merging point " << pointI
                            //    << " at " << mesh.points()[pointI]
                            //    << " into master point "
                            //    << masterPointI
                            //    << " at " << mesh.points()[masterPointI]
                            //    << endl;

                            pointToMaster.insert(pointI, masterPointI);
                        }
                    }
                }
            }
        }
    }

    //- Old: geometric merging. Causes problems for two close shared points.
    //labelList sharedToMerged;
    //pointField mergedPoints;
    //bool hasMerged = CML::mergePoints
    //(
    //    pointField
    //    (
    //        mesh.points(),
    //        sharedPointLabels
    //    ),
    //    mergeDist,
    //    false,
    //    sharedToMerged,
    //    mergedPoints
    //);
    //
    //// Find out which sets of points get merged and create a map from
    //// mesh point to unique point.
    //
    //Map<label> pointToMaster(10*sharedToMerged.size());
    //
    //if (hasMerged)
    //{
    //    labelListList mergeSets
    //    (
    //        invertOneToMany
    //        (
    //            sharedToMerged.size(),
    //            sharedToMerged
    //        )
    //    );
    //
    //    label nMergeSets = 0;
    //
    //    forAll(mergeSets, setI)
    //    {
    //        const labelList& mergeSet = mergeSets[setI];
    //
    //        if (mergeSet.size() > 1)
    //        {
    //            // Take as master the shared point with the lowest mesh
    //            // point label. (rather arbitrarily - could use max or
    //            // any other one of the points)
    //
    //            nMergeSets++;
    //
    //            label masterI = labelMax;
    //
    //            forAll(mergeSet, i)
    //            {
    //                label sharedI = mergeSet[i];
    //
    //                masterI = min(masterI, sharedPointLabels[sharedI]);
    //            }
    //
    //            forAll(mergeSet, i)
    //            {
    //                label sharedI = mergeSet[i];
    //
    //                pointToMaster.insert(sharedPointLabels[sharedI], masterI);
    //            }
    //        }
    //    }
    //
    //    //if (debug)
    //    //{
    //    //    Pout<< "polyMeshAdder : merging:"
    //    //        << pointToMaster.size() << " into " << nMergeSets
    //    //        << " sets." << endl;
    //    //}
    //}

    return pointToMaster;
}


void CML::polyMeshAdder::mergePoints
(
    const polyMesh& mesh,
    const Map<label>& pointToMaster,
    polyTopoChange& meshMod
)
{
    // Remove all non-master points.
    forAll(mesh.points(), pointI)
    {
        Map<label>::const_iterator iter = pointToMaster.find(pointI);

        if (iter != pointToMaster.end())
        {
            if (iter() != pointI)
            {
                meshMod.removePoint(pointI, iter());
            }
        }
    }

    // Modify faces for points. Note: could use pointFaces here but want to
    // avoid addressing calculation.
    const faceList& faces = mesh.faces();

    forAll(faces, faceI)
    {
        const face& f = faces[faceI];

        bool hasMerged = false;

        forAll(f, fp)
        {
            label pointI = f[fp];

            Map<label>::const_iterator iter = pointToMaster.find(pointI);

            if (iter != pointToMaster.end())
            {
                if (iter() != pointI)
                {
                    hasMerged = true;
                    break;
                }
            }
        }

        if (hasMerged)
        {
            face newF(f);

            forAll(f, fp)
            {
                label pointI = f[fp];

                Map<label>::const_iterator iter = pointToMaster.find(pointI);

                if (iter != pointToMaster.end())
                {
                    newF[fp] = iter();
                }
            }

            label patchID = mesh.boundaryMesh().whichPatch(faceI);
            label nei = (patchID == -1 ? mesh.faceNeighbour()[faceI] : -1);
            label zoneID = mesh.faceZones().whichZone(faceI);
            bool zoneFlip = false;

            if (zoneID >= 0)
            {
                const faceZone& fZone = mesh.faceZones()[zoneID];
                zoneFlip = fZone.flipMap()[fZone.whichFace(faceI)];
            }

            meshMod.setAction
            (
                polyModifyFace
                (
                    newF,                       // modified face
                    faceI,                      // label of face
                    mesh.faceOwner()[faceI],    // owner
                    nei,                        // neighbour
                    false,                      // face flip
                    patchID,                    // patch for face
                    false,                      // remove from zone
                    zoneID,                     // zone for face
                    zoneFlip                    // face flip in zone
                )
            );
        }
    }
}


// ************************************************************************* //
