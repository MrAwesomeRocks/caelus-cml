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

Class
    CML::PointEdgeWave

Description
    Wave propagation of information through grid. Every iteration
    information goes through one layer of edges. Templated on information
    that is transferred.

    Templated on information that is transferred.
    Handles parallel and cyclics. Only parallel reasonably tested. Cyclics
    hardly tested.

    Note: whether to propagate depends on the return value of Type::update
    which returns true (i.e. propagate) if the value changes by more than a
    certain tolerance.

    Note: parallel is done in two steps:
      -# transfer patch points in offset notation, i.e. every patch
         point is denoted by a patchface label and an index in this face.
         Receiving end uses that fact that f[0] is shared and order is
         reversed.
      -# do all non-local shared points by means of reduce of data on them.

    Note: cyclics is with offset in patchface as well. Patch is divided into
    two sub patches and the point-point addressing is never explicitly
    calculated but instead use is made of the face-face correspondence.
    (it probably is more efficient to calculate a point-point
    correspondence at the start and then reuse this; task to be done)

SourceFiles
    PointEdgeWave.cpp

\*---------------------------------------------------------------------------*/

#ifndef PointEdgeWave_H
#define PointEdgeWave_H

#include "boolList.hpp"
#include "scalarField.hpp"
#include "tensorField.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes
class polyMesh;
class polyPatch;

/*---------------------------------------------------------------------------*\
                        Class PointEdgeWaveName Declaration
\*---------------------------------------------------------------------------*/

TemplateName(PointEdgeWave);


/*---------------------------------------------------------------------------*\
                           Class PointEdgeWave Declaration
\*---------------------------------------------------------------------------*/

template <class Type, class TrackingData = int>
class PointEdgeWave
:
    public PointEdgeWaveName
{
  // Private static data

        //- Relative tolerance. Stop propagation if relative changes
        //  less than this tolerance (responsibility for checking this is
        //  up to Type implementation)
        static scalar propagationTol_;

        //- Used as default trackdata value to satisfy default template
        //  argument.
        static int dummyTrackData_;


    // Private data

        //- Reference to mesh
        const polyMesh& mesh_;

        //- Wall information for all points
        UList<Type>& allPointInfo_;

        //- Information on all mesh edges
        UList<Type>& allEdgeInfo_;

        //- Additional data to be passed into container
        TrackingData& td_;

        //- Has point changed
        boolList changedPoint_;

        //- List of changed points
        labelList changedPoints_;

        //- Number of changed points
        label nChangedPoints_;

        //- Edges that have changed
        boolList changedEdge_;
        labelList changedEdges_;
        label nChangedEdges_;

        //- Number of cyclic patches
        label nCyclicPatches_;

        //- Number of evaluations
        label nEvals_;

        //- Number of unvisited edges/points
        label nUnvisitedPoints_;
        label nUnvisitedEdges_;


    // Private Member Functions

        //- Adapt pointInfo for leaving domain
        void leaveDomain
        (
            const polyPatch&,
            const List<label>& patchPointLabels,
            List<Type>& pointInfo
        ) const;

        //- Adapt pointInfo for entering domain
        void enterDomain
        (
            const polyPatch&,
            const List<label>& patchPointLabels,
            List<Type>& pointInfo
        ) const;

        //- Transform. Implementation referred to Type
        void transform
        (
            const polyPatch& patch,
            const tensorField& rotTensor,
            List<Type>& pointInfo
        ) const;

        //- Updates pointInfo with information from neighbour. Updates all
        //  statistics.
        bool updatePoint
        (
            const label pointI,
            const label neighbourEdgeI,
            const Type& neighbourInfo,
            Type& pointInfo
        );

        //- Updates pointInfo with information from same point. Updates all
        //  statistics.
        bool updatePoint
        (
            const label pointI,
            const Type& neighbourInfo,
            Type& pointInfo
        );

        //- Updates edgeInfo with information from neighbour. Updates all
        //  statistics.
        bool updateEdge
        (
            const label edgeI,
            const label neighbourPointI,
            const Type& neighbourInfo,
            Type& edgeInfo
        );

        // Parallel, cyclic

            //- Has patches of certain type?
            template <class PatchType>
            label countPatchType() const;

            //- Merge data from across processor boundaries
            void handleProcPatches();

            //- Merge data from across cyclic boundaries
            void handleCyclicPatches();

            //- Explicitly sync all collocated points
            label handleCollocatedPoints();


        //- Disallow default bitwise copy construct
        PointEdgeWave(const PointEdgeWave&);

        //- Disallow default bitwise assignment
        void operator=(const PointEdgeWave&);


public:

    // Static Functions

        //- Access to tolerance
        static scalar propagationTol()
        {
            return propagationTol_;
        }

        //- Change tolerance
        static void setPropagationTol(const scalar tol)
        {
            propagationTol_ = tol;
        }


    // Constructors

        //- Construct from mesh, list of changed points with the Type
        //  for these points. Gets work arrays to operate on, one of size
        //  number of mesh points, the other number of mesh edges.
        //  Iterates until nothing changes or maxIter reached.
        //  (maxIter can be 0)
        PointEdgeWave
        (
            const polyMesh& mesh,
            const labelList& initialPoints,
            const List<Type>& initialPointsInfo,
            UList<Type>& allPointInfo,
            UList<Type>& allEdgeInfo,
            const label maxIter,
            TrackingData& td = dummyTrackData_
        );

        //- Construct from mesh. Use setPointInfo and iterate() to do
        //  actual calculation
        PointEdgeWave
        (
            const polyMesh& mesh,
            UList<Type>& allPointInfo,
            UList<Type>& allEdgeInfo,
            TrackingData& td = dummyTrackData_
        );


    //- Destructor
    ~PointEdgeWave();


    // Member Functions

        //- Access allPointInfo
        UList<Type>& allPointInfo() const
        {
            return allPointInfo_;
        }

        //- Access allEdgeInfo
        UList<Type>& allEdgeInfo() const
        {
            return allEdgeInfo_;
        }

        //- Additional data to be passed into container
        const TrackingData& data() const
        {
            return td_;
        }

        //- Get number of unvisited edges, i.e. edges that were not (yet)
        //  reached from walking across mesh. This can happen from
        //  - not enough iterations done
        //  - a disconnected mesh
        //  - a mesh without walls in it
        label getUnsetEdges() const;

        label getUnsetPoints() const;

        //- Copy initial data into allPointInfo_
        void setPointInfo
        (
            const labelList& changedPoints,
            const List<Type>& changedPointsInfo
        );

        //- Propagate from point to edge. Returns total number of edges
        //  (over all processors) changed.
        label pointToEdge();

        //- Propagate from edge to point. Returns total number of points
        //  (over all processors) changed.
        label edgeToPoint();

        //- Iterate until no changes or maxIter reached. Returns actual
        //  number of iterations.
        label iterate(const label maxIter);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

/*---------------------------------------------------------------------------*\
                        Class listUpdateOp Declaration
\*---------------------------------------------------------------------------*/

//- List update operation
template <class Type, class TrackingData = int>
class listUpdateOp
{
    //- Additional data to be passed into container

    const scalar tol_;

    TrackingData& td_;

public:
    listUpdateOp(const scalar tol, TrackingData& td)
    :
        tol_(tol),
        td_(td)
    {}

    void operator()(List<Type>& x, const List<Type>& y) const
    {
        forAll(x, i)
        {
            if (y[i].valid(td_))
            {
                x[i].updatePoint(y[i], tol_, td_);
            }
        }
    }
};

} // End namespace CML


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "polyMesh.hpp"
#include "processorPolyPatch.hpp"
#include "cyclicPolyPatch.hpp"
#include "OPstream.hpp"
#include "IPstream.hpp"
#include "PstreamCombineReduceOps.hpp"
#include "debug.hpp"
#include "typeInfo.hpp"
#include "globalMeshData.hpp"
#include "pointFields.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

template <class Type, class TrackingData>
CML::scalar CML::PointEdgeWave<Type, TrackingData>::propagationTol_ = 0.01;

template <class Type, class TrackingData>
int CML::PointEdgeWave<Type, TrackingData>::dummyTrackData_ = 12345;

namespace CML
{
    //- Reduction class. If x and y are not equal assign value.
    template<class Type, class TrackingData>
    class combineEqOp
    {
        TrackingData& td_;

        public:
            combineEqOp(TrackingData& td)
            :
                td_(td)
            {}

        void operator()(Type& x, const Type& y) const
        {
            if (!x.valid(td_) && y.valid(td_))
            {
                x = y;
            }
        }
    };
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// Handle leaving domain. Implementation referred to Type
template <class Type, class TrackingData>
void CML::PointEdgeWave<Type, TrackingData>::leaveDomain
(
    const polyPatch& patch,
    const labelList& patchPointLabels,
    List<Type>& pointInfo
) const
{
    const labelList& meshPoints = patch.meshPoints();

    forAll(patchPointLabels, i)
    {
        label patchPointI = patchPointLabels[i];

        const point& pt = patch.points()[meshPoints[patchPointI]];

        pointInfo[i].leaveDomain(patch, patchPointI, pt, td_);
    }
}


// Handle entering domain. Implementation referred to Type
template <class Type, class TrackingData>
void CML::PointEdgeWave<Type, TrackingData>::enterDomain
(
    const polyPatch& patch,
    const labelList& patchPointLabels,
    List<Type>& pointInfo
) const
{
    const labelList& meshPoints = patch.meshPoints();

    forAll(patchPointLabels, i)
    {
        label patchPointI = patchPointLabels[i];

        const point& pt = patch.points()[meshPoints[patchPointI]];

        pointInfo[i].enterDomain(patch, patchPointI, pt, td_);
    }
}


// Transform. Implementation referred to Type
template <class Type, class TrackingData>
void CML::PointEdgeWave<Type, TrackingData>::transform
(
    const polyPatch& patch,
    const tensorField& rotTensor,
    List<Type>& pointInfo
) const
{
    if (rotTensor.size() == 1)
    {
        const tensor& T = rotTensor[0];

        forAll(pointInfo, i)
        {
            pointInfo[i].transform(T, td_);
        }
    }
    else
    {
        FatalErrorInFunction
            << "Non-uniform transformation on patch " << patch.name()
            << " of type " << patch.type()
            << " not supported for point fields"
            << abort(FatalError);

        forAll(pointInfo, i)
        {
            pointInfo[i].transform(rotTensor[i], td_);
        }
    }
}


// Update info for pointI, at position pt, with information from
// neighbouring edge.
// Updates:
//      - changedPoint_, changedPoints_, nChangedPoints_,
//      - statistics: nEvals_, nUnvisitedPoints_
template <class Type, class TrackingData>
bool CML::PointEdgeWave<Type, TrackingData>::updatePoint
(
    const label pointI,
    const label neighbourEdgeI,
    const Type& neighbourInfo,
    Type& pointInfo
)
{
    nEvals_++;

    bool wasValid = pointInfo.valid(td_);

    bool propagate =
        pointInfo.updatePoint
        (
            mesh_,
            pointI,
            neighbourEdgeI,
            neighbourInfo,
            propagationTol_,
            td_
        );

    if (propagate)
    {
        if (!changedPoint_[pointI])
        {
            changedPoint_[pointI] = true;
            changedPoints_[nChangedPoints_++] = pointI;
        }
    }

    if (!wasValid && pointInfo.valid(td_))
    {
        --nUnvisitedPoints_;
    }

    return propagate;
}


// Update info for pointI, at position pt, with information from
// same point.
// Updates:
//      - changedPoint_, changedPoints_, nChangedPoints_,
//      - statistics: nEvals_, nUnvisitedPoints_
template <class Type, class TrackingData>
bool CML::PointEdgeWave<Type, TrackingData>::updatePoint
(
    const label pointI,
    const Type& neighbourInfo,
    Type& pointInfo
)
{
    nEvals_++;

    bool wasValid = pointInfo.valid(td_);

    bool propagate =
        pointInfo.updatePoint
        (
            mesh_,
            pointI,
            neighbourInfo,
            propagationTol_,
            td_
        );

    if (propagate)
    {
        if (!changedPoint_[pointI])
        {
            changedPoint_[pointI] = true;
            changedPoints_[nChangedPoints_++] = pointI;
        }
    }

    if (!wasValid && pointInfo.valid(td_))
    {
        --nUnvisitedPoints_;
    }

    return propagate;
}


// Update info for edgeI, at position pt, with information from
// neighbouring point.
// Updates:
//      - changedEdge_, changedEdges_, nChangedEdges_,
//      - statistics: nEvals_, nUnvisitedEdge_
template <class Type, class TrackingData>
bool CML::PointEdgeWave<Type, TrackingData>::updateEdge
(
    const label edgeI,
    const label neighbourPointI,
    const Type& neighbourInfo,
    Type& edgeInfo
)
{
    nEvals_++;

    bool wasValid = edgeInfo.valid(td_);

    bool propagate =
        edgeInfo.updateEdge
        (
            mesh_,
            edgeI,
            neighbourPointI,
            neighbourInfo,
            propagationTol_,
            td_
        );

    if (propagate)
    {
        if (!changedEdge_[edgeI])
        {
            changedEdge_[edgeI] = true;
            changedEdges_[nChangedEdges_++] = edgeI;
        }
    }

    if (!wasValid && edgeInfo.valid(td_))
    {
        --nUnvisitedEdges_;
    }

    return propagate;
}


// Check if patches of given type name are present
template <class Type, class TrackingData>
template <class PatchType>
CML::label CML::PointEdgeWave<Type, TrackingData>::countPatchType() const
{
    label nPatches = 0;

    forAll(mesh_.boundaryMesh(), patchI)
    {
        if (isA<PatchType>(mesh_.boundaryMesh()[patchI]))
        {
            nPatches++;
        }
    }
    return nPatches;
}


// Transfer all the information to/from neighbouring processors
template <class Type, class TrackingData>
void CML::PointEdgeWave<Type, TrackingData>::handleProcPatches()
{
    // 1. Send all point info on processor patches.

    PstreamBuffers pBufs(Pstream::nonBlocking);

    DynamicList<Type> patchInfo;
    DynamicList<label> thisPoints;
    DynamicList<label> nbrPoints;

    forAll(mesh_.globalData().processorPatches(), i)
    {
        label patchI = mesh_.globalData().processorPatches()[i];
        const processorPolyPatch& procPatch =
            refCast<const processorPolyPatch>(mesh_.boundaryMesh()[patchI]);

        patchInfo.clear();
        patchInfo.reserve(procPatch.nPoints());
        thisPoints.clear();
        thisPoints.reserve(procPatch.nPoints());
        nbrPoints.clear();
        nbrPoints.reserve(procPatch.nPoints());

        // Get all changed points in reverse order
        const labelList& neighbPoints = procPatch.neighbPoints();
        forAll(neighbPoints, thisPointI)
        {
            label meshPointI = procPatch.meshPoints()[thisPointI];
            if (changedPoint_[meshPointI])
            {
                patchInfo.append(allPointInfo_[meshPointI]);
                thisPoints.append(thisPointI);
                nbrPoints.append(neighbPoints[thisPointI]);
            }
        }

        // Adapt for leaving domain
        leaveDomain(procPatch, thisPoints, patchInfo);

        if (debug)
        {
            Pout<< "Processor patch " << patchI << ' ' << procPatch.name()
                << " communicating with " << procPatch.neighbProcNo()
                << "  Sending:" << patchInfo.size() << endl;
        }

        UOPstream toNeighbour(procPatch.neighbProcNo(), pBufs);
        toNeighbour << nbrPoints << patchInfo;
    }


    pBufs.finishedSends();

    //
    // 2. Receive all point info on processor patches.
    //

    forAll(mesh_.globalData().processorPatches(), i)
    {
        label patchI = mesh_.globalData().processorPatches()[i];
        const processorPolyPatch& procPatch =
            refCast<const processorPolyPatch>(mesh_.boundaryMesh()[patchI]);

        List<Type> patchInfo;
        labelList patchPoints;

        {
            UIPstream fromNeighbour(procPatch.neighbProcNo(), pBufs);
            fromNeighbour >> patchPoints >> patchInfo;
        }

        if (debug)
        {
            Pout<< "Processor patch " << patchI << ' ' << procPatch.name()
                << " communicating with " << procPatch.neighbProcNo()
                << "  Received:" << patchInfo.size() << endl;
        }

        // Apply transform to received data for non-parallel planes
        if (!procPatch.parallel())
        {
            transform(procPatch, procPatch.forwardT(), patchInfo);
        }

        // Adapt for entering domain
        enterDomain(procPatch, patchPoints, patchInfo);

        // Merge received info
        const labelList& meshPoints = procPatch.meshPoints();
        forAll(patchInfo, i)
        {
            label meshPointI = meshPoints[patchPoints[i]];

            if (!allPointInfo_[meshPointI].equal(patchInfo[i], td_))
            {
                updatePoint
                (
                    meshPointI,
                    patchInfo[i],
                    allPointInfo_[meshPointI]
                );
            }
        }
    }

    // Collocated points should be handled by face based transfer
    // (since that is how connectivity is worked out)
    // They are also explicitly equalised in handleCollocatedPoints to
    // guarantee identical values.
}


template <class Type, class TrackingData>
void CML::PointEdgeWave<Type, TrackingData>::handleCyclicPatches()
{
    // 1. Send all point info on cyclic patches.

    DynamicList<Type> nbrInfo;
    DynamicList<label> nbrPoints;
    DynamicList<label> thisPoints;

    forAll(mesh_.boundaryMesh(), patchI)
    {
        const polyPatch& patch = mesh_.boundaryMesh()[patchI];

        if (isA<cyclicPolyPatch>(patch))
        {
            const cyclicPolyPatch& cycPatch =
                refCast<const cyclicPolyPatch>(patch);

            nbrInfo.clear();
            nbrInfo.reserve(cycPatch.nPoints());
            nbrPoints.clear();
            nbrPoints.reserve(cycPatch.nPoints());
            thisPoints.clear();
            thisPoints.reserve(cycPatch.nPoints());

            // Collect nbrPatch points that have changed
            {
                const cyclicPolyPatch& nbrPatch = cycPatch.neighbPatch();
                const edgeList& pairs = cycPatch.coupledPoints();
                const labelList& meshPoints = nbrPatch.meshPoints();

                forAll(pairs, pairI)
                {
                    label thisPointI = pairs[pairI][0];
                    label nbrPointI = pairs[pairI][1];
                    label meshPointI = meshPoints[nbrPointI];

                    if (changedPoint_[meshPointI])
                    {
                        nbrInfo.append(allPointInfo_[meshPointI]);
                        nbrPoints.append(nbrPointI);
                        thisPoints.append(thisPointI);
                    }
                }

                // nbr : adapt for leaving domain
                leaveDomain(nbrPatch, nbrPoints, nbrInfo);
            }


            // Apply rotation for non-parallel planes

            if (!cycPatch.parallel())
            {
                // received data from half1
                transform(cycPatch, cycPatch.forwardT(), nbrInfo);
            }

            if (debug)
            {
                Pout<< "Cyclic patch " << patchI << ' ' << patch.name()
                    << "  Changed : " << nbrInfo.size()
                    << endl;
            }

            // Adapt for entering domain
            enterDomain(cycPatch, thisPoints, nbrInfo);

            // Merge received info
            const labelList& meshPoints = cycPatch.meshPoints();
            forAll(nbrInfo, i)
            {
                label meshPointI = meshPoints[thisPoints[i]];

                if (!allPointInfo_[meshPointI].equal(nbrInfo[i], td_))
                {
                    updatePoint
                    (
                        meshPointI,
                        nbrInfo[i],
                        allPointInfo_[meshPointI]
                    );
                }
            }
        }
    }
}


// Guarantee collocated points have same information.
// Return number of points changed.
template<class Type, class TrackingData>
CML::label CML::PointEdgeWave<Type, TrackingData>::handleCollocatedPoints()
{
    // Transfer onto coupled patch
    const globalMeshData& gmd = mesh_.globalData();
    const indirectPrimitivePatch& cpp = gmd.coupledPatch();
    const labelList& meshPoints = cpp.meshPoints();

    const mapDistribute& slavesMap = gmd.globalPointSlavesMap();
    const labelListList& slaves = gmd.globalPointSlaves();

    List<Type> elems(slavesMap.constructSize());
    forAll(meshPoints, pointi)
    {
        elems[pointi] = allPointInfo_[meshPoints[pointi]];
    }

    // Pull slave data onto master (which might or might not have any
    // initialised points). No need to update transformed slots.
    slavesMap.distribute(elems, false);

    // Combine master data with slave data
    combineEqOp<Type, TrackingData> cop(td_);

    forAll(slaves, pointi)
    {
        Type& elem = elems[pointi];

        const labelList& slavePoints = slaves[pointi];

        // Combine master with untransformed slave data
        forAll(slavePoints, j)
        {
            cop(elem, elems[slavePoints[j]]);
        }

        // Copy result back to slave slots
        forAll(slavePoints, j)
        {
            elems[slavePoints[j]] = elem;
        }
    }

    // Push slave-slot data back to slaves
    slavesMap.reverseDistribute(elems.size(), elems, false);

    // Extract back onto mesh
    forAll(meshPoints, pointi)
    {
        if (elems[pointi].valid(td_))
        {
            label meshPointi = meshPoints[pointi];

            Type& elem = allPointInfo_[meshPointi];

            bool wasValid = elem.valid(td_);

            // Like updatePoint but bypass Type::updatePoint with its tolerance
            // checking
            //if (!elem.valid(td_) || !elem.equal(elems[pointi], td_))
            if (!elem.equal(elems[pointi], td_))
            {
                nEvals_++;
                elem = elems[pointi];

                // See if element now valid
                if (!wasValid && elem.valid(td_))
                {
                    --nUnvisitedPoints_;
                }

                // Update database of changed points
                if (!changedPoint_[meshPointi])
                {
                    changedPoint_[meshPointi] = true;
                    changedPoints_[nChangedPoints_++] = meshPointi;
                }
            }
        }
    }

    // Sum nChangedPoints over all procs
    label totNChanged = nChangedPoints_;

    reduce(totNChanged, sumOp<label>());

    return totNChanged;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Iterate, propagating changedPointsInfo across mesh, until no change (or
// maxIter reached). Initial point values specified.
template <class Type, class TrackingData>
CML::PointEdgeWave<Type, TrackingData>::PointEdgeWave
(
    const polyMesh& mesh,
    const labelList& changedPoints,
    const List<Type>& changedPointsInfo,

    UList<Type>& allPointInfo,
    UList<Type>& allEdgeInfo,

    const label maxIter,
    TrackingData& td
)
:
    mesh_(mesh),
    allPointInfo_(allPointInfo),
    allEdgeInfo_(allEdgeInfo),
    td_(td),
    changedPoint_(mesh_.nPoints(), false),
    changedPoints_(mesh_.nPoints()),
    nChangedPoints_(0),
    changedEdge_(mesh_.nEdges(), false),
    changedEdges_(mesh_.nEdges()),
    nChangedEdges_(0),
    nCyclicPatches_(countPatchType<cyclicPolyPatch>()),
    nEvals_(0),
    nUnvisitedPoints_(mesh_.nPoints()),
    nUnvisitedEdges_(mesh_.nEdges())
{
    if (allPointInfo_.size() != mesh_.nPoints())
    {
        FatalErrorInFunction
            << "size of pointInfo work array is not equal to the number"
            << " of points in the mesh" << endl
            << "    pointInfo   :" << allPointInfo_.size() << endl
            << "    mesh.nPoints:" << mesh_.nPoints()
            << exit(FatalError);
    }
    if (allEdgeInfo_.size() != mesh_.nEdges())
    {
        FatalErrorInFunction
            << "size of edgeInfo work array is not equal to the number"
            << " of edges in the mesh" << endl
            << "    edgeInfo   :" << allEdgeInfo_.size() << endl
            << "    mesh.nEdges:" << mesh_.nEdges()
            << exit(FatalError);
    }


    // Set from initial changed points data
    setPointInfo(changedPoints, changedPointsInfo);

    if (debug)
    {
        Info<< typeName << ": Seed points               : "
            << returnReduce(nChangedPoints_, sumOp<label>()) << endl;
    }

    // Iterate until nothing changes
    label iter = iterate(maxIter);

    if ((maxIter > 0) && (iter >= maxIter))
    {
        FatalErrorInFunction
            << "Maximum number of iterations reached. Increase maxIter." << endl
            << "    maxIter:" << maxIter << endl
            << "    nChangedPoints:" << nChangedPoints_ << endl
            << "    nChangedEdges:" << nChangedEdges_ << endl
            << exit(FatalError);
    }
}


template <class Type, class TrackingData>
CML::PointEdgeWave<Type, TrackingData>::PointEdgeWave
(
    const polyMesh& mesh,
    UList<Type>& allPointInfo,
    UList<Type>& allEdgeInfo,
    TrackingData& td
)
:
    mesh_(mesh),
    allPointInfo_(allPointInfo),
    allEdgeInfo_(allEdgeInfo),
    td_(td),
    changedPoint_(mesh_.nPoints(), false),
    changedPoints_(mesh_.nPoints()),
    nChangedPoints_(0),
    changedEdge_(mesh_.nEdges(), false),
    changedEdges_(mesh_.nEdges()),
    nChangedEdges_(0),
    nCyclicPatches_(countPatchType<cyclicPolyPatch>()),
    nEvals_(0),
    nUnvisitedPoints_(mesh_.nPoints()),
    nUnvisitedEdges_(mesh_.nEdges())
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template <class Type, class TrackingData>
CML::PointEdgeWave<Type, TrackingData>::~PointEdgeWave()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


template <class Type, class TrackingData>
CML::label CML::PointEdgeWave<Type, TrackingData>::getUnsetPoints() const
{
    return nUnvisitedPoints_;
}


template <class Type, class TrackingData>
CML::label CML::PointEdgeWave<Type, TrackingData>::getUnsetEdges() const
{
    return nUnvisitedEdges_;
}


// Copy point information into member data
template <class Type, class TrackingData>
void CML::PointEdgeWave<Type, TrackingData>::setPointInfo
(
    const labelList& changedPoints,
    const List<Type>& changedPointsInfo
)
{
    forAll(changedPoints, changedPointI)
    {
        label pointI = changedPoints[changedPointI];

        bool wasValid = allPointInfo_[pointI].valid(td_);

        // Copy info for pointI
        allPointInfo_[pointI] = changedPointsInfo[changedPointI];

        // Maintain count of unset points
        if (!wasValid && allPointInfo_[pointI].valid(td_))
        {
            --nUnvisitedPoints_;
        }

        // Mark pointI as changed, both on list and on point itself.

        if (!changedPoint_[pointI])
        {
            changedPoint_[pointI] = true;
            changedPoints_[nChangedPoints_++] = pointI;
        }
    }

    // Sync
    handleCollocatedPoints();
}


// Propagate information from edge to point. Return number of points changed.
template <class Type, class TrackingData>
CML::label CML::PointEdgeWave<Type, TrackingData>::edgeToPoint()
{
    for
    (
        label changedEdgeI = 0;
        changedEdgeI < nChangedEdges_;
        changedEdgeI++
    )
    {
        label edgeI = changedEdges_[changedEdgeI];

        if (!changedEdge_[edgeI])
        {
            FatalErrorInFunction
                << "edge " << edgeI
                << " not marked as having been changed" << nl
                << "This might be caused by multiple occurrences of the same"
                << " seed point." << abort(FatalError);
        }


        const Type& neighbourWallInfo = allEdgeInfo_[edgeI];

        // Evaluate all connected points (= edge endpoints)
        const edge& e = mesh_.edges()[edgeI];

        forAll(e, eI)
        {
            Type& currentWallInfo = allPointInfo_[e[eI]];

            if (!currentWallInfo.equal(neighbourWallInfo, td_))
            {
                updatePoint
                (
                    e[eI],
                    edgeI,
                    neighbourWallInfo,
                    currentWallInfo
                );
            }
        }

        // Reset status of edge
        changedEdge_[edgeI] = false;
    }

    // Handled all changed edges by now
    nChangedEdges_ = 0;

    if (nCyclicPatches_ > 0)
    {
        // Transfer changed points across cyclic halves
        handleCyclicPatches();
    }
    if (Pstream::parRun())
    {
        // Transfer changed points from neighbouring processors.
        handleProcPatches();
    }

    if (debug)
    {
        Pout<< "Changed points            : " << nChangedPoints_ << endl;
    }

    // Sum nChangedPoints over all procs
    label totNChanged = nChangedPoints_;

    reduce(totNChanged, sumOp<label>());

    return totNChanged;
}


// Propagate information from point to edge. Return number of edges changed.
template <class Type, class TrackingData>
CML::label CML::PointEdgeWave<Type, TrackingData>::pointToEdge()
{
    const labelListList& pointEdges = mesh_.pointEdges();

    for
    (
        label changedPointI = 0;
        changedPointI < nChangedPoints_;
        changedPointI++
    )
    {
        label pointI = changedPoints_[changedPointI];

        if (!changedPoint_[pointI])
        {
            FatalErrorInFunction
                << "Point " << pointI
                << " not marked as having been changed" << nl
                << "This might be caused by multiple occurrences of the same"
                << " seed point." << abort(FatalError);
        }

        const Type& neighbourWallInfo = allPointInfo_[pointI];

        // Evaluate all connected edges

        const labelList& edgeLabels = pointEdges[pointI];
        forAll(edgeLabels, edgeLabelI)
        {
            label edgeI = edgeLabels[edgeLabelI];

            Type& currentWallInfo = allEdgeInfo_[edgeI];

            if (!currentWallInfo.equal(neighbourWallInfo, td_))
            {
                updateEdge
                (
                    edgeI,
                    pointI,
                    neighbourWallInfo,
                    currentWallInfo
                );
            }
        }

        // Reset status of point
        changedPoint_[pointI] = false;
    }

    // Handled all changed points by now
    nChangedPoints_ = 0;

    if (debug)
    {
        Pout<< "Changed edges             : " << nChangedEdges_ << endl;
    }

    // Sum nChangedPoints over all procs
    label totNChanged = nChangedEdges_;

    reduce(totNChanged, sumOp<label>());

    return totNChanged;
}


// Iterate
template <class Type, class TrackingData>
CML::label CML::PointEdgeWave<Type, TrackingData>::iterate
(
    const label maxIter
)
{
    if (nCyclicPatches_ > 0)
    {
        // Transfer changed points across cyclic halves
        handleCyclicPatches();
    }
    if (Pstream::parRun())
    {
        // Transfer changed points from neighbouring processors.
        handleProcPatches();
    }

    nEvals_ = 0;

    label iter = 0;

    while (iter < maxIter)
    {
        while (iter < maxIter)
        {
            if (debug)
            {
                Info<< typeName << ": Iteration " << iter << endl;
            }

            label nEdges = pointToEdge();

            if (debug)
            {
                Info<< typeName << ": Total changed edges       : "
                    << nEdges << endl;
            }

            if (nEdges == 0)
            {
                break;
            }

            label nPoints = edgeToPoint();

            if (debug)
            {
                Info<< typeName << ": Total changed points      : "
                    << nPoints << nl
                    << typeName << ": Total evaluations         : "
                    << returnReduce(nEvals_, sumOp<label>()) << nl
                    << typeName << ": Remaining unvisited points: "
                    << returnReduce(nUnvisitedPoints_, sumOp<label>()) << nl
                    << typeName << ": Remaining unvisited edges : "
                    << returnReduce(nUnvisitedEdges_, sumOp<label>()) << nl
                    << endl;
            }

            if (nPoints == 0)
            {
                break;
            }

            iter++;
        }


        // Enforce collocated points are exactly equal. This might still mean
        // non-collocated points are not equal though. WIP.
        label nPoints = handleCollocatedPoints();
        if (debug)
        {
            Info<< typeName << ": Collocated point sync     : "
                << nPoints << nl << endl;
        }

        if (nPoints == 0)
        {
            break;
        }
    }

    return iter;
}



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
