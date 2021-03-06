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

#include "cartesianMeshExtractor.hpp"
#include "meshOctree.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
// Private member functions

void cartesianMeshExtractor::createPointsAndAddressing()
{
    Info << "Creating octree vertices" << endl;

    Info << "Octree nodes " << octreeCheck_.numberOfNodes() << endl;

    //- set the size of the point field
    pointFieldPMG& points = mesh_.points();
    points.setSize(octreeCheck_.numberOfNodes());

    //- store vertices into the pointField
    const pointField& octreePoints = octreeCheck_.octreePoints();

    forAll(points, pointI)
        points[pointI] = octreePoints[pointI];

    Info << "Finished creating octree vertices" << endl;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
