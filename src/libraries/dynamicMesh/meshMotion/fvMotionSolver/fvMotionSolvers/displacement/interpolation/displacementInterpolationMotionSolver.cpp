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

#include "displacementInterpolationMotionSolver.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "SortableList.hpp"
#include "IOList.hpp"
#include "Tuple2.hpp"
#include "mapPolyMesh.hpp"
#include "interpolateXY.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(displacementInterpolationMotionSolver, 0);

    addToRunTimeSelectionTable
    (
        motionSolver,
        displacementInterpolationMotionSolver,
        dictionary
    );

    template<>
    const word IOList<Tuple2<scalar, vector> >::typeName("scalarVectorTable");
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::displacementInterpolationMotionSolver::
displacementInterpolationMotionSolver
(
    const polyMesh& mesh,
    const IOdictionary& dict
)
:
    displacementMotionSolver(mesh, dict, typeName)
{
    // Get zones and their interpolation tables for displacement
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    List<Pair<word> > faceZoneToTable
    (
        coeffDict().lookup("interpolationTables")
    );

    const faceZoneMesh& fZones = mesh.faceZones();

    times_.setSize(fZones.size());
    displacements_.setSize(fZones.size());

    forAll(faceZoneToTable, i)
    {
        const word& zoneName = faceZoneToTable[i][0];
        label zoneI = fZones.findZoneID(zoneName);

        if (zoneI == -1)
        {
            FatalErrorInFunction
                << "Cannot find zone " << zoneName << endl
                << "Valid zones are " << mesh.faceZones().names()
                << exit(FatalError);
        }

        const word& tableName = faceZoneToTable[i][1];

        IOList<Tuple2<scalar, vector> > table
        (
            IOobject
            (
                tableName,
                mesh.time().constant(),
                "tables",
                mesh,
                IOobject::MUST_READ,
                IOobject::NO_WRITE,
                false
            )
        );

        // Copy table
        times_[zoneI].setSize(table.size());
        displacements_[zoneI].setSize(table.size());

        forAll(table, j)
        {
            times_[zoneI][j] = table[j].first();
            displacements_[zoneI][j] = table[j].second();
        }
    }



    // Sort points into bins according to position relative to faceZones
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Done in all three directions.

    for (direction dir = 0; dir < vector::nComponents; dir++)
    {
        // min and max coordinates of all faceZones
        SortableList<scalar> zoneCoordinates(2*faceZoneToTable.size());

        forAll(faceZoneToTable, i)
        {
            const word& zoneName = faceZoneToTable[i][0];
            const faceZone& fz = fZones[zoneName];

            scalar minCoord = VGREAT;
            scalar maxCoord = -VGREAT;

            forAll(fz().meshPoints(), localI)
            {
                label pointI = fz().meshPoints()[localI];
                const scalar coord = points0()[pointI][dir];
                minCoord = min(minCoord, coord);
                maxCoord = max(maxCoord, coord);
            }

            zoneCoordinates[2*i] = returnReduce(minCoord, minOp<scalar>());
            zoneCoordinates[2*i+1] = returnReduce(maxCoord, maxOp<scalar>());

            if (debug)
            {
                Pout<< "direction " << dir << " : "
                    << "zone " << zoneName
                    << " ranges from coordinate " << zoneCoordinates[2*i]
                    << " to " << zoneCoordinates[2*i+1]
                    << endl;
            }
        }
        zoneCoordinates.sort();

        // Slightly tweak min and max face zone so points sort within
        zoneCoordinates[0] -= SMALL;
        zoneCoordinates.last() += SMALL;

        // Check if we have static min and max mesh bounds
        const scalarField meshCoords(points0().component(dir));

        scalar minCoord = gMin(meshCoords);
        scalar maxCoord = gMax(meshCoords);

        if (debug)
        {
            Pout<< "direction " << dir << " : "
                << "mesh ranges from coordinate " << minCoord << " to "
                << maxCoord << endl;
        }

        // Make copy of zoneCoordinates; include min and max of mesh
        // if necessary. Mark min and max with zoneI=-1.
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        labelList& rangeZone = rangeToZone_[dir];
        labelListList& rangePoints = rangeToPoints_[dir];
        List<scalarField>& rangeWeights = rangeToWeights_[dir];

        scalarField rangeToCoord(zoneCoordinates.size());
        rangeZone.setSize(zoneCoordinates.size());
        label rangeI = 0;

        if (minCoord < zoneCoordinates[0])
        {
            label sz = rangeZone.size();
            rangeToCoord.setSize(sz+1);
            rangeZone.setSize(sz+1);
            rangeToCoord[rangeI] = minCoord-SMALL;
            rangeZone[rangeI] = -1;

            if (debug)
            {
                Pout<< "direction " << dir << " : "
                    << "range " << rangeI << " at coordinate "
                    << rangeToCoord[rangeI] << " from min of mesh "
                    << rangeZone[rangeI] << endl;
            }
            rangeI = 1;
        }
        forAll(zoneCoordinates, i)
        {
            rangeToCoord[rangeI] = zoneCoordinates[i];
            rangeZone[rangeI] = zoneCoordinates.indices()[i]/2;

            if (debug)
            {
                Pout<< "direction " << dir << " : "
                    << "range " << rangeI << " at coordinate "
                    << rangeToCoord[rangeI]
                    << " from zone " << rangeZone[rangeI] << endl;
            }
            rangeI++;
        }
        if (maxCoord > zoneCoordinates.last())
        {
            label sz = rangeToCoord.size();
            rangeToCoord.setSize(sz+1);
            rangeZone.setSize(sz+1);
            rangeToCoord[sz] = maxCoord+SMALL;
            rangeZone[sz] = -1;

            if (debug)
            {
                Pout<< "direction " << dir << " : "
                    << "range " << rangeI << " at coordinate "
                    << rangeToCoord[sz] << " from max of mesh "
                    << rangeZone[sz] << endl;
            }
        }


        // Sort the points
        // ~~~~~~~~~~~~~~~

        // Count all the points inbetween rangeI and rangeI+1
        labelList nRangePoints(rangeToCoord.size(), 0);

        forAll(meshCoords, pointI)
        {
            label rangeI = findLower(rangeToCoord, meshCoords[pointI]);

            if (rangeI == -1 || rangeI == rangeToCoord.size()-1)
            {
                FatalErrorInFunction
                    << "Did not find point " << points0()[pointI]
                    << " coordinate " << meshCoords[pointI]
                    << " in ranges " << rangeToCoord
                    << abort(FatalError);
            }
            nRangePoints[rangeI]++;
        }

        if (debug)
        {
            for (label rangeI = 0; rangeI < rangeToCoord.size()-1; rangeI++)
            {
                // Get the two zones bounding the range
                Pout<< "direction " << dir << " : "
                    << "range from " << rangeToCoord[rangeI]
                    << " to " << rangeToCoord[rangeI+1]
                    << " contains " << nRangePoints[rangeI]
                    << " points." << endl;
            }
        }

        // Sort
        rangePoints.setSize(nRangePoints.size());
        rangeWeights.setSize(nRangePoints.size());
        forAll(rangePoints, rangeI)
        {
            rangePoints[rangeI].setSize(nRangePoints[rangeI]);
            rangeWeights[rangeI].setSize(nRangePoints[rangeI]);
        }
        nRangePoints = 0;
        forAll(meshCoords, pointI)
        {
            label rangeI = findLower(rangeToCoord, meshCoords[pointI]);
            label& nPoints = nRangePoints[rangeI];
            rangePoints[rangeI][nPoints] = pointI;
            rangeWeights[rangeI][nPoints] =
                (meshCoords[pointI]-rangeToCoord[rangeI])
              / (rangeToCoord[rangeI+1]-rangeToCoord[rangeI]);
            nPoints++;
        }
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::displacementInterpolationMotionSolver::
~displacementInterpolationMotionSolver()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::tmp<CML::pointField>
CML::displacementInterpolationMotionSolver::curPoints() const
{
    if (mesh().nPoints() != points0().size())
    {
        FatalErrorInFunction
            << "The number of points in the mesh seems to have changed." << endl
            << "In constant/polyMesh there are " << points0().size()
            << " points; in the current mesh there are " << mesh().nPoints()
            << " points." << exit(FatalError);
    }

    tmp<pointField> tcurPoints(new pointField(points0()));
    pointField& curPoints = tcurPoints();

    // Interpolate the displacement of the face zones.
    vectorField zoneDisp(displacements_.size(), vector::zero);
    forAll(zoneDisp, zoneI)
    {
        if (times_[zoneI].size())
        {
            zoneDisp[zoneI] = interpolateXY
            (
                mesh().time().value(),
                times_[zoneI],
                displacements_[zoneI]
            );
        }
    }
    if (debug)
    {
        Pout<< "Zone displacements:" << zoneDisp << endl;
    }


    // Interpolate the point location
    for (direction dir = 0; dir < vector::nComponents; dir++)
    {
        const labelList& rangeZone = rangeToZone_[dir];
        const labelListList& rangePoints = rangeToPoints_[dir];
        const List<scalarField>& rangeWeights = rangeToWeights_[dir];

        for (label rangeI = 0; rangeI < rangeZone.size()-1; rangeI++)
        {
            const labelList& rPoints = rangePoints[rangeI];
            const scalarField& rWeights = rangeWeights[rangeI];

            // Get the two zones bounding the range
            label minZoneI = rangeZone[rangeI];
            //vector minDisp =
            //    (minZoneI == -1 ? vector::zero : zoneDisp[minZoneI]);
            scalar minDisp = (minZoneI == -1 ? 0.0 : zoneDisp[minZoneI][dir]);
            label maxZoneI = rangeZone[rangeI+1];
            //vector maxDisp =
            //    (maxZoneI == -1 ? vector::zero : zoneDisp[maxZoneI]);
            scalar maxDisp = (maxZoneI == -1 ? 0.0 : zoneDisp[maxZoneI][dir]);

            forAll(rPoints, i)
            {
                label pointI = rPoints[i];
                scalar w = rWeights[i];
                //curPoints[pointI] += (1.0-w)*minDisp+w*maxDisp;
                curPoints[pointI][dir] += (1.0-w)*minDisp+w*maxDisp;
            }
        }
    }
    return tcurPoints;
}


// ************************************************************************* //
