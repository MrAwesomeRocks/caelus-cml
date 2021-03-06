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

#include "lduAddressing.hpp"
#include "demandDrivenData.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::lduAddressing::calcLosort() const
{
    if (losortPtr_)
    {
        FatalErrorInFunction
            << "losort already calculated"
            << abort(FatalError);
    }

    // Scan the neighbour list to find out how many times the cell
    // appears as a neighbour of the face. Done this way to avoid guessing
    // and resizing list
    labelList nNbrOfFace(size(), 0);

    const labelUList& nbr = upperAddr();

    forAll(nbr, nbrI)
    {
        nNbrOfFace[nbr[nbrI]]++;
    }

    // Create temporary neighbour addressing
    labelListList cellNbrFaces(size());

    forAll(cellNbrFaces, cellI)
    {
        cellNbrFaces[cellI].setSize(nNbrOfFace[cellI]);
    }

    // Reset the list of number of neighbours to zero
    nNbrOfFace = 0;

    // Scatter the neighbour faces
    forAll(nbr, nbrI)
    {
        cellNbrFaces[nbr[nbrI]][nNbrOfFace[nbr[nbrI]]] = nbrI;

        nNbrOfFace[nbr[nbrI]]++;
    }

    // Gather the neighbours into the losort array
    losortPtr_ = new labelList(nbr.size(), -1);

    labelList& lst = *losortPtr_;

    // Set counter for losort
    label lstI = 0;

    forAll(cellNbrFaces, cellI)
    {
        const labelList& curNbr = cellNbrFaces[cellI];

        forAll(curNbr, curNbrI)
        {
            lst[lstI] = curNbr[curNbrI];
            lstI++;
        }
    }
}


void CML::lduAddressing::calcOwnerStart() const
{
    if (ownerStartPtr_)
    {
        FatalErrorInFunction
            << "owner start already calculated"
            << abort(FatalError);
    }

    const labelList& own = lowerAddr();

    ownerStartPtr_ = new labelList(size() + 1, own.size());

    labelList& ownStart = *ownerStartPtr_;

    // Set up first lookup by hand
    ownStart[0] = 0;
    label nOwnStart = 0;
    label i = 1;

    forAll(own, faceI)
    {
        label curOwn = own[faceI];

        if (curOwn > nOwnStart)
        {
            while (i <= curOwn)
            {
                ownStart[i++] = faceI;
            }

            nOwnStart = curOwn;
        }
    }
}


void CML::lduAddressing::calcLosortStart() const
{
    if (losortStartPtr_)
    {
        FatalErrorInFunction
            << "losort start already calculated"
            << abort(FatalError);
    }

    losortStartPtr_ = new labelList(size() + 1, 0);

    labelList& lsrtStart = *losortStartPtr_;

    const labelList& nbr = upperAddr();

    const labelList& lsrt = losortAddr();

    // Set up first lookup by hand
    lsrtStart[0] = 0;
    label nLsrtStart = 0;
    label i = 0;

    forAll(lsrt, faceI)
    {
        // Get neighbour
        const label curNbr = nbr[lsrt[faceI]];

        if (curNbr > nLsrtStart)
        {
            while (i <= curNbr)
            {
                lsrtStart[i++] = faceI;
            }

            nLsrtStart = curNbr;
        }
    }

    // Set up last lookup by hand
    lsrtStart[size()] = nbr.size();
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::lduAddressing::~lduAddressing()
{
    deleteDemandDrivenData(losortPtr_);
    deleteDemandDrivenData(ownerStartPtr_);
    deleteDemandDrivenData(losortStartPtr_);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const CML::labelUList& CML::lduAddressing::losortAddr() const
{
    if (!losortPtr_)
    {
        calcLosort();
    }

    return *losortPtr_;
}


const CML::labelUList& CML::lduAddressing::ownerStartAddr() const
{
    if (!ownerStartPtr_)
    {
        calcOwnerStart();
    }

    return *ownerStartPtr_;
}


const CML::labelUList& CML::lduAddressing::losortStartAddr() const
{
    if (!losortStartPtr_)
    {
        calcLosortStart();
    }

    return *losortStartPtr_;
}


// Return edge index given owner and neighbour label
CML::label CML::lduAddressing::triIndex(const label a, const label b) const
{
    label own = min(a, b);

    label nbr = max(a, b);

    label startLabel = ownerStartAddr()[own];

    label endLabel = ownerStartAddr()[own + 1];

    const labelUList& neighbour = upperAddr();

    for (label i = startLabel; i < endLabel; i++)
    {
        if (neighbour[i] == nbr)
        {
            return i;
        }
    }

    // If neighbour has not been found, something has gone seriously
    // wrong with the addressing mechanism
    FatalErrorInFunction
        << "neighbour " << nbr << " not found for owner " << own << ". "
        << "Problem with addressing"
        << abort(FatalError);

    return -1;
}


// ************************************************************************* //
