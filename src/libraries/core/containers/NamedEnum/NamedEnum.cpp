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

#include "NamedEnum.hpp"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Enum, int nEnum>
CML::NamedEnum<Enum, nEnum>::NamedEnum()
:
    HashTable<int>(2*nEnum)
{
    for (int enumI = 0; enumI < nEnum; ++enumI)
    {
        if (!names[enumI] || names[enumI][0] == '\0')
        {
            stringList goodNames(enumI);

            for (int i = 0; i < enumI; ++i)
            {
                goodNames[i] = names[i];
            }

            FatalErrorInFunction
                << "Illegal enumeration name at position " << enumI << endl
                << "after entries " << goodNames << ".\n"
                << "Possibly your NamedEnum<Enum, nEnum>::names array"
                << " is not of size " << nEnum << endl
                << abort(FatalError);
        }
        insert(names[enumI], enumI);
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Enum, int nEnum>
Enum CML::NamedEnum<Enum, nEnum>::read(Istream& is) const
{
    const word name(is);

    HashTable<int>::const_iterator iter = find(name);

    if (iter == HashTable<int>::end())
    {
        FatalIOErrorInFunction(is)
            << name << " is not in enumeration: "
            << sortedToc() << exit(FatalIOError);
    }

    return Enum(iter());
}


template<class Enum, int nEnum>
void CML::NamedEnum<Enum, nEnum>::write(const Enum e, Ostream& os) const
{
    os  << operator[](e);
}


template<class Enum, int nEnum>
CML::stringList CML::NamedEnum<Enum, nEnum>::strings()
{
    stringList lst(nEnum);

    label nElem = 0;
    for (int enumI = 0; enumI < nEnum; ++enumI)
    {
        if (names[enumI] && names[enumI][0])
        {
            lst[nElem++] = names[enumI];
        }
    }

    lst.setSize(nElem);
    return lst;
}


template<class Enum, int nEnum>
CML::wordList CML::NamedEnum<Enum, nEnum>::words()
{
    wordList lst(nEnum);

    label nElem = 0;
    for (int enumI = 0; enumI < nEnum; ++enumI)
    {
        if (names[enumI] && names[enumI][0])
        {
            lst[nElem++] = names[enumI];
        }
    }

    lst.setSize(nElem);
    return lst;
}


// ************************************************************************* //
