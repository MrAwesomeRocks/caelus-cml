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

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::primitiveMesh::calcCells
(
    cellList& cellFaceAddr,
    const labelUList& own,
    const labelUList& nei,
    const label inNCells
)
{
    label nCells = inNCells;

    if (nCells == -1)
    {
        nCells = -1;

        forAll(own, faceI)
        {
            nCells = max(nCells, own[faceI]);
        }
        nCells++;
    }

    // 1. Count number of faces per cell

    labelList ncf(nCells, 0);

    forAll(own, faceI)
    {
        ncf[own[faceI]]++;
    }

    forAll(nei, faceI)
    {
        if (nei[faceI] >= 0)
        {
            ncf[nei[faceI]]++;
        }
    }

    // Create the storage
    cellFaceAddr.setSize(ncf.size());


    // 2. Size and fill cellFaceAddr

    forAll(cellFaceAddr, cellI)
    {
        cellFaceAddr[cellI].setSize(ncf[cellI]);
    }
    ncf = 0;

    forAll(own, faceI)
    {
        label cellI = own[faceI];

        cellFaceAddr[cellI][ncf[cellI]++] = faceI;
    }

    forAll(nei, faceI)
    {
        label cellI = nei[faceI];

        if (cellI >= 0)
        {
            cellFaceAddr[cellI][ncf[cellI]++] = faceI;
        }
    }
}


void CML::primitiveMesh::calcCells() const
{
    // Loop through faceCells and mark up neighbours

    if (debug)
    {
        Pout<< "primitiveMesh::calcCells() : calculating cells"
            << endl;
    }

    // It is an error to attempt to recalculate cells
    // if the pointer is already set
    if (cfPtr_)
    {
        FatalErrorInFunction
            << "cells already calculated"
            << abort(FatalError);
    }
    else
    {
        // Create the storage
        cfPtr_ = new cellList(nCells());
        cellList& cellFaceAddr = *cfPtr_;

        calcCells
        (
            cellFaceAddr,
            faceOwner(),
            faceNeighbour(),
            nCells()
        );
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const CML::cellList& CML::primitiveMesh::cells() const
{
    if (!cfPtr_)
    {
        calcCells();
    }

    return *cfPtr_;
}


// ************************************************************************* //
