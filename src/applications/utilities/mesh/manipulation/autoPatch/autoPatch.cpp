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

Description
    Divides external faces into patches based on (user supplied) feature
    angle.

\*---------------------------------------------------------------------------*/

#include "argList.hpp"
#include "polyMesh.hpp"
#include "Time.hpp"
#include "boundaryMesh.hpp"
#include "repatchPolyTopoChanger.hpp"
#include "unitConversion.hpp"
#include "OFstream.hpp"
#include "ListOps.hpp"

using namespace CML;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Get all feature edges.
void collectFeatureEdges(const boundaryMesh& bMesh, labelList& markedEdges)
{
    markedEdges.setSize(bMesh.mesh().nEdges());

    label markedI = 0;

    forAll(bMesh.featureSegments(), i)
    {
        const labelList& segment = bMesh.featureSegments()[i];

        forAll(segment, j)
        {
            label featEdgeI = segment[j];

            label meshEdgeI = bMesh.featureToEdge()[featEdgeI];

            markedEdges[markedI++] = meshEdgeI;
        }
    }
    markedEdges.setSize(markedI);
}


// Main program:

int main(int argc, char *argv[])
{
#   include "addOverwriteOption.hpp"
    argList::noParallel();
    argList::validArgs.append("feature angle[0-180]");

#   include "setRootCase.hpp"
#   include "createTime.hpp"
    runTime.functionObjects().off();
#   include "createPolyMesh.hpp"
    const word oldInstance = mesh.pointsInstance();

    Info<< "Mesh read in = "
        << runTime.cpuTimeIncrement()
        << " s\n" << endl << endl;


    const scalar featureAngle = args.argRead<scalar>(1);
    const bool overwrite      = args.optionFound("overwrite");

    const scalar minCos = CML::cos(degToRad(featureAngle));

    Info<< "Feature:" << featureAngle << endl
        << "minCos :" << minCos << endl
        << endl;

    //
    // Use boundaryMesh to reuse all the featureEdge stuff in there.
    //

    boundaryMesh bMesh;
    bMesh.read(mesh);

    // Set feature angle (calculate feature edges)
    bMesh.setFeatureEdges(minCos);

    // Collect all feature edges as edge labels
    labelList markedEdges;

    collectFeatureEdges(bMesh, markedEdges);



    // (new) patch ID for every face in mesh.
    labelList patchIDs(bMesh.mesh().size(), -1);

    //
    // Fill patchIDs with values for every face by floodfilling without
    // crossing feature edge.
    //

    // Current patch number.
    label newPatchI = bMesh.patches().size();

    label suffix = 0;

    while (true)
    {
        // Find first unset face.
        label unsetFaceI = findIndex(patchIDs, -1);

        if (unsetFaceI == -1)
        {
            // All faces have patchID set. Exit.
            break;
        }

        // Found unset face. Create patch for it.
        word patchName;
        do
        {
            patchName = "auto" + name(suffix++);
        }
        while (bMesh.findPatchID(patchName) != -1);

        bMesh.addPatch(patchName);

        bMesh.changePatchType(patchName, "patch");


        // Fill visited with all faces reachable from unsetFaceI.
        boolList visited(bMesh.mesh().size());

        bMesh.markFaces(markedEdges, unsetFaceI, visited);


        // Assign all visited faces to current patch
        label nVisited = 0;

        forAll(visited, faceI)
        {
            if (visited[faceI])
            {
                nVisited++;

                patchIDs[faceI] = newPatchI;
            }
        }

        Info<< "Assigned " << nVisited << " faces to patch " << patchName
            << endl << endl;

        newPatchI++;
    }



    const PtrList<boundaryPatch>& patches = bMesh.patches();

    // Create new list of patches with old ones first
    List<polyPatch*> newPatchPtrList(patches.size());

    newPatchI = 0;

    // Copy old patches
    forAll(mesh.boundaryMesh(), patchI)
    {
        const polyPatch& patch = mesh.boundaryMesh()[patchI];

        newPatchPtrList[newPatchI] =
            patch.clone
            (
                mesh.boundaryMesh(),
                newPatchI,
                patch.size(),
                patch.start()
            ).ptr();

        newPatchI++;
    }

    // Add new ones with empty size.
    for (label patchI = newPatchI; patchI < patches.size(); patchI++)
    {
        const boundaryPatch& bp = patches[patchI];

        newPatchPtrList[newPatchI] = polyPatch::New
        (
            polyPatch::typeName,
            bp.name(),
            0,
            mesh.nFaces(),
            newPatchI,
            mesh.boundaryMesh()
        ).ptr();

        newPatchI++;
    }

    if (!overwrite)
    {
        runTime++;
    }


    // Change patches
    repatchPolyTopoChanger polyMeshRepatcher(mesh);
    polyMeshRepatcher.changePatches(newPatchPtrList);


    // Change face ordering

    // Since bMesh read from mesh there is one to one mapping so we don't
    // have to do the geometric stuff.
    const labelList& meshFace = bMesh.meshFace();

    forAll(patchIDs, faceI)
    {
        label meshFaceI = meshFace[faceI];

        polyMeshRepatcher.changePatchID(meshFaceI, patchIDs[faceI]);
    }

    polyMeshRepatcher.repatch();

    // Write resulting mesh
    if (overwrite)
    {
        mesh.setInstance(oldInstance);
    }
    mesh.write();


    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
