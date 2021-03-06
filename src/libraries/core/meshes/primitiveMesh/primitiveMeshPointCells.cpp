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

#include "primitiveMesh.hpp"
#include "cell.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::primitiveMesh::calcPointCells() const
{
    // Loop through cells and mark up points

    if (debug)
    {
        Pout<< "primitiveMesh::calcPointCells() : "
            << "calculating pointCells"
            << endl;

        if (debug == -1)
        {
            // For checking calls:abort so we can quickly hunt down
            // origin of call
            FatalErrorInFunction
                << abort(FatalError);
        }
    }

    // It is an error to attempt to recalculate pointCells
    // if the pointer is already set
    if (pcPtr_)
    {
        FatalErrorInFunction
            << "pointCells already calculated"
            << abort(FatalError);
    }
    else
    {
        const cellList& cf = cells();

        // Count number of cells per point

        labelList npc(nPoints(), 0);

        forAll(cf, cellI)
        {
            const labelList curPoints = cf[cellI].labels(faces());

            forAll(curPoints, pointI)
            {
                label ptI = curPoints[pointI];

                npc[ptI]++;
            }
        }


        // Size and fill cells per point

        pcPtr_ = new labelListList(npc.size());
        labelListList& pointCellAddr = *pcPtr_;

        forAll(pointCellAddr, pointI)
        {
            pointCellAddr[pointI].setSize(npc[pointI]);
        }
        npc = 0;


        forAll(cf, cellI)
        {
            const labelList curPoints = cf[cellI].labels(faces());

            forAll(curPoints, pointI)
            {
                label ptI = curPoints[pointI];

                pointCellAddr[ptI][npc[ptI]++] = cellI;
            }
        }
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const CML::labelListList& CML::primitiveMesh::pointCells() const
{
    if (!pcPtr_)
    {
        calcPointCells();
    }

    return *pcPtr_;
}


const CML::labelList& CML::primitiveMesh::pointCells
(
    const label pointI,
    DynamicList<label>& storage
) const
{
    if (hasPointCells())
    {
        return pointCells()[pointI];
    }
    else
    {
        const labelList& own = faceOwner();
        const labelList& nei = faceNeighbour();
        const labelList& pFaces = pointFaces()[pointI];

        storage.clear();

        forAll(pFaces, i)
        {
            const label faceI = pFaces[i];

            // Append owner
            storage.append(own[faceI]);

            // Append neighbour
            if (faceI < nInternalFaces())
            {
                storage.append(nei[faceI]);
            }
        }

        // Filter duplicates
        if (storage.size() > 1)
        {
            sort(storage);

            label n = 1;
            for (label i = 1; i < storage.size(); i++)
            {
                if (storage[i-1] != storage[i])
                {
                    storage[n++] = storage[i];
                }
            }

            // truncate addressed list
            storage.setSize(n);
        }

        return storage;
    }
}


const CML::labelList& CML::primitiveMesh::pointCells(const label pointI) const
{
    return pointCells(pointI, labels_);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// ************************************************************************* //
