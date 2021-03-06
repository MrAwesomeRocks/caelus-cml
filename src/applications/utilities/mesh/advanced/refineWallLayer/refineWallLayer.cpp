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

Description
    Utility to refine cells next to patches.

    Arguments:
        1: List of patch name regular expressions
        2: The size of the refined cells as a fraction of the edge-length.

    Examples:
        Split the near-wall cells of patch Wall in the middle
            refineWallLayer "(Wall)" 0.5

        Split the near-wall cells of patches Wall1 and Wall2 in the middle
            refineWallLayer "(Wall1 Wall2)" 0.5

        Split the near-wall cells of all patches with names beginning with wall
        with the near-wall cells 10% of the thickness of the original cells
            refineWallLayer '("Wall.*")' 0.1

\*---------------------------------------------------------------------------*/

#include "argList.hpp"
#include "Time.hpp"
#include "polyTopoChange.hpp"
#include "cellCuts.hpp"
#include "cellSet.hpp"
#include "meshCutter.hpp"

using namespace CML;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #include "addOverwriteOption.hpp"
    argList::noParallel();
    argList::validArgs.append("patches");
    argList::validArgs.append("edgeFraction");

    argList::addOption
    (
        "useSet",
        "name",
        "restrict cells to refine based on specified cellSet name"
    );


    #include "setRootCase.hpp"
    #include "createTime.hpp"
    runTime.functionObjects().off();

    #include "createPolyMesh.hpp"
    const word oldInstance = mesh.pointsInstance();

    // Find set of patches from the list of regular expressions provided
    const wordReList patches((IStringStream(args[1])()));
    const labelHashSet patchSet(mesh.boundaryMesh().patchSet(patches));

    const scalar weight  = args.argRead<scalar>(2);
    const bool overwrite = args.optionFound("overwrite");

    if (!patchSet.size())
    {
        FatalErrorInFunction
            << "Cannot find any patches in set " << patches << endl
            << "Valid patches are " << mesh.boundaryMesh().names()
            << exit(FatalError);
    }

    label nPatchFaces = 0;
    label nPatchEdges = 0;

    forAllConstIter(labelHashSet, patchSet, iter)
    {
        nPatchFaces += mesh.boundaryMesh()[iter.key()].size();
        nPatchEdges += mesh.boundaryMesh()[iter.key()].nEdges();
    }

    // Construct from estimate for the number of cells to refine
    labelHashSet cutCells(4*nPatchFaces);

    // Construct from total patch edges in selected patches
    DynamicList<label> allCutEdges(nPatchEdges);
    DynamicList<scalar> allCutEdgeWeights(nPatchEdges);

    // Find cells to refine
    forAllConstIter(labelHashSet, patchSet, iter)
    {
        const polyPatch& pp = mesh.boundaryMesh()[iter.key()];
        const labelList& meshPoints = pp.meshPoints();

        forAll(meshPoints, pointI)
        {
            label meshPointI = meshPoints[pointI];

            const labelList& pCells = mesh.pointCells()[meshPointI];

            forAll(pCells, pCellI)
            {
                cutCells.insert(pCells[pCellI]);
            }
        }
    }

    // Edit list of cells to refine according to specified set
    word setName;
    if (args.optionReadIfPresent("useSet", setName))
    {
        Info<< "Subsetting cells to cut based on cellSet"
            << setName << nl << endl;

        cellSet cells(mesh, setName);

        Info<< "Read " << cells.size() << " cells from cellSet "
            << cells.instance()/cells.local()/cells.name()
            << nl << endl;

        forAllConstIter(cellSet, cells, iter)
        {
            cutCells.erase(iter.key());
        }
        Info<< "Removed from cells to cut all the ones not in set "
            << setName << nl << endl;
    }

    // Mark all mesh points on patch
    boolList vertOnPatch(mesh.nPoints(), false);

    forAllConstIter(labelHashSet, patchSet, iter)
    {
        const polyPatch& pp = mesh.boundaryMesh()[iter.key()];
        const labelList& meshPoints = pp.meshPoints();

        forAll(meshPoints, pointI)
        {
            vertOnPatch[meshPoints[pointI]] = true;
        }
    }

    forAllConstIter(labelHashSet, patchSet, iter)
    {
        const polyPatch& pp = mesh.boundaryMesh()[iter.key()];
        const labelList& meshPoints = pp.meshPoints();

        forAll(meshPoints, pointI)
        {
            label meshPointI = meshPoints[pointI];

            const labelList& pEdges = mesh.pointEdges()[meshPointI];

            forAll(pEdges, pEdgeI)
            {
                const label edgeI = pEdges[pEdgeI];
                const edge& e = mesh.edges()[edgeI];

                label otherPointI = e.otherVertex(meshPointI);

                if (!vertOnPatch[otherPointI])
                {
                    allCutEdges.append(edgeI);

                    if (e.start() == meshPointI)
                    {
                        allCutEdgeWeights.append(weight);
                    }
                    else
                    {
                        allCutEdgeWeights.append(1 - weight);
                    }
                }
            }
        }
    }

    allCutEdges.shrink();
    allCutEdgeWeights.shrink();

    Info<< "Refining:" << nl
        << "    cells:" << cutCells.size() << nl
        << "    edges:" << allCutEdges.size() << endl;

    // Transfer DynamicLists to straight ones.
    scalarField cutEdgeWeights;
    cutEdgeWeights.transfer(allCutEdgeWeights);
    allCutEdgeWeights.clear();


    // Gets cuts across cells from cuts through edges.
    cellCuts cuts
    (
        mesh,
        cutCells.toc(),     // cells candidate for cutting
        labelList(0),       // cut vertices
        allCutEdges,        // cut edges
        cutEdgeWeights      // weight on cut edges
    );

    polyTopoChange meshMod(mesh);

    // Cutting engine
    meshCutter cutter(mesh);

    // Insert mesh refinement into polyTopoChange.
    cutter.setRefinement(cuts, meshMod);

    if (!overwrite)
    {
        runTime++;
    }

    autoPtr<mapPolyMesh> morphMap = meshMod.changeMesh(mesh, false);

    if (morphMap().hasMotionPoints())
    {
        mesh.movePoints(morphMap().preMotionPoints());
    }

    // Update stored labels on meshCutter.
    cutter.updateMesh(morphMap());

    Info<< "Finished refining" << endl;

    if (overwrite)
    {
        mesh.setInstance(oldInstance);
    }

    // Write resulting mesh
    Info<< "Writing refined mesh to time " << runTime.timeName() << endl;

    mesh.write();

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
