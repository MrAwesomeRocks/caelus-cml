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

#include "multiSolidBodyMotionFvMesh.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "volFields.hpp"
#include "transformField.hpp"
#include "cellZoneMesh.hpp"
#include "boolList.hpp"
#include "syncTools.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(multiSolidBodyMotionFvMesh, 0);
    addToRunTimeSelectionTable
    (
        dynamicFvMesh,
        multiSolidBodyMotionFvMesh,
        IOobject
    );
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::multiSolidBodyMotionFvMesh::multiSolidBodyMotionFvMesh(const IOobject& io)
:
    dynamicFvMesh(io),
    dynamicMeshCoeffs_
    (
		IOdictionary
        (
			IOobject
            (
                "dynamicMeshDict",
                io.time().constant(),
                *this,
                IOobject::MUST_READ_IF_MODIFIED,
                IOobject::NO_WRITE,
                false
            )
        ).subDict(typeName + "Coeffs")
    ),
    undisplacedPoints_
    (
		IOobject
        (
            "points",
            io.time().constant(),
            meshSubDir,
            *this,
            IOobject::MUST_READ,
            IOobject::NO_WRITE,
            false
        )
    )
{
    if (undisplacedPoints_.size() != nPoints())
    {
        FatalIOErrorInFunction(dynamicMeshCoeffs_)
            << "Read " << undisplacedPoints_.size()
            << " undisplaced points from " << undisplacedPoints_.objectPath()
            << " but the current mesh has " << nPoints()
            << exit(FatalIOError);
    }


    zoneIDs_.setSize(dynamicMeshCoeffs_.size());
    SBMFs_.setSize(dynamicMeshCoeffs_.size());
    pointIDs_.setSize(dynamicMeshCoeffs_.size());
    label zoneI = 0;

    forAllConstIter(dictionary, dynamicMeshCoeffs_, iter)
    {
        if (iter().isDict())
        {
            zoneIDs_[zoneI] = cellZones().findZoneID(iter().keyword());

            if (zoneIDs_[zoneI] == -1)
            {
                FatalIOErrorInFunction(dynamicMeshCoeffs_)
                    << "Cannot find cellZone named " << iter().keyword()
                    << ". Valid zones are " << cellZones().names()
                    << exit(FatalIOError);
            }

            const dictionary& subDict = iter().dict();

            SBMFs_.set
            (
                zoneI,
                solidBodyMotionFunction::New(subDict, io.time())
            );

            // Collect points of cell zone.
            const cellZone& cz = cellZones()[zoneIDs_[zoneI]];

            boolList movePts(nPoints(), false);

            forAll(cz, i)
            {
                label cellI = cz[i];
                const cell& c = cells()[cellI];
                forAll(c, j)
                {
                    const face& f = faces()[c[j]];
                    forAll(f, k)
                    {
                        label pointI = f[k];
                        movePts[pointI] = true;
                    }
                }
            }

            syncTools::syncPointList(*this, movePts, orEqOp<bool>(), false);

            DynamicList<label> ptIDs(nPoints());
            forAll(movePts, i)
            {
                if (movePts[i])
                {
                    ptIDs.append(i);
                }
            }

            pointIDs_[zoneI].transfer(ptIDs);

            Info<< "Applying solid body motion " << SBMFs_[zoneI].type()
                << " to " << pointIDs_[zoneI].size() << " points of cellZone "
                << iter().keyword() << endl;

            zoneI++;
        }
    }
    zoneIDs_.setSize(zoneI);
    SBMFs_.setSize(zoneI);
    pointIDs_.setSize(zoneI);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::multiSolidBodyMotionFvMesh::~multiSolidBodyMotionFvMesh()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool CML::multiSolidBodyMotionFvMesh::update()
{
    static bool hasWarned = false;

    pointField transformedPts(undisplacedPoints_);

    forAll(zoneIDs_, i)
    {
        const labelList& zonePoints = pointIDs_[i];

        UIndirectList<point>(transformedPts, zonePoints) =
            transformPoints
            (
                SBMFs_[i].transformation(),
                pointField(transformedPts, zonePoints)
            );
    }

    fvMesh::movePoints(transformedPts);

    if (foundObject<volVectorField>("U"))
    {
        const_cast<volVectorField&>(lookupObject<volVectorField>("U"))
            .correctBoundaryConditions();
    }
    else if (!hasWarned)
    {
        hasWarned = true;

        WarningInFunction
            << "Did not find volVectorField U."
            << " Not updating U boundary conditions." << endl;
    }

    return true;
}


// ************************************************************************* //
