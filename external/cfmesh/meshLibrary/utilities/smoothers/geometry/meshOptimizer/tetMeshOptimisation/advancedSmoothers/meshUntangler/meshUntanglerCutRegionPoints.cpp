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

#include "demandDrivenData.hpp"
#include "meshUntangler.hpp"
#include "plane.hpp"
#include "primitiveMesh.hpp"
#include "polyMeshGenModifier.hpp"
#include "helperFunctions.hpp"

//#define DEBUGSmooth

#ifdef DEBUGSmooth
#include "Time.hpp"
#include "objectRegistry.hpp"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

bool meshUntangler::cutRegion::findNewVertices
(
    const plane& plane
)
{
    #ifdef DEBUGSmooth
    Info << "Finding new vertices" << endl;
    #endif

    const DynList<point, 64>& points = *pointsPtr_;

    newVertexLabel_.setSize(points.size());
    newVertexLabel_ = label(-1);

    vertexDistance_.setSize(points.size());

    vertexTypes_.setSize(points.size());
    vertexTypes_ = direction(NONE);

    origNumVertices_ = label(0);

    cPtsPtr_ = new DynList<point, 64>();

    const point& rp = plane.refPoint();
    const vector& n = plane.normal();
    forAll(points, pI)
    {
        const point& p = points[pI];
        vertexDistance_[pI] = ((p - rp) & n);

        if( vertexDistance_[pI] > tol_ )
        {
            cPtsPtr_->append(p);
            newVertexLabel_[pI] = origNumVertices_++;
            vertexTypes_[pI] |= KEEP;
        }
        else if( vertexDistance_[pI] >= -tol_ )
        {
            cPtsPtr_->append(p);
            newVertexLabel_[pI] = origNumVertices_++;
            vertexTypes_[pI] |= INPLANE;
            vertexDistance_[pI] = 0.0;
        }
    }

    #ifdef DEBUGSmooth
    Info << "tolerance " << tol_ << endl;
    Info << "New number of vertices is " << origNumVertices_ << endl;
    forAll(points, pI)
        Info << "Original vertex " << pI << " is " << points[pI]
            << ". Vertex distance from plane is " << vertexDistance_[pI]
            << " and its new label is " << newVertexLabel_[pI] << endl;
    #endif

    if( origNumVertices_ < points.size() )
    {
        return true;
    }
    else
    {
        deleteDemandDrivenData(cPtsPtr_);

        return false;
    }
}

void meshUntangler::cutRegion::removeCoincidentVertices()
{
    const DynList<point, 64>& points = *pointsPtr_;
    DynList<edge, 128>& edges = *edgesPtr_;
    DynList<label, 64> newLabelForPoint;
    newLabelForPoint.setSize(points.size());
    newLabelForPoint = label(-1);

    bool found(false);
    forAll(points, pI)
    {
        if( newLabelForPoint[pI] != label(-1) ) continue;
        for(label pJ=pI+1;pJ<points.size();++pJ)
            if( mag(points[pJ] - points[pI]) < tol_ )
            {
                # ifdef DEBUGSmooth
                Info << "Vertices " << pI << " and " << pJ
                    << " are too close" << endl;
                # endif

                newLabelForPoint[pJ] = pI;
                found = true;
            }
    }

    if( !found )
        return;

    forAll(edges, eI)
    {
        edge& e = edges[eI];
        if( newLabelForPoint[e.start()] != label(-1) )
            e.start() = newLabelForPoint[e.start()];
        if( newLabelForPoint[e.end()] != label(-1) )
            e.end() = newLabelForPoint[e.end()];
    }

    //- remove edges which contain the same vertex
    newEdgeLabel_ = label(-1);
    label edgeLabel(0);

    cEdgesPtr_ = new DynList<edge, 128>();
    forAll(edges, eI)
        if( edges[eI].start() != edges[eI].end() )
        {
            cEdgesPtr_->append(edges[eI]);
            newEdgeLabel_[eI] = edgeLabel++;
        }

    deleteDemandDrivenData(edgesPtr_);
    edgesPtr_ = cEdgesPtr_;
    cEdgesPtr_ = nullptr;

    //- renumber faces
    const DynList<DynList<label, 8>, 64>& faces = *facesPtr_;
    cFacesPtr_ = new DynList<DynList<label, 8>, 64>();
    forAll(faces, fI)
    {
        const DynList<label, 8>& f = faces[fI];

        DynList<label, 8> nf;

        forAll(f, eI)
            if( newEdgeLabel_[f[eI]] != label(-1) )
                nf.append(newEdgeLabel_[f[eI]]);

        if( nf.size() > 2 )
        {
            cFacesPtr_->append(nf);
        }
    }

    deleteDemandDrivenData(facesPtr_);
    facesPtr_ = cFacesPtr_;
    cFacesPtr_ = nullptr;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
