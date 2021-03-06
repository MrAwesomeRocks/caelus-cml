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

#include "skewCorrectionVectors.hpp"
#include "surfaceFields.hpp"
#include "volFields.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(skewCorrectionVectors, 0);
}


// * * * * * * * * * * * * * * * * Constructors * * * * * * * * * * * * * * //

CML::skewCorrectionVectors::skewCorrectionVectors(const fvMesh& mesh)
:
    MeshObject<fvMesh, skewCorrectionVectors>(mesh),
    skew_(true),
    skewCorrectionVectors_(nullptr)
{}


CML::skewCorrectionVectors::~skewCorrectionVectors()
{
    deleteDemandDrivenData(skewCorrectionVectors_);
}


void CML::skewCorrectionVectors::makeSkewCorrectionVectors() const
{
    if (debug)
    {
        Info<< "surfaceInterpolation::makeSkewCorrectionVectors() : "
            << "Constructing skew correction vectors"
            << endl;
    }

    skewCorrectionVectors_ = new surfaceVectorField
    (
        IOobject
        (
            "skewCorrectionVectors",
            mesh_.pointsInstance(),
            mesh_,
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            false
        ),
        mesh_,
        dimless
    );
    surfaceVectorField& SkewCorrVecs = *skewCorrectionVectors_;

    // Set local references to mesh data
    const volVectorField& C = mesh_.C();
    const surfaceVectorField& Cf = mesh_.Cf();
    const surfaceVectorField& Sf = mesh_.Sf();

    const labelUList& owner = mesh_.owner();
    const labelUList& neighbour = mesh_.neighbour();

    forAll(owner, facei)
    {
        label own = owner[facei];
        label nei = neighbour[facei];

        vector d = C[nei] - C[own];
        vector Cpf = Cf[facei] - C[own];

        SkewCorrVecs[facei] = Cpf - ((Sf[facei] & Cpf)/(Sf[facei] & d))*d;
    }


    forAll(SkewCorrVecs.boundaryField(), patchI)
    {
        fvsPatchVectorField& patchSkewCorrVecs =
            SkewCorrVecs.boundaryField()[patchI];

        if (!patchSkewCorrVecs.coupled())
        {
            patchSkewCorrVecs = vector::zero;
        }
        else
        {
            const fvPatch& p = patchSkewCorrVecs.patch();
            const labelUList& faceCells = p.faceCells();
            const vectorField& patchFaceCentres = Cf.boundaryField()[patchI];
            const vectorField& patchSf = Sf.boundaryField()[patchI];
            const vectorField patchD(p.delta());

            forAll(p, patchFaceI)
            {
                vector Cpf =
                    patchFaceCentres[patchFaceI] - C[faceCells[patchFaceI]];

                patchSkewCorrVecs[patchFaceI] =
                    Cpf
                  - (
                        (patchSf[patchFaceI] & Cpf)/
                        (patchSf[patchFaceI] & patchD[patchFaceI])
                    )*patchD[patchFaceI];
            }
        }
    }

    scalar skewCoeff = 0.0;

    if (Sf.internalField().size())
    {
        skewCoeff = max(mag(SkewCorrVecs)*mesh_.deltaCoeffs()).value();
    }

    if (debug)
    {
        Info<< "surfaceInterpolation::makeSkewCorrectionVectors() : "
            << "skew coefficient = " << skewCoeff << endl;
    }

    //skewCoeff = 0.0;

    if (skewCoeff < 1e-5)
    {
        skew_ = false;
        deleteDemandDrivenData(skewCorrectionVectors_);
    }
    else
    {
        skew_ = true;
    }

    if (debug)
    {
        Info<< "surfaceInterpolation::makeSkewCorrectionVectors() : "
            << "Finished constructing skew correction vectors"
            << endl;
    }
}


bool CML::skewCorrectionVectors::skew() const
{
    if (skew_ == true && !skewCorrectionVectors_)
    {
        makeSkewCorrectionVectors();
    }

    return skew_;
}


const CML::surfaceVectorField& CML::skewCorrectionVectors::operator()() const
{
    if (!skew())
    {
        FatalErrorInFunction
            << "Cannot return skewCorrectionVectors; mesh is not skewed"
            << abort(FatalError);
    }

    return *skewCorrectionVectors_;
}


// Do what is necessary if the mesh has moved
bool CML::skewCorrectionVectors::movePoints()
{
    skew_ = true;
    deleteDemandDrivenData(skewCorrectionVectors_);

    return true;
}


// ************************************************************************* //
