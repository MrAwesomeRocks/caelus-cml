/*---------------------------------------------------------------------------*\
Copyright (C) 2012-2016 OpenFOAM Foundation
Copyright (C) 2017 OpenCFD Ltd.
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Caelus is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "primitiveMeshTools.hpp"
#include "syncTools.hpp"
#include "pyramidPointFaceRef.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

CML::scalar CML::primitiveMeshTools::faceSkewness
(
    const primitiveMesh& mesh,
    const pointField& p,
    const vectorField& fCtrs,
    const vectorField& fAreas,

    const label facei,
    const point& ownCc,
    const point& neiCc
)
{
    vector Cpf = fCtrs[facei] - ownCc;
    vector d = neiCc - ownCc;

    // Skewness vector
    vector sv =
        Cpf
      - ((fAreas[facei] & Cpf)/((fAreas[facei] & d) + ROOTVSMALL))*d;
    vector svHat = sv/(mag(sv) + ROOTVSMALL);

    // Normalisation distance calculated as the approximate distance
    // from the face centre to the edge of the face in the direction
    // of the skewness
    scalar fd = 0.2*mag(d) + ROOTVSMALL;
    const face& f = mesh.faces()[facei];
    forAll(f, pi)
    {
        fd = max(fd, mag(svHat & (p[f[pi]] - fCtrs[facei])));
    }

    // Normalised skewness
    return mag(sv)/fd;
}


CML::scalar CML::primitiveMeshTools::boundaryFaceSkewness
(
    const primitiveMesh& mesh,
    const pointField& p,
    const vectorField& fCtrs,
    const vectorField& fAreas,

    const label facei,
    const point& ownCc
)
{
    vector Cpf = fCtrs[facei] - ownCc;

    vector normal = fAreas[facei];
    normal /= mag(normal) + ROOTVSMALL;
    vector d = normal*(normal & Cpf);


    // Skewness vector
    vector sv =
        Cpf
      - ((fAreas[facei] & Cpf)/((fAreas[facei] & d) + ROOTVSMALL))*d;
    vector svHat = sv/(mag(sv) + ROOTVSMALL);

    // Normalisation distance calculated as the approximate distance
    // from the face centre to the edge of the face in the direction
    // of the skewness
    scalar fd = 0.4*mag(d) + ROOTVSMALL;
    const face& f = mesh.faces()[facei];
    forAll(f, pi)
    {
        fd = max(fd, mag(svHat & (p[f[pi]] - fCtrs[facei])));
    }

    // Normalised skewness
    return mag(sv)/fd;
}


CML::scalar CML::primitiveMeshTools::faceOrthogonality
(
    const point& ownCc,
    const point& neiCc,
    const vector& s
)
{
    vector d = neiCc - ownCc;

    return (d & s)/(mag(d)*mag(s) + ROOTVSMALL);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::tmp<CML::scalarField> CML::primitiveMeshTools::faceOrthogonality
(
    const primitiveMesh& mesh,
    const vectorField& areas,
    const vectorField& cc
)
{
    const labelList& own = mesh.faceOwner();
    const labelList& nei = mesh.faceNeighbour();

    tmp<scalarField> tortho(new scalarField(mesh.nInternalFaces()));
    scalarField& ortho = tortho();

    // Internal faces
    forAll(nei, facei)
    {
        ortho[facei] = faceOrthogonality
        (
            cc[own[facei]],
            cc[nei[facei]],
            areas[facei]
        );
    }

    return tortho;
}


CML::tmp<CML::scalarField> CML::primitiveMeshTools::faceSkewness
(
    const primitiveMesh& mesh,
    const pointField& p,
    const vectorField& fCtrs,
    const vectorField& fAreas,
    const vectorField& cellCtrs
)
{
    const labelList& own = mesh.faceOwner();
    const labelList& nei = mesh.faceNeighbour();

    tmp<scalarField> tskew(new scalarField(mesh.nFaces()));
    scalarField& skew = tskew();

    forAll(nei, facei)
    {
        skew[facei] = faceSkewness
        (
            mesh,
            p,
            fCtrs,
            fAreas,

            facei,
            cellCtrs[own[facei]],
            cellCtrs[nei[facei]]
        );
    }


    // Boundary faces: consider them to have only skewness error.
    // (i.e. treat as if mirror cell on other side)

    for (label facei = mesh.nInternalFaces(); facei < mesh.nFaces(); facei++)
    {
        skew[facei] = boundaryFaceSkewness
        (
            mesh,
            p,
            fCtrs,
            fAreas,
            facei,
            cellCtrs[own[facei]]
        );
    }

    return tskew;
}


void CML::primitiveMeshTools::facePyramidVolume
(
    const primitiveMesh& mesh,
    const pointField& points,
    const vectorField& ctrs,

    scalarField& ownPyrVol,
    scalarField& neiPyrVol
)
{
    const labelList& own = mesh.faceOwner();
    const labelList& nei = mesh.faceNeighbour();
    const faceList& f = mesh.faces();

    ownPyrVol.setSize(mesh.nFaces());
    neiPyrVol.setSize(mesh.nInternalFaces());

    forAll(f, facei)
    {
        // Create the owner pyramid
        ownPyrVol[facei] = -pyramidPointFaceRef
        (
            f[facei],
            ctrs[own[facei]]
        ).mag(points);

        if (mesh.isInternalFace(facei))
        {
            // Create the neighbour pyramid - it will have positive volume
            neiPyrVol[facei] = pyramidPointFaceRef
            (
                f[facei],
                ctrs[nei[facei]]
            ).mag(points);
        }
    }
}


void CML::primitiveMeshTools::cellClosedness
(
    const primitiveMesh& mesh,
    const Vector<label>& meshD,
    const vectorField& areas,
    const scalarField& vols,

    scalarField& openness,
    scalarField& aratio
)
{
    const labelList& own = mesh.faceOwner();
    const labelList& nei = mesh.faceNeighbour();

    // Loop through cell faces and sum up the face area vectors for each cell.
    // This should be zero in all vector components

    vectorField sumClosed(mesh.nCells(), Zero);
    vectorField sumMagClosed(mesh.nCells(), Zero);

    forAll(own, facei)
    {
        // Add to owner
        sumClosed[own[facei]] += areas[facei];
        sumMagClosed[own[facei]] += cmptMag(areas[facei]);
    }

    forAll(nei, facei)
    {
        // Subtract from neighbour
        sumClosed[nei[facei]] -= areas[facei];
        sumMagClosed[nei[facei]] += cmptMag(areas[facei]);
    }


    label nDims = 0;
    for (direction dir = 0; dir < vector::nComponents; dir++)
    {
        if (meshD[dir] == 1)
        {
            nDims++;
        }
    }


    // Check the sums
    openness.setSize(mesh.nCells());
    aratio.setSize(mesh.nCells());

    forAll(sumClosed, celli)
    {
        scalar maxOpenness = 0;

        for (direction cmpt=0; cmpt<vector::nComponents; cmpt++)
        {
            maxOpenness = max
            (
                maxOpenness,
                mag(sumClosed[celli][cmpt])
               /(sumMagClosed[celli][cmpt] + ROOTVSMALL)
            );
        }
        openness[celli] = maxOpenness;

        // Calculate the aspect ration as the maximum of Cartesian component
        // aspect ratio to the total area hydraulic area aspect ratio
        scalar minCmpt = VGREAT;
        scalar maxCmpt = -VGREAT;
        for (direction dir = 0; dir < vector::nComponents; dir++)
        {
            if (meshD[dir] == 1)
            {
                minCmpt = min(minCmpt, sumMagClosed[celli][dir]);
                maxCmpt = max(maxCmpt, sumMagClosed[celli][dir]);
            }
        }

        scalar aspectRatio = maxCmpt/(minCmpt + ROOTVSMALL);
        if (nDims == 3)
        {
            scalar v = max(ROOTVSMALL, vols[celli]);

            aspectRatio = max
            (
                aspectRatio,
                1.0/6.0*cmptSum(sumMagClosed[celli])/pow(v, 2.0/3.0)
            );
        }

        aratio[celli] = aspectRatio;
    }
}


CML::tmp<CML::scalarField> CML::primitiveMeshTools::faceConcavity
(
    const scalar maxSin,
    const primitiveMesh& mesh,
    const pointField& p,
    const vectorField& faceAreas
)
{
    const faceList& fcs = mesh.faces();

    vectorField faceNormals(faceAreas);
    faceNormals /= mag(faceNormals) + ROOTVSMALL;

    tmp<scalarField> tfaceAngles(new scalarField(mesh.nFaces()));
    scalarField& faceAngles = tfaceAngles();


    forAll(fcs, facei)
    {
        const face& f = fcs[facei];

        // Get edge from f[0] to f[size-1];
        vector ePrev(p[f.first()] - p[f.last()]);
        scalar magEPrev = mag(ePrev);
        ePrev /= magEPrev + ROOTVSMALL;

        scalar maxEdgeSin = 0.0;

        forAll(f, fp0)
        {
            // Get vertex after fp
            label fp1 = f.fcIndex(fp0);

            // Normalized vector between two consecutive points
            vector e10(p[f[fp1]] - p[f[fp0]]);
            scalar magE10 = mag(e10);
            e10 /= magE10 + ROOTVSMALL;

            if (magEPrev > SMALL && magE10 > SMALL)
            {
                vector edgeNormal = ePrev ^ e10;
                scalar magEdgeNormal = mag(edgeNormal);

                if (magEdgeNormal < maxSin)
                {
                    // Edges (almost) aligned -> face is ok.
                }
                else
                {
                    // Check normal
                    edgeNormal /= magEdgeNormal;

                    if ((edgeNormal & faceNormals[facei]) < SMALL)
                    {
                        maxEdgeSin = max(maxEdgeSin, magEdgeNormal);
                    }
                }
            }

            ePrev = e10;
            magEPrev = magE10;
        }

        faceAngles[facei] = maxEdgeSin;
    }

    return tfaceAngles;
}


CML::tmp<CML::scalarField> CML::primitiveMeshTools::faceFlatness
(
    const primitiveMesh& mesh,
    const pointField& p,
    const vectorField& fCtrs,
    const vectorField& faceAreas
)
{
    const faceList& fcs = mesh.faces();

    // Areas are calculated as the sum of areas. (see
    // primitiveMeshFaceCentresAndAreas.C)
    scalarField magAreas(mag(faceAreas));

    tmp<scalarField> tfaceFlatness(new scalarField(mesh.nFaces(), 1.0));
    scalarField& faceFlatness = tfaceFlatness();


    forAll(fcs, facei)
    {
        const face& f = fcs[facei];

        if (f.size() > 3 && magAreas[facei] > ROOTVSMALL)
        {
            const point& fc = fCtrs[facei];

            // Calculate the sum of magnitude of areas and compare to magnitude
            // of sum of areas.

            scalar sumA = 0.0;

            forAll(f, fp)
            {
                const point& thisPoint = p[f[fp]];
                const point& nextPoint = p[f.nextLabel(fp)];

                // Triangle around fc.
                vector n = 0.5*((nextPoint - thisPoint)^(fc - thisPoint));
                sumA += mag(n);
            }

            faceFlatness[facei] = magAreas[facei]/(sumA + ROOTVSMALL);
        }
    }

    return tfaceFlatness;
}


CML::tmp<CML::scalarField> CML::primitiveMeshTools::cellDeterminant
(
    const primitiveMesh& mesh,
    const Vector<label>& meshD,
    const vectorField& faceAreas,
    const PackedBoolList& internalOrCoupledFace
)
{
    // Determine number of dimensions and (for 2D) missing dimension
    label nDims = 0;
    label twoD = -1;
    for (direction dir = 0; dir < vector::nComponents; dir++)
    {
        if (meshD[dir] == 1)
        {
            nDims++;
        }
        else
        {
            twoD = dir;
        }
    }

    tmp<scalarField> tcellDeterminant(new scalarField(mesh.nCells()));
    scalarField& cellDeterminant = tcellDeterminant();

    const cellList& c = mesh.cells();

    if (nDims == 1)
    {
        cellDeterminant = 1.0;
    }
    else
    {
        forAll(c, celli)
        {
            const labelList& curFaces = c[celli];

            // Calculate local normalization factor
            scalar avgArea = 0;

            label nInternalFaces = 0;

            forAll(curFaces, i)
            {
                if (internalOrCoupledFace[curFaces[i]])
                {
                    avgArea += mag(faceAreas[curFaces[i]]);

                    nInternalFaces++;
                }
            }

            if (nInternalFaces == 0)
            {
                cellDeterminant[celli] = 0;
            }
            else
            {
                avgArea /= nInternalFaces;

                symmTensor areaTensor(Zero);

                forAll(curFaces, i)
                {
                    if (internalOrCoupledFace[curFaces[i]])
                    {
                        areaTensor += sqr(faceAreas[curFaces[i]]/avgArea);
                    }
                }

                if (nDims == 2)
                {
                    // Add the missing eigenvector (such that it does not
                    // affect the determinant)
                    if (twoD == 0)
                    {
                        areaTensor.xx() = 1;
                    }
                    else if (twoD == 1)
                    {
                        areaTensor.yy() = 1;
                    }
                    else
                    {
                        areaTensor.zz() = 1;
                    }
                }

                // Note:
                // - normalise to be 0..1 (since cube has eigenvalues 2 2 2)
                // - we use the determinant (i.e. 3rd invariant) and not e.g.
                //   condition number (= max ev / min ev) since we are
                //   interested in the minimum connectivity and not the
                //   uniformity. Using the condition number on corner cells
                //   leads to uniformity 1 i.e. equally bad in all three
                //   directions which is not what we want.
                cellDeterminant[celli] = mag(det(areaTensor))/8.0;
            }
        }
    }

    return tcellDeterminant;
}


// ************************************************************************* //
