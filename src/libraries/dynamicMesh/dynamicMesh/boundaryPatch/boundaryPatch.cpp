/*---------------------------------------------------------------------------*\
Copyright (C) 2011 OpenFOAM Foundation
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

#include "boundaryPatch.hpp"
#include "dictionary.hpp"
#include "Ostream.hpp"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::boundaryPatch::boundaryPatch
(
    const word& name,
    const label index,
    const label size,
    const label start,
    const word& physicalType
)
:
    patchIdentifier(name, index, physicalType),
    size_(size),
    start_(start)
{}


CML::boundaryPatch::boundaryPatch
(
    const word& name,
    const dictionary& dict,
    const label index
)
:
    patchIdentifier(name, dict, index),
    size_(readLabel(dict.lookup("nFaces"))),
    start_(readLabel(dict.lookup("startFace")))
{}


CML::boundaryPatch::boundaryPatch(const boundaryPatch& p)
:
    patchIdentifier(p.name(), p.index(), p.physicalType()),
    size_(p.size()),
    start_(p.start())
{}


CML::boundaryPatch::boundaryPatch(const boundaryPatch& p, const label index)
:
    patchIdentifier(p.name(), index, p.physicalType()),
    size_(p.size()),
    start_(p.start())
{}


CML::autoPtr<CML::boundaryPatch> CML::boundaryPatch::clone() const
{
    return autoPtr<boundaryPatch>(new boundaryPatch(*this));
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::boundaryPatch::~boundaryPatch()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::boundaryPatch::write(Ostream& os) const
{
    patchIdentifier::write(os);
    os.writeKeyword("nFaces") << size_ << token::END_STATEMENT << nl;
    os.writeKeyword("startFace") << start_ << token::END_STATEMENT << nl;
}


// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //

CML::Ostream& CML::operator<<(Ostream& os, const boundaryPatch& p)
{
    p.write(os);
    os.check("Ostream& operator<<(Ostream& f, const boundaryPatch&)");
    return os;
}


// ************************************************************************* //
