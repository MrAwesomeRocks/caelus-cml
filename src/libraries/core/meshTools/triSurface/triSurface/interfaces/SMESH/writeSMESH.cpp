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

#include "triSurface.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void triSurface::writeSMESH(const bool writeSorted, Ostream& os) const
{
    const pointField& ps = points();

    // Write header
    os  << "# tetgen .smesh file" << endl
        << ps.size() << " 3" << endl;   // 3 dimensions

    // Write vertex coords
    forAll(ps, pointi)
    {
        os  << pointi << ' '
            << ps[pointi].x() << ' '
            << ps[pointi].y() << ' '
            << ps[pointi].z() << endl;
    }

    if (writeSorted)
    {
        labelList faceMap;

        surfacePatchList myPatches(calcPatches(faceMap));

        os  << size() << " 1" << endl;   // 1 attribute: region number

        label faceIndex = 0;

        forAll(myPatches, patchI)
        {
            // Print all faces belonging to this patch

            for
            (
                label patchFaceI = 0;
                patchFaceI < myPatches[patchI].size();
                patchFaceI++
            )
            {
                const label faceI = faceMap[faceIndex++];

                os  << "3 " // triangles
                    << operator[](faceI)[0] << ' '
                    << operator[](faceI)[1] << ' '
                    << operator[](faceI)[2] << ' '
                    << operator[](faceI).region()   // region number
                    << endl;
            }
        }

        os  << '0' << endl      // holes
            << '0' << endl;     // regions
    }
    else
    {
        os  << size() << " 1" << endl;   // 1 attribute: region number

        forAll(*this, faceI)
        {
            os  << "3 "
                << operator[](faceI)[0] << ' '
                << operator[](faceI)[1] << ' '
                << operator[](faceI)[2] << ' '
                << operator[](faceI).region()       // region number
                << endl;
        }

        os  << '0' << endl      // holes
            << '0' << endl;     // regions
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
