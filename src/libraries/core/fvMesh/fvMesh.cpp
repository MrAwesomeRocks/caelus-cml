/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2015 OpenFOAM Foundation
Copyright (C) 2016 Applied CCM
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

#include "fvMesh.hpp"
#include "volFields.hpp"
#include "surfaceFields.hpp"
#include "slicedVolFields.hpp"
#include "slicedSurfaceFields.hpp"
#include "SubField.hpp"
#include "demandDrivenData.hpp"
#include "fvMeshLduAddressing.hpp"
#include "emptyPolyPatch.hpp"
#include "mapPolyMesh.hpp"
#include "MapFvFields.hpp"
#include "fvMeshMapper.hpp"
#include "mapClouds.hpp"
#include "wallFvPatch.hpp"

#include "volPointInterpolation.hpp"
#include "extendedLeastSquaresVectors.hpp"
#include "extendedLeastSquaresVectors.hpp"
#include "leastSquaresVectors.hpp"
#include "skewCorrectionVectors.hpp"


#include "centredCECCellToFaceStencilObject.hpp"
#include "centredCFCCellToFaceStencilObject.hpp"
#include "centredCPCCellToFaceStencilObject.hpp"
#include "centredFECCellToFaceStencilObject.hpp"
#include "upwindCECCellToFaceStencilObject.hpp"
#include "upwindCFCCellToFaceStencilObject.hpp"
#include "upwindCPCCellToFaceStencilObject.hpp"
#include "upwindFECCellToFaceStencilObject.hpp"

#include "centredCFCFaceToCellStencilObject.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


namespace CML
{
    defineTypeNameAndDebug(fvMesh, 0);
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::fvMesh::clearGeomNotOldVol()
{
    slicedVolScalarField::DimensionedInternalField* VPtr =
        static_cast<slicedVolScalarField::DimensionedInternalField*>(VPtr_);
    deleteDemandDrivenData(VPtr);
    VPtr_ = nullptr;

    deleteDemandDrivenData(SfPtr_);
    deleteDemandDrivenData(magSfPtr_);
    deleteDemandDrivenData(CPtr_);
    deleteDemandDrivenData(CgPtr_);
    deleteDemandDrivenData(defectCorrVecsPtr_);
    deleteDemandDrivenData(CfPtr_);
}


void CML::fvMesh::updateGeomNotOldVol()
{
    bool haveV = (VPtr_ != nullptr);
    bool haveSf = (SfPtr_ != nullptr);
    bool haveMagSf = (magSfPtr_ != nullptr);
    bool haveCP = (CPtr_ != nullptr);
    bool haveCPg = (CgPtr_ != nullptr);
    bool haveDefectCorrVec = (defectCorrVecsPtr_ != nullptr);
    bool haveCf = (CfPtr_ != nullptr);

    clearGeomNotOldVol();

    // Now recreate the fields
    if (haveV)
    {
        (void)V();
    }
    if (haveSf)
    {
        (void)Sf();
    }
    if (haveMagSf)
    {
        (void)magSf();
    }
    if (haveCP)
    {
        (void)C();
    }
    if (haveCPg)
    {
        (void)Cg();
    }
    if (haveDefectCorrVec)
    {
        (void)defectCorrVecs();
    }
    if (haveCf)
    {
        (void)Cf();
    }
}


void CML::fvMesh::clearGeom()
{
    clearGeomNotOldVol();

    deleteDemandDrivenData(V0Ptr_);
    deleteDemandDrivenData(V00Ptr_);

    // Mesh motion flux cannot be deleted here because the old-time flux
    // needs to be saved.

    // Things geometry dependent that are not updated.
    volPointInterpolation::Delete(*this);
    extendedLeastSquaresVectors::Delete(*this);
    leastSquaresVectors::Delete(*this);
    skewCorrectionVectors::Delete(*this);
}


void CML::fvMesh::clearAddressing()
{
    deleteDemandDrivenData(lduPtr_);

    // Hack until proper callbacks. Below are all the fvMesh-MeshObjects.

    volPointInterpolation::Delete(*this);
    extendedLeastSquaresVectors::Delete(*this);
    leastSquaresVectors::Delete(*this);
    skewCorrectionVectors::Delete(*this);


    centredCECCellToFaceStencilObject::Delete(*this);
    centredCFCCellToFaceStencilObject::Delete(*this);
    centredCPCCellToFaceStencilObject::Delete(*this);
    centredFECCellToFaceStencilObject::Delete(*this);
    // Is this geometry related - cells distorting to upwind direction?
    upwindCECCellToFaceStencilObject::Delete(*this);
    upwindCFCCellToFaceStencilObject::Delete(*this);
    upwindCPCCellToFaceStencilObject::Delete(*this);
    upwindFECCellToFaceStencilObject::Delete(*this);

    centredCFCFaceToCellStencilObject::Delete(*this);
}


void CML::fvMesh::clearOut()
{
    clearGeom();
    surfaceInterpolation::clearOut();

    clearAddressing();

    // Clear mesh motion flux
    deleteDemandDrivenData(phiPtr_);

    polyMesh::clearOut();
}


const CML::scalarField CML::fvMesh::patchWeights(const fvPatch& patch) const
{
   if (!isA<wallFvPatch>(patch))
      FatalErrorInFunction
         << "patch: " << patch.name() << " is not a wall patch"
         << abort(FatalError);

   scalarField weights(patch.size(), scalar(1.0));

   return weights;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::fvMesh::fvMesh(const IOobject& io, const bool defectCorr, const scalar areaSwitch)
:
    polyMesh(io, defectCorr, areaSwitch),
    surfaceInterpolation(*this),
    fvSchemes(static_cast<const objectRegistry&>(*this)),
    fvSolution(static_cast<const objectRegistry&>(*this)),
    data(static_cast<const objectRegistry&>(*this)),
    boundary_(*this, boundaryMesh()),
    lduPtr_(nullptr),
    curTimeIndex_(time().timeIndex()),
    VPtr_(nullptr),
    V0Ptr_(nullptr),
    V00Ptr_(nullptr),
    SfPtr_(nullptr),
    magSfPtr_(nullptr),
    CPtr_(nullptr),
    CgPtr_(nullptr),
    defectCorrVecsPtr_(nullptr),
    CfPtr_(nullptr),
    phiPtr_(nullptr)
{
    if (debug)
    {
        Info<< "Constructing fvMesh from IOobject"
            << endl;
    }

    // Check the existence of the cell volumes and read if present
    // and set the storage of V00
    if (isFile(time().timePath()/"V0"))
    {
        V0Ptr_ = new DimensionedField<scalar, volMesh>
        (
            IOobject
            (
                "V0",
                time().timeName(),
                *this,
                IOobject::MUST_READ,
                IOobject::NO_WRITE,
                false
            ),
            *this
        );

        V00();
    }

    // Check the existence of the mesh fluxes, read if present and set the
    // mesh to be moving
    if (isFile(time().timePath()/"meshPhi"))
    {
        phiPtr_ = new surfaceScalarField
        (
            IOobject
            (
                "meshPhi",
                time().timeName(),
                *this,
                IOobject::MUST_READ,
                IOobject::NO_WRITE,
                false
            ),
            *this
        );

        // The mesh is now considered moving so the old-time cell volumes
        // will be required for the time derivatives so if they haven't been
        // read initialise to the current cell volumes
        if (!V0Ptr_)
        {
            V0Ptr_ = new DimensionedField<scalar, volMesh>
            (
                IOobject
                (
                    "V0",
                    time().timeName(),
                    *this,
                    IOobject::NO_READ,
                    IOobject::NO_WRITE,
                    false
                ),
                V()
            );
        }

        moving(true);
    }
}


CML::fvMesh::fvMesh
(
    const IOobject& io,
    const Xfer<pointField>& points,
    const cellShapeList& shapes,
    const faceListList& boundaryFaces,
    const wordList& boundaryPatchNames,
    const PtrList<dictionary>& boundaryDicts,
    const word& defaultBoundaryPatchName,
    const word& defaultBoundaryPatchType,
    const bool syncPar,
    const bool defectCorr,
    const scalar areaSwitch
)
:
    polyMesh
    (
        io,
        points,
        shapes,
        boundaryFaces,
        boundaryPatchNames,
        boundaryDicts,
        defaultBoundaryPatchName,
        defaultBoundaryPatchType,
        syncPar,
        defectCorr,
        areaSwitch
    ),
    surfaceInterpolation(*this),
    fvSchemes(static_cast<const objectRegistry&>(*this)),
    fvSolution(static_cast<const objectRegistry&>(*this)),
    data(static_cast<const objectRegistry&>(*this)),
    boundary_(*this, boundaryMesh()),
    lduPtr_(nullptr),
    curTimeIndex_(time().timeIndex()),
    VPtr_(nullptr),
    V0Ptr_(nullptr),
    V00Ptr_(nullptr),
    SfPtr_(nullptr),
    magSfPtr_(nullptr),
    CPtr_(nullptr),
    CgPtr_(nullptr),
    defectCorrVecsPtr_(nullptr),
    CfPtr_(nullptr),
    phiPtr_(nullptr)
{
    if (debug)
    {
        Info<< "Constructing fvMesh from cellShapes" << endl;
    }
}


CML::fvMesh::fvMesh
(
    const IOobject& io,
    const Xfer<pointField>& points,
    const Xfer<faceList>& faces,
    const Xfer<labelList>& allOwner,
    const Xfer<labelList>& allNeighbour,
    const bool syncPar,
    const bool defectCorr,
    const scalar areaSwitch
)
:
    polyMesh(io, points, faces, allOwner, allNeighbour, syncPar, defectCorr, areaSwitch),
    surfaceInterpolation(*this),
    fvSchemes(static_cast<const objectRegistry&>(*this)),
    fvSolution(static_cast<const objectRegistry&>(*this)),
    data(static_cast<const objectRegistry&>(*this)),
    boundary_(*this, boundaryMesh()),
    lduPtr_(nullptr),
    curTimeIndex_(time().timeIndex()),
    VPtr_(nullptr),
    V0Ptr_(nullptr),
    V00Ptr_(nullptr),
    SfPtr_(nullptr),
    magSfPtr_(nullptr),
    CPtr_(nullptr),
    CgPtr_(nullptr),
    defectCorrVecsPtr_(nullptr),
    CfPtr_(nullptr),
    phiPtr_(nullptr)
{
    if (debug)
    {
        Info<< "Constructing fvMesh from components" << endl;
    }
}


CML::fvMesh::fvMesh
(
    const IOobject& io,
    const Xfer<pointField>& points,
    const Xfer<faceList>& faces,
    const Xfer<cellList>& cells,
    const bool syncPar,
    const bool defectCorr,
    const scalar areaSwitch
)
:
    polyMesh(io, points, faces, cells, syncPar, defectCorr, areaSwitch),
    surfaceInterpolation(*this),
    fvSchemes(static_cast<const objectRegistry&>(*this)),
    fvSolution(static_cast<const objectRegistry&>(*this)),
    data(static_cast<const objectRegistry&>(*this)),
    boundary_(*this),
    lduPtr_(nullptr),
    curTimeIndex_(time().timeIndex()),
    VPtr_(nullptr),
    V0Ptr_(nullptr),
    V00Ptr_(nullptr),
    SfPtr_(nullptr),
    magSfPtr_(nullptr),
    CPtr_(nullptr),
    CgPtr_(nullptr),
    defectCorrVecsPtr_(nullptr),
    CfPtr_(nullptr),
    phiPtr_(nullptr)
{
    if (debug)
    {
        Info<< "Constructing fvMesh from components" << endl;
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::fvMesh::~fvMesh()
{
    clearOut();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::fvMesh::addFvPatches
(
    const List<polyPatch*> & p,
    const bool validBoundary
)
{
    if (boundary().size())
    {
        FatalErrorInFunction
            << " boundary already exists"
            << abort(FatalError);
    }

    // first add polyPatches
    addPatches(p, validBoundary);
    boundary_.addPatches(boundaryMesh());
}


void CML::fvMesh::removeFvBoundary()
{
    if (debug)
    {
        Info<< "void fvMesh::removeFvBoundary(): "
            << "Removing boundary patches."
            << endl;
    }

    // Remove fvBoundaryMesh data first.
    boundary_.clear();
    boundary_.setSize(0);
    polyMesh::removeBoundary();

    clearOut();
}


CML::polyMesh::readUpdateState CML::fvMesh::readUpdate()
{
    if (debug)
    {
        Info<< "polyMesh::readUpdateState fvMesh::readUpdate() : "
            << "Updating fvMesh.  ";
    }

    polyMesh::readUpdateState state = polyMesh::readUpdate();

    if (state == polyMesh::TOPO_PATCH_CHANGE)
    {
        if (debug)
        {
            Info<< "Boundary and topological update" << endl;
        }

        boundary_.readUpdate(boundaryMesh());

        clearOut();

    }
    else if (state == polyMesh::TOPO_CHANGE)
    {
        if (debug)
        {
            Info<< "Topological update" << endl;
        }

        clearOut();
    }
    else if (state == polyMesh::POINTS_MOVED)
    {
        if (debug)
        {
            Info<< "Point motion update" << endl;
        }

        clearGeom();
    }
    else
    {
        if (debug)
        {
            Info<< "No update" << endl;
        }
    }

    return state;
}


const CML::fvBoundaryMesh& CML::fvMesh::boundary() const
{
    return boundary_;
}


const CML::lduAddressing& CML::fvMesh::lduAddr() const
{
    if (!lduPtr_)
    {
        lduPtr_ = new fvMeshLduAddressing(*this);
    }

    return *lduPtr_;
}


void CML::fvMesh::mapFields(const mapPolyMesh& meshMap)
{
    // Create a mapper
    const fvMeshMapper mapper(*this, meshMap);

    // Map all the volFields in the objectRegistry
    MapGeometricFields<scalar, fvPatchField, fvMeshMapper, volMesh>
    (mapper);
    MapGeometricFields<vector, fvPatchField, fvMeshMapper, volMesh>
    (mapper);
    MapGeometricFields<sphericalTensor, fvPatchField, fvMeshMapper, volMesh>
    (mapper);
    MapGeometricFields<symmTensor, fvPatchField, fvMeshMapper, volMesh>
    (mapper);
    MapGeometricFields<tensor, fvPatchField, fvMeshMapper, volMesh>
    (mapper);

    // Map all the surfaceFields in the objectRegistry
    MapGeometricFields<scalar, fvsPatchField, fvMeshMapper, surfaceMesh>
    (mapper);
    MapGeometricFields<vector, fvsPatchField, fvMeshMapper, surfaceMesh>
    (mapper);
    MapGeometricFields<symmTensor, fvsPatchField, fvMeshMapper, surfaceMesh>
    (mapper);
    MapGeometricFields<symmTensor, fvsPatchField, fvMeshMapper, surfaceMesh>
    (mapper);
    MapGeometricFields<tensor, fvsPatchField, fvMeshMapper, surfaceMesh>
    (mapper);

    // Map all the dimensionedFields in the objectRegistry
    MapDimensionedFields<scalar, fvMeshMapper, volMesh>(mapper);
    MapDimensionedFields<vector, fvMeshMapper, volMesh>(mapper);
    MapDimensionedFields<sphericalTensor, fvMeshMapper, volMesh>(mapper);
    MapDimensionedFields<symmTensor, fvMeshMapper, volMesh>(mapper);
    MapDimensionedFields<tensor, fvMeshMapper, volMesh>(mapper);

    // Map all the clouds in the objectRegistry
    mapClouds(*this, meshMap);


    const labelList& cellMap = meshMap.cellMap();

    // Map the old volume. Just map to new cell labels.
    if (V0Ptr_)
    {
        scalarField& V0 = *V0Ptr_;

        scalarField savedV0(V0);
        V0.setSize(nCells());

        forAll(V0, i)
        {
            if (cellMap[i] > -1)
            {
                V0[i] = savedV0[cellMap[i]];
            }
            else
            {
                V0[i] = 0.0;
            }
        }
    }

    // Map the old-old volume. Just map to new cell labels.
    if (V00Ptr_)
    {
        scalarField& V00 = *V00Ptr_;

        scalarField savedV00(V00);
        V00.setSize(nCells());

        forAll(V00, i)
        {
            if (cellMap[i] > -1)
            {
                V00[i] = savedV00[cellMap[i]];
            }
            else
            {
                V00[i] = 0.0;
            }
        }
    }
}


// Temporary helper function to call move points on
// MeshObjects
template<class Type>
void MeshObjectMovePoints(const CML::fvMesh& mesh)
{
    if (mesh.thisDb().foundObject<Type>(Type::typeName))
    {
        const_cast<Type&>
        (
            mesh.thisDb().lookupObject<Type>
            (
                Type::typeName
            )
        ).movePoints();
    }
}


CML::tmp<CML::scalarField> CML::fvMesh::movePoints(const pointField& p)
{
    // Grab old time volumes if the time has been incremented
    if (curTimeIndex_ < time().timeIndex())
    {
        if (V00Ptr_ && V0Ptr_)
        {
            *V00Ptr_ = *V0Ptr_;
        }

        if (V0Ptr_)
        {
            *V0Ptr_ = V();
        }
        else
        {
            V0Ptr_ = new DimensionedField<scalar, volMesh>
            (
                IOobject
                (
                    "V0",
                    time().timeName(),
                    *this,
                    IOobject::NO_READ,
                    IOobject::NO_WRITE
                ),
                V()
            );
        }

        curTimeIndex_ = time().timeIndex();
    }


    if (!phiPtr_)
    {
        // Create mesh motion flux
        phiPtr_ = new surfaceScalarField
        (
            IOobject
            (
                "meshPhi",
                this->time().timeName(),
                *this,
                IOobject::NO_READ,
                IOobject::NO_WRITE,
                false
            ),
            *this,
            dimVolume/dimTime
        );
    }
    else
    {
        // Grab old time mesh motion fluxes if the time has been incremented
        if (phiPtr_->timeIndex() != time().timeIndex())
        {
            phiPtr_->oldTime();
        }
    }

    surfaceScalarField& phi = *phiPtr_;

    // Move the polyMesh and set the mesh motion fluxes to the swept-volumes

    scalar rDeltaT = 1.0/time().deltaTValue();

    tmp<scalarField> tsweptVols = polyMesh::movePoints(p);
    scalarField& sweptVols = tsweptVols();

    phi.internalField() = scalarField::subField(sweptVols, nInternalFaces());
    phi.internalField() *= rDeltaT;

    const fvPatchList& patches = boundary();

    forAll(patches, patchI)
    {
        phi.boundaryField()[patchI] = patches[patchI].patchSlice(sweptVols);
        phi.boundaryField()[patchI] *= rDeltaT;
    }

    // Update or delete the local geometric properties as early as possible so
    // they can be used if necessary. These get recreated here instead of
    // demand driven since they might do parallel transfers which can conflict
    // with when they're actually being used.
    // Note that between above "polyMesh::movePoints(p)" and here nothing
    // should use the local geometric properties.
    updateGeomNotOldVol();


    // Update other local data
    boundary_.movePoints();
    surfaceInterpolation::movePoints();


    // Hack until proper callbacks. Below are all the fvMesh MeshObjects with a
    // movePoints function.
    MeshObjectMovePoints<volPointInterpolation>(*this);
    MeshObjectMovePoints<extendedLeastSquaresVectors>(*this);
    MeshObjectMovePoints<leastSquaresVectors>(*this);
    MeshObjectMovePoints<skewCorrectionVectors>(*this);

    return tsweptVols;
}


void CML::fvMesh::updateMesh(const mapPolyMesh& mpm)
{
    // Update polyMesh. This needs to keep volume existent!
    polyMesh::updateMesh(mpm);

    // Clear the sliced fields
    clearGeomNotOldVol();

    // Map all fields using current (i.e. not yet mapped) volume
    mapFields(mpm);

    // Clear the current volume and other geometry factors
    surfaceInterpolation::clearOut();

    clearAddressing();

    // handleMorph() should also clear out the surfaceInterpolation.
    // This is a temporary solution
    surfaceInterpolation::movePoints();
}


bool CML::fvMesh::writeObjects
(
    IOstream::streamFormat fmt,
    IOstream::versionNumber ver,
    IOstream::compressionType cmp
) const
{
    return polyMesh::writeObject(fmt, ver, cmp);
}


//- Write mesh using IO settings from the time
bool CML::fvMesh::write() const
{
    bool ok = true;
    if (phiPtr_)
    {
        ok = phiPtr_->write();
    }

    return ok && polyMesh::write();
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

bool CML::fvMesh::operator!=(const fvMesh& bm) const
{
    return &bm != this;
}


bool CML::fvMesh::operator==(const fvMesh& bm) const
{
    return &bm == this;
}


// ************************************************************************* //
