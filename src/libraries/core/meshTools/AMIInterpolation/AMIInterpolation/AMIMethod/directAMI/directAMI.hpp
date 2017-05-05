/*---------------------------------------------------------------------------*\
Copyright (C) 2013-2014 OpenFOAM Foundation
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

Class
    Foam::directAMI

Description
    Direct mapped Arbitrary Mesh Interface (AMI) method


\*---------------------------------------------------------------------------*/

#ifndef directAMI_H
#define directAMI_H

#include "AMIMethod.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                          Class directAMI Declaration
\*---------------------------------------------------------------------------*/

template<class SourcePatch, class TargetPatch>
class directAMI
:
    public AMIMethod<SourcePatch, TargetPatch>
{

private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        directAMI(const directAMI&);

        //- Disallow default bitwise assignment
        void operator=(const directAMI&);

        // Marching front

            //- Append to list of src face seed indices
            void appendToDirectSeeds
            (
                labelList& mapFlag,
                labelList& srcTgtSeed,
                DynamicList<label>& srcSeeds,
                DynamicList<label>& nonOverlapFaces,
                label& srcFaceI,
                label& tgtFaceI
            ) const;

            //- Restart the advancing front - typically happens for
            //  disconnected regions
            void restartAdvancingFront
            (
                labelList& mapFlag,
                DynamicList<label>& nonOverlapFaces,
                label& srcFaceI,
                label& tgtFaceI
            ) const;


        // Evaluation

            //- Area of intersection between source and target faces
            scalar interArea
            (
                const label srcFaceI,
                const label tgtFaceI
            ) const;


public:

    //- Runtime type information
    TypeName("directAMI");


    // Constructors

        //- Construct from components
        directAMI
        (
            const SourcePatch& srcPatch,
            const TargetPatch& tgtPatch,
            const scalarField& srcMagSf,
            const scalarField& tgtMagSf,
            const faceAreaIntersect::triangulationMode& triMode,
            const bool reverseTarget = false,
            const bool requireMatch = true
        );


    //- Destructor
    virtual ~directAMI();


    // Member Functions

        // Manipulation

            //- Update addressing and weights
            virtual void calculate
            (
                labelListList& srcAddress,
                scalarListList& srcWeights,
                labelListList& tgtAddress,
                scalarListList& tgtWeights,
                label srcFaceI = -1,
                label tgtFaceI = -1
            );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class SourcePatch, class TargetPatch>
void CML::directAMI<SourcePatch, TargetPatch>::appendToDirectSeeds
(
    labelList& mapFlag,
    labelList& srcTgtSeed,
    DynamicList<label>& srcSeeds,
    DynamicList<label>& nonOverlapFaces,
    label& srcFaceI,
    label& tgtFaceI
) const
{
    const labelList& srcNbr = this->srcPatch_.faceFaces()[srcFaceI];
    const labelList& tgtNbr = this->tgtPatch_.faceFaces()[tgtFaceI];

    const pointField& srcPoints = this->srcPatch_.points();
    const pointField& tgtPoints = this->tgtPatch_.points();

    const vectorField& srcCf = this->srcPatch_.faceCentres();

    forAll(srcNbr, i)
    {
        label srcI = srcNbr[i];

        if ((mapFlag[srcI] == 0) && (srcTgtSeed[srcI] == -1))
        {
            // first attempt: match by comparing face centres
            const face& srcF = this->srcPatch_[srcI];
            const point& srcC = srcCf[srcI];

            scalar tol = GREAT;
            forAll(srcF, fpI)
            {
                const point& p = srcPoints[srcF[fpI]];
                scalar d2 = magSqr(p - srcC);
                if (d2 < tol)
                {
                    tol = d2;
                }
            }
            tol = max(SMALL, 0.0001*sqrt(tol));

            bool found = false;
            forAll(tgtNbr, j)
            {
                label tgtI = tgtNbr[j];
                const face& tgtF = this->tgtPatch_[tgtI];
                const point tgtC = tgtF.centre(tgtPoints);

                if (mag(srcC - tgtC) < tol)
                {
                    // new match - append to lists
                    found = true;

                    srcTgtSeed[srcI] = tgtI;
                    srcSeeds.append(srcI);

                    break;
                }
            }

            // second attempt: match by shooting a ray into the tgt face
            if (!found)
            {
                const vector srcN = srcF.normal(srcPoints);

                forAll(tgtNbr, j)
                {
                    label tgtI = tgtNbr[j];
                    const face& tgtF = this->tgtPatch_[tgtI];
                    pointHit ray = tgtF.ray(srcCf[srcI], srcN, tgtPoints);

                    if (ray.hit())
                    {
                        // new match - append to lists
                        found = true;

                        srcTgtSeed[srcI] = tgtI;
                        srcSeeds.append(srcI);

                        break;
                    }
                }
            }

            // no match available for source face srcI
            if (!found)
            {
                mapFlag[srcI] = -1;
                nonOverlapFaces.append(srcI);

                if (debug)
                {
                    Pout<< "source face not found: id=" << srcI
                        << " centre=" << srcCf[srcI]
                        << " normal=" << srcF.normal(srcPoints)
                        << " points=" << srcF.points(srcPoints)
                        << endl;

                    Pout<< "target neighbours:" << nl;
                    forAll(tgtNbr, j)
                    {
                        label tgtI = tgtNbr[j];
                        const face& tgtF = this->tgtPatch_[tgtI];

                        Pout<< "face id: " << tgtI
                            << " centre=" << tgtF.centre(tgtPoints)
                            << " normal=" << tgtF.normal(tgtPoints)
                            << " points=" << tgtF.points(tgtPoints)
                            << endl;
                    }
                }
            }
        }
    }

    if (srcSeeds.size())
    {
        srcFaceI = srcSeeds.remove();
        tgtFaceI = srcTgtSeed[srcFaceI];
    }
    else
    {
        srcFaceI = -1;
        tgtFaceI = -1;
    }
}


template<class SourcePatch, class TargetPatch>
void CML::directAMI<SourcePatch, TargetPatch>::restartAdvancingFront
(
    labelList& mapFlag,
    DynamicList<label>& nonOverlapFaces,
    label& srcFaceI,
    label& tgtFaceI
) const
{
    forAll(mapFlag, faceI)
    {
        if (mapFlag[faceI] == 0)
        {
            tgtFaceI = this->findTargetFace(faceI);

            if (tgtFaceI < 0)
            {
                mapFlag[faceI] = -1;
                nonOverlapFaces.append(faceI);
            }
            else
            {
                srcFaceI = faceI;
                break;
            }
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class SourcePatch, class TargetPatch>
CML::directAMI<SourcePatch, TargetPatch>::directAMI
(
    const SourcePatch& srcPatch,
    const TargetPatch& tgtPatch,
    const scalarField& srcMagSf,
    const scalarField& tgtMagSf,
    const faceAreaIntersect::triangulationMode& triMode,
    const bool reverseTarget,
    const bool requireMatch
)
:
    AMIMethod<SourcePatch, TargetPatch>
    (
        srcPatch,
        tgtPatch,
        srcMagSf,
        tgtMagSf,
        triMode,
        reverseTarget,
        requireMatch
    )
{}


// * * * * * * * * * * * * * * * * Destructor * * * * * * * * * * * * * * * //

template<class SourcePatch, class TargetPatch>
CML::directAMI<SourcePatch, TargetPatch>::~directAMI()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class SourcePatch, class TargetPatch>
void CML::directAMI<SourcePatch, TargetPatch>::calculate
(
    labelListList& srcAddress,
    scalarListList& srcWeights,
    labelListList& tgtAddress,
    scalarListList& tgtWeights,
    label srcFaceI,
    label tgtFaceI
)
{
    bool ok =
        this->initialise
        (
            srcAddress,
            srcWeights,
            tgtAddress,
            tgtWeights,
            srcFaceI,
            tgtFaceI
        );

    if (!ok)
    {
        return;
    }


    // temporary storage for addressing and weights
    List<DynamicList<label> > srcAddr(this->srcPatch_.size());
    List<DynamicList<label> > tgtAddr(this->tgtPatch_.size());


    // construct weights and addressing
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // list of faces currently visited for srcFaceI to avoid multiple hits
    DynamicList<label> srcSeeds(10);

    // list to keep track of tgt faces used to seed src faces
    labelList srcTgtSeed(srcAddr.size(), -1);
    srcTgtSeed[srcFaceI] = tgtFaceI;

    // list to keep track of whether src face can be mapped
    // 1 = mapped, 0 = untested, -1 = cannot map
    labelList mapFlag(srcAddr.size(), 0);

    label nTested = 0;
    DynamicList<label> nonOverlapFaces;
    do
    {
        srcAddr[srcFaceI].append(tgtFaceI);
        tgtAddr[tgtFaceI].append(srcFaceI);

        mapFlag[srcFaceI] = 1;

        nTested++;

        // Do advancing front starting from srcFaceI, tgtFaceI
        appendToDirectSeeds
        (
            mapFlag,
            srcTgtSeed,
            srcSeeds,
            nonOverlapFaces,
            srcFaceI,
            tgtFaceI
        );

        if (srcFaceI < 0 && nTested < this->srcPatch_.size())
        {
            restartAdvancingFront(mapFlag, nonOverlapFaces, srcFaceI, tgtFaceI);
        }

    } while (srcFaceI >= 0);

    if (nonOverlapFaces.size() != 0)
    {
        Pout<< "    AMI: " << nonOverlapFaces.size()
            << " non-overlap faces identified"
            << endl;

        this->srcNonOverlap_.transfer(nonOverlapFaces);
    }

    // transfer data to persistent storage
    forAll(srcAddr, i)
    {
        scalar magSf = this->srcMagSf_[i];
//        srcWeights[i] = scalarList(srcAddr[i].size(), magSf);
        srcWeights[i] = scalarList(1, magSf);
        srcAddress[i].transfer(srcAddr[i]);
    }
    forAll(tgtAddr, i)
    {
        scalar magSf = this->tgtMagSf_[i];
//        tgtWeights[i] = scalarList(tgtAddr[i].size(), magSf);
        tgtWeights[i] = scalarList(1, magSf);
        tgtAddress[i].transfer(tgtAddr[i]);
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
