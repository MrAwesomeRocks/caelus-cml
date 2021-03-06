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

#include "displacementLayeredMotionMotionSolver.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "pointEdgeStructuredWalk.hpp"
#include "pointFields.hpp"
#include "PointEdgeWave.hpp"
#include "syncTools.hpp"
#include "interpolationTable.hpp"
#include "mapPolyMesh.hpp"
#include "pointConstraints.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(displacementLayeredMotionMotionSolver, 0);

    addToRunTimeSelectionTable
    (
        motionSolver,
        displacementLayeredMotionMotionSolver,
        dictionary
    );
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::displacementLayeredMotionMotionSolver::calcZoneMask
(
    const label cellZoneI,
    PackedBoolList& isZonePoint,
    PackedBoolList& isZoneEdge
) const
{
    if (cellZoneI == -1)
    {
        isZonePoint.setSize(mesh().nPoints());
        isZonePoint = 1;

        isZoneEdge.setSize(mesh().nEdges());
        isZoneEdge = 1;
    }
    else
    {
        const cellZone& cz = mesh().cellZones()[cellZoneI];

        label nPoints = 0;
        forAll(cz, i)
        {
            const labelList& cPoints = mesh().cellPoints(cz[i]);
            forAll(cPoints, cPointI)
            {
                if (!isZonePoint[cPoints[cPointI]])
                {
                    isZonePoint[cPoints[cPointI]] = 1;
                    nPoints++;
                }
            }
        }
        syncTools::syncPointList
        (
            mesh(),
            isZonePoint,
            orEqOp<unsigned int>(),
            0
        );


        // Mark edge inside cellZone
        label nEdges = 0;
        forAll(cz, i)
        {
            const labelList& cEdges = mesh().cellEdges(cz[i]);
            forAll(cEdges, cEdgeI)
            {
                if (!isZoneEdge[cEdges[cEdgeI]])
                {
                    isZoneEdge[cEdges[cEdgeI]] = 1;
                    nEdges++;
                }
            }
        }
        syncTools::syncEdgeList
        (
            mesh(),
            isZoneEdge,
            orEqOp<unsigned int>(),
            0
        );

        if (debug)
        {
            Info<< "On cellZone " << cz.name()
                << " marked " << returnReduce(nPoints, sumOp<label>())
                << " points and " << returnReduce(nEdges, sumOp<label>())
                << " edges." << endl;
        }
    }
}


// Find distance to starting point
void CML::displacementLayeredMotionMotionSolver::walkStructured
(
    const label cellZoneI,
    const PackedBoolList& isZonePoint,
    const PackedBoolList& isZoneEdge,
    const labelList& seedPoints,
    const vectorField& seedData,
    scalarField& distance,
    vectorField& data
) const
{
    List<pointEdgeStructuredWalk> seedInfo(seedPoints.size());

    forAll(seedPoints, i)
    {
        seedInfo[i] = pointEdgeStructuredWalk
        (
            points0()[seedPoints[i]],  // location of data
            points0()[seedPoints[i]],  // previous location
            0.0,
            seedData[i]
        );
    }

    // Current info on points
    List<pointEdgeStructuredWalk> allPointInfo(mesh().nPoints());

    // Mark points inside cellZone.
    // Note that we use points0, not mesh.points()
    // so as not to accumulate errors.
    forAll(isZonePoint, pointI)
    {
        if (isZonePoint[pointI])
        {
            allPointInfo[pointI] = pointEdgeStructuredWalk
            (
                points0()[pointI],  // location of data
                vector::max,        // not valid
                0.0,
                vector::zero        // passive data
            );
        }
    }

    // Current info on edges
    List<pointEdgeStructuredWalk> allEdgeInfo(mesh().nEdges());

    // Mark edges inside cellZone
    forAll(isZoneEdge, edgeI)
    {
        if (isZoneEdge[edgeI])
        {
            allEdgeInfo[edgeI] = pointEdgeStructuredWalk
            (
                mesh().edges()[edgeI].centre(points0()),    // location of data
                vector::max,                                // not valid
                0.0,
                vector::zero
            );
        }
    }

    // Walk
    PointEdgeWave<pointEdgeStructuredWalk> wallCalc
    (
        mesh(),
        seedPoints,
        seedInfo,

        allPointInfo,
        allEdgeInfo,
        mesh().globalData().nTotalPoints()  // max iterations
    );

    // Extract distance and passive data
    forAll(allPointInfo, pointI)
    {
        if (isZonePoint[pointI])
        {
            distance[pointI] = allPointInfo[pointI].dist();
            data[pointI] = allPointInfo[pointI].data();
        }
    }
}


// Evaluate faceZone patch
CML::tmp<CML::vectorField>
CML::displacementLayeredMotionMotionSolver::faceZoneEvaluate
(
    const faceZone& fz,
    const labelList& meshPoints,
    const dictionary& dict,
    const PtrList<pointVectorField>& patchDisp,
    const label patchI
) const
{
    tmp<vectorField> tfld(new vectorField(meshPoints.size()));
    vectorField& fld = tfld();

    const word type(dict.lookup("type"));

    if (type == "fixedValue")
    {
        fld = vectorField("value", dict, meshPoints.size());
    }
    else if (type == "timeVaryingUniformFixedValue")
    {
        interpolationTable<vector> timeSeries(dict);

        fld = timeSeries(mesh().time().timeOutputValue());
    }
    else if (type == "slip")
    {
        if ((patchI % 2) != 1)
        {
            FatalIOErrorInFunction(*this)
                << "slip can only be used on second faceZonePatch of pair."
                << "FaceZone:" << fz.name()
                << exit(FatalIOError);
        }
        // Use field set by previous bc
        fld = vectorField(patchDisp[patchI-1], meshPoints);
    }
    else if (type == "follow")
    {
        // Only on boundary faces - follow boundary conditions
        fld = vectorField(pointDisplacement_, meshPoints);
    }
    else if (type == "uniformFollow")
    {
        // Reads name of name of patch. Then get average point dislacement on
        // patch. That becomes the value of fld.
        const word patchName(dict.lookup("patch"));
        label patchID = mesh().boundaryMesh().findPatchID(patchName);
        pointField pdf
        (
            pointDisplacement_.boundaryField()[patchID].patchInternalField()
        );
        fld = gAverage(pdf);
    }
    else
    {
        FatalIOErrorInFunction(*this)
            << "Unknown faceZonePatch type " << type << " for faceZone "
            << fz.name() << exit(FatalIOError);
    }
    return tfld;
}


void CML::displacementLayeredMotionMotionSolver::cellZoneSolve
(
    const label cellZoneI,
    const dictionary& zoneDict
)
{
    PackedBoolList isZonePoint(mesh().nPoints());
    PackedBoolList isZoneEdge(mesh().nEdges());
    calcZoneMask(cellZoneI, isZonePoint, isZoneEdge);

    const dictionary& patchesDict = zoneDict.subDict("boundaryField");

    if (patchesDict.size() != 2)
    {
        FatalIOErrorInFunction(*this)
            << "Can only handle 2 faceZones (= patches) per cellZone. "
            << " cellZone:" << cellZoneI
            << " patches:" << patchesDict.toc()
            << exit(FatalIOError);
    }

    PtrList<scalarField> patchDist(patchesDict.size());
    PtrList<pointVectorField> patchDisp(patchesDict.size());

    // Allocate the fields
    label patchI = 0;
    forAllConstIter(dictionary, patchesDict, patchIter)
    {
        const word& faceZoneName = patchIter().keyword();
        label zoneI = mesh().faceZones().findZoneID(faceZoneName);
        if (zoneI == -1)
        {
            FatalIOErrorInFunction(*this)
                << "Cannot find faceZone " << faceZoneName
                << endl << "Valid zones are " << mesh().faceZones().names()
                << exit(FatalIOError);
        }

        // Determine the points of the faceZone within the cellZone
        const faceZone& fz = mesh().faceZones()[zoneI];

        patchDist.set(patchI, new scalarField(mesh().nPoints()));
        patchDisp.set
        (
            patchI,
            new pointVectorField
            (
                IOobject
                (
                    mesh().cellZones()[cellZoneI].name() + "_" + fz.name(),
                    mesh().time().timeName(),
                    mesh(),
                    IOobject::NO_READ,
                    IOobject::NO_WRITE,
                    false
                ),
                pointDisplacement_  // to inherit the boundary conditions
            )
        );

        patchI++;
    }



    // 'correctBoundaryConditions'
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Loops over all the faceZones and walks their boundary values

    // Make sure we can pick up bc values from field
    pointDisplacement_.correctBoundaryConditions();

    patchI = 0;
    forAllConstIter(dictionary, patchesDict, patchIter)
    {
        const word& faceZoneName = patchIter().keyword();
        const dictionary& faceZoneDict = patchIter().dict();

        // Determine the points of the faceZone within the cellZone
        const faceZone& fz = mesh().faceZones()[faceZoneName];
        const labelList& fzMeshPoints = fz().meshPoints();
        DynamicList<label> meshPoints(fzMeshPoints.size());
        forAll(fzMeshPoints, i)
        {
            if (isZonePoint[fzMeshPoints[i]])
            {
                meshPoints.append(fzMeshPoints[i]);
            }
        }

        // Get initial value for all the faceZone points
        tmp<vectorField> tseed = faceZoneEvaluate
        (
            fz,
            meshPoints,
            faceZoneDict,
            patchDisp,
            patchI
        );

        if (debug)
        {
            Info<< "For cellZone:" << cellZoneI
                << " for faceZone:" << fz.name()
                << " nPoints:" << tseed().size()
                << " have patchField:"
                << " max:" << gMax(tseed())
                << " min:" << gMin(tseed())
                << " avg:" << gAverage(tseed())
                << endl;
        }

        // Set distance and transported value
        walkStructured
        (
            cellZoneI,
            isZonePoint,
            isZoneEdge,

            meshPoints,
            tseed,
            patchDist[patchI],
            patchDisp[patchI]
        );

        // Implement real bc.
        patchDisp[patchI].correctBoundaryConditions();

        patchI++;
    }


    // Solve
    // ~~~~~

    if (debug)
    {
        // Normalised distance
        pointScalarField distance
        (
            IOobject
            (
                mesh().cellZones()[cellZoneI].name() + ":distance",
                mesh().time().timeName(),
                mesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE,
                false
            ),
            pointMesh::New(mesh()),
            dimensionedScalar("zero", dimLength, 0.0)
        );
        forAll(distance, pointI)
        {
            scalar d1 = patchDist[0][pointI];
            scalar d2 = patchDist[1][pointI];
            if (d1 + d2 > SMALL)
            {
                scalar s = d1/(d1 + d2);
                distance[pointI] = s;
            }
        }

        Info<< "Writing " << pointScalarField::typeName
            << distance.name() << " to "
            << mesh().time().timeName() << endl;
        distance.write();
    }


    const word interpolationScheme = zoneDict.lookup("interpolationScheme");

    if (interpolationScheme == "oneSided")
    {
        forAll(pointDisplacement_, pointi)
        {
            if (isZonePoint[pointi])
            {
                pointDisplacement_[pointi] = patchDisp[0][pointi];
            }
        }
    }
    else if (interpolationScheme == "linear")
    {
        forAll(pointDisplacement_, pointI)
        {
            if (isZonePoint[pointI])
            {
                scalar d1 = patchDist[0][pointI];
                scalar d2 = patchDist[1][pointI];
                scalar s = d1/(d1 + d2 + VSMALL);

                const vector& pd1 = patchDisp[0][pointI];
                const vector& pd2 = patchDisp[1][pointI];

                pointDisplacement_[pointI] = (1 - s)*pd1 + s*pd2;
            }
        }
    }
    else
    {
            FatalErrorInFunction
            << "Invalid interpolationScheme: " << interpolationScheme
            << ". Valid schemes are 'oneSided' and 'linear'"
            << exit(FatalError);
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::displacementLayeredMotionMotionSolver::
displacementLayeredMotionMotionSolver
(
    const polyMesh& mesh,
    const IOdictionary& dict
)
:
    displacementMotionSolver(mesh, dict, typeName)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::displacementLayeredMotionMotionSolver::
~displacementLayeredMotionMotionSolver()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::tmp<CML::pointField>
CML::displacementLayeredMotionMotionSolver::curPoints() const
{
    tmp<pointField> tcurPoints
    (
        points0() + pointDisplacement_.internalField()
    );

    return tcurPoints;
}


void CML::displacementLayeredMotionMotionSolver::solve()
{
    // The points have moved so before interpolation update the motionSolver
    movePoints(mesh().points());

    // Apply boundary conditions
    pointDisplacement_.boundaryField().updateCoeffs();

    // Solve motion on all regions (=cellZones)
    const dictionary& regionDicts = coeffDict().subDict("regions");
    forAllConstIter(dictionary, regionDicts, regionIter)
    {
        const word& cellZoneName = regionIter().keyword();
        const dictionary& regionDict = regionIter().dict();

        label zoneI = mesh().cellZones().findZoneID(cellZoneName);

        Info<< "solving for zone: " << cellZoneName << endl;

        if (zoneI == -1)
        {
            FatalIOErrorInFunction(*this)
                << "Cannot find cellZone " << cellZoneName
                << endl << "Valid zones are " << mesh().cellZones().names()
                << exit(FatalIOError);
        }

        cellZoneSolve(zoneI, regionDict);
    }

    // Update pointDisplacement for solved values
    const pointConstraints& pcs =
        pointConstraints::New(pointDisplacement_.mesh());
    pcs.constrainDisplacement(pointDisplacement_, false);
}


void CML::displacementLayeredMotionMotionSolver::updateMesh
(
    const mapPolyMesh& mpm
)
{
    displacementMotionSolver::updateMesh(mpm);

    const vectorField displacement(this->newPoints() - points0_);

    forAll(points0_, pointi)
    {
        label oldPointi = mpm.pointMap()[pointi];

        if (oldPointi >= 0)
        {
            label masterPointi = mpm.reversePointMap()[oldPointi];

            if ((masterPointi != pointi))
            {
                // newly inserted point in this cellZone

                // need to set point0 so that it represents the position that
                // it would have had if it had existed for all time
                points0_[pointi] -= displacement[pointi];
            }
        }
    }
}


// ************************************************************************* //
