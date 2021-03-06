/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2018 OpenFOAM Foundation
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

#include "treeBoundBox.hpp"
#include "ListOps.hpp"
#include "OFstream.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

const CML::scalar CML::treeBoundBox::great(GREAT);

const CML::treeBoundBox CML::treeBoundBox::greatBox
(
    vector(-GREAT, -GREAT, -GREAT),
    vector(GREAT, GREAT, GREAT)
);


const CML::treeBoundBox CML::treeBoundBox::invertedBox
(
    vector(GREAT, GREAT, GREAT),
    vector(-GREAT, -GREAT, -GREAT)
);


//! \cond - skip documentation : local scope only
const CML::label facesArray[6][4] =
{
    {0, 4, 6, 2}, // left
    {1, 3, 7, 5}, // right
    {0, 1, 5, 4}, // bottom
    {2, 6, 7, 3}, // top
    {0, 2, 3, 1}, // back
    {4, 5, 7, 6}  // front
};
//! \endcond


const CML::faceList CML::treeBoundBox::faces
(
    initListList<face, label, 6, 4>(facesArray)
);


//! \cond - skip documentation : local scope only
const CML::label edgesArray[12][2] =
{
    {0, 1}, // 0
    {1, 3},
    {2, 3}, // 2
    {0, 2},
    {4, 5}, // 4
    {5, 7},
    {6, 7}, // 6
    {4, 6},
    {0, 4}, // 8
    {1, 5},
    {3, 7}, // 10
    {2, 6}
};
//! \endcond


const CML::edgeList CML::treeBoundBox::edges
(
    //initListList<edge, label, 12, 2>(edgesArray)
    calcEdges(edgesArray)
);


const CML::FixedList<CML::vector, 6> CML::treeBoundBox::faceNormals
(
    calcFaceNormals()
);


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

CML::edgeList CML::treeBoundBox::calcEdges(const label edgesArray[12][2])
{
    edgeList edges(12);
    forAll(edges, edgeI)
    {
        edges[edgeI][0] = edgesArray[edgeI][0];
        edges[edgeI][1] = edgesArray[edgeI][1];
    }
    return edges;
}


CML::FixedList<CML::vector, 6> CML::treeBoundBox::calcFaceNormals()
{
    FixedList<vector, 6> normals;
    normals[LEFT]   = vector(-1,  0,  0);
    normals[RIGHT]  = vector( 1,  0,  0);
    normals[BOTTOM] = vector( 0, -1,  0);
    normals[TOP]    = vector( 0,  1,  0);
    normals[BACK]   = vector( 0,  0, -1);
    normals[FRONT]  = vector( 0,  0,  1);
    return normals;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::treeBoundBox::treeBoundBox(const UList<point>& points)
:
    boundBox(points, false)
{
    if (points.empty())
    {
        WarningInFunction
            << "cannot find bounding box for zero-sized pointField, "
            << "returning zero" << endl;

        return;
    }
}


CML::treeBoundBox::treeBoundBox
(
    const UList<point>& points,
    const labelUList& indices
)
:
    boundBox(points, indices, false)
{
    if (points.empty() || indices.empty())
    {
        WarningInFunction
            << "cannot find bounding box for zero-sized pointField, "
            << "returning zero" << endl;

        return;
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::tmp<CML::pointField> CML::treeBoundBox::points() const
{
    tmp<pointField> tPts = tmp<pointField>(new pointField(8));
    pointField& points = tPts();

    forAll(points, octant)
    {
        points[octant] = corner(octant);
    }

    return tPts;
}


CML::treeBoundBox CML::treeBoundBox::subBbox(const direction octant) const
{
    return subBbox(midpoint(), octant);
}


// Octant to bounding box using permutation only.
CML::treeBoundBox CML::treeBoundBox::subBbox
(
    const point& mid,
    const direction octant
) const
{
    if (octant > 7)
    {
        FatalErrorInFunction
            << "octant should be [0..7]"
            << abort(FatalError);
    }

    // start with a copy of this bounding box and adjust limits accordingly
    treeBoundBox subBb(*this);
    point& bbMin = subBb.min();
    point& bbMax = subBb.max();

    if (octant & treeBoundBox::RIGHTHALF)
    {
        bbMin.x() = mid.x();    // mid -> max
    }
    else
    {
        bbMax.x() = mid.x();    // min -> mid
    }

    if (octant & treeBoundBox::TOPHALF)
    {
        bbMin.y() = mid.y();    // mid -> max
    }
    else
    {
        bbMax.y() = mid.y();    // min -> mid
    }

    if (octant & treeBoundBox::FRONTHALF)
    {
        bbMin.z() = mid.z();    // mid -> max
    }
    else
    {
        bbMax.z() = mid.z();    // min -> mid
    }

    return subBb;
}


bool CML::treeBoundBox::overlaps
(
    const point& centre,
    const scalar radiusSqr
) const
{
    // Find out where centre is in relation to bb.
    // Find nearest point on bb.
    scalar distSqr = 0;

    for (direction dir = 0; dir < vector::nComponents; dir++)
    {
        scalar d0 = min()[dir] - centre[dir];
        scalar d1 = max()[dir] - centre[dir];

        if ((d0 > 0) != (d1 > 0))
        {
            // centre inside both extrema. This component does not add any
            // distance.
        }
        else if (CML::mag(d0) < CML::mag(d1))
        {
            distSqr += d0*d0;
        }
        else
        {
            distSqr += d1*d1;
        }

        if (distSqr > radiusSqr)
        {
            return false;
        }
    }

    return true;
}


// line intersection. Returns true if line (start to end) inside
// bb or intersects bb. Sets pt to intersection.
//
// Sutherlands algorithm:
//   loop
//     - start = intersection of line with one of the planes bounding
//       the bounding box
//     - stop if start inside bb (return true)
//     - stop if start and end in same 'half' (e.g. both above bb)
//       (return false)
//
// Uses posBits to efficiently determine 'half' in which start and end
// point are.
//
// Note:
//   - sets coordinate to exact position: e.g. pt.x() = min().x()
//     since plane intersect routine might have truncation error.
//     This makes sure that posBits tests 'inside'
bool CML::treeBoundBox::intersects
(
    const point& overallStart,
    const vector& overallVec,
    const point& start,
    const point& end,
    point& pt,
    direction& ptOnFaces
) const
{
    // Sutherlands algorithm:
    //   loop
    //     - start = intersection of line with one of the planes bounding
    //       the bounding box
    //     - stop if start inside bb (return true)
    //     - stop if start and end in same 'half' (e.g. both above bb)
    //       (return false)
    //
    // Uses posBits to efficiently determine 'half' in which start and end
    // point are.
    //
    // Note:
    //   - sets coordinate to exact position: e.g. pt.x() = min().x()
    //     since plane intersect routine might have truncation error.
    //     This makes sure that posBits tests 'inside'

    const direction endBits = posBits(end);
    pt = start;

    // Allow maximum of 3 clips.
    for (label i = 0; i < 4; ++i)
    {
        direction ptBits = posBits(pt);

        if (ptBits == 0)
        {
            // pt inside bb
            ptOnFaces = faceBits(pt);
            return true;
        }

        if ((ptBits & endBits) != 0)
        {
            // pt and end in same block outside of bb
            ptOnFaces = faceBits(pt);
            return false;
        }

        if (ptBits & LEFTBIT)
        {
            // Intersect with plane V=min, n=-1,0,0
            if (CML::mag(overallVec.x()) > VSMALL)
            {
                scalar s = (min().x() - overallStart.x())/overallVec.x();
                pt.x() = min().x();
                pt.y() = overallStart.y() + overallVec.y()*s;
                pt.z() = overallStart.z() + overallVec.z()*s;
            }
            else
            {
                // Vector not in x-direction. But still intersecting bb planes.
                // So must be close - just snap to bb.
                pt.x() = min().x();
            }
        }
        else if (ptBits & RIGHTBIT)
        {
            // Intersect with plane V=max, n=1,0,0
            if (CML::mag(overallVec.x()) > VSMALL)
            {
                scalar s = (max().x() - overallStart.x())/overallVec.x();
                pt.x() = max().x();
                pt.y() = overallStart.y() + overallVec.y()*s;
                pt.z() = overallStart.z() + overallVec.z()*s;
            }
            else
            {
                pt.x() = max().x();
            }
        }
        else if (ptBits & BOTTOMBIT)
        {
            // Intersect with plane V=min, n=0,-1,0
            if (CML::mag(overallVec.y()) > VSMALL)
            {
                scalar s = (min().y() - overallStart.y())/overallVec.y();
                pt.x() = overallStart.x() + overallVec.x()*s;
                pt.y() = min().y();
                pt.z() = overallStart.z() + overallVec.z()*s;
            }
            else
            {
                pt.x() = min().y();
            }
        }
        else if (ptBits & TOPBIT)
        {
            // Intersect with plane V=max, n=0,1,0
            if (CML::mag(overallVec.y()) > VSMALL)
            {
                scalar s = (max().y() - overallStart.y())/overallVec.y();
                pt.x() = overallStart.x() + overallVec.x()*s;
                pt.y() = max().y();
                pt.z() = overallStart.z() + overallVec.z()*s;
            }
            else
            {
                pt.y() = max().y();
            }
        }
        else if (ptBits & BACKBIT)
        {
            // Intersect with plane V=min, n=0,0,-1
            if (CML::mag(overallVec.z()) > VSMALL)
            {
                scalar s = (min().z() - overallStart.z())/overallVec.z();
                pt.x() = overallStart.x() + overallVec.x()*s;
                pt.y() = overallStart.y() + overallVec.y()*s;
                pt.z() = min().z();
            }
            else
            {
                pt.z() = min().z();
            }
        }
        else if (ptBits & FRONTBIT)
        {
            // Intersect with plane V=max, n=0,0,1
            if (CML::mag(overallVec.z()) > VSMALL)
            {
                scalar s = (max().z() - overallStart.z())/overallVec.z();
                pt.x() = overallStart.x() + overallVec.x()*s;
                pt.y() = overallStart.y() + overallVec.y()*s;
                pt.z() = max().z();
            }
            else
            {
                pt.z() = max().z();
            }
        }
    }

    // Can end up here if the end point is on the edge of the boundBox
    return true;
}


bool CML::treeBoundBox::intersects
(
    const point& start,
    const point& end,
    point& pt
) const
{
    direction ptBits;
    return intersects(start, end-start, start, end, pt, ptBits);
}


bool CML::treeBoundBox::contains(const vector& dir, const point& pt) const
{
    // Compare all components against min and max of bb

    for (direction cmpt=0; cmpt<3; cmpt++)
    {
        if (pt[cmpt] < min()[cmpt])
        {
            return false;
        }
        else if (pt[cmpt] == min()[cmpt])
        {
            // On edge. Outside if direction points outwards.
            if (dir[cmpt] < 0)
            {
                return false;
            }
        }

        if (pt[cmpt] > max()[cmpt])
        {
            return false;
        }
        else if (pt[cmpt] == max()[cmpt])
        {
            // On edge. Outside if direction points outwards.
            if (dir[cmpt] > 0)
            {
                return false;
            }
        }
    }

    // All components inside bb
    return true;
}


// Code position of pt on bounding box faces
CML::direction CML::treeBoundBox::faceBits(const point& pt) const
{
    direction faceBits = 0;
    if (pt.x() == min().x())
    {
        faceBits |= LEFTBIT;
    }
    else if (pt.x() == max().x())
    {
        faceBits |= RIGHTBIT;
    }

    if (pt.y() == min().y())
    {
        faceBits |= BOTTOMBIT;
    }
    else if (pt.y() == max().y())
    {
        faceBits |= TOPBIT;
    }

    if (pt.z() == min().z())
    {
        faceBits |= BACKBIT;
    }
    else if (pt.z() == max().z())
    {
        faceBits |= FRONTBIT;
    }
    return faceBits;
}


// Code position of point relative to box
CML::direction CML::treeBoundBox::posBits(const point& pt) const
{
    direction posBits = 0;

    if (pt.x() < min().x())
    {
        posBits |= LEFTBIT;
    }
    else if (pt.x() > max().x())
    {
        posBits |= RIGHTBIT;
    }

    if (pt.y() < min().y())
    {
        posBits |= BOTTOMBIT;
    }
    else if (pt.y() > max().y())
    {
        posBits |= TOPBIT;
    }

    if (pt.z() < min().z())
    {
        posBits |= BACKBIT;
    }
    else if (pt.z() > max().z())
    {
        posBits |= FRONTBIT;
    }
    return posBits;
}


// nearest and furthest corner coordinate.
// !names of treeBoundBox::min() and treeBoundBox::max() are confusing!
void CML::treeBoundBox::calcExtremities
(
    const point& pt,
    point& nearest,
    point& furthest
) const
{
    scalar nearX, nearY, nearZ;
    scalar farX, farY, farZ;

    if (CML::mag(min().x() - pt.x()) < CML::mag(max().x() - pt.x()))
    {
        nearX = min().x();
        farX = max().x();
    }
    else
    {
        nearX = max().x();
        farX = min().x();
    }

    if (CML::mag(min().y() - pt.y()) < CML::mag(max().y() - pt.y()))
    {
        nearY = min().y();
        farY = max().y();
    }
    else
    {
        nearY = max().y();
        farY = min().y();
    }

    if (CML::mag(min().z() - pt.z()) < CML::mag(max().z() - pt.z()))
    {
        nearZ = min().z();
        farZ = max().z();
    }
    else
    {
        nearZ = max().z();
        farZ = min().z();
    }

    nearest = point(nearX, nearY, nearZ);
    furthest = point(farX, farY, farZ);
}


CML::scalar CML::treeBoundBox::maxDist(const point& pt) const
{
    point near, far;
    calcExtremities(pt, near, far);

    return CML::mag(far - pt);
}


// Distance comparator
// Compare all vertices of bounding box against all of other bounding
// box to see if all vertices of one are nearer
CML::label CML::treeBoundBox::distanceCmp
(
    const point& pt,
    const treeBoundBox& other
) const
{
    //
    // Distance point <-> nearest and furthest away vertex of this
    //

    point nearThis, farThis;

    // get nearest and furthest away vertex
    calcExtremities(pt, nearThis, farThis);

    const scalar minDistThis =
        sqr(nearThis.x() - pt.x())
     +  sqr(nearThis.y() - pt.y())
     +  sqr(nearThis.z() - pt.z());
    const scalar maxDistThis =
        sqr(farThis.x() - pt.x())
     +  sqr(farThis.y() - pt.y())
     +  sqr(farThis.z() - pt.z());

    //
    // Distance point <-> other
    //

    point nearOther, farOther;

    // get nearest and furthest away vertex
    other.calcExtremities(pt, nearOther, farOther);

    const scalar minDistOther =
        sqr(nearOther.x() - pt.x())
     +  sqr(nearOther.y() - pt.y())
     +  sqr(nearOther.z() - pt.z());
    const scalar maxDistOther =
        sqr(farOther.x() - pt.x())
     +  sqr(farOther.y() - pt.y())
     +  sqr(farOther.z() - pt.z());

    //
    // Categorize
    //
    if (maxDistThis < minDistOther)
    {
        // All vertices of this are nearer to point than any vertex of other
        return -1;
    }
    else if (minDistThis > maxDistOther)
    {
        // All vertices of this are further from point than any vertex of other
        return 1;
    }
    else
    {
        // Mixed bag
        return 0;
    }
}


void CML::treeBoundBox::writeOBJ(const fileName& fName) const
{
    OFstream str(fName);

    Info<< "Dumping bounding box " << *this << " as lines to obj file "
        << str.name() << endl;

    pointField bbPoints(points());

    forAll(bbPoints, i)
    {
        const point& pt = bbPoints[i];

        str<< "v " << pt.x() << ' ' << pt.y() << ' ' << pt.z() << endl;
    }

    forAll(treeBoundBox::edges, i)
    {
        const edge& e = treeBoundBox::edges[i];

        str<< "l " << e[0]+1 <<  ' ' << e[1]+1 << nl;
    }
}


// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //

bool CML::operator==(const treeBoundBox& a, const treeBoundBox& b)
{
    return operator==
    (
        static_cast<const boundBox&>(a),
        static_cast<const boundBox&>(b)
    );
}


bool CML::operator!=(const treeBoundBox& a, const treeBoundBox& b)
{
    return !(a == b);
}


// * * * * * * * * * * * * * * * Ostream Operator  * * * * * * * * * * * * * //

CML::Ostream& CML::operator<<(Ostream& os, const treeBoundBox& bb)
{
    return os << static_cast<const boundBox&>(bb);
}


CML::Istream& CML::operator>>(Istream& is, treeBoundBox& bb)
{
    return is >> static_cast<boundBox&>(bb);
}


// ************************************************************************* //
