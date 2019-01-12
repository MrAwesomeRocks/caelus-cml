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

#include "ListOps.hpp"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

const CML::labelList CML::emptyLabelList = CML::labelList(0);


// * * * * * * * * * * * * * * * Global Functions  * * * * * * * * * * * * * //

CML::labelList CML::invert
(
    const label len,
    const labelUList& map
)
{
    labelList inverse(len, -1);

    forAll(map, i)
    {
        label newPos = map[i];

        if (newPos >= 0)
        {
            if (inverse[newPos] >= 0)
            {
                FatalErrorInFunction
                    << "Map is not one-to-one. At index " << i
                    << " element " << newPos << " has already occurred before"
                    << nl << "Please use invertOneToMany instead"
                    << abort(FatalError);
            }

            inverse[newPos] = i;
        }
    }
    return inverse;
}


CML::labelListList CML::invertOneToMany
(
    const label len,
    const labelUList& map
)
{
    labelList nElems(len, 0);

    forAll(map, i)
    {
        if (map[i] >= 0)
        {
            nElems[map[i]]++;
        }
    }

    labelListList inverse(len);

    forAll(nElems, i)
    {
        inverse[i].setSize(nElems[i]);
        nElems[i] = 0;
    }

    forAll(map, i)
    {
        label newI = map[i];

        if (newI >= 0)
        {
            inverse[newI][nElems[newI]++] = i;
        }
    }

    return inverse;
}


CML::labelList CML::identity(const label len)
{
    labelList map(len);

    forAll(map, i)
    {
        map[i] = i;
    }
    return map;
}


// ************************************************************************* //
