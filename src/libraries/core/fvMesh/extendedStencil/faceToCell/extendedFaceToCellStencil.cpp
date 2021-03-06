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

#include "extendedFaceToCellStencil.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::extendedFaceToCellStencil::extendedFaceToCellStencil(const polyMesh& mesh)
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
