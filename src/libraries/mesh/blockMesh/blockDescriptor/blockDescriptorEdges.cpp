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

\*---------------------------------------------------------------------------*/

#include "blockDescriptor.hpp"
#include "lineEdge.hpp"
#include "lineDivide.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::blockDescriptor::makeBlockEdges()
{
    const label ni = meshDensity_.x();
    const label nj = meshDensity_.y();
    const label nk = meshDensity_.z();

    // These edges correspond to the "hex" cellModel

    // X-direction
    setEdge(0,  0, 1, ni);
    setEdge(1,  3, 2, ni);
    setEdge(2,  7, 6, ni);
    setEdge(3,  4, 5, ni);

    // Y-direction
    setEdge(4,  0, 3, nj);
    setEdge(5,  1, 2, nj);
    setEdge(6,  5, 6, nj);
    setEdge(7,  4, 7, nj);

    // Z-direction
    setEdge(8,  0, 4, nk);
    setEdge(9,  1, 5, nk);
    setEdge(10, 2, 6, nk);
    setEdge(11, 3, 7, nk);
}


void CML::blockDescriptor::setEdge
(
    label edgeI,
    label start,
    label end,
    label nDiv
)
{
    // Set reference to the list of labels defining the block
    const labelList& blockLabels = blockShape_;

    // Set reference to global list of points
    const pointField blockPoints = blockShape_.points(blockPointField_);

    // Set the edge points/weights
    // The edge is a straight-line if it is not in the list of curvedEdges

    forAll(curvedEdges_, cedgeI)
    {
        const curvedEdge& cedge = curvedEdges_[cedgeI];

        int cmp = cedge.compare(blockLabels[start], blockLabels[end]);

        if (cmp)
        {
            if (cmp > 0)
            {
                // Curve has the same orientation

                // Divide the line
                lineDivide divEdge(cedge, nDiv, expand_[edgeI]);

                edgePoints_[edgeI]  = divEdge.points();
                edgeWeights_[edgeI] = divEdge.lambdaDivisions();
            }
            else
            {
                // Curve has the opposite orientation

                // Divide the line
                lineDivide divEdge(cedge, nDiv, expand_[edgeI].inv());

                const pointField& p = divEdge.points();
                const scalarList& d = divEdge.lambdaDivisions();

                edgePoints_[edgeI].setSize(p.size());
                edgeWeights_[edgeI].setSize(d.size());

                label pMax = p.size() - 1;
                forAll(p, pI)
                {
                    edgePoints_[edgeI][pI]  = p[pMax - pI];
                    edgeWeights_[edgeI][pI] = 1.0 - d[pMax - pI];
                }
            }

            // Found curved-edge: done
            return;
        }
    }


    // Not curved-edge: divide the edge as a straight line
    lineDivide divEdge
    (
        lineEdge(blockPoints, start, end),
        nDiv,
        expand_[edgeI]
    );

    edgePoints_[edgeI]  = divEdge.points();
    edgeWeights_[edgeI] = divEdge.lambdaDivisions();
}


// ************************************************************************* //
