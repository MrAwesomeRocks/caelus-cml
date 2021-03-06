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
#include "ListOps.hpp"


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const CML::labelListList& CML::primitiveMesh::edgeCells() const
{
    if (!ecPtr_)
    {
        if (debug)
        {
            Pout<< "primitiveMesh::edgeCells() : calculating edgeCells" << endl;

            if (debug == -1)
            {
                // For checking calls:abort so we can quickly hunt down
                // origin of call
                FatalErrorInFunction
                    << abort(FatalError);
            }
        }
        // Invert cellEdges
        ecPtr_ = new labelListList(nEdges());
        invertManyToMany(nEdges(), cellEdges(), *ecPtr_);
    }

    return *ecPtr_;
}


const CML::labelList& CML::primitiveMesh::edgeCells
(
    const label edgeI,
    DynamicList<label>& storage
) const
{
    if (hasEdgeCells())
    {
        return edgeCells()[edgeI];
    }
    else
    {
        const labelList& own = faceOwner();
        const labelList& nei = faceNeighbour();

        // Construct edgeFaces
        DynamicList<label> eFacesStorage;
        const labelList& eFaces = edgeFaces(edgeI, eFacesStorage);

        storage.clear();

        // Do quadratic insertion.
        forAll(eFaces, i)
        {
            label faceI = eFaces[i];

            {
                label ownCellI = own[faceI];

                // Check if not already in storage
                forAll(storage, j)
                {
                    if (storage[j] == ownCellI)
                    {
                        ownCellI = -1;
                        break;
                    }
                }

                if (ownCellI != -1)
                {
                    storage.append(ownCellI);
                }
            }

            if (isInternalFace(faceI))
            {
                label neiCellI = nei[faceI];

                forAll(storage, j)
                {
                    if (storage[j] == neiCellI)
                    {
                        neiCellI = -1;
                        break;
                    }
                }

                if (neiCellI != -1)
                {
                    storage.append(neiCellI);
                }
            }
        }

        return storage;
    }
}


const CML::labelList& CML::primitiveMesh::edgeCells(const label edgeI) const
{
    return edgeCells(edgeI, labels_);
}


// ************************************************************************* //
