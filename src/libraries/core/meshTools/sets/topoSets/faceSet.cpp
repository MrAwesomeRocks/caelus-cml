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

#include "faceSet.hpp"
#include "mapPolyMesh.hpp"
#include "polyMesh.hpp"
#include "syncTools.hpp"

#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(faceSet, 0);

addToRunTimeSelectionTable(topoSet, faceSet, word);
addToRunTimeSelectionTable(topoSet, faceSet, size);
addToRunTimeSelectionTable(topoSet, faceSet, set);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

faceSet::faceSet(const IOobject& obj)
:
    topoSet(obj, typeName)
{}


faceSet::faceSet
(
    const polyMesh& mesh,
    const word& name,
    readOption r,
    writeOption w
)
:
    topoSet(mesh, typeName, name, r, w)
{
    check(mesh.nFaces());
}


faceSet::faceSet
(
    const polyMesh& mesh,
    const word& name,
    const label size,
    writeOption w
)
:
    topoSet(mesh, name, size, w)
{}


faceSet::faceSet
(
    const polyMesh& mesh,
    const word& name,
    const topoSet& set,
    writeOption w
)
:
    topoSet(mesh, name, set, w)
{}


faceSet::faceSet
(
    const polyMesh& mesh,
    const word& name,
    const labelHashSet& set,
    writeOption w
)
:
    topoSet(mesh, name, set, w)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

faceSet::~faceSet()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void faceSet::sync(const polyMesh& mesh)
{
    boolList set(mesh.nFaces(), false);

    forAllConstIter(faceSet, *this, iter)
    {
        set[iter.key()] = true;
    }
    syncTools::syncFaceList(mesh, set, orEqOp<bool>());

    label nAdded = 0;

    forAll(set, faceI)
    {
        if (set[faceI])
        {
            if (insert(faceI))
            {
                nAdded++;
            }
        }
        else if (found(faceI))
        {
            FatalErrorInFunction
                << "Problem : syncing removed faces from set."
                << abort(FatalError);
        }
    }

    reduce(nAdded, sumOp<label>());
    if (debug && nAdded > 0)
    {
        Info<< "Added an additional " << nAdded
            << " faces on coupled patches. "
            << "(processorPolyPatch, cyclicPolyPatch)" << endl;
    }
}


label faceSet::maxSize(const polyMesh& mesh) const
{
    return mesh.nFaces();
}


void faceSet::updateMesh(const mapPolyMesh& morphMap)
{
    updateLabels(morphMap.reverseFaceMap());
}


void faceSet::writeDebug
(
    Ostream& os,
    const primitiveMesh& mesh,
    const label maxLen
) const
{
    topoSet::writeDebug(os, mesh.faceCentres(), maxLen);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
