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

#include "pointMVCWeight.hpp"
#include "polyMesh.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

int CML::pointMVCWeight::debug
(
    debug::debugSwitch("pointMVCWeight", 0)
);

CML::scalar CML::pointMVCWeight::tol(SMALL);


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

void CML::pointMVCWeight::calcWeights
(
    const Map<label>& toLocal,
    const face& f,
    const DynamicList<point>& u,
    const scalarField& dist,
    scalarField& weights
) const
{
    weights.setSize(toLocal.size());
    weights = 0.0;

    scalarField theta(f.size());

    // recompute theta, the theta computed previously are not robust
    forAll(f, j)
    {
        label jPlus1 = f.fcIndex(j);
        scalar l = mag(u[j] - u[jPlus1]);
        theta[j] = 2.0*CML::asin(l/2.0);
    }

    scalar sumWeight = 0;
    forAll(f, j)
    {
        label pid = toLocal[f[j]];
        label jMin1 = f.rcIndex(j);
        weights[pid] =
            1.0
          / dist[pid]
          * (CML::tan(theta[jMin1]/2.0) + CML::tan(theta[j]/2.0));
        sumWeight += weights[pid];
    }

    if (sumWeight >= tol)
    {
        weights /= sumWeight;
    }
}


void CML::pointMVCWeight::calcWeights
(
    const polyMesh& mesh,
    const labelList& toGlobal,
    const Map<label>& toLocal,
    const vector& position,
    const vectorField& uVec,
    const scalarField& dist,
    scalarField& weights
) const
{
    // Loop over all triangles of all polygons of cell to compute weights
    DynamicList<scalar> alpha(100);
    DynamicList<scalar> theta(100);
    DynamicList<point> u(100);

    const CML::cell& cFaces = mesh.cells()[cellIndex_];

    forAll(cFaces, iter)
    {
        label faceI = cFaces[iter];
        const face& f = mesh.faces()[faceI];

        //Pout<< "face:" << faceI << " at:"
        //    << pointField(mesh.points(), f)
        //    << endl;

        // Collect the uVec for the face
        forAll(f, j)
        {
            u(j) = uVec[toLocal[f[j]]];
        }

        vector v(point::zero);
        forAll(f, j)
        {
            label jPlus1 = f.fcIndex(j);
            //Pout<< "    uj:" << u[j] << " ujPlus1:" << u[jPlus1] << endl;

            vector temp = u[j] ^ u[jPlus1];

            scalar magTemp = mag(temp);

            if (magTemp < VSMALL)
            {
                continue;
            }

            temp /= magTemp;

            //Pout<< "    uj:" << u[j] << " ujPlus1:" << u[jPlus1]
            //    << " temp:" << temp << endl;

            scalar l = min(mag(u[j] - u[jPlus1]), 2.0);
            scalar angle = 2.0*CML::asin(l/2.0);

            //Pout<< "    j:" << j << " l:" << l << " angle:" << angle << endl;

            v += 0.5*angle*temp;
        }

        scalar vNorm = mag(v);
        v /= vNorm;

        // Make sure v points towards the polygon
        //if (((v&u[0]) < 0) != (mesh.faceOwner()[faceI] != cellIndex_))
        //{
        //    FatalErrorInFunction
        //        << "v:" << v << " u[0]:" << u[0]
        //        << exit(FatalError);
        //}

        if ((v & u[0]) < 0)
        {
            v = -v;
        }

        //Pout<< "    v:" << v << endl;

        // angles between edges
        forAll(f, j)
        {
            label jPlus1 = f.fcIndex(j);
            //Pout<< "    uj:" << u[j] << " ujPlus1:" << u[jPlus1] << endl;

            vector n0 = u[j]^v;
            n0 /= mag(n0);
            vector n1 = u[jPlus1]^v;
            n1 /= mag(n1);

            scalar l = min(mag(n0 - n1), 2.0);
            //Pout<< "    l:" << l << endl;
            alpha(j) = 2.0*CML::asin(l/2.0);

            vector temp = n0^n1;
            if ((temp&v) < 0.0)
            {
                alpha[j] = -alpha[j];
            }

            l = min(mag(u[j] - v), 2.0);
            //Pout<< "    l:" << l << endl;
            theta(j) = 2.0*CML::asin(l/2.0);
        }


        bool outlierFlag = false;
        forAll(f, j)
        {
            if (mag(theta[j]) < tol)
            {
                outlierFlag = true;

                label pid = toLocal[f[j]];
                weights[pid] += vNorm / dist[pid];
                break;
            }
        }

        if (outlierFlag)
        {
            continue;
        }

        scalar sum = 0.0;
        forAll(f, j)
        {
            label jMin1 = f.rcIndex(j);
            sum +=
                1.0
              / CML::tan(theta[j])
              * (CML::tan(alpha[j]/2.0) + CML::tan(alpha[jMin1]/2.0));
        }

        // The special case when x lies on the polygon, handle it using 2D mvc.
        // In the 2D case, alpha = theta
        if (mag(sum) < tol)
        {
            // Calculate weights using face vertices only
            calcWeights(toLocal, f, u, dist, weights);
            return;
        }


        // Normal 3D case
        forAll(f, j)
        {
            label pid = toLocal[f[j]];
            label jMin1 = f.rcIndex(j);
            weights[pid] +=
                vNorm
              / sum
              / dist[pid]
              / CML::sin(theta[j])
              * (CML::tan(alpha[j]/2.0) + CML::tan(alpha[jMin1]/2.0));
        }
    }

    // normalise weights
    scalar sumWeight = sum(weights);

    if (mag(sumWeight) < tol)
    {
        return;
    }
    weights /= sumWeight;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::pointMVCWeight::pointMVCWeight
(
    const polyMesh& mesh,
    const vector& position,
    const label cellIndex,
    const label faceIndex
)
:
    cellIndex_((cellIndex != -1) ? cellIndex : mesh.faceOwner()[faceIndex])
{
    // Addressing - face vertices to local points and vice versa
    const labelList& toGlobal = mesh.cellPoints()[cellIndex_];
    Map<label> toLocal(2*toGlobal.size());
    forAll(toGlobal, i)
    {
        toLocal.insert(toGlobal[i], i);
    }


    // Initialise weights
    weights_.setSize(toGlobal.size());
    weights_ = 0.0;


    // Point-to-vertex vectors and distances
    vectorField uVec(toGlobal.size());
    scalarField dist(toGlobal.size());

    forAll(toGlobal, pid)
    {
        const point& pt = mesh.points()[toGlobal[pid]];

        uVec[pid] = pt-position;
        dist[pid] = mag(uVec[pid]);

        // Special case: point is close to vertex
        if (dist[pid] < tol)
        {
            weights_[pid] = 1.0;
            return;
        }
    }

    // Project onto unit sphere
    uVec /= dist;


    if (faceIndex < 0)
    {
        // Face data not supplied
        calcWeights
        (
            mesh,
            toGlobal,
            toLocal,
            position,
            uVec,
            dist,

            weights_
        );
    }
    else
    {
        DynamicList<point> u(100);
        const face& f = mesh.faces()[faceIndex];
        // Collect the uVec for the face
        forAll(f, j)
        {
            u(j) = uVec[toLocal[f[j]]];
        }

        // Calculate weights for face only
        calcWeights(toLocal, f, u, dist, weights_);
    }
}


// ************************************************************************* //
