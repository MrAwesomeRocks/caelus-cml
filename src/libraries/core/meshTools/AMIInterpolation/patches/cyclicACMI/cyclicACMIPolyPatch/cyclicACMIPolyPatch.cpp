/*---------------------------------------------------------------------------*\
Copyright (C) 2013-2015 OpenFOAM Foundation
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

#include "cyclicACMIPolyPatch.hpp"
#include "SubField.hpp"
#include "Time.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(cyclicACMIPolyPatch, 0);

    addToRunTimeSelectionTable(polyPatch, cyclicACMIPolyPatch, word);
    addToRunTimeSelectionTable(polyPatch, cyclicACMIPolyPatch, dictionary);
}

const CML::scalar CML::cyclicACMIPolyPatch::tolerance_ = 1e-6;

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void CML::cyclicACMIPolyPatch::initPatchFaceAreas() const
{
    if (!empty() && faceAreas0_.empty())
    {
        faceAreas0_ = faceAreas();
    }

    const cyclicACMIPolyPatch& nbrACMI =
        refCast<const cyclicACMIPolyPatch>(this->neighbPatch());

    if (!nbrACMI.empty() && nbrACMI.faceAreas0().empty())
    {
        nbrACMI.initPatchFaceAreas();
    }
}


void CML::cyclicACMIPolyPatch::resetAMI
(
    const AMIPatchToPatchInterpolation::interpolationMethod&
) const
{
    if (owner())
    {
        const polyPatch& nonOverlapPatch = this->nonOverlapPatch();

        initPatchFaceAreas();

        // Reset patch face areas based on original patch for AMI calculation
        vectorField::subField Sf = faceAreas();
        vectorField::subField noSf = nonOverlapPatch.faceAreas();

        forAll(Sf, faceI)
        {
            Sf[faceI] = faceAreas0_[faceI];
            noSf[faceI] = faceAreas0_[faceI];
        }

        // Calculate the AMI using partial face-area-weighted
        cyclicAMIPolyPatch::resetAMI
        (
            AMIPatchToPatchInterpolation::imPartialFaceAreaWeight
        );

        srcMask_ =
            min(scalar(1) - tolerance_, max(tolerance_, AMI().srcWeightsSum()));

        tgtMask_ =
            min(scalar(1) - tolerance_, max(tolerance_, AMI().tgtWeightsSum()));

        forAll(Sf, faceI)
        {
            Sf[faceI] *= srcMask_[faceI];
            noSf[faceI] *= 1.0 - srcMask_[faceI];
        }

        setNeighbourFaceAreas();

        // Set the updated flag
        updated_ = true;
    }
}


void CML::cyclicACMIPolyPatch::setNeighbourFaceAreas() const
{
    const cyclicACMIPolyPatch& cp =
        refCast<const cyclicACMIPolyPatch>(this->neighbPatch());
    const polyPatch& pp = cp.nonOverlapPatch();

    const vectorField& faceAreas0 = cp.faceAreas0();

    if (tgtMask_.size() == cp.size())
    {
        vectorField::subField Sf = cp.faceAreas();
        vectorField::subField noSf = pp.faceAreas();

        forAll(Sf, faceI)
        {
            Sf[faceI] = tgtMask_[faceI]*faceAreas0[faceI];
            noSf[faceI] = (1.0 - tgtMask_[faceI])*faceAreas0[faceI];
        }
    }
    else
    {
        WarningInFunction
            << "Target mask size differs to that of the neighbour patch\n"
            << "    May occur when decomposing." << endl;
    }
}


void CML::cyclicACMIPolyPatch::initGeometry(PstreamBuffers& pBufs)
{
    // Initialise the AMI so that base geometry (e.g. cell volumes) are
    // correctly evaluated
    resetAMI();

    cyclicAMIPolyPatch::initGeometry(pBufs);
}


void CML::cyclicACMIPolyPatch::calcGeometry(PstreamBuffers& pBufs)
{
    cyclicAMIPolyPatch::calcGeometry(pBufs);
}


void CML::cyclicACMIPolyPatch::initMovePoints
(
    PstreamBuffers& pBufs,
    const pointField& p
)
{
    cyclicAMIPolyPatch::initMovePoints(pBufs, p);
}


void CML::cyclicACMIPolyPatch::movePoints
(
    PstreamBuffers& pBufs,
    const pointField& p
)
{
    cyclicAMIPolyPatch::movePoints(pBufs, p);
}


void CML::cyclicACMIPolyPatch::initUpdateMesh(PstreamBuffers& pBufs)
{
    cyclicAMIPolyPatch::initUpdateMesh(pBufs);
}


void CML::cyclicACMIPolyPatch::updateMesh(PstreamBuffers& pBufs)
{
    cyclicAMIPolyPatch::updateMesh(pBufs);
}


void CML::cyclicACMIPolyPatch::clearGeom()
{
    cyclicAMIPolyPatch::clearGeom();
}


const CML::scalarField& CML::cyclicACMIPolyPatch::srcMask() const
{
    return srcMask_;
}


const CML::scalarField& CML::cyclicACMIPolyPatch::tgtMask() const
{
    return tgtMask_;
}


// * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * * //

CML::cyclicACMIPolyPatch::cyclicACMIPolyPatch
(
    const word& name,
    const label size,
    const label start,
    const label index,
    const polyBoundaryMesh& bm,
    const word& patchType,
    const transformType transform
)
:
    cyclicAMIPolyPatch(name, size, start, index, bm, patchType, transform),
    faceAreas0_(),
    nonOverlapPatchName_(word::null),
    nonOverlapPatchID_(-1),
    srcMask_(),
    tgtMask_(),
    updated_(false)
{
    AMIRequireMatch_ = false;

    // Non-overlapping patch might not be valid yet so cannot determine
    // associated patchID
}


CML::cyclicACMIPolyPatch::cyclicACMIPolyPatch
(
    const word& name,
    const dictionary& dict,
    const label index,
    const polyBoundaryMesh& bm,
    const word& patchType
)
:
    cyclicAMIPolyPatch(name, dict, index, bm, patchType),
    faceAreas0_(),
    nonOverlapPatchName_(dict.lookup("nonOverlapPatch")),
    nonOverlapPatchID_(-1),
    srcMask_(),
    tgtMask_(),
    updated_(false)
{
    AMIRequireMatch_ = false;

    if (nonOverlapPatchName_ == name)
    {
        FatalIOErrorInFunction(dict)
            << "Non-overlapping patch name " << nonOverlapPatchName_
            << " cannot be the same as this patch " << name
            << exit(FatalIOError);
    }

    // Non-overlapping patch might not be valid yet so cannot determine
    // associated patchID
}


CML::cyclicACMIPolyPatch::cyclicACMIPolyPatch
(
    const cyclicACMIPolyPatch& pp,
    const polyBoundaryMesh& bm
)
:
    cyclicAMIPolyPatch(pp, bm),
    faceAreas0_(),
    nonOverlapPatchName_(pp.nonOverlapPatchName_),
    nonOverlapPatchID_(-1),
    srcMask_(),
    tgtMask_(),
    updated_(false)
{
    AMIRequireMatch_ = false;

    // Non-overlapping patch might not be valid yet so cannot determine
    // associated patchID
}


CML::cyclicACMIPolyPatch::cyclicACMIPolyPatch
(
    const cyclicACMIPolyPatch& pp,
    const polyBoundaryMesh& bm,
    const label index,
    const label newSize,
    const label newStart,
    const word& nbrPatchName,
    const word& nonOverlapPatchName
)
:
    cyclicAMIPolyPatch(pp, bm, index, newSize, newStart, nbrPatchName),
    faceAreas0_(),
    nonOverlapPatchName_(nonOverlapPatchName),
    nonOverlapPatchID_(-1),
    srcMask_(),
    tgtMask_(),
    updated_(false)
{
    AMIRequireMatch_ = false;

    if (nonOverlapPatchName_ == name())
    {
        FatalErrorInFunction
            << "Non-overlapping patch name " << nonOverlapPatchName_
            << " cannot be the same as this patch " << name()
            << exit(FatalError);
    }

    // Non-overlapping patch might not be valid yet so cannot determine
    // associated patchID
}


CML::cyclicACMIPolyPatch::cyclicACMIPolyPatch
(
    const cyclicACMIPolyPatch& pp,
    const polyBoundaryMesh& bm,
    const label index,
    const labelUList& mapAddressing,
    const label newStart
)
:
    cyclicAMIPolyPatch(pp, bm, index, mapAddressing, newStart),
    faceAreas0_(),
    nonOverlapPatchName_(pp.nonOverlapPatchName_),
    nonOverlapPatchID_(-1),
    srcMask_(),
    tgtMask_(),
    updated_(false)
{
    AMIRequireMatch_ = false;
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::cyclicACMIPolyPatch::~cyclicACMIPolyPatch()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const CML::cyclicACMIPolyPatch& CML::cyclicACMIPolyPatch::neighbPatch() const
{
    const polyPatch& pp = this->boundaryMesh()[neighbPatchID()];
    return refCast<const cyclicACMIPolyPatch>(pp);
}


CML::label CML::cyclicACMIPolyPatch::nonOverlapPatchID() const
{
    if (nonOverlapPatchID_ == -1)
    {
        nonOverlapPatchID_ =
            this->boundaryMesh().findPatchID(nonOverlapPatchName_);

        if (nonOverlapPatchID_ == -1)
        {
            FatalErrorInFunction
                << "Illegal non-overlapping patch name " << nonOverlapPatchName_
                << nl << "Valid patch names are "
                << this->boundaryMesh().names()
                << exit(FatalError);
        }

        if (nonOverlapPatchID_ < index())
        {
            FatalErrorInFunction
                << "Boundary ordering error: " << type()
                << " patch must be defined prior to its non-overlapping patch"
                << nl
                << type() << " patch: " << name() << ", ID:" << index() << nl
                << "Non-overlap patch: " << nonOverlapPatchName_
                << ", ID:" << nonOverlapPatchID_ << nl
                << exit(FatalError);
        }

        const polyPatch& noPp = this->boundaryMesh()[nonOverlapPatchID_];

        bool ok = true;

        if (size() == noPp.size())
        {
            const scalarField magSf(mag(faceAreas()));
            const scalarField noMagSf(mag(noPp.faceAreas()));

            forAll(magSf, faceI)
            {
                scalar ratio = mag(magSf[faceI]/(noMagSf[faceI] + ROOTVSMALL));

                if (ratio - 1 > tolerance_)
                {
                    ok = false;
                    break;
                }
            }
        }
        else
        {
            ok = false;
        }

        if (!ok)
        {
            FatalErrorInFunction
                << "Inconsistent ACMI patches " << name() << " and "
                << noPp.name() << ".  Patches should have identical topology"
                << exit(FatalError);
        }
    }

    return nonOverlapPatchID_;
}


void CML::cyclicACMIPolyPatch::calcGeometry
(
    const primitivePatch& referPatch,
    const pointField& thisCtrs,
    const vectorField& thisAreas,
    const pointField& thisCc,
    const pointField& nbrCtrs,
    const vectorField& nbrAreas,
    const pointField& nbrCc
)
{
    cyclicAMIPolyPatch::calcGeometry
    (
        referPatch,
        thisCtrs,
        thisAreas,
        thisCc,
        nbrCtrs,
        nbrAreas,
        nbrCc
    );
}


void CML::cyclicACMIPolyPatch::initOrder
(
    PstreamBuffers& pBufs,
    const primitivePatch& pp
) const
{
    cyclicAMIPolyPatch::initOrder(pBufs, pp);
}


bool CML::cyclicACMIPolyPatch::order
(
    PstreamBuffers& pBufs,
    const primitivePatch& pp,
    labelList& faceMap,
    labelList& rotation
) const
{
    return cyclicAMIPolyPatch::order(pBufs, pp, faceMap, rotation);
}


void CML::cyclicACMIPolyPatch::write(Ostream& os) const
{
    cyclicAMIPolyPatch::write(os);

    os.writeKeyword("nonOverlapPatch") << nonOverlapPatchName_
        << token::END_STATEMENT << nl;
}


// ************************************************************************* //
