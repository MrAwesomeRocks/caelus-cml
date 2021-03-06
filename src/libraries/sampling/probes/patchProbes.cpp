/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2018 OpenFOAM Foundation
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

#include "patchProbes.hpp"
#include "volFields.hpp"
#include "IOmanip.hpp"
// For 'nearInfo' helper class only
#include "mappedPatchBase.hpp"
#include "treeBoundBox.hpp"
#include "treeDataFace.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(patchProbes, 0);
}

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::patchProbes::findElements(const fvMesh& mesh)
{
    (void)mesh.tetBasePtIs();

    const polyBoundaryMesh& bm = mesh.boundaryMesh();

    label patchi = bm.findPatchID(patchName_);

    if (patchi == -1)
    {
        FatalErrorInFunction
            << " Unknown patch name "
            << patchName_ << endl
            << exit(FatalError);
    }

     // All the info for nearest. Construct to miss
    List<mappedPatchBase::nearInfo> nearest(this->size());

    const polyPatch& pp = bm[patchi];

    if (pp.size() > 0)
    {
        labelList bndFaces(pp.size());
        forAll(bndFaces, i)
        {
            bndFaces[i] =  pp.start() + i;
        }

        treeBoundBox overallBb(pp.points());
        overallBb = overallBb.extend(1e-4);

        const indexedOctree<treeDataFace> boundaryTree
        (
            treeDataFace    // all information needed to search faces
            (
                false,                      // do not cache bb
                mesh,
                bndFaces                    // patch faces only
            ),
            overallBb,                      // overall search domain
            8,                              // maxLevel
            10,                             // leafsize
            3.0                             // duplicity
        );


        forAll(probeLocations(), probei)
        {
            const point sample = probeLocations()[probei];

            scalar span = boundaryTree.bb().mag();

            pointIndexHit info = boundaryTree.findNearest
            (
                sample,
                CML::sqr(span)
            );

            if (!info.hit())
            {
                info = boundaryTree.findNearest
                (
                    sample,
                    CML::sqr(GREAT)
                );
            }

            label facei = boundaryTree.shapes().faceLabels()[info.index()];

            const label patchi = bm.whichPatch(facei);

            if (isA<emptyPolyPatch>(bm[patchi]))
            {
                WarningInFunction
                << " The sample point: " << sample
                << " belongs to " << patchi
                << " which is an empty patch. This is not permitted. "
                << " This sample will not be included "
                << endl;
            }
            else
            {
                const point& fc = mesh.faceCentres()[facei];

                mappedPatchBase::nearInfo sampleInfo;

                sampleInfo.first() = pointIndexHit
                (
                    true,
                    fc,
                    facei
                );

                sampleInfo.second().first() = magSqr(fc-sample);
                sampleInfo.second().second() = Pstream::myProcNo();

                nearest[probei]= sampleInfo;
            }
        }
    }


    // Find nearest.
    Pstream::listCombineGather(nearest, mappedPatchBase::nearestEqOp());
    Pstream::listCombineScatter(nearest);

    if (debug)
    {
        Info<< "patchProbes::findElements" << " : " << endl;
        forAll(nearest, sampleI)
        {
            label proci = nearest[sampleI].second().second();
            label localI = nearest[sampleI].first().index();

            Info<< "    " << sampleI << " coord:"<< operator[](sampleI)
                << " found on processor:" << proci
                << " in local cell/face:" << localI
                << " with fc:" << nearest[sampleI].first().rawPoint() << endl;
        }
    }


    // Extract any local faces to sample
    elementList_.setSize(nearest.size(), -1);

    forAll(nearest, sampleI)
    {
        if (nearest[sampleI].second().second() == Pstream::myProcNo())
        {
            // Store the face to sample
            elementList_[sampleI] = nearest[sampleI].first().index();
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::patchProbes::patchProbes
(
    const word& name,
    const objectRegistry& obr,
    const dictionary& dict,
    const bool loadFromFiles
)
:
    probes(name, obr, dict, loadFromFiles)
{
    // When constructing probes above it will have called the
    // probes::findElements (since the virtual mechanism not yet operating).
    // Not easy to workaround (apart from feeding through flag into constructor)
    // so clear out any cells found for now.
    elementList_.clear();
    faceList_.clear();

    read(dict);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::patchProbes::~patchProbes()
{}


void CML::patchProbes::write()
{
    if (this->size() && prepare())
    {
        sampleAndWrite(scalarFields_);
        sampleAndWrite(vectorFields_);
        sampleAndWrite(sphericalTensorFields_);
        sampleAndWrite(symmTensorFields_);
        sampleAndWrite(tensorFields_);

        sampleAndWriteSurfaceFields(surfaceScalarFields_);
        sampleAndWriteSurfaceFields(surfaceVectorFields_);
        sampleAndWriteSurfaceFields(surfaceSphericalTensorFields_);
        sampleAndWriteSurfaceFields(surfaceSymmTensorFields_);
        sampleAndWriteSurfaceFields(surfaceTensorFields_);
    }
}

void CML::patchProbes::read(const dictionary& dict)
{
    dict.lookup("patchName") >> patchName_;
    probes::read(dict);
}


// ************************************************************************* //
