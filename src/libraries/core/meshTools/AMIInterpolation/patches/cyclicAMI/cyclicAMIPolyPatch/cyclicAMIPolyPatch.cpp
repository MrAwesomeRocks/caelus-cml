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

#include "cyclicAMIPolyPatch.hpp"
#include "transformField.hpp"
#include "SubField.hpp"
#include "polyMesh.hpp"
#include "Time.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "faceAreaIntersect.hpp"
#include "ops.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(cyclicAMIPolyPatch, 0);

    addToRunTimeSelectionTable(polyPatch, cyclicAMIPolyPatch, word);
    addToRunTimeSelectionTable(polyPatch, cyclicAMIPolyPatch, dictionary);
}


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

CML::vector CML::cyclicAMIPolyPatch::findFaceNormalMaxRadius
(
    const pointField& faceCentres
) const
{
    // Determine a face furthest away from the axis

    const vectorField n((faceCentres - rotationCentre_) ^ rotationAxis_);

    const scalarField magRadSqr(magSqr(n));

    label facei = findMax(magRadSqr);

    if (debug)
    {
        Info<< "findFaceMaxRadius(const pointField&) : patch: " << name() << nl
            << "    rotFace  = " << facei << nl
            << "    point    = " << faceCentres[facei] << nl
            << "    distance = " << CML::sqrt(magRadSqr[facei])
            << endl;
    }

    return n[facei];
}


// * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * * //

void CML::cyclicAMIPolyPatch::calcTransforms()
{
    // Half0
    const cyclicAMIPolyPatch& half0 = *this;
    vectorField half0Areas(half0.size());
    forAll(half0, facei)
    {
        half0Areas[facei] = half0[facei].normal(half0.points());
    }

    // Half1
    const cyclicAMIPolyPatch& half1 = neighbPatch();
    vectorField half1Areas(half1.size());
    forAll(half1, facei)
    {
        half1Areas[facei] = half1[facei].normal(half1.points());
    }

    calcTransforms
    (
        half0,
        half0.faceCentres(),
        half0Areas,
        half1.faceCentres(),
        half1Areas
    );

    if (debug)
    {
        Pout<< "calcTransforms() : patch: " << name() << nl
            << "    forwardT = " << forwardT() << nl
            << "    reverseT = " << reverseT() << nl
            << "    separation = " << separation() << nl
            << "    collocated = " << collocated() << nl << endl;
    }
}


void CML::cyclicAMIPolyPatch::calcTransforms
(
    const primitivePatch& half0,
    const pointField& half0Ctrs,
    const vectorField& half0Areas,
    const pointField& half1Ctrs,
    const vectorField& half1Areas
)
{
    if (transform() != neighbPatch().transform())
    {
        FatalErrorInFunction
            << "Patch " << name()
            << " has transform type " << transformTypeNames[transform()]
            << ", neighbour patch " << nbrPatchName_
            << " has transform type "
            << neighbPatch().transformTypeNames[neighbPatch().transform()]
            << exit(FatalError);
    }


    // Calculate transformation tensors

    switch (transform())
    {
        case ROTATIONAL:
        {
            tensor revT = tensor::zero;

            if (rotationAngleDefined_)
            {
                tensor T(rotationAxis_*rotationAxis_);

                tensor S
                (
                    0, -rotationAxis_.z(), rotationAxis_.y(),
                    rotationAxis_.z(), 0, -rotationAxis_.x(),
                    -rotationAxis_.y(), rotationAxis_.x(), 0
                );

                tensor revTPos
                (
                    T
                  + cos(rotationAngle_)*(tensor::I - T)
                  + sin(rotationAngle_)*S
                );

                tensor revTNeg
                (
                    T
                  + cos(-rotationAngle_)*(tensor::I - T)
                  + sin(-rotationAngle_)*S
                );

                // Check - assume correct angle when difference in face areas
                // is the smallest
                vector transformedAreaPos = gSum(half1Areas & revTPos);
                vector transformedAreaNeg = gSum(half1Areas & revTNeg);
                vector area0 = gSum(half0Areas);

                // Areas have opposite sign, so sum should be zero when
                // correct rotation applied
                scalar errorPos = mag(transformedAreaPos + area0);
                scalar errorNeg = mag(transformedAreaNeg + area0);

                if (errorPos < errorNeg)
                {
                    revT = revTPos;
                }
                else
                {
                    revT = revTNeg;
                    rotationAngle_ *= -1;
                }

                scalar areaError =
                    min(errorPos, errorNeg)/(mag(area0) + ROOTVSMALL);

                if (areaError > matchTolerance())
                {
                    WarningInFunction
                        << "Patch areas are not consistent within "
                        << 100*matchTolerance()
                        << " % indicating a possible error in the specified "
                        << "angle of rotation" << nl
                        << "    owner patch     : " << name() << nl
                        << "    neighbour patch : " << neighbPatch().name()
                        << nl
                        << "    angle           : "
                        << radToDeg(rotationAngle_) << " deg" << nl
                        << "    area error      : " << 100*areaError << " %"
                        << "    match tolerance : " <<  matchTolerance()
                        << endl;
                }

                if (debug)
                {
                    scalar theta = radToDeg(rotationAngle_);

                    Pout<< "cyclicAMIPolyPatch::calcTransforms: patch:"
                        << name()
                        << " Specified rotation:"
                        << " swept angle: " << theta << " [deg]"
                        << " reverse transform: " << revT
                        << endl;
                }
            }
            else
            {
                point n0 = vector::zero;
                point n1 = vector::zero;
                if (half0Ctrs.size())
                {
                    n0 = findFaceNormalMaxRadius(half0Ctrs);
                }
                if (half1Ctrs.size())
                {
                    n1 = -findFaceNormalMaxRadius(half1Ctrs);
                }

                reduce(n0, maxMagSqrOp<point>());
                reduce(n1, maxMagSqrOp<point>());

                n0 /= mag(n0) + VSMALL;
                n1 /= mag(n1) + VSMALL;

                // Extended tensor from two local coordinate systems calculated
                // using normal and rotation axis
                const tensor E0
                (
                    rotationAxis_,
                    (n0 ^ rotationAxis_),
                    n0
                );
                const tensor E1
                (
                    rotationAxis_,
                    (-n1 ^ rotationAxis_),
                    -n1
                );
                revT = E1.T() & E0;

                if (debug)
                {
                    scalar theta = radToDeg(acos(-(n0 & n1)));

                    Pout<< "cyclicAMIPolyPatch::calcTransforms: patch:"
                        << name()
                        << " Specified rotation:"
                        << " n0:" << n0 << " n1:" << n1
                        << " swept angle: " << theta << " [deg]"
                        << " reverse transform: " << revT
                        << endl;
                }
            }

            const_cast<tensorField&>(forwardT()) = tensorField(1, revT.T());
            const_cast<tensorField&>(reverseT()) = tensorField(1, revT);
            const_cast<vectorField&>(separation()).setSize(0);
            const_cast<boolList&>(collocated()) = boolList(1, false);

            break;
        }
        case TRANSLATIONAL:
        {
            if (debug)
            {
                Pout<< "cyclicAMIPolyPatch::calcTransforms : patch:" << name()
                    << " Specified translation : " << separationVector_
                    << endl;
            }

            const_cast<tensorField&>(forwardT()).clear();
            const_cast<tensorField&>(reverseT()).clear();
            const_cast<vectorField&>(separation()) = vectorField
            (
                1,
                separationVector_
            );
            const_cast<boolList&>(collocated()) = boolList(1, false);

            break;
        }
        default:
        {
            if (debug)
            {
                Pout<< "patch:" << name()
                    << " Assuming cyclic AMI pairs are colocated" << endl;
            }

            const_cast<tensorField&>(forwardT()).clear();
            const_cast<tensorField&>(reverseT()).clear();
            const_cast<vectorField&>(separation()).setSize(0);
            const_cast<boolList&>(collocated()) = boolList(1, true);

            break;
        }
    }

    if (debug)
    {
        Pout<< "patch: " << name() << nl
            << "    forwardT = " << forwardT() << nl
            << "    reverseT = " << reverseT() << nl
            << "    separation = " << separation() << nl
            << "    collocated = " << collocated() << nl << endl;
    }
}


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void CML::cyclicAMIPolyPatch::resetAMI
(
    const AMIPatchToPatchInterpolation::interpolationMethod& AMIMethod
) const
{
    if (owner())
    {
        AMIPtr_.clear();

        const polyPatch& nbr = neighbPatch();
        pointField nbrPoints
        (
            neighbPatch().boundaryMesh().mesh().points(),
            neighbPatch().meshPoints()
        );

        if (debug)
        {
            const Time& t = boundaryMesh().mesh().time();
            OFstream os(t.path()/name() + "_neighbourPatch-org.obj");
            meshTools::writeOBJ(os, neighbPatch().localFaces(), nbrPoints);
        }

        // Transform neighbour patch to local system
        transformPosition(nbrPoints);
        primitivePatch nbrPatch0
        (
            SubList<face>
            (
                nbr.localFaces(),
                nbr.size()
            ),
            nbrPoints
        );

        if (debug)
        {
            const Time& t = boundaryMesh().mesh().time();
            OFstream osN(t.path()/name() + "_neighbourPatch-trans.obj");
            meshTools::writeOBJ(osN, nbrPatch0.localFaces(), nbrPoints);

            OFstream osO(t.path()/name() + "_ownerPatch.obj");
            meshTools::writeOBJ(osO, this->localFaces(), localPoints());
        }

        // Construct/apply AMI interpolation to determine addressing and weights
        AMIPtr_.reset
        (
            new AMIPatchToPatchInterpolation
            (
                *this,
                nbrPatch0,
                surfPtr(),
                faceAreaIntersect::tmMesh,
                AMIRequireMatch_,
                AMIMethod,
                AMILowWeightCorrection_,
                AMIReverse_
            )
        );

        if (debug)
        {
            Pout<< "cyclicAMIPolyPatch : " << name()
                << " constructed AMI with " << nl
                << "    " << "srcAddress:" << AMIPtr_().srcAddress().size()
                << nl
                << "    " << "tgAddress :" << AMIPtr_().tgtAddress().size()
                << nl << endl;
        }
    }
}


void CML::cyclicAMIPolyPatch::initGeometry(PstreamBuffers& pBufs)
{
    polyPatch::initGeometry(pBufs);
}


void CML::cyclicAMIPolyPatch::calcGeometry(PstreamBuffers& pBufs)
{
    calcGeometry
    (
        *this,
        faceCentres(),
        faceAreas(),
        faceCellCentres(),
        neighbPatch().faceCentres(),
        neighbPatch().faceAreas(),
        neighbPatch().faceCellCentres()
    );
}


void CML::cyclicAMIPolyPatch::initMovePoints
(
    PstreamBuffers& pBufs,
    const pointField& p
)
{
    polyPatch::initMovePoints(pBufs, p);

    // See below. Clear out any local geometry
    primitivePatch::movePoints(p);
}


void CML::cyclicAMIPolyPatch::movePoints
(
    PstreamBuffers& pBufs,
    const pointField& p
)
{
    polyPatch::movePoints(pBufs, p);

    calcTransforms();

    // Note: resetAMI is called whilst in geometry update. So the slave
    // side might not have reached 'movePoints'. Is explicitly handled by
    // - clearing geometry of neighbour inside initMovePoints
    // - not using localPoints() inside resetAMI
    resetAMI();
}


void CML::cyclicAMIPolyPatch::initUpdateMesh(PstreamBuffers& pBufs)
{
    polyPatch::initUpdateMesh(pBufs);
    AMIPtr_.clear();
}


void CML::cyclicAMIPolyPatch::updateMesh(PstreamBuffers& pBufs)
{
    polyPatch::updateMesh(pBufs);
}


void CML::cyclicAMIPolyPatch::clearGeom()
{
    AMIPtr_.clear();
    polyPatch::clearGeom();
}


// * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * * //

CML::cyclicAMIPolyPatch::cyclicAMIPolyPatch
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
    coupledPolyPatch(name, size, start, index, bm, patchType, transform),
    nbrPatchName_(word::null),
    nbrPatchID_(-1),
    rotationAxis_(vector::zero),
    rotationCentre_(point::zero),
    rotationAngleDefined_(false),
    rotationAngle_(0.0),
    separationVector_(vector::zero),
    AMIPtr_(nullptr),
    AMIReverse_(false),
    AMIRequireMatch_(true),
    AMILowWeightCorrection_(-1.0),
    surfPtr_(nullptr),
    surfDict_(fileName("surface"))
{
    // Neighbour patch might not be valid yet so no transformation
    // calculation possible
}


CML::cyclicAMIPolyPatch::cyclicAMIPolyPatch
(
    const word& name,
    const dictionary& dict,
    const label index,
    const polyBoundaryMesh& bm,
    const word& patchType
)
:
    coupledPolyPatch(name, dict, index, bm, patchType),
    nbrPatchName_(dict.lookupOrDefault<word>("neighbourPatch", "")),
    nbrPatchID_(-1),
    rotationAxis_(vector::zero),
    rotationCentre_(point::zero),
    rotationAngleDefined_(false),
    rotationAngle_(0.0),
    separationVector_(vector::zero),
    AMIPtr_(nullptr),
    AMIReverse_(dict.lookupOrDefault<bool>("flipNormals", false)),
    AMIRequireMatch_(true),
    AMILowWeightCorrection_(dict.lookupOrDefault("lowWeightCorrection", -1.0)),
    surfPtr_(nullptr),
    surfDict_(dict.subOrEmptyDict("surface"))
{
    if (nbrPatchName_ == name)
    {
        FatalIOErrorInFunction(dict)
            << "Neighbour patch name " << nbrPatchName_
            << " cannot be the same as this patch " << name
            << exit(FatalIOError);
    }

    switch (transform())
    {
        case ROTATIONAL:
        {
            dict.lookup("rotationAxis") >> rotationAxis_;
            dict.lookup("rotationCentre") >> rotationCentre_;
            if (dict.readIfPresent("rotationAngle", rotationAngle_))
            {
                rotationAngleDefined_ = true;
                rotationAngle_ = degToRad(rotationAngle_);

                if (debug)
                {
                    Info<< "rotationAngle: " << rotationAngle_ << " [rad]"
                        <<  endl;
                }
            }

            scalar magRot = mag(rotationAxis_);
            if (magRot < SMALL)
            {
                FatalIOErrorInFunction(dict)
                    << "Illegal rotationAxis " << rotationAxis_ << endl
                    << "Please supply a non-zero vector."
                    << exit(FatalIOError);
            }
            rotationAxis_ /= magRot;

            break;
        }
        case TRANSLATIONAL:
        {
            dict.lookup("separationVector") >> separationVector_;
            break;
        }
        default:
        {
            // No additional info required
        }
    }

    // Neighbour patch might not be valid yet so no transformation
    // calculation possible
}


CML::cyclicAMIPolyPatch::cyclicAMIPolyPatch
(
    const cyclicAMIPolyPatch& pp,
    const polyBoundaryMesh& bm
)
:
    coupledPolyPatch(pp, bm),
    nbrPatchName_(pp.nbrPatchName_),
    nbrPatchID_(-1),
    rotationAxis_(pp.rotationAxis_),
    rotationCentre_(pp.rotationCentre_),
    rotationAngleDefined_(pp.rotationAngleDefined_),
    rotationAngle_(pp.rotationAngle_),
    separationVector_(pp.separationVector_),
    AMIPtr_(nullptr),
    AMIReverse_(pp.AMIReverse_),
    AMIRequireMatch_(pp.AMIRequireMatch_),
    AMILowWeightCorrection_(pp.AMILowWeightCorrection_),
    surfPtr_(nullptr),
    surfDict_(pp.surfDict_)
{
    // Neighbour patch might not be valid yet so no transformation
    // calculation possible
}


CML::cyclicAMIPolyPatch::cyclicAMIPolyPatch
(
    const cyclicAMIPolyPatch& pp,
    const polyBoundaryMesh& bm,
    const label index,
    const label newSize,
    const label newStart,
    const word& nbrPatchName
)
:
    coupledPolyPatch(pp, bm, index, newSize, newStart),
    nbrPatchName_(nbrPatchName),
    nbrPatchID_(-1),
    rotationAxis_(pp.rotationAxis_),
    rotationCentre_(pp.rotationCentre_),
    rotationAngleDefined_(pp.rotationAngleDefined_),
    rotationAngle_(pp.rotationAngle_),
    separationVector_(pp.separationVector_),
    AMIPtr_(nullptr),
    AMIReverse_(pp.AMIReverse_),
    AMIRequireMatch_(pp.AMIRequireMatch_),
    AMILowWeightCorrection_(pp.AMILowWeightCorrection_),
    surfPtr_(nullptr),
    surfDict_(pp.surfDict_)
{
    if (nbrPatchName_ == name())
    {
        FatalErrorInFunction
            << "Neighbour patch name " << nbrPatchName_
            << " cannot be the same as this patch " << name()
            << exit(FatalError);
    }

    // Neighbour patch might not be valid yet so no transformation
    // calculation possible
}


CML::cyclicAMIPolyPatch::cyclicAMIPolyPatch
(
    const cyclicAMIPolyPatch& pp,
    const polyBoundaryMesh& bm,
    const label index,
    const labelUList& mapAddressing,
    const label newStart
)
:
    coupledPolyPatch(pp, bm, index, mapAddressing, newStart),
    nbrPatchName_(pp.nbrPatchName_),
    nbrPatchID_(-1),
    rotationAxis_(pp.rotationAxis_),
    rotationCentre_(pp.rotationCentre_),
    rotationAngleDefined_(pp.rotationAngleDefined_),
    rotationAngle_(pp.rotationAngle_),
    separationVector_(pp.separationVector_),
    AMIPtr_(nullptr),
    AMIReverse_(pp.AMIReverse_),
    AMIRequireMatch_(pp.AMIRequireMatch_),
    AMILowWeightCorrection_(pp.AMILowWeightCorrection_),
    surfPtr_(nullptr),
    surfDict_(pp.surfDict_)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::cyclicAMIPolyPatch::~cyclicAMIPolyPatch()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::label CML::cyclicAMIPolyPatch::neighbPatchID() const
{
    if (nbrPatchID_ == -1)
    {
        nbrPatchID_ = this->boundaryMesh().findPatchID(nbrPatchName_);

        if (nbrPatchID_ == -1)
        {
            FatalErrorInFunction
                << "Illegal neighbourPatch name " << nbrPatchName_
                << nl << "Valid patch names are "
                << this->boundaryMesh().names()
                << exit(FatalError);
        }

        // Check that it is a cyclic AMI patch
        const cyclicAMIPolyPatch& nbrPatch =
            refCast<const cyclicAMIPolyPatch>
            (
                this->boundaryMesh()[nbrPatchID_]
            );

        if (nbrPatch.neighbPatchName() != name())
        {
            WarningInFunction
                << "Patch " << name()
                << " specifies neighbour patch " << neighbPatchName()
                << nl << " but that in return specifies "
                << nbrPatch.neighbPatchName() << endl;
        }
    }

    return nbrPatchID_;
}


bool CML::cyclicAMIPolyPatch::owner() const
{
    return index() < neighbPatchID();
}


const CML::autoPtr<CML::searchableSurface>&
CML::cyclicAMIPolyPatch::surfPtr() const
{
    const word surfType(surfDict_.lookupOrDefault<word>("type", "none"));

    if (!surfPtr_.valid() && owner() && surfType != "none")
    {
        word surfName(surfDict_.lookupOrDefault("name", name()));

        const polyMesh& mesh = boundaryMesh().mesh();

        surfPtr_ =
            searchableSurface::New
            (
                surfType,
                IOobject
                (
                    surfName,
                    mesh.time().constant(),
                    "triSurface",
                    mesh,
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                ),
                surfDict_
            );
    }

    return surfPtr_;
}


const CML::AMIPatchToPatchInterpolation& CML::cyclicAMIPolyPatch::AMI() const
{
    if (!owner())
    {
        FatalErrorInFunction
            << "AMI interpolator only available to owner patch"
            << abort(FatalError);
    }

    if (!AMIPtr_.valid())
    {
        resetAMI();
    }

    return AMIPtr_();
}


bool CML::cyclicAMIPolyPatch::applyLowWeightCorrection() const
{
    if (owner())
    {
        return AMI().applyLowWeightCorrection();
    }
    else
    {
        return neighbPatch().AMI().applyLowWeightCorrection();
    }
}


void CML::cyclicAMIPolyPatch::transformPosition(pointField& l) const
{
    if (!parallel())
    {
        if (transform() == ROTATIONAL)
        {
            l = CML::transform(forwardT(), l - rotationCentre_)
              + rotationCentre_;
        }
        else
        {
            l = CML::transform(forwardT(), l);
        }
    }
    else if (separated())
    {
        // transformPosition gets called on the receiving side,
        // separation gets calculated on the sending side so subtract

        const vectorField& s = separation();
        if (s.size() == 1)
        {
            forAll(l, i)
            {
                l[i] -= s[0];
            }
        }
        else
        {
            l -= s;
        }
    }
}


void CML::cyclicAMIPolyPatch::transformPosition
(
    point& l,
    const label faceI
) const
{
    if (!parallel())
    {
        const tensor& T =
        (
            forwardT().size() == 1
          ? forwardT()[0]
          : forwardT()[faceI]
        );

        if (transform() == ROTATIONAL)
        {
            l = CML::transform(T, l - rotationCentre_) + rotationCentre_;
        }
        else
        {
            l = CML::transform(T, l);
        }
    }
    else if (separated())
    {
        const vector& s =
        (
            separation().size() == 1
          ? separation()[0]
          : separation()[faceI]
        );

        l -= s;
    }
}


void CML::cyclicAMIPolyPatch::reverseTransformPosition
(
    point& l,
    const label faceI
) const
{
    if (!parallel())
    {
        const tensor& T =
        (
            reverseT().size() == 1
          ? reverseT()[0]
          : reverseT()[faceI]
        );

        if (transform() == ROTATIONAL)
        {
            l = CML::transform(T, l - rotationCentre_) + rotationCentre_;
        }
        else
        {
            l = CML::transform(T, l);
        }
    }
    else if (separated())
    {
        const vector& s =
        (
            separation().size() == 1
          ? separation()[0]
          : separation()[faceI]
        );

        l += s;
    }
}


void CML::cyclicAMIPolyPatch::reverseTransformDirection
(
    vector& d,
    const label faceI
) const
{
    if (!parallel())
    {
        const tensor& T =
        (
            reverseT().size() == 1
          ? reverseT()[0]
          : reverseT()[faceI]
        );

        d = CML::transform(T, d);
    }
}


void CML::cyclicAMIPolyPatch::calcGeometry
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
    calcTransforms
    (
        referPatch,
        thisCtrs,
        thisAreas,
        nbrCtrs,
        nbrAreas
    );
}


void CML::cyclicAMIPolyPatch::initOrder
(
    PstreamBuffers& pBufs,
    const primitivePatch& pp
) const
{}


bool CML::cyclicAMIPolyPatch::order
(
    PstreamBuffers& pBufs,
    const primitivePatch& pp,
    labelList& faceMap,
    labelList& rotation
) const
{
    faceMap.setSize(pp.size());
    faceMap = -1;

    rotation.setSize(pp.size());
    rotation = 0;

    // do nothing
    return false;
}


CML::label CML::cyclicAMIPolyPatch::pointFace
(
    const label faceI,
    const vector& n,
    point& p
) const
{
    point prt(p);
    reverseTransformPosition(prt, faceI);

    vector nrt(n);
    reverseTransformDirection(nrt, faceI);

    label nbrFaceI = -1;

    if (owner())
    {
        nbrFaceI = AMI().tgtPointFace
        (
            *this,
            neighbPatch(),
            nrt,
            faceI,
            prt
        );
    }
    else
    {
        nbrFaceI = neighbPatch().AMI().srcPointFace
        (
            neighbPatch(),
            *this,
            nrt,
            faceI,
            prt
        );
    }

    if (nbrFaceI >= 0)
    {
        p = prt;
    }

    return nbrFaceI;
}


void CML::cyclicAMIPolyPatch::write(Ostream& os) const
{
    coupledPolyPatch::write(os);
    if (!nbrPatchName_.empty())
    {
        os.writeKeyword("neighbourPatch") << nbrPatchName_
            << token::END_STATEMENT << nl;
    }

    switch (transform())
    {
        case ROTATIONAL:
        {
            os.writeKeyword("rotationAxis") << rotationAxis_
                << token::END_STATEMENT << nl;
            os.writeKeyword("rotationCentre") << rotationCentre_
                << token::END_STATEMENT << nl;

            if (rotationAngleDefined_)
            {
                os.writeKeyword("rotationAngle") << radToDeg(rotationAngle_)
                    << token::END_STATEMENT << nl;
            }

            break;
        }
        case TRANSLATIONAL:
        {
            os.writeKeyword("separationVector") << separationVector_
                << token::END_STATEMENT << nl;
            break;
        }
        case NOORDERING:
        {
            break;
        }
        default:
        {
            // No additional info to write
        }
    }

    if (AMIReverse_)
    {
        os.writeKeyword("flipNormals") << AMIReverse_
            << token::END_STATEMENT << nl;
    }

    if (AMILowWeightCorrection_ > 0)
    {
        os.writeKeyword("lowWeightCorrection") << AMILowWeightCorrection_
            << token::END_STATEMENT << nl;
    }

    if (!surfDict_.empty())
    {
        os.writeKeyword(surfDict_.dictName());
        os  << surfDict_;
    }
}


// ************************************************************************* //
