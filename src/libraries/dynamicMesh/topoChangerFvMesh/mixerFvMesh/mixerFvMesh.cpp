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

#include "mixerFvMesh.hpp"
#include "Time.hpp"
#include "regionSplit.hpp"
#include "slidingInterface.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "mapPolyMesh.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(mixerFvMesh, 0);

    addToRunTimeSelectionTable(topoChangerFvMesh, mixerFvMesh, IOobject);
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::mixerFvMesh::addZonesAndModifiers()
{
    // Add zones and modifiers for motion action

    if
    (
        pointZones().size()
     || faceZones().size()
     || cellZones().size()
     || topoChanger_.size()
    )
    {
        Info<< "void mixerFvMesh::addZonesAndModifiers() : "
            << "Zones and modifiers already present.  Skipping."
            << endl;

        return;
    }

    Info<< "Time = " << time().timeName() << endl
        << "Adding zones and modifiers to the mesh" << endl;

    // Add zones
    List<pointZone*> pz(1);

    // Add an empty zone for cut points

    pz[0] = new pointZone
    (
        "cutPointZone",
        labelList(0),
        0,
        pointZones()
    );


    // Do face zones for slider

    List<faceZone*> fz(3);

    // Inner slider
    const word innerSliderName(motionDict_.subDict("slider").lookup("inside"));
    const polyPatch& innerSlider = boundaryMesh()[innerSliderName];

    labelList isf(innerSlider.size());

    forAll(isf, i)
    {
        isf[i] = innerSlider.start() + i;
    }

    fz[0] = new faceZone
    (
        "insideSliderZone",
        isf,
        boolList(innerSlider.size(), false),
        0,
        faceZones()
    );

    // Outer slider
    const word outerSliderName(motionDict_.subDict("slider").lookup("outside"));
    const polyPatch& outerSlider = boundaryMesh()[outerSliderName];

    labelList osf(outerSlider.size());

    forAll(osf, i)
    {
        osf[i] = outerSlider.start() + i;
    }

    fz[1] = new faceZone
    (
        "outsideSliderZone",
        osf,
        boolList(outerSlider.size(), false),
        1,
        faceZones()
    );

    // Add empty zone for cut faces
    fz[2] = new faceZone
    (
        "cutFaceZone",
        labelList(0),
        boolList(0, false),
        2,
        faceZones()
    );

    List<cellZone*> cz(1);

    // Mark every cell with its topological region
    regionSplit rs(*this);

    // Get the region of the cell containing the origin.
    label originRegion = rs[findNearestCell(cs().origin())];

    labelList movingCells(nCells());
    label nMovingCells = 0;

    forAll(rs, cellI)
    {
        if (rs[cellI] == originRegion)
        {
            movingCells[nMovingCells] = cellI;
            nMovingCells++;
        }
    }

    movingCells.setSize(nMovingCells);
    Info<< "Number of cells in the moving region: " << nMovingCells << endl;

    cz[0] = new cellZone
    (
        "movingCells",
        movingCells,
        0,
        cellZones()
    );

    Info<< "Adding point, face and cell zones" << endl;
    addZones(pz, fz, cz);

    // Add a topology modifier
    Info<< "Adding topology modifiers" << endl;
    topoChanger_.setSize(1);
    topoChanger_.set
    (
        0,
        new slidingInterface
        (
            "mixerSlider",
            0,
            topoChanger_,
            outerSliderName + "Zone",
            innerSliderName + "Zone",
            "cutPointZone",
            "cutFaceZone",
            outerSliderName,
            innerSliderName,
            slidingInterface::INTEGRAL
        )
    );
    topoChanger_.writeOpt() = IOobject::AUTO_WRITE;

    write();
}


void CML::mixerFvMesh::calcMovingMasks() const
{
    if (debug)
    {
        Info<< "void mixerFvMesh::calcMovingMasks() const : "
            << "Calculating point and cell masks"
            << endl;
    }

    if (movingPointsMaskPtr_)
    {
        FatalErrorInFunction
            << "point mask already calculated"
            << abort(FatalError);
    }

    // Set the point mask
    movingPointsMaskPtr_ = new scalarField(points().size(), 0);
    scalarField& movingPointsMask = *movingPointsMaskPtr_;

    const cellList& c = cells();
    const faceList& f = faces();

    const labelList& cellAddr = cellZones()["movingCells"];

    forAll(cellAddr, cellI)
    {
        const cell& curCell = c[cellAddr[cellI]];

        forAll(curCell, faceI)
        {
            // Mark all the points as moving
            const face& curFace = f[curCell[faceI]];

            forAll(curFace, pointI)
            {
                movingPointsMask[curFace[pointI]] = 1;
            }
        }
    }

    const word innerSliderZoneName
    (
        word(motionDict_.subDict("slider").lookup("inside"))
      + "Zone"
    );

    const labelList& innerSliderAddr = faceZones()[innerSliderZoneName];

    forAll(innerSliderAddr, faceI)
    {
        const face& curFace = f[innerSliderAddr[faceI]];

        forAll(curFace, pointI)
        {
            movingPointsMask[curFace[pointI]] = 1;
        }
    }

    const word outerSliderZoneName
    (
        word(motionDict_.subDict("slider").lookup("outside"))
      + "Zone"
    );

    const labelList& outerSliderAddr = faceZones()[outerSliderZoneName];

    forAll(outerSliderAddr, faceI)
    {
        const face& curFace = f[outerSliderAddr[faceI]];

        forAll(curFace, pointI)
        {
            movingPointsMask[curFace[pointI]] = 0;
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from components
CML::mixerFvMesh::mixerFvMesh
(
    const IOobject& io
)
:
    topoChangerFvMesh(io),
    motionDict_
    (
        IOdictionary
        (
            IOobject
            (
                "dynamicMeshDict",
                time().constant(),
                *this,
                IOobject::MUST_READ_IF_MODIFIED,
                IOobject::NO_WRITE,
                false
            )
        ).subDict(typeName + "Coeffs")
    ),
    csPtr_
    (
        coordinateSystem::New
        (
            "coordinateSystem",
            motionDict_.subDict("coordinateSystem")
        )
    ),
    rpm_(readScalar(motionDict_.lookup("rpm"))),
    movingPointsMaskPtr_(nullptr)
{
    addZonesAndModifiers();

    Info<< "Mixer mesh:" << nl
        << "    origin: " << cs().origin() << nl
        << "    axis: " << cs().axis() << nl
        << "    rpm: " << rpm_ << endl;
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::mixerFvMesh::~mixerFvMesh()
{
    deleteDemandDrivenData(movingPointsMaskPtr_);
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// Return moving points mask.  Moving points marked with 1
const CML::scalarField& CML::mixerFvMesh::movingPointsMask() const
{
    if (!movingPointsMaskPtr_)
    {
        calcMovingMasks();
    }

    return *movingPointsMaskPtr_;
}


bool CML::mixerFvMesh::update()
{
     // Rotational speed needs to be converted from rpm
    movePoints
    (
        csPtr_->globalPosition
        (
            csPtr_->localPosition(points())
          + vector(0, rpm_*360.0*time().deltaTValue()/60.0, 0)
            *movingPointsMask()
        )
    );

    // Make changes. Use inflation (so put new points in topoChangeMap)
    autoPtr<mapPolyMesh> topoChangeMap = topoChanger_.changeMesh(true);

    if (topoChangeMap.valid())
    {
        if (debug)
        {
            Info<< "Mesh topology is changing" << endl;
        }

        deleteDemandDrivenData(movingPointsMaskPtr_);
    }

    movePoints
    (
        csPtr_->globalPosition
        (
            csPtr_->localPosition(oldPoints())
          + vector(0, rpm_*360.0*time().deltaTValue()/60.0, 0)
            *movingPointsMask()
        )
    );

    return true;
}


// ************************************************************************* //
