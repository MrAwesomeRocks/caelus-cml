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

#include "wedgePolyPatch.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "SubField.hpp"
#include "transform.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(wedgePolyPatch, 0);

    addToRunTimeSelectionTable(polyPatch, wedgePolyPatch, word);
    addToRunTimeSelectionTable(polyPatch, wedgePolyPatch, dictionary);
}

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

void CML::wedgePolyPatch::initTransforms()
{
    if (size() > 0)
    {
        const pointField& points = this->points();

        patchNormal_ = operator[](0).normal(points);
        patchNormal_ /= mag(patchNormal_);

        centreNormal_ =
            vector
            (
                sign(patchNormal_.x())*(max(mag(patchNormal_.x()), 0.5) - 0.5),
                sign(patchNormal_.y())*(max(mag(patchNormal_.y()), 0.5) - 0.5),
                sign(patchNormal_.z())*(max(mag(patchNormal_.z()), 0.5) - 0.5)
            );
        centreNormal_ /= mag(centreNormal_);

        cosAngle_ = centreNormal_ & patchNormal_;

        const scalar cnCmptSum =
            centreNormal_.x() + centreNormal_.y() + centreNormal_.z();

        if (mag(cnCmptSum) < (1 - SMALL))
        {
            FatalErrorInFunction
                << "wedge " << name()
                << " centre plane does not align with a coordinate plane by "
                << 1 - mag(cnCmptSum)
                << exit(FatalError);
        }

        axis_ = centreNormal_ ^ patchNormal_;
        scalar magAxis = mag(axis_);

        if (magAxis < SMALL)
        {
            FatalErrorInFunction
                << "wedge " << name()
                << " plane aligns with a coordinate plane." << nl
                << "    The wedge plane should make a small angle (~2.5deg)"
                   " with the coordinate plane" << nl
                << "    and the the pair of wedge planes should be symmetric"
                << " about the coordinate plane." << nl
                << "    Normal of wedge plane is " << patchNormal_
                << " , implied coordinate plane direction is " << centreNormal_
                << exit(FatalError);
        }

        axis_ /= magAxis;

        faceT_ = rotationTensor(centreNormal_, patchNormal_);
        cellT_ = faceT_ & faceT_;
    }
}


// * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * * * * //

CML::wedgePolyPatch::wedgePolyPatch
(
    const word& name,
    const label size,
    const label start,
    const label index,
    const polyBoundaryMesh& bm,
    const word& patchType
)
:
    polyPatch(name, size, start, index, bm, patchType)
{
    initTransforms();
}


CML::wedgePolyPatch::wedgePolyPatch
(
    const word& name,
    const dictionary& dict,
    const label index,
    const polyBoundaryMesh& bm,
    const word& patchType
)
:
    polyPatch(name, dict, index, bm, patchType)
{
    initTransforms();
}


CML::wedgePolyPatch::wedgePolyPatch
(
    const wedgePolyPatch& pp,
    const polyBoundaryMesh& bm
)
:
    polyPatch(pp, bm)
{
    initTransforms();
}


CML::wedgePolyPatch::wedgePolyPatch
(
    const wedgePolyPatch& pp,
    const polyBoundaryMesh& bm,
    const label index,
    const label newSize,
    const label newStart
)
:
    polyPatch(pp, bm, index, newSize, newStart)
{
    initTransforms();
}


CML::wedgePolyPatch::wedgePolyPatch
(
    const wedgePolyPatch& pp,
    const polyBoundaryMesh& bm,
    const label index,
    const labelUList& mapAddressing,
    const label newStart
)
:
    polyPatch(pp, bm, index, mapAddressing, newStart)
{
    initTransforms();
}


// ************************************************************************* //
