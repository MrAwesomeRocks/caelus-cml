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

#include "topoSet.hpp"
#include "mapPolyMesh.hpp"
#include "polyMesh.hpp"
#include "boundBox.hpp"
#include "Time.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(topoSet, 0);
    defineRunTimeSelectionTable(topoSet, word);
    defineRunTimeSelectionTable(topoSet, size);
    defineRunTimeSelectionTable(topoSet, set);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

CML::autoPtr<CML::topoSet> CML::topoSet::New
(
    const word& setType,
    const polyMesh& mesh,
    const word& name,
    readOption r,
    writeOption w
)
{
    wordConstructorTable::iterator cstrIter =
        wordConstructorTablePtr_->find(setType);

    if (cstrIter == wordConstructorTablePtr_->end())
    {
        FatalErrorInFunction
            << "Unknown set type " << setType
            << endl << endl
            << "Valid set types : " << endl
            << wordConstructorTablePtr_->sortedToc()
            << exit(FatalError);
    }

    return autoPtr<topoSet>(cstrIter()(mesh, name, r, w));
}


CML::autoPtr<CML::topoSet> CML::topoSet::New
(
    const word& setType,
    const polyMesh& mesh,
    const word& name,
    const label size,
    writeOption w
)
{
    sizeConstructorTable::iterator cstrIter =
        sizeConstructorTablePtr_->find(setType);

    if (cstrIter == sizeConstructorTablePtr_->end())
    {
        FatalErrorInFunction
            << "Unknown set type " << setType
            << endl << endl
            << "Valid set types : " << endl
            << sizeConstructorTablePtr_->sortedToc()
            << exit(FatalError);
    }

    return autoPtr<topoSet>(cstrIter()(mesh, name, size, w));
}


CML::autoPtr<CML::topoSet> CML::topoSet::New
(
    const word& setType,
    const polyMesh& mesh,
    const word& name,
    const topoSet& set,
    writeOption w
)
{
    setConstructorTable::iterator cstrIter =
        setConstructorTablePtr_->find(setType);

    if (cstrIter == setConstructorTablePtr_->end())
    {
        FatalErrorInFunction
            << "Unknown set type " << setType
            << endl << endl
            << "Valid set types : " << endl
            << setConstructorTablePtr_->sortedToc()
            << exit(FatalError);
    }

    return autoPtr<topoSet>(cstrIter()(mesh, name, set, w));
}

CML::fileName CML::topoSet::topoSet::localPath
(
    const polyMesh& mesh,
    const word& name
)
{
    return mesh.facesInstance()/mesh.dbDir()/polyMesh::meshSubDir/"sets"/name;
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// Update stored cell numbers using map.
// Do in two passes to prevent allocation if nothing changed.
void CML::topoSet::topoSet::updateLabels(const labelList& map)
{
    // Iterate over map to see if anything changed
    bool changed = false;

    forAllConstIter(labelHashSet, *this, iter)
    {
        if ((iter.key() < 0) || (iter.key() > map.size()))
        {
            FatalErrorInFunction
                << "Illegal content " << iter.key() << " of set:" << name()
                << " of type " << type() << endl
                << "Value should be between 0 and " << map.size()-1
                << abort(FatalError);
        }

        const label newCellI = map[iter.key()];

        if (newCellI != iter.key())
        {
            changed = true;

            break;
        }
    }

    // Relabel (use second Map to prevent overlapping)
    if (changed)
    {
        labelHashSet newSet(2*size());

        forAllConstIter(labelHashSet, *this, iter)
        {
            const label newCellI = map[iter.key()];

            if (newCellI >= 0)
            {
                newSet.insert(newCellI);
            }
        }

        transfer(newSet);
    }
}

void CML::topoSet::topoSet::check(const label maxLabel)
{
    forAllConstIter(topoSet, *this, iter)
    {
        if ((iter.key() < 0) || (iter.key() > maxLabel))
        {
            FatalErrorInFunction
                << "Illegal content " << iter.key() << " of set:" << name()
                << " of type " << type() << endl
                << "Value should be between 0 and " << maxLabel
                << abort(FatalError);
        }
    }
}


// Write maxElem elements, starting at iter. Updates iter and elemI.
void CML::topoSet::writeDebug
(
    Ostream& os,
    const label maxElem,
    topoSet::const_iterator& iter,
    label& elemI
) const
{
    label n = 0;

    for (; (iter != end()) && (n < maxElem); ++iter)
    {
        if ((n != 0) && ((n % 10) == 0))
        {
            os << endl;
        }
        os << iter.key() << ' ';

        n++;
        elemI++;
    }
}


// Write maxElem elements, starting at iter. Updates iter and elemI.
void CML::topoSet::writeDebug
(
    Ostream& os,
    const pointField& coords,
    const label maxElem,
    topoSet::const_iterator& iter,
    label& elemI
) const
{
    label n = 0;

    for (; (iter != end()) && (n < maxElem); ++iter)
    {
        if ((n != 0) && ((n % 3) == 0))
        {
            os << endl;
        }
        os << iter.key() << coords[iter.key()] << ' ';

        n++;
        elemI++;
    }
}


void CML::topoSet::writeDebug
(
    Ostream& os,
    const pointField& coords,
    const label maxLen
) const
{
    // Bounding box of contents.
    boundBox bb(pointField(coords, toc()), true);

    os  << "Set bounding box: min = "
        << bb.min() << "    max = " << bb.max() << " meters. " << endl << endl;

    label n = 0;

    topoSet::const_iterator iter = begin();

    if (size() <= maxLen)
    {
        writeDebug(os, coords, maxLen, iter, n);
    }
    else
    {
        label halfLen = maxLen/2;

        os  << "Size larger than " << maxLen << ". Printing first and last "
            << halfLen << " elements:" << endl << endl;

        writeDebug(os, coords, halfLen, iter, n);

        os  << nl << "  .." << nl << endl;

        for (; n < size() - halfLen; ++n)
        {
            ++iter;
        }

        writeDebug(os, coords, halfLen, iter, n);
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::topoSet::topoSet(const IOobject& obj, const word& wantedType)
:
    regIOobject(obj)
{
    if
    (
        readOpt() == IOobject::MUST_READ
     || readOpt() == IOobject::MUST_READ_IF_MODIFIED
     || (
            readOpt() == IOobject::READ_IF_PRESENT
         && headerOk()
        )
    )
    {
        if (readStream(wantedType).good())
        {
            readStream(wantedType) >> static_cast<labelHashSet&>(*this);

            close();
        }
    }
}


CML::topoSet::topoSet
(
    const polyMesh& mesh,
    const word& wantedType,
    const word& name,
    readOption r,
    writeOption w
)
:
    regIOobject
    (
        IOobject
        (
            name,
            mesh.time().findInstance
            (
                mesh.dbDir()/polyMesh::meshSubDir/"sets",
                word::null,
                IOobject::MUST_READ,
                mesh.facesInstance()
            ),
            polyMesh::meshSubDir/"sets",
            mesh,
            r,
            w
        )
    )
{
    if
    (
        readOpt() == IOobject::MUST_READ
     || readOpt() == IOobject::MUST_READ_IF_MODIFIED
     || (
            readOpt() == IOobject::READ_IF_PRESENT
         && headerOk()
        )
    )
    {
        if (readStream(wantedType).good())
        {
            readStream(wantedType) >> static_cast<labelHashSet&>(*this);

            close();
        }
    }
}


CML::topoSet::topoSet
(
    const polyMesh& mesh,
    const word& name,
    const label size,
    writeOption w
)
:
    regIOobject
    (
        IOobject
        (
            name,
            mesh.time().findInstance
            (
                mesh.dbDir()/polyMesh::meshSubDir/"sets",
                word::null,
                IOobject::NO_READ,
                mesh.facesInstance()
            ),
            polyMesh::meshSubDir/"sets",
            mesh,
            IOobject::NO_READ,
            w
        )
    ),
    labelHashSet(size)
{}


CML::topoSet::topoSet
(
    const polyMesh& mesh,
    const word& name,
    const labelHashSet& set,
    writeOption w
)
:
    regIOobject
    (
        IOobject
        (
            name,
            mesh.time().findInstance
            (
                mesh.dbDir()/polyMesh::meshSubDir/"sets",
                word::null,
                IOobject::NO_READ,
                mesh.facesInstance()
            ),
            polyMesh::meshSubDir/"sets",
            mesh,
            IOobject::NO_READ,
            w
        )
    ),
    labelHashSet(set)
{}


CML::topoSet::topoSet(const IOobject& obj, const label size)
:
    regIOobject(obj),
    labelHashSet(size)
{}


CML::topoSet::topoSet(const IOobject& obj, const labelHashSet& set)
:
    regIOobject(obj),
    labelHashSet(set)
{}



// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::topoSet::~topoSet()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::topoSet::invert(const label maxLen)
{
    // Keep copy of current set.
    labelHashSet currentSet(*this);

    clear();
    resize(2*(maxLen - currentSet.size()));

    for (label cellI = 0; cellI < maxLen; cellI++)
    {
        if (!currentSet.found(cellI))
        {
            insert(cellI);
        }
    }

}


void CML::topoSet::subset(const topoSet& set)
{
    // Keep copy of current set.
    labelHashSet currentSet(*this);

    clear();
    resize(2*min(currentSet.size(), set.size()));

    forAllConstIter(labelHashSet, currentSet, iter)
    {
        if (set.found(iter.key()))
        {
            // element present in both currentSet and set.
            insert(iter.key());
        }
    }
}


void CML::topoSet::addSet(const topoSet& set)
{
    forAllConstIter(topoSet, set, iter)
    {
        insert(iter.key());
    }
}


void CML::topoSet::deleteSet(const topoSet& set)
{
    forAllConstIter(topoSet, set, iter)
    {
        erase(iter.key());
    }
}


void CML::topoSet::sync(const polyMesh&)
{
    NotImplemented;
}


void CML::topoSet::writeDebug(Ostream& os, const label maxLen) const
{
    label n = 0;

    topoSet::const_iterator iter = begin();

    if (size() <= maxLen)
    {
        writeDebug(os, maxLen, iter, n);
    }
    else
    {
        label halfLen = maxLen/2;

        os  << "Size larger than " << maxLen << ". Printing first and last "
            << halfLen << " elements:" << endl << endl;

        writeDebug(os, halfLen, iter, n);

        os  << nl << "  .." << nl << endl;

        for (; n < size() - halfLen; ++n)
        {
            ++iter;
        }

        writeDebug(os, halfLen, iter, n);
    }
}


//void CML::topoSet::writeDebug
//(
//    Ostream&,
//    const primitiveMesh&,
//    const label
//) const
//{
//    NotImplemented;
//}


bool CML::topoSet::writeData(Ostream& os) const
{
    return (os << *this).good();
}


void CML::topoSet::updateMesh(const mapPolyMesh&)
{
    NotImplemented;
}


////- Return max index+1.
//label topoSet::maxSize(const polyMesh&) const
//{
//    NotImplemented;
//
//    return -1;
//}

// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

void CML::topoSet::operator=(const topoSet& rhs)
{
    labelHashSet::operator=(rhs);
}


// ************************************************************************* //
