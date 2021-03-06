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

#include "dictionaryEntry.hpp"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::dictionaryEntry::dictionaryEntry
(
    const keyType& key,
    const dictionary& parentDict,
    const dictionary& dict
)
:
    entry(key),
    dictionary(parentDict, dict)
{}


CML::dictionaryEntry::dictionaryEntry
(
    const dictionary& parentDict,
    const dictionaryEntry& dictEnt
)
:
    entry(dictEnt),
    dictionary(parentDict, dictEnt)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::label CML::dictionaryEntry::startLineNumber() const
{
    if (size())
    {
        return first()->startLineNumber();
    }
    else
    {
        return -1;
    }
}

CML::label CML::dictionaryEntry::endLineNumber() const
{
    if (size())
    {
        return last()->endLineNumber();
    }
    else
    {
        return -1;
    }
}


CML::ITstream& CML::dictionaryEntry::stream() const
{
    FatalIOErrorInFunction(*this)
        << "Attempt to return dictionary entry as a primitive"
        << abort(FatalIOError);

    return lookup("");
}


const CML::dictionary& CML::dictionaryEntry::dict() const
{
    return *this;
}


CML::dictionary& CML::dictionaryEntry::dict()
{
    return *this;
}


// ************************************************************************* //
