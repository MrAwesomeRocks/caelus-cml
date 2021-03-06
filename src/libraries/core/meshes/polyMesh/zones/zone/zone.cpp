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

#include "zone.hpp"
#include "IOstream.hpp"
#include "demandDrivenData.hpp"
#include "HashSet.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


namespace CML
{
    defineTypeNameAndDebug(zone, 0);
}


// * * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * //

const CML::Map<CML::label>& CML::zone::lookupMap() const
{
    if (!lookupMapPtr_)
    {
        calcLookupMap();
    }

    return *lookupMapPtr_;
}


void CML::zone::calcLookupMap() const
{
    if (debug)
    {
        Info<< "void zone::calcLookupMap() const: "
            << "Calculating lookup map"
            << endl;
    }

    if (lookupMapPtr_)
    {
        FatalErrorInFunction
            << "Lookup map already calculated" << nl
            << abort(FatalError);
    }

    const labelList& addr = *this;

    lookupMapPtr_ = new Map<label>(2*addr.size());
    Map<label>& lm = *lookupMapPtr_;

    forAll(addr, i)
    {
        lm.insert(addr[i], i);
    }

    if (debug)
    {
        Info<< "void zone::calcLookupMap() const: "
            << "Finished calculating lookup map"
            << endl;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::zone::zone
(
    const word& name,
    const labelUList& addr,
    const label index
)
:
    labelList(addr),
    name_(name),
    index_(index),
    lookupMapPtr_(nullptr)
{}


CML::zone::zone
(
    const word& name,
    const Xfer<labelList>& addr,
    const label index
)
:
    labelList(addr),
    name_(name),
    index_(index),
    lookupMapPtr_(nullptr)
{}


CML::zone::zone
(
    const word& name,
    const dictionary& dict,
    const word& labelsName,
    const label index
)
:
    labelList(dict.lookup(labelsName)),
    name_(name),
    index_(index),
    lookupMapPtr_(nullptr)
{}


CML::zone::zone
(
    const zone& z,
    const labelUList& addr,
    const label index
)
:
    labelList(addr),
    name_(z.name()),
    index_(index),
    lookupMapPtr_(nullptr)
{}


CML::zone::zone
(
    const zone& z,
    const Xfer<labelList>& addr,
    const label index
)
:
    labelList(addr),
    name_(z.name()),
    index_(index),
    lookupMapPtr_(nullptr)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::zone::~zone()
{
    clearAddressing();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::label CML::zone::localID(const label globalCellID) const
{
    const Map<label>& lm = lookupMap();

    Map<label>::const_iterator lmIter = lm.find(globalCellID);

    if (lmIter == lm.end())
    {
        return -1;
    }
    else
    {
        return lmIter();
    }
}


void CML::zone::clearAddressing()
{
    deleteDemandDrivenData(lookupMapPtr_);
}


bool CML::zone::checkDefinition(const label maxSize, const bool report) const
{
    const labelList& addr = *this;

    bool hasError = false;

    // To check for duplicate entries
    labelHashSet elems(size());

    forAll(addr, i)
    {
        if (addr[i] < 0 || addr[i] >= maxSize)
        {
            hasError = true;

            if (report)
            {
                SeriousErrorInFunction
                    << "Zone " << name_
                    << " contains invalid index label " << addr[i] << nl
                    << "Valid index labels are 0.."
                    << maxSize-1 << endl;
            }
            else
            {
                // w/o report - can stop checking now
                break;
            }
        }
        else if (!elems.insert(addr[i]))
        {
            if (report)
            {
                WarningInFunction
                    << "Zone " << name_
                    << " contains duplicate index label " << addr[i] << endl;
            }
        }
    }

    return hasError;
}


void CML::zone::write(Ostream& os) const
{
    os  << nl << name_
        << nl << static_cast<const labelList&>(*this);
}


// * * * * * * * * * * * * * * * Ostream Operator  * * * * * * * * * * * * * //

CML::Ostream& CML::operator<<(Ostream& os, const zone& z)
{
    z.write(os);
    os.check("Ostream& operator<<(Ostream& f, const zone& z");
    return os;
}


// ************************************************************************* //
