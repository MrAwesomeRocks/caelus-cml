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

#include "extendedCellToFaceStencil.hpp"
#include "globalIndex.hpp"
#include "syncTools.hpp"
#include "SortableList.hpp"

/* * * * * * * * * * * * * * * Static Member Data  * * * * * * * * * * * * * */


namespace CML
{
    defineTypeNameAndDebug(extendedCellToFaceStencil, 0);
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::extendedCellToFaceStencil::writeStencilStats
(
    Ostream& os,
    const labelListList& stencil,
    const mapDistribute& map
)
{
    label sumSize = 0;
    label nSum = 0;
    label minSize = labelMax;
    label maxSize = labelMin;

    forAll(stencil, i)
    {
        const labelList& sCells = stencil[i];

        if (sCells.size() > 0)
        {
            sumSize += sCells.size();
            nSum++;
            minSize = min(minSize, sCells.size());
            maxSize = max(maxSize, sCells.size());
        }
    }
    reduce(sumSize, sumOp<label>());
    reduce(nSum, sumOp<label>());

    reduce(minSize, minOp<label>());
    reduce(maxSize, maxOp<label>());

    os  << "Stencil size :" << nl
        << "    average : " << scalar(sumSize)/nSum << nl
        << "    min     : " << minSize << nl
        << "    max     : " << maxSize << nl
        << endl;

    // Sum all sent data
    label nSent = 0;
    label nLocal = 0;
    forAll(map.subMap(), procI)
    {
        if (procI != Pstream::myProcNo())
        {
            nSent += map.subMap()[procI].size();
        }
        else
        {
            nLocal += map.subMap()[procI].size();
        }
    }

    os  << "Local data size : " << returnReduce(nLocal, sumOp<label>()) << nl
        << "Sent data size  : " << returnReduce(nSent, sumOp<label>()) << nl
        << endl;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::extendedCellToFaceStencil::extendedCellToFaceStencil(const polyMesh& mesh)
:
    mesh_(mesh)
{
    // Check for transformation - not supported.
    const polyBoundaryMesh& patches = mesh.boundaryMesh();

    forAll(patches, patchI)
    {
        if (patches[patchI].coupled())
        {
            const coupledPolyPatch& cpp =
                refCast<const coupledPolyPatch>(patches[patchI]);

            if (!cpp.parallel() || cpp.separated())
            {
                FatalErrorInFunction
                    << "Coupled patches with transformations not supported."
                    << endl
                    << "Problematic patch " << cpp.name() << exit(FatalError);
            }
        }
    }
}


// ************************************************************************* //
