/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2018 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Caelus is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "Pstream.hpp"
#include "functionObjectList.hpp"
#include "streamLine.hpp"
#include "fvMesh.hpp"
#include "streamLineParticleCloud.hpp"
#include "ReadFields_.hpp"
#include "meshSearch.hpp"
#include "sampledSet.hpp"
#include "globalIndex.hpp"
#include "mapDistribute.hpp"
#include "interpolationCellPoint.hpp"
#include "PatchTools.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(streamLine, 0);

    template<>
    const char* NamedEnum<streamLine::trackDirection, 3>::names[] =
        {"forward", "backward", "both"};

    const NamedEnum<streamLine::trackDirection, 3>
        streamLine::trackDirectionNames_;
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

CML::autoPtr<CML::indirectPrimitivePatch>
CML::streamLine::wallPatch() const
{
    const fvMesh& mesh = dynamic_cast<const fvMesh&>(obr_);

    const polyBoundaryMesh& patches = mesh.boundaryMesh();

    label nFaces = 0;

    forAll(patches, patchi)
    {
        if (isA<wallPolyPatch>(patches[patchi]))
        {
            nFaces += patches[patchi].size();
        }
    }

    labelList addressing(nFaces);

    nFaces = 0;

    forAll(patches, patchi)
    {
        if (isA<wallPolyPatch>(patches[patchi]))
        {
            const polyPatch& pp = patches[patchi];

            forAll(pp, i)
            {
                addressing[nFaces++] = pp.start()+i;
            }
        }
    }

    return autoPtr<indirectPrimitivePatch>
    (
        new indirectPrimitivePatch
        (
            IndirectList<face>
            (
                mesh.faces(),
                addressing
            ),
            mesh.points()
        )
    );
}


void CML::streamLine::track()
{
    const Time& runTime = obr_.time();
    const fvMesh& mesh = dynamic_cast<const fvMesh&>(obr_);

    IDLList<streamLineParticle> initialParticles;
    streamLineParticleCloud particles
    (
        mesh,
        cloudName_,
        initialParticles
    );

    const sampledSet& seedPoints = sampledSetPtr_();

    forAll(seedPoints, i)
    {
        particles.addParticle
        (
            new streamLineParticle
            (
                mesh,
                seedPoints[i],
                seedPoints.cells()[i],
                lifeTime_
            )
        );
    }

    label nSeeds = returnReduce(particles.size(), sumOp<label>());

    Info << "    seeded " << nSeeds << " particles" << endl;

    // Read or lookup fields
    PtrList<volScalarField> vsFlds;
    PtrList<interpolation<scalar> > vsInterp;
    PtrList<volVectorField> vvFlds;
    PtrList<interpolation<vector> > vvInterp;

    label UIndex = -1;

    if (loadFromFiles_)
    {
        IOobjectList allObjects(mesh, runTime.timeName());

        IOobjectList objects(2*fields_.size());
        forAll(fields_, i)
        {
            objects.add(*allObjects[fields_[i]]);
        }

        ReadFields(mesh, objects, vsFlds);
        vsInterp.setSize(vsFlds.size());
        forAll(vsFlds, i)
        {
            vsInterp.set
            (
                i,
                interpolation<scalar>::New
                (
                    interpolationScheme_,
                    vsFlds[i]
                )
            );
        }
        ReadFields(mesh, objects, vvFlds);
        vvInterp.setSize(vvFlds.size());
        forAll(vvFlds, i)
        {
            vvInterp.set
            (
                i,
                interpolation<vector>::New
                (
                    interpolationScheme_,
                    vvFlds[i]
                )
            );
        }
    }
    else
    {
        label nScalar = 0;
        label nVector = 0;

        forAll(fields_, i)
        {
            if (mesh.foundObject<volScalarField>(fields_[i]))
            {
                nScalar++;
            }
            else if (mesh.foundObject<volVectorField>(fields_[i]))
            {
                nVector++;
            }
            else
            {
                FatalErrorInFunction
                    << "Cannot find field " << fields_[i] << nl
                    << "Valid scalar fields are:"
                    << mesh.names(volScalarField::typeName) << nl
                    << "Valid vector fields are:"
                    << mesh.names(volVectorField::typeName)
                    << exit(FatalError);
            }
        }
        vsInterp.setSize(nScalar);
        nScalar = 0;
        vvInterp.setSize(nVector);
        nVector = 0;

        forAll(fields_, i)
        {
            if (mesh.foundObject<volScalarField>(fields_[i]))
            {
                const volScalarField& f = mesh.lookupObject<volScalarField>
                (
                    fields_[i]
                );
                vsInterp.set
                (
                    nScalar++,
                    interpolation<scalar>::New
                    (
                        interpolationScheme_,
                        f
                    )
                );
            }
            else if (mesh.foundObject<volVectorField>(fields_[i]))
            {
                const volVectorField& f = mesh.lookupObject<volVectorField>
                (
                    fields_[i]
                );

                if (f.name() == UName_)
                {
                    UIndex = nVector;
                }

                vvInterp.set
                (
                    nVector++,
                    interpolation<vector>::New
                    (
                        interpolationScheme_,
                        f
                    )
                );
            }
        }
    }

    // Store the names
    scalarNames_.setSize(vsInterp.size());
    forAll(vsInterp, i)
    {
        scalarNames_[i] = vsInterp[i].psi().name();
    }
    vectorNames_.setSize(vvInterp.size());
    forAll(vvInterp, i)
    {
        vectorNames_[i] = vvInterp[i].psi().name();
    }

    // Check that we know the index of U in the interpolators.

    if (UIndex == -1)
    {
        FatalErrorInFunction
            << "Cannot find field to move particles with : " << UName_ << nl
            << "This field has to be present in the sampled fields " << fields_
            << " and in the objectRegistry."
            << exit(FatalError);
    }

    // Sampled data
    // ~~~~~~~~~~~~

    // Size to maximum expected sizes.
    allTracks_.clear();
    allTracks_.setCapacity(nSeeds);
    allScalars_.setSize(vsInterp.size());
    forAll(allScalars_, i)
    {
        allScalars_[i].clear();
        allScalars_[i].setCapacity(nSeeds);
    }
    allVectors_.setSize(vvInterp.size());
    forAll(allVectors_, i)
    {
        allVectors_[i].clear();
        allVectors_[i].setCapacity(nSeeds);
    }


    // Additional particle info
    streamLineParticle::trackingData td
    (
        particles,
        vsInterp,
        vvInterp,
        UIndex,         // index of U in vvInterp

        trackDirection_ == trackDirection::FORWARD,

        nSubCycle_,     // automatic track control:step through cells in steps?
        trackLength_,   // fixed track length

        allTracks_,
        allScalars_,
        allVectors_
    );


    // Set very large dt. Note: cannot use GREAT since 1/GREAT is SMALL
    // which is a trigger value for the tracking...
    const scalar trackTime = CML::sqrt(GREAT);

    // Track
    if (trackDirection_ == trackDirection::BOTH)
    {
        initialParticles = particles;
    }

    particles.move(particles, td, trackTime);

    if (trackDirection_ == trackDirection::BOTH)
    {
        particles.IDLList<streamLineParticle>::operator=(initialParticles);
        td.trackForward_ = !td.trackForward_;
        particles.move(particles, td, trackTime);
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::streamLine::streamLine
(
    const word& name,
    const objectRegistry& obr,
    const dictionary& dict,
    const bool loadFromFiles
)
:
    dict_(dict),
    name_(name),
    obr_(obr),
    loadFromFiles_(loadFromFiles),
    active_(true),
    nSubCycle_(0)
{
    // Only active if a fvMesh is available
    if (isA<fvMesh>(obr_))
    {
        read(dict_);
    }
    else
    {
        active_ = false;
        WarningInFunction
            << "No fvMesh available, deactivating."
            << nl << endl;
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::streamLine::~streamLine()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::streamLine::read(const dictionary& dict)
{
    if (active_)
    {
        Info<< type() << " " << name_ << ":" << nl;

        //dict_ = dict;
        dict.lookup("fields") >> fields_;
        if (dict.found("UName"))
        {
            dict.lookup("UName") >> UName_;
        }
        else
        {
            UName_ = "U";
            if (dict.found("U"))
            {
                IOWarningInFunction(dict)
                    << "Using deprecated entry \"U\"."
                    << " Please use \"UName\" instead."
                    << endl;
                dict.lookup("U") >> UName_;
            }
        }

        if (findIndex(fields_, UName_) == -1)
        {
            FatalIOErrorInFunction(dict)
                << "Velocity field for tracking " << UName_
                << " should be present in the list of fields " << fields_
                << exit(FatalIOError);
        }

        // The trackForward entry is maintained here for backwards compatibility
        if (!dict.found("direction") && dict.found("trackForward"))
        {
            trackDirection_ =
                dict.lookupType<bool>("trackForward")
              ? trackDirection::FORWARD
              : trackDirection::BACKWARD;
        }
        else
        {
            trackDirection_ = trackDirectionNames_[word(dict.lookup("direction"))];
        }

        dict.lookup("lifeTime") >> lifeTime_;
        if (lifeTime_ < 1)
        {
            FatalErrorInFunction
                << "Illegal value " << lifeTime_ << " for lifeTime"
                << exit(FatalError);
        }

        dict.lookup("lifeTime") >> lifeTime_;
        if (lifeTime_ < 1)
        {
            FatalErrorInFunction
                << "Illegal value " << lifeTime_ << " for lifeTime"
                << exit(FatalError);
        }

        bool subCycling = dict.found("nSubCycle");
        bool fixedLength = dict.found("trackLength");

        if (subCycling && fixedLength)
        {
            FatalIOErrorInFunction(dict)
                << "Cannot both specify automatic time stepping (through '"
                << "nSubCycle' specification) and fixed track length (through '"
                << "trackLength')"
                << exit(FatalIOError);
        }


        nSubCycle_ = 1;
        if (dict.readIfPresent("nSubCycle", nSubCycle_))
        {
            trackLength_ = VGREAT;
            if (nSubCycle_ < 1)
            {
                nSubCycle_ = 1;
            }
            Info<< "    automatic track length specified through"
                << " number of sub cycles : " << nSubCycle_ << nl << endl;
        }
        else
        {
            dict.lookup("trackLength") >> trackLength_;

            Info<< "    fixed track length specified : "
                << trackLength_ << nl << endl;
        }


        interpolationScheme_ = dict.lookupOrDefault
        (
            "interpolationScheme",
            interpolationCellPoint<scalar>::typeName
        );

        //Info<< "    using interpolation " << interpolationScheme_
        //    << endl;

        cloudName_ = dict.lookupOrDefault<word>("cloudName", "streamLine");
        dict.lookup("seedSampleSet") >> seedSet_;

        const fvMesh& mesh = dynamic_cast<const fvMesh&>(obr_);

        meshSearchPtr_.reset(new meshSearch(mesh));

        const dictionary& coeffsDict = dict.subDict(seedSet_ + "Coeffs");
        sampledSetPtr_ = sampledSet::New
        (
            seedSet_,
            mesh,
            meshSearchPtr_(),
            coeffsDict
        );
        coeffsDict.lookup("axis") >> sampledSetAxis_;

        scalarFormatterPtr_ = writer<scalar>::New(dict.lookup("setFormat"));
        vectorFormatterPtr_ = writer<vector>::New(dict.lookup("setFormat"));
    }
}


void CML::streamLine::execute()
{
//    const Time& runTime = obr_.time();
//    Pout<< "**streamLine::execute : time:" << runTime.timeName() << endl;
//
//    bool isOutputTime = false;
//
//    const functionObjectList& fobs = runTime.functionObjects();
//
//    forAll(fobs, i)
//    {
//        if (isA<streamLineFunctionObject>(fobs[i]))
//        {
//            const streamLineFunctionObject& fo =
//                dynamic_cast<const streamLineFunctionObject&>(fobs[i]);
//
//            if (fo.name() == name_)
//            {
//                Pout<< "found me:" << i << endl;
//                if (fo.outputControl().output())
//                {
//                    isOutputTime = true;
//                    break;
//                }
//            }
//        }
//    }
//
//
//    if (active_ && isOutputTime)
//    {
//        track();
//    }
}


void CML::streamLine::end()
{}


void CML::streamLine::timeSet()
{}


void CML::streamLine::write()
{
    if (active_)
    {
        Info<< type() << " " << name_ << " output:" << nl;

        const Time& runTime = obr_.time();
        const fvMesh& mesh = dynamic_cast<const fvMesh&>(obr_);


        // Do all injection and tracking
        track();


        if (Pstream::parRun())
        {
            // Append slave tracks to master ones
            // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

            globalIndex globalTrackIDs(allTracks_.size());

            // Construct a distribution map to pull all to the master.
            labelListList sendMap(Pstream::nProcs());
            labelListList recvMap(Pstream::nProcs());

            if (Pstream::master())
            {
                // Master: receive all. My own first, then consecutive
                // processors.
                label trackI = 0;

                forAll(recvMap, procI)
                {
                    labelList& fromProc = recvMap[procI];
                    fromProc.setSize(globalTrackIDs.localSize(procI));
                    forAll(fromProc, i)
                    {
                        fromProc[i] = trackI++;
                    }
                }
            }

            labelList& toMaster = sendMap[0];
            toMaster.setSize(globalTrackIDs.localSize());
            forAll(toMaster, i)
            {
                toMaster[i] = i;
            }

            const mapDistribute distMap
            (
                globalTrackIDs.size(),
                sendMap.xfer(),
                recvMap.xfer()
            );


            // Distribute the track positions. Note: use scheduled comms
            // to prevent buffering.
            allTracks_.shrink();
            mapDistributeBase::distribute
            (
                Pstream::scheduled,
                distMap.schedule(),
                distMap.constructSize(),
                distMap.subMap(),
                false,
                distMap.constructMap(),
                false,
                allTracks_,
                flipOp()
            );
            allTracks_.setCapacity(allTracks_.size());

            // Distribute the scalars
            forAll(allScalars_, scalarI)
            {
                allScalars_[scalarI].shrink();
                mapDistributeBase::distribute
                (
                    Pstream::scheduled,
                    distMap.schedule(),
                    distMap.constructSize(),
                    distMap.subMap(),
                    false,
                    distMap.constructMap(),
                    false,
                    allScalars_[scalarI],
                    flipOp()
                );
                allScalars_[scalarI].setCapacity(allScalars_[scalarI].size());
            }
            // Distribute the vectors
            forAll(allVectors_, vectorI)
            {
                allVectors_[vectorI].shrink();
                mapDistributeBase::distribute
                (
                    Pstream::scheduled,
                    distMap.schedule(),
                    distMap.constructSize(),
                    distMap.subMap(),
                    false,
                    distMap.constructMap(),
                    false,
                    allVectors_[vectorI],
                    flipOp()
                );
                allVectors_[vectorI].setCapacity(allVectors_[vectorI].size());
            }
        }


        label n = 0;
        forAll(allTracks_, trackI)
        {
            n += allTracks_[trackI].size();
        }

        Info<< "    Tracks:" << allTracks_.size() << nl
            << "    Total samples:" << n
            << endl;


        // Massage into form suitable for writers
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        if (Pstream::master() && allTracks_.size())
        {
            // Make output directory

            fileName vtkPath
            (
                Pstream::parRun()
              ? runTime.path()/".."/"postProcessing"/"sets"/name()
              : runTime.path()/"postProcessing"/"sets"/name()
            );
            if (mesh.name() != fvMesh::defaultRegion)
            {
                vtkPath = vtkPath/mesh.name();
            }
            vtkPath = vtkPath/mesh.time().timeName();

            mkDir(vtkPath);

            // Convert track positions

            PtrList<coordSet> tracks(allTracks_.size());
            forAll(allTracks_, trackI)
            {
                tracks.set
                (
                    trackI,
                    new coordSet
                    (
                        "track" + CML::name(trackI),
                        sampledSetAxis_                 //"xyz"
                    )
                );
                tracks[trackI].transfer(allTracks_[trackI]);
            }

            // Convert scalar values

            if (allScalars_.size() > 0)
            {
                List<List<scalarField> > scalarValues(allScalars_.size());

                forAll(allScalars_, scalarI)
                {
                    DynamicList<scalarList>& allTrackVals =
                        allScalars_[scalarI];
                    scalarValues[scalarI].setSize(allTrackVals.size());

                    forAll(allTrackVals, trackI)
                    {
                        scalarList& trackVals = allTrackVals[trackI];
                        scalarValues[scalarI][trackI].transfer(trackVals);
                    }
                }

                fileName vtkFile
                (
                    vtkPath
                  / scalarFormatterPtr_().getFileName
                    (
                        tracks[0],
                        scalarNames_
                    )
                );

                Info<< "    Writing data to " << vtkFile.path() << endl;

                scalarFormatterPtr_().write
                (
                    true,           // writeTracks
                    tracks,
                    scalarNames_,
                    scalarValues,
                    OFstream(vtkFile)()
                );
            }

            // Convert vector values

            if (allVectors_.size() > 0)
            {
                List<List<vectorField> > vectorValues(allVectors_.size());

                forAll(allVectors_, vectorI)
                {
                    DynamicList<vectorList>& allTrackVals =
                        allVectors_[vectorI];
                    vectorValues[vectorI].setSize(allTrackVals.size());

                    forAll(allTrackVals, trackI)
                    {
                        vectorList& trackVals = allTrackVals[trackI];
                        vectorValues[vectorI][trackI].transfer(trackVals);
                    }
                }

                fileName vtkFile
                (
                    vtkPath
                  / vectorFormatterPtr_().getFileName
                    (
                        tracks[0],
                        vectorNames_
                    )
                );

                //Info<< "    Writing vector data to " << vtkFile << endl;

                vectorFormatterPtr_().write
                (
                    true,           // writeTracks
                    tracks,
                    vectorNames_,
                    vectorValues,
                    OFstream(vtkFile)()
                );
            }
        }
    }
}


void CML::streamLine::updateMesh(const mapPolyMesh&)
{
    read(dict_);
}


void CML::streamLine::movePoints(const pointField&)
{
    // Moving mesh affects the search tree
    read(dict_);
}


//void CML::streamLine::readUpdate(const polyMesh::readUpdateState state)
//{
//    if (state != UNCHANGED)
//    {
//        read(dict_);
//    }
//}


// ************************************************************************* //
