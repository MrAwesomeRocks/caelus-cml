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

#include "globalMeshData.hpp"
#include "Time.hpp"
#include "Pstream.hpp"
#include "PstreamCombineReduceOps.hpp"
#include "processorPolyPatch.hpp"
#include "demandDrivenData.hpp"
#include "globalPoints.hpp"
#include "polyMesh.hpp"
#include "mapDistribute.hpp"
#include "labelIOList.hpp"
#include "PackedList.hpp"
#include "mergePoints.hpp"
#include "matchPoints.hpp"
#include "OFstream.hpp"
#include "globalIndexAndTransform.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


namespace CML
{
    defineTypeNameAndDebug(globalMeshData, 0);
}


// Geometric matching tolerance. Factor of mesh bounding box.
const CML::scalar CML::globalMeshData::matchTol_ = 1E-8;

namespace CML
{
template<>
class minEqOp<labelPair>
{
public:
    void operator()(labelPair& x, const labelPair& y) const
    {
        x[0] = min(x[0], y[0]);
        x[1] = min(x[1], y[1]);
    }
};
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// Collect processor patch addressing.
void CML::globalMeshData::initProcAddr()
{
    processorPatchIndices_.setSize(mesh_.boundaryMesh().size());
    processorPatchIndices_ = -1;

    processorPatchNeighbours_.setSize(mesh_.boundaryMesh().size());
    processorPatchNeighbours_ = -1;

    // Construct processor patch indexing. processorPatchNeighbours_ only
    // set if running in parallel!
    processorPatches_.setSize(mesh_.boundaryMesh().size());

    label nNeighbours = 0;

    forAll(mesh_.boundaryMesh(), patchi)
    {
        if (isA<processorPolyPatch>(mesh_.boundaryMesh()[patchi]))
        {
            processorPatches_[nNeighbours] = patchi;
            processorPatchIndices_[patchi] = nNeighbours++;
        }
    }
    processorPatches_.setSize(nNeighbours);


    if (Pstream::parRun())
    {
        PstreamBuffers pBufs(Pstream::nonBlocking);

        // Send indices of my processor patches to my neighbours
        forAll(processorPatches_, i)
        {
            label patchi = processorPatches_[i];

            UOPstream toNeighbour
            (
                refCast<const processorPolyPatch>
                (
                    mesh_.boundaryMesh()[patchi]
                ).neighbProcNo(),
                pBufs
            );

            toNeighbour << processorPatchIndices_[patchi];
        }

        pBufs.finishedSends();

        forAll(processorPatches_, i)
        {
            label patchi = processorPatches_[i];

            UIPstream fromNeighbour
            (
                refCast<const processorPolyPatch>
                (
                    mesh_.boundaryMesh()[patchi]
                ).neighbProcNo(),
                pBufs
            );

            fromNeighbour >> processorPatchNeighbours_[patchi];
        }
    }
}


void CML::globalMeshData::calcSharedPoints() const
{
    if
    (
        nGlobalPoints_ != -1
     || sharedPointLabelsPtr_.valid()
     || sharedPointAddrPtr_.valid()
    )
    {
        FatalErrorInFunction
            << "Shared point addressing already done" << abort(FatalError);
    }

    // Calculate all shared points (exclude points that are only
    // on two coupled patches). This does all the hard work.
    globalPoints parallelPoints(mesh_, false, true);

    // Count the number of master points
    label nMaster = 0;
    forAll(parallelPoints.pointPoints(), i)
    {
        const labelList& pPoints = parallelPoints.pointPoints()[i];
        const labelList& transPPoints =
            parallelPoints.transformedPointPoints()[i];

        if (pPoints.size()+transPPoints.size() > 0)
        {
            nMaster++;
        }
    }

    // Allocate global numbers
    globalIndex masterNumbering(nMaster);

    nGlobalPoints_ = masterNumbering.size();


    // Push master number to slaves
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // 1. Fill master and slave slots
    nMaster = 0;
    labelList master(parallelPoints.map().constructSize(), -1);
    forAll(parallelPoints.pointPoints(), i)
    {
        const labelList& pPoints = parallelPoints.pointPoints()[i];
        const labelList& transPPoints =
            parallelPoints.transformedPointPoints()[i];

        if (pPoints.size()+transPPoints.size() > 0)
        {
            master[i] = masterNumbering.toGlobal(nMaster);
            forAll(pPoints, j)
            {
                master[pPoints[j]] = master[i];
            }
            forAll(transPPoints, j)
            {
                master[transPPoints[j]] = master[i];
            }
            nMaster++;
        }
    }


    // 2. Push slave slots back to local storage on originating processor
    // For all the four types of points:
    // - local master : already set
    // - local transformed slave point : the reverse transform at
    //   reverseDistribute will have copied it back to its originating local
    //   point
    // - remote untransformed slave point : sent back to originating processor
    // - remote transformed slave point : the reverse transform will
    //   copy it back into the remote slot which then gets sent back to
    //   originating processor

    parallelPoints.map().reverseDistribute
    (
        parallelPoints.map().constructSize(),
        master
    );


    // Collect all points that are a master or refer to a master.
    nMaster = 0;
    forAll(parallelPoints.pointPoints(), i)
    {
        if (master[i] != -1)
        {
            nMaster++;
        }
    }

    sharedPointLabelsPtr_.reset(new labelList(nMaster));
    labelList& sharedPointLabels = sharedPointLabelsPtr_();
    sharedPointAddrPtr_.reset(new labelList(nMaster));
    labelList& sharedPointAddr = sharedPointAddrPtr_();
    nMaster = 0;

    forAll(parallelPoints.pointPoints(), i)
    {
        if (master[i] != -1)
        {
            // I am master or slave
            sharedPointLabels[nMaster] = i;
            sharedPointAddr[nMaster] = master[i];
            nMaster++;
        }
    }

    if (debug)
    {
        Pout<< "globalMeshData : nGlobalPoints_:" << nGlobalPoints_ << nl
            << "globalMeshData : sharedPointLabels_:"
            << sharedPointLabelsPtr_().size() << nl
            << "globalMeshData : sharedPointAddr_:"
            << sharedPointAddrPtr_().size() << endl;
    }
}


// Given information about locally used edges allocate global shared edges.
void CML::globalMeshData::countSharedEdges
(
    const EdgeMap<labelList>& procSharedEdges,
    EdgeMap<label>& globalShared,
    label& sharedEdgeI
)
{
    // Count occurrences of procSharedEdges in global shared edges table.
    forAllConstIter(EdgeMap<labelList>, procSharedEdges, iter)
    {
        const edge& e = iter.key();

        EdgeMap<label>::iterator globalFnd = globalShared.find(e);

        if (globalFnd == globalShared.end())
        {
            // First time occurrence of this edge. Check how many we are adding.
            if (iter().size() == 1)
            {
                // Only one edge. Mark with special value.
                globalShared.insert(e, -1);
            }
            else
            {
                // Edge used more than once (even by local shared edges alone)
                // so allocate proper shared edge label.
                globalShared.insert(e, sharedEdgeI++);
            }
        }
        else
        {
            if (globalFnd() == -1)
            {
                // Second time occurrence of this edge. Assign proper
                // edge label.
                globalFnd() = sharedEdgeI++;
            }
        }
    }
}


// Shared edges are shared between multiple processors. By their nature both
// of their endpoints are shared points. (but not all edges using two shared
// points are shared edges! There might e.g. be an edge between two unrelated
// clusters of shared points)
void CML::globalMeshData::calcSharedEdges() const
{
    if
    (
        nGlobalEdges_ != -1
     || sharedEdgeLabelsPtr_.valid()
     || sharedEdgeAddrPtr_.valid()
    )
    {
        FatalErrorInFunction
            << "Shared edge addressing already done" << abort(FatalError);
    }


    const labelList& sharedPtAddr = sharedPointAddr();
    const labelList& sharedPtLabels = sharedPointLabels();

    // Since don't want to construct pointEdges for whole mesh create
    // Map for all shared points.
    Map<label> meshToShared(2*sharedPtLabels.size());
    forAll(sharedPtLabels, i)
    {
        meshToShared.insert(sharedPtLabels[i], i);
    }

    // Find edges using shared points. Store correspondence to local edge
    // numbering. Note that multiple local edges can have the same shared
    // points! (for cyclics or separated processor patches)
    EdgeMap<labelList> localShared(2*sharedPtAddr.size());

    const edgeList& edges = mesh_.edges();

    forAll(edges, edgeI)
    {
        const edge& e = edges[edgeI];

        Map<label>::const_iterator e0Fnd = meshToShared.find(e[0]);

        if (e0Fnd != meshToShared.end())
        {
            Map<label>::const_iterator e1Fnd = meshToShared.find(e[1]);

            if (e1Fnd != meshToShared.end())
            {
                // Found edge which uses shared points. Probably shared.

                // Construct the edge in shared points (or rather global indices
                // of the shared points)
                edge sharedEdge
                (
                    sharedPtAddr[e0Fnd()],
                    sharedPtAddr[e1Fnd()]
                );

                EdgeMap<labelList>::iterator iter =
                    localShared.find(sharedEdge);

                if (iter == localShared.end())
                {
                    // First occurrence of this point combination. Store.
                    localShared.insert(sharedEdge, labelList(1, edgeI));
                }
                else
                {
                    // Add this edge to list of edge labels.
                    labelList& edgeLabels = iter();

                    label sz = edgeLabels.size();
                    edgeLabels.setSize(sz+1);
                    edgeLabels[sz] = edgeI;
                }
            }
        }
    }


    // Now we have a table on every processors which gives its edges which use
    // shared points. Send this all to the master and have it allocate
    // global edge numbers for it. But only allocate a global edge number for
    // edge if it is used more than once!
    // Note that we are now sending the whole localShared to the master whereas
    // we only need the local count (i.e. the number of times a global edge is
    // used). But then this only gets done once so not too bothered about the
    // extra global communication.

    EdgeMap<label> globalShared(nGlobalPoints());

    if (Pstream::master())
    {
        label sharedEdgeI = 0;

        // Merge my shared edges into the global list
        if (debug)
        {
            Pout<< "globalMeshData::calcSharedEdges : Merging in from proc0 : "
                << localShared.size() << endl;
        }
        countSharedEdges(localShared, globalShared, sharedEdgeI);

        // Receive data from slaves and insert
        if (Pstream::parRun())
        {
            for
            (
                int slave=Pstream::firstSlave();
                slave<=Pstream::lastSlave();
                slave++
            )
            {
                // Receive the edges using shared points from the slave.
                IPstream fromSlave(Pstream::blocking, slave);
                EdgeMap<labelList> procSharedEdges(fromSlave);

                if (debug)
                {
                    Pout<< "globalMeshData::calcSharedEdges : "
                        << "Merging in from proc"
                        << CML::name(slave) << " : " << procSharedEdges.size()
                        << endl;
                }
                countSharedEdges(procSharedEdges, globalShared, sharedEdgeI);
            }
        }

        // Now our globalShared should have some edges with -1 as edge label
        // These were only used once so are not proper shared edges.
        // Remove them.
        {
            EdgeMap<label> oldSharedEdges(globalShared);

            globalShared.clear();

            forAllConstIter(EdgeMap<label>, oldSharedEdges, iter)
            {
                if (iter() != -1)
                {
                    globalShared.insert(iter.key(), iter());
                }
            }
            if (debug)
            {
                Pout<< "globalMeshData::calcSharedEdges : Filtered "
                    << oldSharedEdges.size()
                    << " down to " << globalShared.size() << endl;
            }
        }


        // Send back to slaves.
        if (Pstream::parRun())
        {
            for
            (
                int slave=Pstream::firstSlave();
                slave<=Pstream::lastSlave();
                slave++
            )
            {
                // Receive the edges using shared points from the slave.
                OPstream toSlave(Pstream::blocking, slave);
                toSlave << globalShared;
            }
        }
    }
    else
    {
        // Send local edges to master
        {
            OPstream toMaster(Pstream::blocking, Pstream::masterNo());

            toMaster << localShared;
        }
        // Receive merged edges from master.
        {
            IPstream fromMaster(Pstream::blocking, Pstream::masterNo());

            fromMaster >> globalShared;
        }
    }

    // Now use the global shared edges list (globalShared) to classify my local
    // ones (localShared)

    nGlobalEdges_ = globalShared.size();

    DynamicList<label> dynSharedEdgeLabels(globalShared.size());
    DynamicList<label> dynSharedEdgeAddr(globalShared.size());

    forAllConstIter(EdgeMap<labelList>, localShared, iter)
    {
        const edge& e = iter.key();

        EdgeMap<label>::const_iterator edgeFnd = globalShared.find(e);

        if (edgeFnd != globalShared.end())
        {
            // My local edge is indeed a shared one. Go through all local edge
            // labels with this point combination.
            const labelList& edgeLabels = iter();

            forAll(edgeLabels, i)
            {
                // Store label of local mesh edge
                dynSharedEdgeLabels.append(edgeLabels[i]);

                // Store label of shared edge
                dynSharedEdgeAddr.append(edgeFnd());
            }
        }
    }

    sharedEdgeLabelsPtr_.reset(new labelList());
    labelList& sharedEdgeLabels = sharedEdgeLabelsPtr_();
    sharedEdgeLabels.transfer(dynSharedEdgeLabels);

    sharedEdgeAddrPtr_.reset(new labelList());
    labelList& sharedEdgeAddr = sharedEdgeAddrPtr_();
    sharedEdgeAddr.transfer(dynSharedEdgeAddr);

    if (debug)
    {
        Pout<< "globalMeshData : nGlobalEdges_:" << nGlobalEdges_ << nl
            << "globalMeshData : sharedEdgeLabels:" << sharedEdgeLabels.size()
            << nl
            << "globalMeshData : sharedEdgeAddr:" << sharedEdgeAddr.size()
            << endl;
    }
}


void CML::globalMeshData::calcGlobalPointSlaves() const
{
    if (debug)
    {
        Pout<< "globalMeshData::calcGlobalPointSlaves() :"
            << " calculating coupled master to slave point addressing."
            << endl;
    }

    // Calculate connected points for master points.
    globalPoints globalData(mesh_, coupledPatch(), true, true);

    globalPointSlavesPtr_.reset
    (
        new labelListList
        (
            globalData.pointPoints().xfer()
        )
    );
    globalPointTransformedSlavesPtr_.reset
    (
        new labelListList
        (
            globalData.transformedPointPoints().xfer()
        )
    );

    globalPointSlavesMapPtr_.reset
    (
        new mapDistribute
        (
            globalData.map().xfer()
        )
    );
}


void CML::globalMeshData::calcPointConnectivity
(
    List<labelPairList>& allPointConnectivity
) const
{
    const globalIndexAndTransform& transforms = globalTransforms();
    const labelListList& slaves = globalPointSlaves();
    const labelListList& transformedSlaves = globalPointTransformedSlaves();


    // Create field with my local data
    labelPairList myData(globalPointSlavesMap().constructSize());
    forAll(slaves, pointI)
    {
        myData[pointI] = globalIndexAndTransform::encode
        (
            Pstream::myProcNo(),
            pointI,
            transforms.nullTransformIndex()
        );
    }
    // Send over.
    globalPointSlavesMap().distribute(myData);


    // String of connected points with their transform
    allPointConnectivity.setSize(globalPointSlavesMap().constructSize());
    forAll(slaves, pointI)
    {
        // Reconstruct string of connected points
        const labelList& pSlaves = slaves[pointI];
        const labelList& pTransformSlaves = transformedSlaves[pointI];
        labelPairList& pConnectivity = allPointConnectivity[pointI];
        pConnectivity.setSize(1+pSlaves.size()+pTransformSlaves.size());
        label connI = 0;

        // Add myself
        pConnectivity[connI++] = myData[pointI];
        // Add untransformed points
        forAll(pSlaves, i)
        {
            pConnectivity[connI++] = myData[pSlaves[i]];
        }
        // Add transformed points.
        forAll(pTransformSlaves, i)
        {
            // Get transform from index
            label transformI = globalPointSlavesMap().whichTransform
            (
                pTransformSlaves[i]
            );
            // Add transform to connectivity
            const labelPair& n = myData[pTransformSlaves[i]];
            label procI = globalIndexAndTransform::processor(n);
            label index = globalIndexAndTransform::index(n);
            pConnectivity[connI++] = globalIndexAndTransform::encode
            (
                procI,
                index,
                transformI
            );
        }

        // Put back in slots
        forAll(pSlaves, i)
        {
            allPointConnectivity[pSlaves[i]] = pConnectivity;
        }
        forAll(pTransformSlaves, i)
        {
            allPointConnectivity[pTransformSlaves[i]] = pConnectivity;
        }
    }
    globalPointSlavesMap().reverseDistribute
    (
        allPointConnectivity.size(),
        allPointConnectivity
    );
}


void CML::globalMeshData::calcGlobalPointEdges
(
    labelListList& globalPointEdges,
    List<labelPairList>& globalPointPoints
) const
{
    const edgeList& edges = coupledPatch().edges();
    const labelListList& pointEdges = coupledPatch().pointEdges();
    const globalIndex& globalEdgeNumbers = globalEdgeNumbering();
    const labelListList& slaves = globalPointSlaves();
    const labelListList& transformedSlaves = globalPointTransformedSlaves();

    // Create local version
    globalPointEdges.setSize(globalPointSlavesMap().constructSize());
    globalPointPoints.setSize(globalPointSlavesMap().constructSize());
    forAll(pointEdges, pointI)
    {
        const labelList& pEdges = pointEdges[pointI];
        labelList& globalPEdges = globalPointEdges[pointI];
        globalPEdges.setSize(pEdges.size());
        forAll(pEdges, i)
        {
            globalPEdges[i] = globalEdgeNumbers.toGlobal(pEdges[i]);
        }

        labelPairList& globalPPoints = globalPointPoints[pointI];
        globalPPoints.setSize(pEdges.size());
        forAll(pEdges, i)
        {
            label otherPointI = edges[pEdges[i]].otherVertex(pointI);
            globalPPoints[i] = globalIndexAndTransform::encode
            (
                Pstream::myProcNo(),
                otherPointI,
                globalTransforms().nullTransformIndex()
            );
        }
    }

    // Pull slave data to master. Dummy transform.
    globalPointSlavesMap().distribute(globalPointEdges);
    globalPointSlavesMap().distribute(globalPointPoints);
    // Add all pointEdges
    forAll(slaves, pointI)
    {
        const labelList& pSlaves = slaves[pointI];
        const labelList& pTransformSlaves = transformedSlaves[pointI];

        label n = 0;
        forAll(pSlaves, i)
        {
            n += globalPointEdges[pSlaves[i]].size();
        }
        forAll(pTransformSlaves, i)
        {
            n += globalPointEdges[pTransformSlaves[i]].size();
        }

        // Add all the point edges of the slaves to those of the (master) point
        {
            labelList& globalPEdges = globalPointEdges[pointI];
            label sz = globalPEdges.size();
            globalPEdges.setSize(sz+n);
            forAll(pSlaves, i)
            {
                const labelList& otherData = globalPointEdges[pSlaves[i]];
                forAll(otherData, j)
                {
                    globalPEdges[sz++] = otherData[j];
                }
            }
            forAll(pTransformSlaves, i)
            {
                const labelList& otherData =
                    globalPointEdges[pTransformSlaves[i]];
                forAll(otherData, j)
                {
                    globalPEdges[sz++] = otherData[j];
                }
            }

            // Put back in slots
            forAll(pSlaves, i)
            {
                globalPointEdges[pSlaves[i]] = globalPEdges;
            }
            forAll(pTransformSlaves, i)
            {
                globalPointEdges[pTransformSlaves[i]] = globalPEdges;
            }
        }


        // Same for corresponding pointPoints
        {
            labelPairList& globalPPoints = globalPointPoints[pointI];
            label sz = globalPPoints.size();
            globalPPoints.setSize(sz + n);

            // Add untransformed points
            forAll(pSlaves, i)
            {
                const labelPairList& otherData = globalPointPoints[pSlaves[i]];
                forAll(otherData, j)
                {
                    globalPPoints[sz++] = otherData[j];
                }
            }
            // Add transformed points.
            forAll(pTransformSlaves, i)
            {
                // Get transform from index
                label transformI = globalPointSlavesMap().whichTransform
                (
                    pTransformSlaves[i]
                );

                const labelPairList& otherData =
                    globalPointPoints[pTransformSlaves[i]];
                forAll(otherData, j)
                {
                    // Add transform to connectivity
                    const labelPair& n = otherData[j];
                    label procI = globalIndexAndTransform::processor(n);
                    label index = globalIndexAndTransform::index(n);
                    globalPPoints[sz++] = globalIndexAndTransform::encode
                    (
                        procI,
                        index,
                        transformI
                    );
                }
            }

            // Put back in slots
            forAll(pSlaves, i)
            {
                globalPointPoints[pSlaves[i]] = globalPPoints;
            }
            forAll(pTransformSlaves, i)
            {
                globalPointPoints[pTransformSlaves[i]] = globalPPoints;
            }
        }
    }
    // Push back
    globalPointSlavesMap().reverseDistribute
    (
        globalPointEdges.size(),
        globalPointEdges
    );
    // Push back
    globalPointSlavesMap().reverseDistribute
    (
        globalPointPoints.size(),
        globalPointPoints
    );
}


// Find transformation to take remotePoint to localPoint. Use info to find
// the transforms.
CML::label CML::globalMeshData::findTransform
(
    const labelPairList& info,
    const labelPair& remotePoint,
    const label localPoint
) const
{
    const label remoteProcI = globalIndexAndTransform::processor(remotePoint);
    const label remoteIndex = globalIndexAndTransform::index(remotePoint);

    label remoteTransformI = -1;
    label localTransformI = -1;
    forAll(info, i)
    {
        label procI = globalIndexAndTransform::processor(info[i]);
        label pointI = globalIndexAndTransform::index(info[i]);
        label transformI = globalIndexAndTransform::transformIndex(info[i]);

        if (procI == Pstream::myProcNo() && pointI == localPoint)
        {
            localTransformI = transformI;
            //Pout<< "For local :" << localPoint
            //    << " found transform:" << localTransformI
            //    << endl;
        }
        if (procI == remoteProcI && pointI == remoteIndex)
        {
            remoteTransformI = transformI;
            //Pout<< "For remote:" << remotePoint
            //    << " found transform:" << remoteTransformI
            //    << " at index:" << i
            //    << endl;
        }
    }

    if (remoteTransformI == -1 || localTransformI == -1)
    {
        FatalErrorInFunction
            << "Problem. Cannot find " << remotePoint
            << " or " << localPoint << " in " << info
            << endl
            << "remoteTransformI:" << remoteTransformI << endl
            << "localTransformI:" << localTransformI
            << abort(FatalError);
    }

    return globalTransforms().subtractTransformIndex
    (
        remoteTransformI,
        localTransformI
    );
}


void CML::globalMeshData::calcGlobalEdgeSlaves() const
{
    if (debug)
    {
        Pout<< "globalMeshData::calcGlobalEdgeSlaves() :"
            << " calculating coupled master to slave edge addressing." << endl;
    }

    const edgeList& edges = coupledPatch().edges();
    const globalIndex& globalEdgeNumbers = globalEdgeNumbering();


    // The whole problem with deducting edge-connectivity from
    // point-connectivity is that one of the the endpoints might be
    // a local master but the other endpoint might not. So we first
    // need to make sure that all points know about connectivity and
    // the transformations.


    // 1. collect point connectivity - basically recreating globalPoints output.
    // All points will now have a string of points. The transforms are
    // in respect to the master.
    List<labelPairList> allPointConnectivity;
    calcPointConnectivity(allPointConnectivity);


    // 2. Get all pointEdges and pointPoints
    // Coupled point to global coupled edges and corresponding endpoint.
    labelListList globalPointEdges;
    List<labelPairList> globalPointPoints;
    calcGlobalPointEdges(globalPointEdges, globalPointPoints);


    // 3. Now all points have
    //      - all the connected points with original transform
    //      - all the connected global edges

    // Now all we need to do is go through all the edges and check
    // both endpoints. If there is a edge between the two which is
    // produced by transforming both points in the same way it is a shared
    // edge.

    // Collect strings of connected edges.
    List<labelPairList> allEdgeConnectivity(edges.size());

    forAll(edges, edgeI)
    {
        const edge& e = edges[edgeI];
        const labelList& pEdges0 = globalPointEdges[e[0]];
        const labelPairList& pPoints0 = globalPointPoints[e[0]];
        const labelList& pEdges1 = globalPointEdges[e[1]];
        const labelPairList& pPoints1 = globalPointPoints[e[1]];

        // Most edges will be size 2
        DynamicList<labelPair> eEdges(2);
        // Append myself.
        eEdges.append
        (
            globalIndexAndTransform::encode
            (
                Pstream::myProcNo(),
                edgeI,
                globalTransforms().nullTransformIndex()
            )
        );

        forAll(pEdges0, i)
        {
            forAll(pEdges1, j)
            {
                if
                (
                    pEdges0[i] == pEdges1[j]
                 && pEdges0[i] != globalEdgeNumbers.toGlobal(edgeI)
                )
                {
                    // Found a shared edge. Now check if the endpoints
                    // go through the same transformation.
                    // Local: e[0]    remote:pPoints1[j]
                    // Local: e[1]    remote:pPoints0[i]


                    // Find difference in transforms to go from point on remote
                    // edge (pPoints1[j]) to this point.

                    label transform0 = findTransform
                    (
                        allPointConnectivity[e[0]],
                        pPoints1[j],
                        e[0]
                    );
                    label transform1 = findTransform
                    (
                        allPointConnectivity[e[1]],
                        pPoints0[i],
                        e[1]
                    );

                    if (transform0 == transform1)
                    {
                        label procI = globalEdgeNumbers.whichProcID(pEdges0[i]);
                        eEdges.append
                        (
                            globalIndexAndTransform::encode
                            (
                                procI,
                                globalEdgeNumbers.toLocal(procI, pEdges0[i]),
                                transform0
                            )
                        );
                    }
                }
            }
        }

        allEdgeConnectivity[edgeI].transfer(eEdges);
        sort(allEdgeConnectivity[edgeI], globalIndexAndTransform::less());
    }

    // We now have - in allEdgeConnectivity - a list of edges which are shared
    // between multiple processors. Filter into non-transformed and transformed
    // connections.

    globalEdgeSlavesPtr_.reset(new labelListList(edges.size()));
    labelListList& globalEdgeSlaves = globalEdgeSlavesPtr_();
    List<labelPairList> transformedEdges(edges.size());
    forAll(allEdgeConnectivity, edgeI)
    {
        const labelPairList& edgeInfo = allEdgeConnectivity[edgeI];
        if (edgeInfo.size() >= 2)
        {
            const labelPair& masterInfo = edgeInfo[0];

            // Check if master edge (= first element (since sorted)) is me.
            if
            (
                (
                    globalIndexAndTransform::processor(masterInfo)
                 == Pstream::myProcNo()
                )
             && (globalIndexAndTransform::index(masterInfo) == edgeI)
            )
            {
                // Sort into transformed and untransformed
                labelList& eEdges = globalEdgeSlaves[edgeI];
                eEdges.setSize(edgeInfo.size()-1);

                labelPairList& trafoEEdges = transformedEdges[edgeI];
                trafoEEdges.setSize(edgeInfo.size()-1);

                label nonTransformI = 0;
                label transformI = 0;

                for (label i = 1; i < edgeInfo.size(); i++)
                {
                    const labelPair& info = edgeInfo[i];
                    label procI = globalIndexAndTransform::processor(info);
                    label index = globalIndexAndTransform::index(info);
                    label transform = globalIndexAndTransform::transformIndex
                    (
                        info
                    );

                    if (transform == globalTransforms().nullTransformIndex())
                    {
                        eEdges[nonTransformI++] = globalEdgeNumbers.toGlobal
                        (
                            procI,
                            index
                        );
                    }
                    else
                    {
                        trafoEEdges[transformI++] = info;
                    }
                }

                eEdges.setSize(nonTransformI);
                trafoEEdges.setSize(transformI);
            }
        }
    }


    // Construct map
    globalEdgeTransformedSlavesPtr_.reset(new labelListList());

    List<Map<label> > compactMap(Pstream::nProcs());
    globalEdgeSlavesMapPtr_.reset
    (
        new mapDistribute
        (
            globalEdgeNumbers,
            globalEdgeSlaves,

            globalTransforms(),
            transformedEdges,
            globalEdgeTransformedSlavesPtr_(),

            compactMap
        )
    );


    if (debug)
    {
        Pout<< "globalMeshData::calcGlobalEdgeSlaves() :"
            << " coupled edges:" << edges.size()
            << " additional coupled edges:"
            << globalEdgeSlavesMapPtr_().constructSize() - edges.size()
            << endl;
    }
}


void CML::globalMeshData::calcGlobalEdgeOrientation() const
{
    if (debug)
    {
        Pout<< "globalMeshData::calcGlobalEdgeOrientation() :"
            << " calculating edge orientation w.r.t. master edge." << endl;
    }

    const globalIndex& globalPoints = globalPointNumbering();

    // 1. Determine master point
    labelList masterPoint;
    {
        const mapDistribute& map = globalPointSlavesMap();

        masterPoint.setSize(map.constructSize());
        masterPoint = labelMax;

        for (label pointI = 0; pointI < coupledPatch().nPoints(); pointI++)
        {
            masterPoint[pointI] = globalPoints.toGlobal(pointI);
        }
        syncData
        (
            masterPoint,
            globalPointSlaves(),
            globalPointTransformedSlaves(),
            map,
            minEqOp<label>()
        );
    }

    // Now all points should know who is master by comparing their global
    // pointID with the masterPointID. We now can use this information
    // to find the orientation of the master edge.

    {
        const mapDistribute& map = globalEdgeSlavesMap();
        const labelListList& slaves = globalEdgeSlaves();
        const labelListList& transformedSlaves = globalEdgeTransformedSlaves();

        // Distribute orientation of master edge (in masterPoint numbering)
        labelPairList masterEdgeVerts(map.constructSize());
        masterEdgeVerts = labelPair(labelMax, labelMax);

        for (label edgeI = 0; edgeI < coupledPatch().nEdges(); edgeI++)
        {
            if
            (
                (
                    slaves[edgeI].size()
                  + transformedSlaves[edgeI].size()
                )
              > 0
            )
            {
                // I am master. Fill in my masterPoint equivalent.

                const edge& e = coupledPatch().edges()[edgeI];
                masterEdgeVerts[edgeI] = labelPair
                (
                    masterPoint[e[0]],
                    masterPoint[e[1]]
                );
            }
        }
        syncData
        (
            masterEdgeVerts,
            slaves,
            transformedSlaves,
            map,
            minEqOp<labelPair>()
        );

        // Now check my edges on how they relate to the master's edgeVerts
        globalEdgeOrientationPtr_.reset
        (
            new PackedBoolList(coupledPatch().nEdges())
        );
        PackedBoolList& globalEdgeOrientation = globalEdgeOrientationPtr_();

        forAll(coupledPatch().edges(), edgeI)
        {
            const edge& e = coupledPatch().edges()[edgeI];
            const labelPair masterE
            (
                masterPoint[e[0]],
                masterPoint[e[1]]
            );

            label stat = labelPair::compare
            (
                masterE,
                masterEdgeVerts[edgeI]
            );
            if (stat == 0)
            {
                FatalErrorInFunction
                    << "problem : my edge:" << e
                    << " in master points:" << masterE
                    << " v.s. masterEdgeVerts:" << masterEdgeVerts[edgeI]
                    << exit(FatalError);
            }
            else
            {
                globalEdgeOrientation[edgeI] = (stat == 1);
            }
        }
    }

    if (debug)
    {
        Pout<< "globalMeshData::calcGlobalEdgeOrientation() :"
            << " finished calculating edge orientation."
            << endl;
    }
}


// Calculate uncoupled boundary faces (without calculating
// primitiveMesh::pointFaces())
void CML::globalMeshData::calcPointBoundaryFaces
(
    labelListList& pointBoundaryFaces
) const
{
    const polyBoundaryMesh& bMesh = mesh_.boundaryMesh();
    const Map<label>& meshPointMap = coupledPatch().meshPointMap();

    // 1. Count

    labelList nPointFaces(coupledPatch().nPoints(), 0);

    forAll(bMesh, patchI)
    {
        const polyPatch& pp = bMesh[patchI];

        if (!pp.coupled())
        {
            forAll(pp, i)
            {
                const face& f = pp[i];

                forAll(f, fp)
                {
                    Map<label>::const_iterator iter = meshPointMap.find
                    (
                        f[fp]
                    );
                    if (iter != meshPointMap.end())
                    {
                        nPointFaces[iter()]++;
                    }
                }
            }
        }
    }


    // 2. Size

    pointBoundaryFaces.setSize(coupledPatch().nPoints());
    forAll(nPointFaces, pointI)
    {
        pointBoundaryFaces[pointI].setSize(nPointFaces[pointI]);
    }
    nPointFaces = 0;


    // 3. Fill

    forAll(bMesh, patchI)
    {
        const polyPatch& pp = bMesh[patchI];

        if (!pp.coupled())
        {
            forAll(pp, i)
            {
                const face& f = pp[i];
                forAll(f, fp)
                {
                    Map<label>::const_iterator iter = meshPointMap.find
                    (
                        f[fp]
                    );
                    if (iter != meshPointMap.end())
                    {
                        label bFaceI =
                             pp.start() + i - mesh_.nInternalFaces();
                        pointBoundaryFaces[iter()][nPointFaces[iter()]++] =
                            bFaceI;
                    }
                }
            }
        }
    }
}


void CML::globalMeshData::calcGlobalPointBoundaryFaces() const
{
    if (debug)
    {
        Pout<< "globalMeshData::calcGlobalPointBoundaryFaces() :"
            << " calculating coupled point to boundary face addressing."
            << endl;
    }

    // Construct local point to (uncoupled)boundaryfaces.
    labelListList pointBoundaryFaces;
    calcPointBoundaryFaces(pointBoundaryFaces);


    // Global indices for boundary faces
    globalBoundaryFaceNumberingPtr_.reset
    (
        new globalIndex(mesh_.nFaces()-mesh_.nInternalFaces())
    );
    globalIndex& globalIndices = globalBoundaryFaceNumberingPtr_();


    // Convert local boundary faces to global numbering
    globalPointBoundaryFacesPtr_.reset
    (
        new labelListList(globalPointSlavesMap().constructSize())
    );
    labelListList& globalPointBoundaryFaces = globalPointBoundaryFacesPtr_();

    forAll(pointBoundaryFaces, pointI)
    {
        const labelList& bFaces = pointBoundaryFaces[pointI];
        labelList& globalFaces = globalPointBoundaryFaces[pointI];
        globalFaces.setSize(bFaces.size());
        forAll(bFaces, i)
        {
            globalFaces[i] = globalIndices.toGlobal(bFaces[i]);
        }
    }


    // Pull slave pointBoundaryFaces to master
    globalPointSlavesMap().distribute
    (
        globalPointBoundaryFaces,
        true    // put data on transformed points into correct slots
    );


    // Merge slave labels into master globalPointBoundaryFaces.
    // Split into untransformed and transformed values.
    const labelListList& pointSlaves = globalPointSlaves();
    const labelListList& pointTransformSlaves =
        globalPointTransformedSlaves();


    // Any faces coming in through transformation
    List<labelPairList> transformedFaces(pointSlaves.size());


    forAll(pointSlaves, pointI)
    {
        const labelList& slaves = pointSlaves[pointI];
        const labelList& transformedSlaves = pointTransformSlaves[pointI];

        if (slaves.size() > 0)
        {
            labelList& myBFaces = globalPointBoundaryFaces[pointI];
            label sz = myBFaces.size();

            // Count
            label n = 0;
            forAll(slaves, i)
            {
                n += globalPointBoundaryFaces[slaves[i]].size();
            }
            // Fill
            myBFaces.setSize(sz+n);
            n = sz;
            forAll(slaves, i)
            {
                const labelList& slaveBFaces =
                    globalPointBoundaryFaces[slaves[i]];

                // Add all slaveBFaces. Note that need to check for
                // uniqueness only in case of cyclics.

                forAll(slaveBFaces, j)
                {
                    label slave = slaveBFaces[j];
                    if (findIndex(SubList<label>(myBFaces, sz), slave) == -1)
                    {
                        myBFaces[n++] = slave;
                    }
                }
            }
            myBFaces.setSize(n);
        }


        if (transformedSlaves.size() > 0)
        {
            const labelList& untrafoFaces = globalPointBoundaryFaces[pointI];

            labelPairList& myBFaces = transformedFaces[pointI];
            label sz = myBFaces.size();

            // Count
            label n = 0;
            forAll(transformedSlaves, i)
            {
                n += globalPointBoundaryFaces[transformedSlaves[i]].size();
            }
            // Fill
            myBFaces.setSize(sz+n);
            n = sz;
            forAll(transformedSlaves, i)
            {
                label transformI = globalPointSlavesMap().whichTransform
                (
                    transformedSlaves[i]
                );

                const labelList& slaveBFaces =
                    globalPointBoundaryFaces[transformedSlaves[i]];

                forAll(slaveBFaces, j)
                {
                    label slave = slaveBFaces[j];
                    // Check that same face not already present untransformed
                    if (findIndex(untrafoFaces, slave)== -1)
                    {
                        label procI = globalIndices.whichProcID(slave);
                        label faceI = globalIndices.toLocal(procI, slave);

                        myBFaces[n++] = globalIndexAndTransform::encode
                        (
                            procI,
                            faceI,
                            transformI
                        );
                    }
                }
            }
            myBFaces.setSize(n);
        }


        if (slaves.size() + transformedSlaves.size() == 0)
        {
             globalPointBoundaryFaces[pointI].clear();
        }
    }

    // Construct a map to get the face data directly
    List<Map<label> > compactMap(Pstream::nProcs());

    globalPointTransformedBoundaryFacesPtr_.reset
    (
        new labelListList(transformedFaces.size())
    );

    globalPointBoundaryFacesMapPtr_.reset
    (
        new mapDistribute
        (
            globalIndices,
            globalPointBoundaryFaces,

            globalTransforms(),
            transformedFaces,
            globalPointTransformedBoundaryFacesPtr_(),

            compactMap
        )
    );
    globalPointBoundaryFaces.setSize(coupledPatch().nPoints());
    globalPointTransformedBoundaryFacesPtr_().setSize(coupledPatch().nPoints());

    if (debug)
    {
        Pout<< "globalMeshData::calcGlobalPointBoundaryFaces() :"
            << " coupled points:" << coupledPatch().nPoints()
            << " local boundary faces:" <<  globalIndices.localSize()
            << " additional coupled faces:"
            <<  globalPointBoundaryFacesMapPtr_().constructSize()
              - globalIndices.localSize()
            << endl;
    }
}


void CML::globalMeshData::calcGlobalPointBoundaryCells() const
{
    if (debug)
    {
        Pout<< "globalMeshData::calcGlobalPointBoundaryCells() :"
            << " calculating coupled point to boundary cell addressing."
            << endl;
    }

    // Create map of boundary cells and point-cell addressing
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    label bCellI = 0;
    Map<label> meshCellMap(4*coupledPatch().nPoints());
    DynamicList<label> cellMap(meshCellMap.size());

    // Create addressing for point to boundary cells (local)
    labelListList pointBoundaryCells(coupledPatch().nPoints());

    forAll(coupledPatch().meshPoints(), pointI)
    {
        label meshPointI = coupledPatch().meshPoints()[pointI];
        const labelList& pCells = mesh_.pointCells(meshPointI);

        labelList& bCells = pointBoundaryCells[pointI];
        bCells.setSize(pCells.size());

        forAll(pCells, i)
        {
            label cellI = pCells[i];
            Map<label>::iterator fnd = meshCellMap.find(cellI);

            if (fnd != meshCellMap.end())
            {
                bCells[i] = fnd();
            }
            else
            {
                meshCellMap.insert(cellI, bCellI);
                cellMap.append(cellI);
                bCells[i] = bCellI;
                bCellI++;
            }
        }
    }


    boundaryCellsPtr_.reset(new labelList());
    labelList& boundaryCells = boundaryCellsPtr_();
    boundaryCells.transfer(cellMap.shrink());


    // Convert point-cells to global (boundary)cell numbers
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    globalBoundaryCellNumberingPtr_.reset
    (
        new globalIndex(boundaryCells.size())
    );
    globalIndex& globalIndices = globalBoundaryCellNumberingPtr_();


    globalPointBoundaryCellsPtr_.reset
    (
        new labelListList(globalPointSlavesMap().constructSize())
    );
    labelListList& globalPointBoundaryCells = globalPointBoundaryCellsPtr_();

    forAll(pointBoundaryCells, pointI)
    {
        const labelList& pCells = pointBoundaryCells[pointI];
        labelList& globalCells = globalPointBoundaryCells[pointI];
        globalCells.setSize(pCells.size());
        forAll(pCells, i)
        {
            globalCells[i] = globalIndices.toGlobal(pCells[i]);
        }
    }


    // Pull slave pointBoundaryCells to master
    globalPointSlavesMap().distribute
    (
        globalPointBoundaryCells,
        true    // put data on transformed points into correct slots
    );


    // Merge slave labels into master globalPointBoundaryCells
    const labelListList& pointSlaves = globalPointSlaves();
    const labelListList& pointTransformSlaves =
        globalPointTransformedSlaves();

    List<labelPairList> transformedCells(pointSlaves.size());


    forAll(pointSlaves, pointI)
    {
        const labelList& slaves = pointSlaves[pointI];
        const labelList& transformedSlaves = pointTransformSlaves[pointI];

        if (slaves.size() > 0)
        {
            labelList& myBCells = globalPointBoundaryCells[pointI];
            label sz = myBCells.size();

            // Count
            label n = 0;
            forAll(slaves, i)
            {
                n += globalPointBoundaryCells[slaves[i]].size();
            }
            // Fill
            myBCells.setSize(sz+n);
            n = sz;
            forAll(slaves, i)
            {
                const labelList& slaveBCells =
                    globalPointBoundaryCells[slaves[i]];

                // Add all slaveBCells. Note that need to check for
                // uniqueness only in case of cyclics.

                forAll(slaveBCells, j)
                {
                    label slave = slaveBCells[j];
                    if (findIndex(SubList<label>(myBCells, sz), slave) == -1)
                    {
                        myBCells[n++] = slave;
                    }
                }
            }
            myBCells.setSize(n);
        }


        if (transformedSlaves.size() > 0)
        {
            const labelList& untrafoCells = globalPointBoundaryCells[pointI];

            labelPairList& myBCells = transformedCells[pointI];
            label sz = myBCells.size();

            // Count
            label n = 0;
            forAll(transformedSlaves, i)
            {
                n += globalPointBoundaryCells[transformedSlaves[i]].size();
            }
            // Fill
            myBCells.setSize(sz+n);
            n = sz;
            forAll(transformedSlaves, i)
            {
                label transformI = globalPointSlavesMap().whichTransform
                (
                    transformedSlaves[i]
                );

                const labelList& slaveBCells =
                    globalPointBoundaryCells[transformedSlaves[i]];

                forAll(slaveBCells, j)
                {
                    label slave = slaveBCells[j];

                    // Check that same cell not already present untransformed
                    if (findIndex(untrafoCells, slave)== -1)
                    {
                        label procI = globalIndices.whichProcID(slave);
                        label cellI = globalIndices.toLocal(procI, slave);
                        myBCells[n++] = globalIndexAndTransform::encode
                        (
                            procI,
                            cellI,
                            transformI
                        );
                    }
                }
            }
            myBCells.setSize(n);
        }

        if (slaves.size() + transformedSlaves.size() == 0)
        {
             globalPointBoundaryCells[pointI].clear();
        }
    }

    // Construct a map to get the cell data directly
    List<Map<label> > compactMap(Pstream::nProcs());

    globalPointTransformedBoundaryCellsPtr_.reset
    (
        new labelListList(transformedCells.size())
    );

    globalPointBoundaryCellsMapPtr_.reset
    (
        new mapDistribute
        (
            globalIndices,
            globalPointBoundaryCells,

            globalTransforms(),
            transformedCells,
            globalPointTransformedBoundaryCellsPtr_(),

            compactMap
        )
    );
    globalPointBoundaryCells.setSize(coupledPatch().nPoints());
    globalPointTransformedBoundaryCellsPtr_().setSize(coupledPatch().nPoints());

    if (debug)
    {
        Pout<< "globalMeshData::calcGlobalPointBoundaryCells() :"
            << " coupled points:" << coupledPatch().nPoints()
            << " local boundary cells:" <<  globalIndices.localSize()
            << " additional coupled cells:"
            <<  globalPointBoundaryCellsMapPtr_().constructSize()
              - globalIndices.localSize()
            << endl;
    }
}


void CML::globalMeshData::calcGlobalCoPointSlaves() const
{
    if (debug)
    {
        Pout<< "globalMeshData::calcGlobalCoPointSlaves() :"
            << " calculating coupled master to collocated"
            << " slave point addressing." << endl;
    }

    // Calculate connected points for master points.
    globalPoints globalData(mesh_, coupledPatch(), true, false);

    globalCoPointSlavesPtr_.reset
    (
        new labelListList
        (
            globalData.pointPoints().xfer()
        )
    );
    globalCoPointSlavesMapPtr_.reset
    (
        new mapDistribute
        (
            globalData.map().xfer()
        )
    );

    if (debug)
    {
        Pout<< "globalMeshData::calcGlobalCoPointSlaves() :"
            << " finished calculating coupled master to collocated"
            << " slave point addressing." << endl;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from polyMesh
CML::globalMeshData::globalMeshData(const polyMesh& mesh)
:
    processorTopology(mesh.boundaryMesh()),
    mesh_(mesh),
    nTotalPoints_(-1),
    nTotalFaces_(-1),
    nTotalCells_(-1),
    processorPatches_(0),
    processorPatchIndices_(0),
    processorPatchNeighbours_(0),
    nGlobalPoints_(-1),
    sharedPointLabelsPtr_(nullptr),
    sharedPointAddrPtr_(nullptr),
    sharedPointGlobalLabelsPtr_(nullptr),
    nGlobalEdges_(-1),
    sharedEdgeLabelsPtr_(nullptr),
    sharedEdgeAddrPtr_(nullptr)
{
    updateMesh();
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::globalMeshData::~globalMeshData()
{
    clearOut();
}


void CML::globalMeshData::clearOut()
{
    // Point
    nGlobalPoints_ = -1;
    sharedPointLabelsPtr_.clear();
    sharedPointAddrPtr_.clear();
    sharedPointGlobalLabelsPtr_.clear();

    // Edge
    nGlobalEdges_ = -1;
    sharedEdgeLabelsPtr_.clear();
    sharedEdgeAddrPtr_.clear();

    // Coupled patch
    coupledPatchPtr_.clear();
    coupledPatchMeshEdgesPtr_.clear();
    coupledPatchMeshEdgeMapPtr_.clear();
    globalTransformsPtr_.clear();

    // Point
    globalPointNumberingPtr_.clear();
    globalPointSlavesPtr_.clear();
    globalPointTransformedSlavesPtr_.clear();
    globalPointSlavesMapPtr_.clear();
    // Edge
    globalEdgeNumberingPtr_.clear();
    globalEdgeSlavesPtr_.clear();
    globalEdgeTransformedSlavesPtr_.clear();
    globalEdgeOrientationPtr_.clear();
    globalEdgeSlavesMapPtr_.clear();

    // Face
    globalBoundaryFaceNumberingPtr_.clear();
    globalPointBoundaryFacesPtr_.clear();
    globalPointTransformedBoundaryFacesPtr_.clear();
    globalPointBoundaryFacesMapPtr_.clear();

    // Cell
    boundaryCellsPtr_.clear();
    globalBoundaryCellNumberingPtr_.clear();
    globalPointBoundaryCellsPtr_.clear();
    globalPointTransformedBoundaryCellsPtr_.clear();
    globalPointBoundaryCellsMapPtr_.clear();

    // Other: collocated points
    globalCoPointSlavesPtr_.clear();
    globalCoPointSlavesMapPtr_.clear();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// Return shared point global labels.
const CML::labelList& CML::globalMeshData::sharedPointGlobalLabels() const
{
    if (!sharedPointGlobalLabelsPtr_.valid())
    {
        sharedPointGlobalLabelsPtr_.reset
        (
            new labelList(sharedPointLabels().size())
        );
        labelList& sharedPointGlobalLabels = sharedPointGlobalLabelsPtr_();

        IOobject addrHeader
        (
            "pointProcAddressing",
            mesh_.facesInstance()/mesh_.meshSubDir,
            mesh_,
            IOobject::MUST_READ
        );

        if (addrHeader.headerOk())
        {
            // There is a pointProcAddressing file so use it to get labels
            // on the original mesh
            Pout<< "globalMeshData::sharedPointGlobalLabels : "
                << "Reading pointProcAddressing" << endl;

            labelIOList pointProcAddressing(addrHeader);

            const labelList& pointLabels = sharedPointLabels();

            forAll(pointLabels, i)
            {
                // Get my mesh point
                label pointI = pointLabels[i];

                // Map to mesh point of original mesh
                sharedPointGlobalLabels[i] = pointProcAddressing[pointI];
            }
        }
        else
        {
            Pout<< "globalMeshData::sharedPointGlobalLabels :"
                << " Setting pointProcAddressing to -1" << endl;

            sharedPointGlobalLabels = -1;
        }
    }
    return sharedPointGlobalLabelsPtr_();
}


// Collect coordinates of shared points. (does parallel communication!)
CML::pointField CML::globalMeshData::sharedPoints() const
{
    // Get all processors to send their shared points to master.
    // (not very efficient)

    pointField sharedPoints(nGlobalPoints());
    const labelList& pointAddr = sharedPointAddr();
    const labelList& pointLabels = sharedPointLabels();

    if (Pstream::master())
    {
        // Master:
        // insert my own data first
        forAll(pointLabels, i)
        {
            label sharedPointI = pointAddr[i];

            sharedPoints[sharedPointI] = mesh_.points()[pointLabels[i]];
        }

        // Receive data from slaves and insert
        for
        (
            int slave=Pstream::firstSlave();
            slave<=Pstream::lastSlave();
            slave++
        )
        {
            IPstream fromSlave(Pstream::blocking, slave);

            labelList nbrSharedPointAddr;
            pointField nbrSharedPoints;
            fromSlave >> nbrSharedPointAddr >> nbrSharedPoints;

            forAll(nbrSharedPointAddr, i)
            {
                label sharedPointI = nbrSharedPointAddr[i];

                sharedPoints[sharedPointI] = nbrSharedPoints[i];
            }
        }

        // Send back
        for
        (
            int slave=Pstream::firstSlave();
            slave<=Pstream::lastSlave();
            slave++
        )
        {
            OPstream toSlave
            (
                Pstream::blocking,
                slave,
                sharedPoints.size()*sizeof(vector::zero)
            );
            toSlave << sharedPoints;
        }
    }
    else
    {
        // Slave:
        // send points
        {
            OPstream toMaster(Pstream::blocking, Pstream::masterNo());

            toMaster
                << pointAddr
                << UIndirectList<point>(mesh_.points(), pointLabels)();
        }

        // Receive sharedPoints
        {
            IPstream fromMaster(Pstream::blocking, Pstream::masterNo());
            fromMaster >> sharedPoints;
        }
    }

    return sharedPoints;
}


// Collect coordinates of shared points. (does parallel communication!)
CML::pointField CML::globalMeshData::geometricSharedPoints() const
{
    // Get coords of my shared points
    pointField sharedPoints(mesh_.points(), sharedPointLabels());

    // Append from all processors
    combineReduce(sharedPoints, plusEqOp<pointField>());

    // Merge tolerance
    scalar tolDim = matchTol_ * mesh_.bounds().mag();

    // And see how many are unique
    labelList pMap;
    pointField mergedPoints;

    CML::mergePoints
    (
        sharedPoints,   // coordinates to merge
        tolDim,         // tolerance
        false,          // verbosity
        pMap,
        mergedPoints
    );

    return mergedPoints;
}


CML::label CML::globalMeshData::nGlobalPoints() const
{
    if (nGlobalPoints_ == -1)
    {
        calcSharedPoints();
    }
    return nGlobalPoints_;
}


const CML::labelList& CML::globalMeshData::sharedPointLabels() const
{
    if (!sharedPointLabelsPtr_.valid())
    {
        calcSharedPoints();
    }
    return sharedPointLabelsPtr_();
}


const CML::labelList& CML::globalMeshData::sharedPointAddr() const
{
    if (!sharedPointAddrPtr_.valid())
    {
        calcSharedPoints();
    }
    return sharedPointAddrPtr_();
}


CML::label CML::globalMeshData::nGlobalEdges() const
{
    if (nGlobalEdges_ == -1)
    {
        calcSharedEdges();
    }
    return nGlobalEdges_;
}


const CML::labelList& CML::globalMeshData::sharedEdgeLabels() const
{
    if (!sharedEdgeLabelsPtr_.valid())
    {
        calcSharedEdges();
    }
    return sharedEdgeLabelsPtr_();
}


const CML::labelList& CML::globalMeshData::sharedEdgeAddr() const
{
    if (!sharedEdgeAddrPtr_.valid())
    {
        calcSharedEdges();
    }
    return sharedEdgeAddrPtr_();
}


const CML::indirectPrimitivePatch& CML::globalMeshData::coupledPatch() const
{
    if (!coupledPatchPtr_.valid())
    {
        const polyBoundaryMesh& bMesh = mesh_.boundaryMesh();

        label nCoupled = 0;

        forAll(bMesh, patchI)
        {
            const polyPatch& pp = bMesh[patchI];

            if (pp.coupled())
            {
                nCoupled += pp.size();
            }
        }
        labelList coupledFaces(nCoupled);
        nCoupled = 0;

        forAll(bMesh, patchI)
        {
            const polyPatch& pp = bMesh[patchI];

            if (pp.coupled())
            {
                label faceI = pp.start();

                forAll(pp, i)
                {
                    coupledFaces[nCoupled++] = faceI++;
                }
            }
        }

        coupledPatchPtr_.reset
        (
            new indirectPrimitivePatch
            (
                IndirectList<face>
                (
                    mesh_.faces(),
                    coupledFaces
                ),
                mesh_.points()
            )
        );

        if (debug)
        {
            Pout<< "globalMeshData::coupledPatch() :"
                << " constructed  coupled faces patch:"
                << "  faces:" << coupledPatchPtr_().size()
                << "  points:" << coupledPatchPtr_().nPoints()
                << endl;
        }
    }
    return coupledPatchPtr_();
}


const CML::labelList& CML::globalMeshData::coupledPatchMeshEdges() const
{
    if (!coupledPatchMeshEdgesPtr_.valid())
    {
        coupledPatchMeshEdgesPtr_.reset
        (
            new labelList
            (
                coupledPatch().meshEdges
                (
                    mesh_.edges(),
                    mesh_.pointEdges()
                )
            )
        );
    }
    return coupledPatchMeshEdgesPtr_();
}


const CML::Map<CML::label>& CML::globalMeshData::coupledPatchMeshEdgeMap()
const
{
    if (!coupledPatchMeshEdgeMapPtr_.valid())
    {
        const labelList& me = coupledPatchMeshEdges();

        coupledPatchMeshEdgeMapPtr_.reset(new Map<label>(2*me.size()));
        Map<label>& em = coupledPatchMeshEdgeMapPtr_();

        forAll(me, i)
        {
            em.insert(me[i], i);
        }
    }
    return coupledPatchMeshEdgeMapPtr_();
}


const CML::globalIndex& CML::globalMeshData::globalPointNumbering() const
{
    if (!globalPointNumberingPtr_.valid())
    {
        globalPointNumberingPtr_.reset
        (
            new globalIndex(coupledPatch().nPoints())
        );
    }
    return globalPointNumberingPtr_();
}


const CML::globalIndexAndTransform&
CML::globalMeshData::globalTransforms() const
{
    if (!globalTransformsPtr_.valid())
    {
        globalTransformsPtr_.reset(new globalIndexAndTransform(mesh_));
    }
    return globalTransformsPtr_();
}


const CML::labelListList& CML::globalMeshData::globalPointSlaves() const
{
    if (!globalPointSlavesPtr_.valid())
    {
        calcGlobalPointSlaves();
    }
    return globalPointSlavesPtr_();
}


const CML::labelListList& CML::globalMeshData::globalPointTransformedSlaves()
const
{
    if (!globalPointTransformedSlavesPtr_.valid())
    {
        calcGlobalPointSlaves();
    }
    return globalPointTransformedSlavesPtr_();
}


const CML::mapDistribute& CML::globalMeshData::globalPointSlavesMap() const
{
    if (!globalPointSlavesMapPtr_.valid())
    {
        calcGlobalPointSlaves();
    }
    return globalPointSlavesMapPtr_();
}


const CML::globalIndex& CML::globalMeshData::globalEdgeNumbering() const
{
    if (!globalEdgeNumberingPtr_.valid())
    {
        globalEdgeNumberingPtr_.reset
        (
            new globalIndex(coupledPatch().nEdges())
        );
    }
    return globalEdgeNumberingPtr_();
}


const CML::labelListList& CML::globalMeshData::globalEdgeSlaves() const
{
    if (!globalEdgeSlavesPtr_.valid())
    {
        calcGlobalEdgeSlaves();
    }
    return globalEdgeSlavesPtr_();
}


const CML::labelListList& CML::globalMeshData::globalEdgeTransformedSlaves()
const
{
    if (!globalEdgeTransformedSlavesPtr_.valid())
    {
        calcGlobalEdgeSlaves();
    }
    return globalEdgeTransformedSlavesPtr_();
}


const CML::PackedBoolList& CML::globalMeshData::globalEdgeOrientation() const
{
    if (!globalEdgeOrientationPtr_.valid())
    {
        calcGlobalEdgeOrientation();
    }
    return globalEdgeOrientationPtr_();
}


const CML::mapDistribute& CML::globalMeshData::globalEdgeSlavesMap() const
{
    if (!globalEdgeSlavesMapPtr_.valid())
    {
        calcGlobalEdgeSlaves();
    }
    return globalEdgeSlavesMapPtr_();
}


const CML::globalIndex& CML::globalMeshData::globalBoundaryFaceNumbering()
const
{
    if (!globalBoundaryFaceNumberingPtr_.valid())
    {
        calcGlobalPointBoundaryFaces();
    }
    return globalBoundaryFaceNumberingPtr_();
}


const CML::labelListList& CML::globalMeshData::globalPointBoundaryFaces()
const
{
    if (!globalPointBoundaryFacesPtr_.valid())
    {
        calcGlobalPointBoundaryFaces();
    }
    return globalPointBoundaryFacesPtr_();
}


const CML::labelListList&
CML::globalMeshData::globalPointTransformedBoundaryFaces() const
{
    if (!globalPointTransformedBoundaryFacesPtr_.valid())
    {
        calcGlobalPointBoundaryFaces();
    }
    return globalPointTransformedBoundaryFacesPtr_();
}


const CML::mapDistribute& CML::globalMeshData::globalPointBoundaryFacesMap()
const
{
    if (!globalPointBoundaryFacesMapPtr_.valid())
    {
        calcGlobalPointBoundaryFaces();
    }
    return globalPointBoundaryFacesMapPtr_();
}


const CML::labelList& CML::globalMeshData::boundaryCells() const
{
    if (!boundaryCellsPtr_.valid())
    {
        calcGlobalPointBoundaryCells();
    }
    return boundaryCellsPtr_();
}


const CML::globalIndex& CML::globalMeshData::globalBoundaryCellNumbering()
const
{
    if (!globalBoundaryCellNumberingPtr_.valid())
    {
        calcGlobalPointBoundaryCells();
    }
    return globalBoundaryCellNumberingPtr_();
}


const CML::labelListList& CML::globalMeshData::globalPointBoundaryCells()
const
{
    if (!globalPointBoundaryCellsPtr_.valid())
    {
        calcGlobalPointBoundaryCells();
    }
    return globalPointBoundaryCellsPtr_();
}


const CML::labelListList&
CML::globalMeshData::globalPointTransformedBoundaryCells() const
{
    if (!globalPointTransformedBoundaryCellsPtr_.valid())
    {
        calcGlobalPointBoundaryCells();
    }
    return globalPointTransformedBoundaryCellsPtr_();
}


const CML::mapDistribute& CML::globalMeshData::globalPointBoundaryCellsMap()
const
{
    if (!globalPointBoundaryCellsMapPtr_.valid())
    {
        calcGlobalPointBoundaryCells();
    }
    return globalPointBoundaryCellsMapPtr_();
}


const CML::labelListList& CML::globalMeshData::globalCoPointSlaves() const
{
    if (!globalCoPointSlavesPtr_.valid())
    {
        calcGlobalCoPointSlaves();
    }
    return globalCoPointSlavesPtr_();
}


const CML::mapDistribute& CML::globalMeshData::globalCoPointSlavesMap() const
{
    if (!globalCoPointSlavesMapPtr_.valid())
    {
        calcGlobalCoPointSlaves();
    }
    return globalCoPointSlavesMapPtr_();
}


CML::autoPtr<CML::globalIndex> CML::globalMeshData::mergePoints
(
    labelList& pointToGlobal,
    labelList& uniquePoints
) const
{
    const indirectPrimitivePatch& cpp = coupledPatch();
    const globalIndex& globalCoupledPoints = globalPointNumbering();
    // Use collocated only
    const labelListList& pointSlaves = globalCoPointSlaves();
    const mapDistribute& pointSlavesMap = globalCoPointSlavesMap();


    // Points are either
    // - master with slaves
    // - slave with a master
    // - other (since e.g. non-collocated cyclics not connected)

    labelList masterGlobalPoint(cpp.nPoints(), -1);
    forAll(masterGlobalPoint, pointI)
    {
        const labelList& slavePoints = pointSlaves[pointI];
        if (slavePoints.size() > 0)
        {
            masterGlobalPoint[pointI] = globalCoupledPoints.toGlobal(pointI);
        }
    }

    // Sync by taking max
    syncData
    (
        masterGlobalPoint,
        pointSlaves,
        labelListList(cpp.nPoints()),   // no transforms
        pointSlavesMap,
        maxEqOp<label>()
    );


    // 1. Count number of masters on my processor.
    label nMaster = 0;
    PackedBoolList isMaster(mesh_.nPoints(), 1);
    forAll(pointSlaves, pointI)
    {
        if (masterGlobalPoint[pointI] == -1)
        {
            // unconnected point (e.g. from separated cyclic)
            nMaster++;
        }
        else if
        (
            masterGlobalPoint[pointI]
         == globalCoupledPoints.toGlobal(pointI)
        )
        {
            // connected master
            nMaster++;
        }
        else
        {
            // connected slave point
            isMaster[cpp.meshPoints()[pointI]] = 0;
        }
    }

    label myUniquePoints = mesh_.nPoints() - cpp.nPoints() + nMaster;

    //Pout<< "Points :" << nl
    //    << "    mesh             : " << mesh_.nPoints() << nl
    //    << "    of which coupled : " << cpp.nPoints() << nl
    //    << "    of which master  : " << nMaster << nl
    //    << endl;


    // 2. Create global indexing for unique points.
    autoPtr<globalIndex> globalPointsPtr(new globalIndex(myUniquePoints));


    // 3. Assign global point numbers. Keep slaves unset.
    pointToGlobal.setSize(mesh_.nPoints());
    pointToGlobal = -1;
    uniquePoints.setSize(myUniquePoints);
    nMaster = 0;

    forAll(isMaster, meshPointI)
    {
        if (isMaster[meshPointI])
        {
            pointToGlobal[meshPointI] = globalPointsPtr().toGlobal(nMaster);
            uniquePoints[nMaster] = meshPointI;
            nMaster++;
        }
    }


    // 4. Push global index for coupled points to slaves.
    {
        labelList masterToGlobal(pointSlavesMap.constructSize(), -1);

        forAll(pointSlaves, pointI)
        {
            const labelList& slaves = pointSlaves[pointI];

            if (slaves.size() > 0)
            {
                // Duplicate master globalpoint into slave slots
                label meshPointI = cpp.meshPoints()[pointI];
                masterToGlobal[pointI] = pointToGlobal[meshPointI];
                forAll(slaves, i)
                {
                    masterToGlobal[slaves[i]] = masterToGlobal[pointI];
                }
            }
        }

        // Send back
        pointSlavesMap.reverseDistribute(cpp.nPoints(), masterToGlobal);

        // On slave copy master index into overal map.
        forAll(pointSlaves, pointI)
        {
            label meshPointI = cpp.meshPoints()[pointI];

            if (!isMaster[meshPointI])
            {
                pointToGlobal[meshPointI] = masterToGlobal[pointI];
            }
        }
    }

    return globalPointsPtr;
}


CML::autoPtr<CML::globalIndex> CML::globalMeshData::mergePoints
(
    const labelList& meshPoints,
    const Map<label>& meshPointMap,
    labelList& pointToGlobal,
    labelList& uniqueMeshPoints
) const
{
    const indirectPrimitivePatch& cpp = coupledPatch();
    const labelListList& pointSlaves = globalCoPointSlaves();
    const mapDistribute& pointSlavesMap = globalCoPointSlavesMap();


    // The patch points come in two variants:
    // - not on a coupled patch so guaranteed unique
    // - on a coupled patch
    // If the point is on a coupled patch the problem is that the
    // master-slave structure (globalPointSlaves etc.) assigns one of the
    // coupled points to be the master but this master point is not
    // necessarily on the patch itself! (it might just be connected to the
    // patch point via coupled patches).


    // Determine mapping:
    // - from patch point to coupled point (or -1)
    // - from coupled point to global patch point
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    globalIndex globalPPoints(meshPoints.size());

    labelList patchToCoupled(meshPoints.size(), -1);
    label nCoupled = 0;
    labelList coupledToGlobalPatch(pointSlavesMap.constructSize(), -1);

    // Note: loop over patch since usually smaller
    forAll(meshPoints, patchPointI)
    {
        label meshPointI = meshPoints[patchPointI];

        Map<label>::const_iterator iter = cpp.meshPointMap().find(meshPointI);

        if (iter != cpp.meshPointMap().end())
        {
            patchToCoupled[patchPointI] = iter();
            coupledToGlobalPatch[iter()] = globalPPoints.toGlobal(patchPointI);
            nCoupled++;
        }
    }

    //Pout<< "Patch:" << nl
    //    << "    points:" << meshPoints.size() << nl
    //    << "    of which on coupled patch:" << nCoupled << endl;


    // Determine master of connected points
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Problem is that the coupled master might not be on the patch. So
    // work out the best patch-point master from all connected points.
    // - if the coupled master is on the patch it becomes the patch-point master
    // - else the slave with the lowest numbered patch point label

    // Get all data on master
    pointSlavesMap.distribute(coupledToGlobalPatch);
    forAll(pointSlaves, coupledPointI)
    {
        const labelList& slaves = pointSlaves[coupledPointI];

        if (slaves.size() > 0)
        {
            // I am master. What is the best candidate for patch-point master
            label masterI = labelMax;
            if (coupledToGlobalPatch[coupledPointI] != -1)
            {
                // I am master and on the coupled patch. Use me.
                masterI = coupledToGlobalPatch[coupledPointI];
            }
            else
            {
                // Get min of slaves as master.
                forAll(slaves, i)
                {
                    label slavePp = coupledToGlobalPatch[slaves[i]];
                    if (slavePp != -1 && slavePp < masterI)
                    {
                        masterI = slavePp;
                    }
                }
            }

            if (masterI != labelMax)
            {
                // Push back
                coupledToGlobalPatch[coupledPointI] = masterI;
                forAll(slaves, i)
                {
                    coupledToGlobalPatch[slaves[i]] = masterI;
                }
            }
        }
    }
    pointSlavesMap.reverseDistribute(cpp.nPoints(), coupledToGlobalPatch);


    // Generate compact numbering for master points
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Now coupledToGlobalPatch is the globalIndex of the master point.
    // Now every processor can check whether they hold it and generate a
    // compact numbering.

    label nMasters = 0;
    forAll(meshPoints, patchPointI)
    {
        if (patchToCoupled[patchPointI] == -1)
        {
            nMasters++;
        }
        else
        {
            label coupledPointI = patchToCoupled[patchPointI];
            if
            (
                globalPPoints.toGlobal(patchPointI)
             == coupledToGlobalPatch[coupledPointI]
            )
            {
                // I am the master
                nMasters++;
            }
        }
    }

    autoPtr<globalIndex> globalPointsPtr(new globalIndex(nMasters));

    //Pout<< "Patch:" << nl
    //    << "    points:" << meshPoints.size() << nl
    //    << "    of which on coupled patch:" << nCoupled << nl
    //    << "    of which master:" << nMasters << endl;



    // Push back compact numbering for master point onto slaves
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    pointToGlobal.setSize(meshPoints.size());
    pointToGlobal = -1;
    uniqueMeshPoints.setSize(nMasters);

    // Sync master in global point numbering so all know the master point.
    // Initialise globalMaster to be -1 except at a globalMaster.
    labelList globalMaster(cpp.nPoints(), -1);

    nMasters = 0;
    forAll(meshPoints, patchPointI)
    {
        if (patchToCoupled[patchPointI] == -1)
        {
            uniqueMeshPoints[nMasters++] = meshPoints[patchPointI];
        }
        else
        {
            label coupledPointI = patchToCoupled[patchPointI];
            if
            (
                globalPPoints.toGlobal(patchPointI)
             == coupledToGlobalPatch[coupledPointI]
            )
            {
                globalMaster[coupledPointI] =
                    globalPointsPtr().toGlobal(nMasters);
                uniqueMeshPoints[nMasters++] = meshPoints[patchPointI];
            }
        }
    }


    // Sync by taking max
    syncData
    (
        globalMaster,
        pointSlaves,
        labelListList(cpp.nPoints()),   // no transforms
        pointSlavesMap,
        maxEqOp<label>()
    );


    // Now everyone has the master point in globalPointsPtr numbering. Fill
    // in the pointToGlobal map.
    nMasters = 0;
    forAll(meshPoints, patchPointI)
    {
        if (patchToCoupled[patchPointI] == -1)
        {
            pointToGlobal[patchPointI] = globalPointsPtr().toGlobal(nMasters++);
        }
        else
        {
            label coupledPointI = patchToCoupled[patchPointI];
            pointToGlobal[patchPointI] = globalMaster[coupledPointI];

            if
            (
                globalPPoints.toGlobal(patchPointI)
             == coupledToGlobalPatch[coupledPointI]
            )
            {
                nMasters++;
            }
        }
    }

    return globalPointsPtr;
}


void CML::globalMeshData::movePoints(const pointField& newPoints)
{
    // Topology does not change and we don't store any geometry so nothing
    // needs to be done.
    // Only global transformations might change but this is not really
    // supported.
}


// Update all data after morph
void CML::globalMeshData::updateMesh()
{
    // Clear out old data
    clearOut();

    // Do processor patch addressing
    initProcAddr();

    scalar tolDim = matchTol_ * mesh_.bounds().mag();

    if (debug)
    {
        Pout<< "globalMeshData : merge dist:" << tolDim << endl;
    }

    // Total number of faces.
    nTotalFaces_ = returnReduce(mesh_.nFaces(), sumOp<label>());

    if (debug)
    {
        Pout<< "globalMeshData : nTotalFaces_:" << nTotalFaces_ << endl;
    }

    nTotalCells_ = returnReduce(mesh_.nCells(), sumOp<label>());

    if (debug)
    {
        Pout<< "globalMeshData : nTotalCells_:" << nTotalCells_ << endl;
    }

    nTotalPoints_ = returnReduce(mesh_.nPoints(), sumOp<label>());

    if (debug)
    {
        Pout<< "globalMeshData : nTotalPoints_:" << nTotalPoints_ << endl;
    }
}


// ************************************************************************* //
