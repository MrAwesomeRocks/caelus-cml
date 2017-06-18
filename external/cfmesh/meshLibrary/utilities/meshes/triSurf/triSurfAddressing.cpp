/*---------------------------------------------------------------------------*\
Copyright (C) Creative Fields, Ltd.
-------------------------------------------------------------------------------
License
    This file is part of cfMesh.

    cfMesh is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 3 of the License, or (at your
    option) any later version.

    cfMesh is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with cfMesh.  If not, see <http://www.gnu.org/licenses/>.

Author: Franjo Juretic (franjo.juretic@c-fields.com)

\*---------------------------------------------------------------------------*/

#include "triSurfAddressing.hpp"
#include "VRWGraphSMPModifier.hpp"
#include "demandDrivenData.hpp"

#include <set>

# ifdef USE_OMP
#include <omp.h>
# endif

namespace CML
{

void triSurfAddressing::calculatePointFacets() const
{
    pointFacetsPtr_ = new VRWGraph();

    VRWGraphSMPModifier(*pointFacetsPtr_).reverseAddressing(facets_);
}

void triSurfAddressing::calculateEdges() const
{
    edgesPtr_ = new edgeLongList();

    const VRWGraph& pFacets = pointFacets();

    # ifdef USE_OMP
    label nThreads = 3 * omp_get_num_procs();
    if( pFacets.size() < 1000 )
        nThreads = 1;
    # else
    const label nThreads(1);
    # endif

    labelList nEdgesForThread(nThreads);

    label edgeI(0);

    # ifdef USE_OMP
    # pragma omp parallel num_threads(nThreads)
    # endif
    {
        edgeLongList edgesHelper;

        # ifdef USE_OMP
        # pragma omp for schedule(static)
        # endif
        forAll(pFacets, pI)
        {
            std::set<std::pair<label, label> > edgesAtPoint;

            forAllRow(pFacets, pI, pfI)
            {
                const label triI = pFacets(pI, pfI);
                const labelledTri& tri = facets_[triI];

                forAll(tri, i)
                {
                    if( tri[i] == pI )
                    {
                        if( tri[(i+1)%3] >= pI )
                            edgesAtPoint.insert
                            (
                                std::make_pair(pI, tri[(i+1)%3])
                            );
                        if( tri[(i+2)%3] >= pI )
                            edgesAtPoint.insert
                            (
                                std::make_pair(pI, tri[(i+2)%3])
                            );
                    }
                }
            }

            std::set<std::pair<label, label> >::const_iterator it;
            for(it=edgesAtPoint.begin();it!=edgesAtPoint.end();++it)
                edgesHelper.append(edge(it->first, it->second));
        }

        //- this enables other threads to see the number of edges
        //- generated by each thread
        # ifdef USE_OMP
        const label threadI = omp_get_thread_num();
        # else
        const label threadI(0);
        # endif
        nEdgesForThread[threadI] = edgesHelper.size();

        # ifdef USE_OMP
        # pragma omp critical
        # endif
        edgeI += edgesHelper.size();

        # ifdef USE_OMP
        # pragma omp barrier

        # pragma omp master
        # endif
        edgesPtr_->setSize(edgeI);

        # ifdef USE_OMP
        # pragma omp barrier
        # endif

        //- find the starting position of the edges generated by this thread
        //- in the global list of edges
        label localStart(0);
        for(label i=0;i<threadI;++i)
            localStart += nEdgesForThread[i];

        //- store edges into the global list
        forAll(edgesHelper, i)
            edgesPtr_->operator[](localStart+i) = edgesHelper[i];
    }
}

void triSurfAddressing::calculateFacetEdges() const
{
    const edgeLongList& edges = this->edges();
    const VRWGraph& pointFaces = this->pointFacets();

    facetEdgesPtr_ = new VRWGraph(facets_.size(), 3, -1);
    VRWGraph& faceEdges = *facetEdgesPtr_;

    # ifdef USE_OMP
    # pragma omp parallel for schedule(dynamic, 100)
    # endif
    forAll(edges, edgeI)
    {
        const edge ee = edges[edgeI];
        const label pI = ee.start();

        forAllRow(pointFaces, pI, pfI)
        {
            const label fI = pointFaces(pI, pfI);

            const labelledTri& tri = facets_[fI];
            forAll(tri, eI)
            {
                const edge e(tri[eI], tri[(eI+1)%3]);

                if( e == ee )
                {
                    faceEdges(fI, eI) = edgeI;
                }
            }
        }
    }

    # ifdef DEBUGTriSurfAddressing
    forAll(faceEdges, triI)
    {
        forAllRow(faceEdges, triI, feI)
        {
            if( facets_[triI][feI] < 0 || facets_[triI][feI] >= points_.size() )
                FatalErrorIn
                (
                    "void triSurfAddressing::calculateFacetEdges() const"
                ) << "Invalid entry in triangle " << triI
                  << " " << facets_[triI] << abort(FatalError);

            const label edgeI = faceEdges(triI, feI);

            if( edgeI < 0 || edgeI >= edges.size() )
                FatalErrorIn
                (
                    "void triSurfAddressing::calculateFacetEdges() const"
                ) << "Invalid entry in face " << triI << " "
                     << facets_[triI] << " edges "
                     << faceEdges[triI] << abort(FatalError);
        }
    }
    # endif
}

void triSurfAddressing::calculateEdgeFacets() const
{
    const edgeLongList& edges = this->edges();
    const VRWGraph& faceEdges = this->facetEdges();

    edgeFacetsPtr_ = new VRWGraph(edges.size());

    VRWGraphSMPModifier(*edgeFacetsPtr_).reverseAddressing(faceEdges);
}

void triSurfAddressing::calculatePointEdges() const
{
    const edgeLongList& edges = this->edges();

    pointEdgesPtr_ = new VRWGraph(points_.size());

    pointEdgesPtr_->reverseAddressing(edges);
}

void triSurfAddressing::calculateFacetFacetsEdges() const
{
    facetFacetsEdgesPtr_ = new VRWGraph();

    const VRWGraph& facetEdges = this->facetEdges();
    const VRWGraph& edgeFacets = this->edgeFacets();

    facetFacetsEdgesPtr_->setSize(facets_.size());

    forAll(facetEdges, facetI)
    {
        labelHashSet fLabels;

        forAllRow(facetEdges, facetI, feI)
        {
            const label edgeI = facetEdges(facetI, feI);

            forAllRow(edgeFacets, edgeI, efI)
                fLabels.insert(edgeFacets(edgeI, efI));
        }

        facetFacetsEdgesPtr_->setRowSize(facetI, fLabels.size());

        label counter(0);
        forAllConstIter(labelHashSet, fLabels, iter)
            facetFacetsEdgesPtr_->operator()(facetI, counter++) = iter.key();
    }
}

void triSurfAddressing::calculatePointNormals() const
{
    const VRWGraph& pFacets = pointFacets();
    const vectorField& fNormals = facetNormals();

    pointNormalsPtr_ = new vectorField(pFacets.size());

    const label size = pFacets.size();
    # ifdef USE_OMP
    # pragma omp parallel for if( size > 1000 )
    # endif
    for(label pI=0;pI<size;++pI)
    {
        vector normal(vector::zero);

        forAllRow(pFacets, pI, pfI)
            normal += fNormals[pFacets(pI, pfI)];

        const scalar d = mag(normal);
        if( d > VSMALL )
        {
            normal /= d;
        }
        else
        {
            normal = vector::zero;
        }

        (*pointNormalsPtr_)[pI] = normal;
    }
}

void triSurfAddressing::calculateFacetNormals() const
{
    facetNormalsPtr_ = new vectorField(facets_.size());

    # ifdef USE_OMP
    # pragma omp parallel for schedule(dynamic, 40)
    # endif
    forAll(facets_, fI)
    {
        vector v = facets_[fI].normal(points_);
        v /= (mag(v) + VSMALL);
        (*facetNormalsPtr_)[fI] = v;
    }
}

void triSurfAddressing::calculateFacetCentres() const
{
    facetCentresPtr_ = new vectorField(facets_.size());

    # ifdef USE_OMP
    # pragma omp parallel for schedule(dynamic, 40)
    # endif
    forAll(facets_, fI)
        (*facetCentresPtr_)[fI] = facets_[fI].centre(points_);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

triSurfAddressing::triSurfAddressing
(
    const pointField& points,
    const LongList<labelledTri>& triangles)
:
    points_(points),
    facets_(triangles),
    pointFacetsPtr_(NULL),
    edgesPtr_(NULL),
    facetEdgesPtr_(NULL),
    edgeFacetsPtr_(NULL),
    pointEdgesPtr_(NULL),
    facetFacetsEdgesPtr_(NULL),
    pointNormalsPtr_(NULL),
    facetNormalsPtr_(NULL),
    facetCentresPtr_(NULL)

{}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

triSurfAddressing::~triSurfAddressing()
{
    clearAddressing();
    clearGeometry();
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void triSurfAddressing::clearAddressing()
{
    deleteDemandDrivenData(pointFacetsPtr_);
    deleteDemandDrivenData(edgesPtr_);
    deleteDemandDrivenData(facetEdgesPtr_);
    deleteDemandDrivenData(edgeFacetsPtr_);
    deleteDemandDrivenData(pointEdgesPtr_);
    deleteDemandDrivenData(facetFacetsEdgesPtr_);
}

void triSurfAddressing::clearGeometry()
{
    deleteDemandDrivenData(pointNormalsPtr_);
    deleteDemandDrivenData(facetNormalsPtr_);
    deleteDemandDrivenData(facetCentresPtr_);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //