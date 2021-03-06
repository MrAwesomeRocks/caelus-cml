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

#include "faceZoneSet.hpp"
#include "mapPolyMesh.hpp"
#include "polyMesh.hpp"

#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(faceZoneSet, 0);

addToRunTimeSelectionTable(topoSet, faceZoneSet, word);
addToRunTimeSelectionTable(topoSet, faceZoneSet, size);
addToRunTimeSelectionTable(topoSet, faceZoneSet, set);


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void faceZoneSet::updateSet()
{
    labelList order;
    sortedOrder(addressing_, order);
    addressing_ = UIndirectList<label>(addressing_, order)();
    flipMap_ = UIndirectList<bool>(flipMap_, order)();

    faceSet::clearStorage();
    faceSet::resize(2*addressing_.size());
    forAll(addressing_, i)
    {
        faceSet::insert(addressing_[i]);
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

faceZoneSet::faceZoneSet
(
    const polyMesh& mesh,
    const word& name,
    readOption r,
    writeOption w
)
:
    faceSet(mesh, name, 1000),  // do not read faceSet
    mesh_(mesh),
    addressing_(0),
    flipMap_(0)
{
    const faceZoneMesh& faceZones = mesh.faceZones();
    label zoneID = faceZones.findZoneID(name);

    if
    (
        (r == IOobject::MUST_READ)
     || (r == IOobject::MUST_READ_IF_MODIFIED)
     || (r == IOobject::READ_IF_PRESENT && zoneID != -1)
    )
    {
        const faceZone& fz = faceZones[zoneID];
        addressing_ = fz;
        flipMap_ = fz.flipMap();
    }

    updateSet();

    check(mesh.nFaces());
}


faceZoneSet::faceZoneSet
(
    const polyMesh& mesh,
    const word& name,
    const label size,
    writeOption w
)
:
    faceSet(mesh, name, size, w),
    mesh_(mesh),
    addressing_(0),
    flipMap_(0)
{
    updateSet();
}


faceZoneSet::faceZoneSet
(
    const polyMesh& mesh,
    const word& name,
    const topoSet& set,
    writeOption w
)
:
    faceSet(mesh, name, set.size(), w),
    mesh_(mesh),
    addressing_(refCast<const faceZoneSet>(set).addressing()),
    flipMap_(refCast<const faceZoneSet>(set).flipMap())
{
    updateSet();
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

faceZoneSet::~faceZoneSet()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void faceZoneSet::invert(const label maxLen)
{
    // Count
    label n = 0;

    for (label faceI = 0; faceI < maxLen; faceI++)
    {
        if (!found(faceI))
        {
            n++;
        }
    }

    // Fill
    addressing_.setSize(n);
    flipMap_.setSize(n);
    n = 0;

    for (label faceI = 0; faceI < maxLen; faceI++)
    {
        if (!found(faceI))
        {
            addressing_[n] = faceI;
            flipMap_[n] = false;         //? or true?
            n++;
        }
    }
    updateSet();
}


void faceZoneSet::subset(const topoSet& set)
{
    label nConflict = 0;

    DynamicList<label> newAddressing(addressing_.size());
    DynamicList<bool> newFlipMap(flipMap_.size());

    Map<label> faceToIndex(addressing_.size());
    forAll(addressing_, i)
    {
        faceToIndex.insert(addressing_[i], i);
    }

    const faceZoneSet& fSet = refCast<const faceZoneSet>(set);

    forAll(fSet.addressing(), i)
    {
        label faceI = fSet.addressing()[i];

        Map<label>::const_iterator iter = faceToIndex.find(faceI);

        if (iter != faceToIndex.end())
        {
            label index = iter();

            if (fSet.flipMap()[i] != flipMap_[index])
            {
                nConflict++;
            }
            newAddressing.append(faceI);
            newFlipMap.append(flipMap_[index]);
        }
    }

    if (nConflict > 0)
    {
        WarningInFunction
            << "subset : there are " << nConflict
            << " faces with different orientation in faceZonesSets "
            << name() << " and " << set.name() << endl;
    }

    addressing_.transfer(newAddressing);
    flipMap_.transfer(newFlipMap);
    updateSet();
}


void faceZoneSet::addSet(const topoSet& set)
{
    label nConflict = 0;

    DynamicList<label> newAddressing(addressing_);
    DynamicList<bool> newFlipMap(flipMap_);

    Map<label> faceToIndex(addressing_.size());
    forAll(addressing_, i)
    {
        faceToIndex.insert(addressing_[i], i);
    }

    const faceZoneSet& fSet = refCast<const faceZoneSet>(set);

    forAll(fSet.addressing(), i)
    {
        label faceI = fSet.addressing()[i];

        Map<label>::const_iterator iter = faceToIndex.find(faceI);

        if (iter != faceToIndex.end())
        {
            label index = iter();

            if (fSet.flipMap()[i] != flipMap_[index])
            {
                nConflict++;
            }
        }
        else
        {
            newAddressing.append(faceI);
            newFlipMap.append(fSet.flipMap()[i]);
        }
    }

    if (nConflict > 0)
    {
        WarningInFunction
            << "addSet : there are " << nConflict
            << " faces with different orientation in faceZonesSets "
            << name() << " and " << set.name() << endl;
    }

    addressing_.transfer(newAddressing);
    flipMap_.transfer(newFlipMap);
    updateSet();
}


void faceZoneSet::deleteSet(const topoSet& set)
{
    label nConflict = 0;

    DynamicList<label> newAddressing(addressing_.size());
    DynamicList<bool> newFlipMap(flipMap_.size());

    const faceZoneSet& fSet = refCast<const faceZoneSet>(set);

    Map<label> faceToIndex(fSet.addressing().size());
    forAll(fSet.addressing(), i)
    {
        faceToIndex.insert(fSet.addressing()[i], i);
    }

    forAll(addressing_, i)
    {
        label faceI = addressing_[i];

        Map<label>::const_iterator iter = faceToIndex.find(faceI);

        if (iter != faceToIndex.end())
        {
            label index = iter();

            if (fSet.flipMap()[index] != flipMap_[i])
            {
                nConflict++;
            }
        }
        else
        {
            // Not found in fSet so add
            newAddressing.append(faceI);
            newFlipMap.append(fSet.flipMap()[i]);
        }
    }

    if (nConflict > 0)
    {
        WarningInFunction
            << "deleteSet : there are " << nConflict
            << " faces with different orientation in faceZonesSets "
            << name() << " and " << set.name() << endl;
    }

    addressing_.transfer(newAddressing);
    flipMap_.transfer(newFlipMap);
    updateSet();
}


void faceZoneSet::sync(const polyMesh& mesh)
{}


label faceZoneSet::maxSize(const polyMesh& mesh) const
{
    return mesh.nFaces();
}


//- Write using given format, version and compression
bool faceZoneSet::writeObject
(
    IOstream::streamFormat s,
    IOstream::versionNumber v,
    IOstream::compressionType c
) const
{
    // Write shadow faceSet
    word oldTypeName = typeName;
    const_cast<word&>(type()) = faceSet::typeName;
    bool ok = faceSet::writeObject(s, v, c);
    const_cast<word&>(type()) = oldTypeName;

    // Modify faceZone
    faceZoneMesh& faceZones = const_cast<polyMesh&>(mesh_).faceZones();
    label zoneID = faceZones.findZoneID(name());

    if (zoneID == -1)
    {
        zoneID = faceZones.size();

        faceZones.setSize(zoneID+1);
        faceZones.set
        (
            zoneID,
            new faceZone
            (
                name(),
                addressing_,
                flipMap_,
                zoneID,
                faceZones
            )
        );
    }
    else
    {
        faceZones[zoneID].resetAddressing(addressing_, flipMap_);
    }
    faceZones.clearAddressing();

    return ok && faceZones.write();
}


void faceZoneSet::updateMesh(const mapPolyMesh& morphMap)
{
    // faceZone
    labelList newAddressing(addressing_.size());
    boolList newFlipMap(flipMap_.size());

    label n = 0;
    forAll(addressing_, i)
    {
        label faceI = addressing_[i];
        label newFaceI = morphMap.reverseFaceMap()[faceI];
        if (newFaceI >= 0)
        {
            newAddressing[n] = newFaceI;
            newFlipMap[n] = flipMap_[i];
            n++;
        }
    }
    newAddressing.setSize(n);
    newFlipMap.setSize(n);

    addressing_.transfer(newAddressing);
    flipMap_.transfer(newFlipMap);

    updateSet();
}


void faceZoneSet::writeDebug
(
    Ostream& os,
    const primitiveMesh& mesh,
    const label maxLen
) const
{
    faceSet::writeDebug(os, mesh, maxLen);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
