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

Application
    redistributePar

Description
    Redistributes existing decomposed mesh and fields according to the current
    settings in the decomposeParDict file.

    Must be run on maximum number of source and destination processors.
    Balances mesh and writes new mesh to new time directory.

    Can also work like decomposePar:
    \verbatim
        # Create empty processor directories (have to exist for argList)
        mkdir processor0
                ..
        mkdir processorN

        # Copy undecomposed polyMesh
        cp -r constant processor0

        # Distribute
        mpirun -np ddd redistributePar -parallel
    \endverbatim
\*---------------------------------------------------------------------------*/

#include "fvMesh.hpp"
#include "decompositionMethod.hpp"
#include "PstreamReduceOps.hpp"
#include "fvCFD.hpp"
#include "fvMeshDistribute.hpp"
#include "mapDistributePolyMesh.hpp"
#include "IOobjectList.hpp"
#include "globalIndex.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Tolerance (as fraction of the bounding box). Needs to be fairly lax since
// usually meshes get written with limited precision (6 digits)
static const scalar defaultMergeTol = 1E-6;


// Read mesh if available. Otherwise create empty mesh with same non-proc
// patches as proc0 mesh. Requires all processors to have all patches
// (and in same order).
autoPtr<fvMesh> createMesh
(
    const Time& runTime,
    const word& regionName,
    const fileName& instDir,
    const bool haveMesh
)
{
    //Pout<< "Create mesh for time = "
    //    << runTime.timeName() << nl << endl;

    IOobject io
    (
        regionName,
        instDir,
        runTime,
        IOobject::MUST_READ
    );

    if (!haveMesh)
    {
        // Create dummy mesh. Only used on procs that don't have mesh.
        IOobject noReadIO(io);
        noReadIO.readOpt() = IOobject::NO_READ;
        fvMesh dummyMesh
        (
            noReadIO,
            xferCopy(pointField()),
            xferCopy(faceList()),
            xferCopy(labelList()),
            xferCopy(labelList()),
            false
        );
        // Add some dummy zones so upon reading it does not read them
        // from the undecomposed case. Should be done as extra argument to
        // regIOobject::readStream?
        List<pointZone*> pz
        (
            1,
            new pointZone
            (
                "dummyPointZone",
                labelList(0),
                0,
                dummyMesh.pointZones()
            )
        );
        List<faceZone*> fz
        (
            1,
            new faceZone
            (
                "dummyFaceZone",
                labelList(0),
                boolList(0),
                0,
                dummyMesh.faceZones()
            )
        );
        List<cellZone*> cz
        (
            1,
            new cellZone
            (
                "dummyCellZone",
                labelList(0),
                0,
                dummyMesh.cellZones()
            )
        );
        dummyMesh.addZones(pz, fz, cz);
        //Pout<< "Writing dummy mesh to " << dummyMesh.polyMesh::objectPath()
        //    << endl;
        dummyMesh.write();
    }

    //Pout<< "Reading mesh from " << io.objectPath() << endl;
    autoPtr<fvMesh> meshPtr(new fvMesh(io));
    fvMesh& mesh = meshPtr();


    // Sync patches
    // ~~~~~~~~~~~~

    if (Pstream::master())
    {
        // Send patches
        for
        (
            int slave=Pstream::firstSlave();
            slave<=Pstream::lastSlave();
            slave++
        )
        {
            OPstream toSlave(Pstream::scheduled, slave);
            toSlave << mesh.boundaryMesh();
        }
    }
    else
    {
        // Receive patches
        IPstream fromMaster(Pstream::scheduled, Pstream::masterNo());
        PtrList<entry> patchEntries(fromMaster);

        if (haveMesh)
        {
            // Check master names against mine

            const polyBoundaryMesh& patches = mesh.boundaryMesh();

            forAll(patchEntries, patchI)
            {
                const entry& e = patchEntries[patchI];
                const word type(e.dict().lookup("type"));
                const word& name = e.keyword();

                if (type == processorPolyPatch::typeName)
                {
                    break;
                }

                if (patchI >= patches.size())
                {
                    FatalErrorIn
                    (
                        "createMesh(const Time&, const fileName&, const bool)"
                    )   << "Non-processor patches not synchronised."
                        << endl
                        << "Processor " << Pstream::myProcNo()
                        << " has only " << patches.size()
                        << " patches, master has "
                        << patchI
                        << exit(FatalError);
                }

                if
                (
                    type != patches[patchI].type()
                 || name != patches[patchI].name()
                )
                {
                    FatalErrorIn
                    (
                        "createMesh(const Time&, const fileName&, const bool)"
                    )   << "Non-processor patches not synchronised."
                        << endl
                        << "Master patch " << patchI
                        << " name:" << type
                        << " type:" << type << endl
                        << "Processor " << Pstream::myProcNo()
                        << " patch " << patchI
                        << " has name:" << patches[patchI].name()
                        << " type:" << patches[patchI].type()
                        << exit(FatalError);
                }
            }
        }
        else
        {
            // Add patch
            List<polyPatch*> patches(patchEntries.size());
            label nPatches = 0;

            forAll(patchEntries, patchI)
            {
                const entry& e = patchEntries[patchI];
                const word type(e.dict().lookup("type"));
                const word& name = e.keyword();

                if (type == processorPolyPatch::typeName)
                {
                    break;
                }

                //Pout<< "Adding patch:" << nPatches
                //    << " name:" << name << " type:" << type << endl;

                dictionary patchDict(e.dict());
                patchDict.remove("nFaces");
                patchDict.add("nFaces", 0);
                patchDict.remove("startFace");
                patchDict.add("startFace", 0);

                patches[patchI] = polyPatch::New
                (
                    name,
                    patchDict,
                    nPatches++,
                    mesh.boundaryMesh()
                ).ptr();
            }
            patches.setSize(nPatches);
            mesh.addFvPatches(patches, false);  // no parallel comms

            //// Write empty mesh now we have correct patches
            //meshPtr().write();
        }
    }


    // Determine zones
    // ~~~~~~~~~~~~~~~

    wordList pointZoneNames(mesh.pointZones().names());
    Pstream::scatter(pointZoneNames);
    wordList faceZoneNames(mesh.faceZones().names());
    Pstream::scatter(faceZoneNames);
    wordList cellZoneNames(mesh.cellZones().names());
    Pstream::scatter(cellZoneNames);

    if (!haveMesh)
    {
        // Add the zones. Make sure to remove the old dummy ones first
        mesh.pointZones().clear();
        mesh.faceZones().clear();
        mesh.cellZones().clear();

        List<pointZone*> pz(pointZoneNames.size());
        forAll(pointZoneNames, i)
        {
            pz[i] = new pointZone
            (
                pointZoneNames[i],
                labelList(0),
                i,
                mesh.pointZones()
            );
        }
        List<faceZone*> fz(faceZoneNames.size());
        forAll(faceZoneNames, i)
        {
            fz[i] = new faceZone
            (
                faceZoneNames[i],
                labelList(0),
                boolList(0),
                i,
                mesh.faceZones()
            );
        }
        List<cellZone*> cz(cellZoneNames.size());
        forAll(cellZoneNames, i)
        {
            cz[i] = new cellZone
            (
                cellZoneNames[i],
                labelList(0),
                i,
                mesh.cellZones()
            );
        }
        mesh.addZones(pz, fz, cz);
    }


    if (!haveMesh)
    {
        // We created a dummy mesh file above. Delete it.
        const fileName meshFiles = runTime.path()/io.instance()/mesh.meshDir();
        //Pout<< "Removing dummy mesh at " << meshFiles << endl;
        mesh.removeFiles();
        rmDir(meshFiles);
    }

    // Force recreation of globalMeshData.
    mesh.clearOut();
    mesh.globalData();


    // Do some checks.

    // Check if the boundary definition is unique
    mesh.boundaryMesh().checkDefinition(true);
    // Check if the boundary processor patches are correct
    mesh.boundaryMesh().checkParallelSync(true);
    // Check names of zones are equal
    mesh.cellZones().checkDefinition(true);
    mesh.cellZones().checkParallelSync(true);
    mesh.faceZones().checkDefinition(true);
    mesh.faceZones().checkParallelSync(true);
    mesh.pointZones().checkDefinition(true);
    mesh.pointZones().checkParallelSync(true);

    return meshPtr;
}


// Get merging distance when matching face centres
scalar getMergeDistance
(
    const argList& args,
    const Time& runTime,
    const boundBox& bb
)
{
    scalar mergeTol = defaultMergeTol;
    args.optionReadIfPresent("mergeTol", mergeTol);

    scalar writeTol =
        CML::pow(scalar(10.0), -scalar(IOstream::defaultPrecision()));

    Info<< "Merge tolerance : " << mergeTol << nl
        << "Write tolerance : " << writeTol << endl;

    if (runTime.writeFormat() == IOstream::ASCII && mergeTol < writeTol)
    {
        FatalErrorIn("getMergeDistance")
            << "Your current settings specify ASCII writing with "
            << IOstream::defaultPrecision() << " digits precision." << endl
            << "Your merging tolerance (" << mergeTol << ") is finer than this."
            << endl
            << "Please change your writeFormat to binary"
            << " or increase the writePrecision" << endl
            << "or adjust the merge tolerance (-mergeTol)."
            << exit(FatalError);
    }

    scalar mergeDist = mergeTol * bb.mag();

    Info<< "Overall meshes bounding box : " << bb << nl
        << "Relative tolerance          : " << mergeTol << nl
        << "Absolute matching distance  : " << mergeDist << nl
        << endl;

    return mergeDist;
}


//void printMeshData(Ostream& os, const polyMesh& mesh)
//{
//    os  << "Number of points:           " << mesh.points().size() << nl
//        << "          faces:            " << mesh.faces().size() << nl
//        << "          internal faces:   " << mesh.faceNeighbour().size() << nl
//        << "          cells:            " << mesh.cells().size() << nl
//        << "          boundary patches: " << mesh.boundaryMesh().size() << nl
//        << "          point zones:      " << mesh.pointZones().size() << nl
//        << "          face zones:       " << mesh.faceZones().size() << nl
//        << "          cell zones:       " << mesh.cellZones().size() << nl;
//}


void printMeshData(const polyMesh& mesh)
{
    // Collect all data on master

    globalIndex globalCells(mesh.nCells());
    labelListList patchNeiProcNo(Pstream::nProcs());
    labelListList patchSize(Pstream::nProcs());
    const labelList& pPatches = mesh.globalData().processorPatches();
    patchNeiProcNo[Pstream::myProcNo()].setSize(pPatches.size());
    patchSize[Pstream::myProcNo()].setSize(pPatches.size());
    forAll(pPatches, i)
    {
        const processorPolyPatch& ppp = refCast<const processorPolyPatch>
        (
            mesh.boundaryMesh()[pPatches[i]]
        );
        patchNeiProcNo[Pstream::myProcNo()][i] = ppp.neighbProcNo();
        patchSize[Pstream::myProcNo()][i] = ppp.size();
    }
    Pstream::gatherList(patchNeiProcNo);
    Pstream::gatherList(patchSize);


    // Print stats

    globalIndex globalBoundaryFaces(mesh.nFaces()-mesh.nInternalFaces());

    label maxProcCells = 0;
    label totProcFaces = 0;
    label maxProcPatches = 0;
    label totProcPatches = 0;
    label maxProcFaces = 0;

    for (label procI = 0; procI < Pstream::nProcs(); procI++)
    {
        Info<< endl
            << "Processor " << procI << nl
            << "    Number of cells = " << globalCells.localSize(procI)
            << endl;

        label nProcFaces = 0;

        const labelList& nei = patchNeiProcNo[procI];

        forAll(patchNeiProcNo[procI], i)
        {
            Info<< "    Number of faces shared with processor "
                << patchNeiProcNo[procI][i] << " = " << patchSize[procI][i]
                << endl;

            nProcFaces += patchSize[procI][i];
        }

        Info<< "    Number of processor patches = " << nei.size() << nl
            << "    Number of processor faces = " << nProcFaces << nl
            << "    Number of boundary faces = "
            << globalBoundaryFaces.localSize(procI) << endl;

        maxProcCells = max(maxProcCells, globalCells.localSize(procI));
        totProcFaces += nProcFaces;
        totProcPatches += nei.size();
        maxProcPatches = max(maxProcPatches, nei.size());
        maxProcFaces = max(maxProcFaces, nProcFaces);
    }

    // Stats

    scalar avgProcCells = scalar(globalCells.size())/Pstream::nProcs();
    scalar avgProcPatches = scalar(totProcPatches)/Pstream::nProcs();
    scalar avgProcFaces = scalar(totProcFaces)/Pstream::nProcs();

    // In case of all faces on one processor. Just to avoid division by 0.
    if (totProcPatches == 0)
    {
        avgProcPatches = 1;
    }
    if (totProcFaces == 0)
    {
        avgProcFaces = 1;
    }

    Info<< nl
        << "Number of processor faces = " << totProcFaces/2 << nl
        << "Max number of cells = " << maxProcCells
        << " (" << 100.0*(maxProcCells-avgProcCells)/avgProcCells
        << "% above average " << avgProcCells << ")" << nl
        << "Max number of processor patches = " << maxProcPatches
        << " (" << 100.0*(maxProcPatches-avgProcPatches)/avgProcPatches
        << "% above average " << avgProcPatches << ")" << nl
        << "Max number of faces between processors = " << maxProcFaces
        << " (" << 100.0*(maxProcFaces-avgProcFaces)/avgProcFaces
        << "% above average " << avgProcFaces << ")" << nl
        << endl;
}


// Debugging: write volScalarField with decomposition for post processing.
void writeDecomposition
(
    const word& name,
    const fvMesh& mesh,
    const labelList& decomp
)
{
    Info<< "Writing wanted cell distribution to volScalarField " << name
        << " for postprocessing purposes." << nl << endl;

    volScalarField procCells
    (
        IOobject
        (
            name,
            mesh.time().timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE,
            false                   // do not register
        ),
        mesh,
        dimensionedScalar(name, dimless, -1),
        zeroGradientFvPatchScalarField::typeName
    );

    forAll(procCells, cI)
    {
        procCells[cI] = decomp[cI];
    }
    procCells.write();
}


// Read vol or surface fields
//template<class T, class Mesh>
template<class GeoField>
void readFields
(
    const boolList& haveMesh,
    const fvMesh& mesh,
    const autoPtr<fvMeshSubset>& subsetterPtr,
    IOobjectList& allObjects,
    PtrList<GeoField>& fields
)
{
    //typedef GeometricField<T, fvPatchField, Mesh> fldType;

    // Get my objects of type
    IOobjectList objects(allObjects.lookupClass(GeoField::typeName));
    // Check that we all have all objects
    wordList objectNames = objects.toc();
    // Get master names
    wordList masterNames(objectNames);
    Pstream::scatter(masterNames);

    if (haveMesh[Pstream::myProcNo()] && objectNames != masterNames)
    {
        FatalErrorIn("readFields()")
            << "differing fields of type " << GeoField::typeName
            << " on processors." << endl
            << "Master has:" << masterNames << endl
            << Pstream::myProcNo() << " has:" << objectNames
            << abort(FatalError);
    }

    fields.setSize(masterNames.size());

    // Have master send all fields to processors that don't have a mesh
    if (Pstream::master())
    {
        forAll(masterNames, i)
        {
            const word& name = masterNames[i];
            IOobject& io = *objects[name];
            io.writeOpt() = IOobject::AUTO_WRITE;

            // Load field
            fields.set(i, new GeoField(io, mesh));

            // Create zero sized field and send
            if (subsetterPtr.valid())
            {
                tmp<GeoField> tsubfld = subsetterPtr().interpolate(fields[i]);

                // Send to all processors that don't have a mesh
                for (label procI = 1; procI < Pstream::nProcs(); procI++)
                {
                    if (!haveMesh[procI])
                    {
                        OPstream toProc(Pstream::blocking, procI);
                        toProc<< tsubfld();
                    }
                }
            }
        }
    }
    else if (!haveMesh[Pstream::myProcNo()])
    {
        // Don't have mesh (nor fields). Receive empty field from master.

        forAll(masterNames, i)
        {
            const word& name = masterNames[i];

            // Receive field
            IPstream fromMaster(Pstream::blocking, Pstream::masterNo());
            dictionary fieldDict(fromMaster);

            fields.set
            (
                i,
                new GeoField
                (
                    IOobject
                    (
                        name,
                        mesh.time().timeName(),
                        mesh,
                        IOobject::NO_READ,
                        IOobject::AUTO_WRITE
                    ),
                    mesh,
                    fieldDict
                )
            );

            //// Write it for next time round (since mesh gets written as well)
            //fields[i].write();
        }
    }
    else
    {
        // Have mesh so just try to load
        forAll(masterNames, i)
        {
            const word& name = masterNames[i];
            IOobject& io = *objects[name];
            io.writeOpt() = IOobject::AUTO_WRITE;

            // Load field
            fields.set(i, new GeoField(io, mesh));
        }
    }
}


// Debugging: compare two fields.
void compareFields
(
    const scalar tolDim,
    const volVectorField& a,
    const volVectorField& b
)
{
    forAll(a, cellI)
    {
        if (mag(b[cellI] - a[cellI]) > tolDim)
        {
            FatalErrorIn
            (
                "compareFields"
                "(const scalar, const volVectorField&, const volVectorField&)"
            )   << "Did not map volVectorField correctly:" << nl
                << "cell:" << cellI
                << " transfer b:" << b[cellI]
                << " real cc:" << a[cellI]
                << abort(FatalError);
        }
    }
    forAll(a.boundaryField(), patchI)
    {
        // We have real mesh cellcentre and
        // mapped original cell centre.

        const fvPatchVectorField& aBoundary =
            a.boundaryField()[patchI];

        const fvPatchVectorField& bBoundary =
            b.boundaryField()[patchI];

        if (!bBoundary.coupled())
        {
            forAll(aBoundary, i)
            {
                if (mag(aBoundary[i] - bBoundary[i]) > tolDim)
                {
                    WarningIn
                    (
                        "compareFields"
                        "(const scalar, const volVectorField&"
                        ", const volVectorField&)"
                    )   << "Did not map volVectorField correctly:"
                        << endl
                        << "patch:" << patchI << " patchFace:" << i
                        << " cc:" << endl
                        << "    real    :" << aBoundary[i] << endl
                        << "    mapped  :" << bBoundary[i] << endl
                        << "This might be just a precision entry"
                        << " on writing the mesh." << endl;
                        //<< abort(FatalError);
                }
            }
        }
    }
}


// Main program:

int main(int argc, char *argv[])
{
#   include "addRegionOption.hpp"
#   include "addOverwriteOption.hpp"
    argList::addOption
    (
        "mergeTol",
        "scalar",
        "specify the merge distance relative to the bounding box size "
        "(default 1E-6)"
    );
    // Include explicit constant options, have zero from time range
    timeSelector::addOptions();
#   include "setRootCase.hpp"

    if (env("FOAM_SIGFPE"))
    {
        WarningIn(args.executable())
            << "Detected floating point exception trapping (FOAM_SIGFPE)."
            << " This might give" << nl
            << "    problems when mapping fields. Switch it off in case"
            << " of problems." << endl;
    }


    // Create processor directory if non-existing
    if (!Pstream::master() && !isDir(args.path()))
    {
        Pout<< "Creating case directory " << args.path() << endl;
        mkDir(args.path());
    }


    // Make sure we do not use the master-only reading.
    regIOobject::fileModificationChecking = regIOobject::timeStamp;
#   include "createTime.hpp"
    // Allow override of time
    instantList times = timeSelector::selectIfPresent(runTime, args);
    runTime.setTime(times[0], 0);

    runTime.functionObjects().off();

    word regionName = polyMesh::defaultRegion;
    fileName meshSubDir;

    if (args.optionReadIfPresent("region", regionName))
    {
        meshSubDir = regionName/polyMesh::meshSubDir;
    }
    else
    {
        meshSubDir = polyMesh::meshSubDir;
    }
    Info<< "Using mesh subdirectory " << meshSubDir << nl << endl;

    const bool overwrite = args.optionFound("overwrite");


    // Get time instance directory. Since not all processors have meshes
    // just use the master one everywhere.

    fileName masterInstDir;
    if (Pstream::master())
    {
        masterInstDir = runTime.findInstance(meshSubDir, "points");
    }
    Pstream::scatter(masterInstDir);

    // Check who has a mesh
    const fileName meshPath = runTime.path()/masterInstDir/meshSubDir;

    Info<< "Found points in " << meshPath << nl << endl;


    boolList haveMesh(Pstream::nProcs(), false);
    haveMesh[Pstream::myProcNo()] = isDir(meshPath);
    Pstream::gatherList(haveMesh);
    Pstream::scatterList(haveMesh);
    Info<< "Per processor mesh availability : " << haveMesh << endl;
    const bool allHaveMesh = (findIndex(haveMesh, false) == -1);

    // Create mesh
    autoPtr<fvMesh> meshPtr = createMesh
    (
        runTime,
        regionName,
        masterInstDir,
        haveMesh[Pstream::myProcNo()]
    );
    fvMesh& mesh = meshPtr();

    //Pout<< "Read mesh:" << endl;
    //printMeshData(Pout, mesh);
    //Pout<< endl;



    IOdictionary decompositionDict
    (
        IOobject
        (
            "decomposeParDict",
            runTime.system(),
            mesh,
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE
        )
    );

    labelList finalDecomp;

    // Create decompositionMethod and new decomposition
    {
        autoPtr<decompositionMethod> decomposer
        (
            decompositionMethod::New
            (
                decompositionDict
            )
        );

        if (!decomposer().parallelAware())
        {
            WarningIn(args.executable())
                << "You have selected decomposition method "
                << decomposer().typeName
                << " which does" << endl
                << "not synchronise the decomposition across"
                << " processor patches." << endl
                << "    You might want to select a decomposition method which"
                << " is aware of this. Continuing."
                << endl;
        }

        finalDecomp = decomposer().decompose(mesh, mesh.cellCentres());
    }

    // Dump decomposition to volScalarField
    if (!overwrite)
    {
        writeDecomposition("decomposition", mesh, finalDecomp);
    }


    // Create 0 sized mesh to do all the generation of zero sized
    // fields on processors that have zero sized meshes. Note that this is
    // only nessecary on master but since polyMesh construction with
    // Pstream::parRun does parallel comms we have to do it on all
    // processors
    autoPtr<fvMeshSubset> subsetterPtr;

    if (!allHaveMesh)
    {
        // Find last non-processor patch.
        const polyBoundaryMesh& patches = mesh.boundaryMesh();

        label nonProcI = -1;

        forAll(patches, patchI)
        {
            if (isA<processorPolyPatch>(patches[patchI]))
            {
                break;
            }
            nonProcI++;
        }

        if (nonProcI == -1)
        {
            FatalErrorIn(args.executable())
                << "Cannot find non-processor patch on processor "
                << Pstream::myProcNo() << endl
                << " Current patches:" << patches.names() << abort(FatalError);
        }

        // Subset 0 cells, no parallel comms. This is used to create zero-sized
        // fields.
        subsetterPtr.reset(new fvMeshSubset(mesh));
        subsetterPtr().setLargeCellSubset(labelHashSet(0), nonProcI, false);
    }


    // Get original objects (before incrementing time!)
    IOobjectList objects(mesh, runTime.timeName());
    // We don't want to map the decomposition (mapping already tested when
    // mapping the cell centre field)
    IOobjectList::iterator iter = objects.find("decomposition");
    if (iter != objects.end())
    {
        objects.erase(iter);
    }

    PtrList<volScalarField> volScalarFields;
    readFields
    (
        haveMesh,
        mesh,
        subsetterPtr,
        objects,
        volScalarFields
    );

    PtrList<volVectorField> volVectorFields;
    readFields
    (
        haveMesh,
        mesh,
        subsetterPtr,
        objects,
        volVectorFields
    );

    PtrList<volSphericalTensorField> volSphereTensorFields;
    readFields
    (
        haveMesh,
        mesh,
        subsetterPtr,
        objects,
        volSphereTensorFields
    );

    PtrList<volSymmTensorField> volSymmTensorFields;
    readFields
    (
        haveMesh,
        mesh,
        subsetterPtr,
        objects,
        volSymmTensorFields
    );

    PtrList<volTensorField> volTensorFields;
    readFields
    (
        haveMesh,
        mesh,
        subsetterPtr,
        objects,
        volTensorFields
    );

    PtrList<surfaceScalarField> surfScalarFields;
    readFields
    (
        haveMesh,
        mesh,
        subsetterPtr,
        objects,
        surfScalarFields
    );

    PtrList<surfaceVectorField> surfVectorFields;
    readFields
    (
        haveMesh,
        mesh,
        subsetterPtr,
        objects,
        surfVectorFields
    );

    PtrList<surfaceSphericalTensorField> surfSphereTensorFields;
    readFields
    (
        haveMesh,
        mesh,
        subsetterPtr,
        objects,
        surfSphereTensorFields
    );

    PtrList<surfaceSymmTensorField> surfSymmTensorFields;
    readFields
    (
        haveMesh,
        mesh,
        subsetterPtr,
        objects,
        surfSymmTensorFields
    );

    PtrList<surfaceTensorField> surfTensorFields;
    readFields
    (
        haveMesh,
        mesh,
        subsetterPtr,
        objects,
        surfTensorFields
    );


    // Debugging: Create additional volField that will be mapped.
    // Used to test correctness of mapping
    //volVectorField mapCc("mapCc", 1*mesh.C());

    // Global matching tolerance
    const scalar tolDim = getMergeDistance
    (
        args,
        runTime,
        mesh.bounds()
    );

    // Mesh distribution engine
    fvMeshDistribute distributor(mesh, tolDim);

    //Pout<< "Wanted distribution:"
    //    << distributor.countCells(finalDecomp) << nl << endl;

    // Do actual sending/receiving of mesh
    autoPtr<mapDistributePolyMesh> map = distributor.distribute(finalDecomp);

    //// Distribute any non-registered data accordingly
    //map().distributeFaceData(faceCc);


    // Print a bit
    // Print some statistics
    Info<< "After distribution:" << endl;
    printMeshData(mesh);


    if (!overwrite)
    {
        runTime++;
    }
    else
    {
        mesh.setInstance(masterInstDir);
    }
    Info<< "Writing redistributed mesh to " << runTime.timeName() << nl << endl;
    mesh.write();


    // Debugging: test mapped cellcentre field.
    //compareFields(tolDim, mesh.C(), mapCc);

    // Print nice message
    // ~~~~~~~~~~~~~~~~~~

    // Determine which processors remain so we can print nice final message.
    labelList nFaces(Pstream::nProcs());
    nFaces[Pstream::myProcNo()] = mesh.nFaces();
    Pstream::gatherList(nFaces);
    Pstream::scatterList(nFaces);

    Info<< nl
        << "You can pick up the redecomposed mesh from the polyMesh directory"
        << " in " << runTime.timeName() << "." << nl
        << "If you redecomposed the mesh to less processors you can delete"
        << nl
        << "the processor directories with 0 sized meshes in them." << nl
        << "Below is a sample set of commands to do this."
        << " Take care when issuing these" << nl
        << "commands." << nl << endl;

    forAll(nFaces, procI)
    {
        fileName procDir = "processor" + name(procI);

        if (nFaces[procI] == 0)
        {
            Info<< "    rm -r " << procDir.c_str() << nl;
        }
        else
        {
            fileName timeDir = procDir/runTime.timeName()/meshSubDir;
            fileName constDir = procDir/runTime.constant()/meshSubDir;

            Info<< "    rm -r " << constDir.c_str() << nl
                << "    mv " << timeDir.c_str()
                << ' ' << constDir.c_str() << nl;
        }
    }
    Info<< endl;


    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
