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

#include "nearWallDist.hpp"
#include "fvMesh.hpp"
#include "cellDistFuncs.hpp"
#include "wallFvPatch.hpp"
#include "surfaceFields.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::nearWallDist::calculate()
{
    cellDistFuncs wallUtils(mesh_);

    // Get patch ids of walls
    labelHashSet wallPatchIDs(wallUtils.getPatchIDs<wallPolyPatch>());

    // Size neighbours array for maximum possible

    labelList neighbours(wallUtils.maxPatchSize(wallPatchIDs));


    // Correct all cells with face on wall

    const volVectorField& cellCentres = mesh_.C();

    forAll(mesh_.boundary(), patchI)
    {
        fvPatchScalarField& ypatch = operator[](patchI);

        const fvPatch& patch = mesh_.boundary()[patchI];

        if (isA<wallFvPatch>(patch))
        {
            const polyPatch& pPatch = patch.patch();

            const labelUList& faceCells = patch.faceCells();

            // Check cells with face on wall
            forAll(patch, patchFaceI)
            {
                label nNeighbours = wallUtils.getPointNeighbours
                (
                    pPatch,
                    patchFaceI,
                    neighbours
                );

                label minFaceI = -1;

                ypatch[patchFaceI] = wallUtils.smallestDist
                (
                    cellCentres[faceCells[patchFaceI]],
                    pPatch,
                    nNeighbours,
                    neighbours,
                    minFaceI
                );
            }
        }
        else
        {
            ypatch = 0.0;
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::nearWallDist::nearWallDist(const CML::fvMesh& mesh)
:
    volScalarField::GeometricBoundaryField
    (
        mesh.boundary(),
        mesh.V(),           // Dummy internal field,
        calculatedFvPatchScalarField::typeName
    ),
    mesh_(mesh)
{
    calculate();
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::nearWallDist::~nearWallDist()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::nearWallDist::correct()
{
    if (mesh_.topoChanging())
    {
        const DimensionedField<scalar, volMesh>& V = mesh_.V();
        const fvBoundaryMesh& bnd = mesh_.boundary();

        this->setSize(bnd.size());
        forAll(*this, patchI)
        {
            this->set
            (
                patchI,
                fvPatchField<scalar>::New
                (
                    calculatedFvPatchScalarField::typeName,
                    bnd[patchI],
                    V
                )
            );
        }
    }

    calculate();
}


// ************************************************************************* //
