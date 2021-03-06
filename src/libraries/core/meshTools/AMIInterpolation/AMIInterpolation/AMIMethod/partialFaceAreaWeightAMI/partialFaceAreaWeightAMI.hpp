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
    CML::partialFaceAreaWeightAMI

Description
    Partial face area weighted Arbitrary Mesh Interface (AMI) method


\*---------------------------------------------------------------------------*/

#ifndef partialFaceAreaWeightAMI_H
#define partialFaceAreaWeightAMI_H

#include "faceAreaWeightAMI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                  Class partialFaceAreaWeightAMI Declaration
\*---------------------------------------------------------------------------*/

template<class SourcePatch, class TargetPatch>
class partialFaceAreaWeightAMI
:
    public faceAreaWeightAMI<SourcePatch, TargetPatch>
{

private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        partialFaceAreaWeightAMI(const partialFaceAreaWeightAMI&);

        //- Disallow default bitwise assignment
        void operator=(const partialFaceAreaWeightAMI&);

        // Marching front

            //- Set the source and target seed faces
            virtual void setNextFaces
            (
                label& startSeedI,
                label& srcFaceI,
                label& tgtFaceI,
                const boolList& mapFlag,
                labelList& seedFaces,
                const DynamicList<label>& visitedFaces,
                bool errorOnNotFound = true
            ) const;


public:

    //- Runtime type information
    TypeName("partialFaceAreaWeightAMI");


    // Constructors

        //- Construct from components
        partialFaceAreaWeightAMI
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
    virtual ~partialFaceAreaWeightAMI();


    // Member Functions

        // Access

            //- Flag to indicate that interpolation patches are conformal
            virtual bool conformal() const;


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
void CML::partialFaceAreaWeightAMI<SourcePatch, TargetPatch>::setNextFaces
(
    label& startSeedI,
    label& srcFaceI,
    label& tgtFaceI,
    const boolList& mapFlag,
    labelList& seedFaces,
    const DynamicList<label>& visitedFaces,
    const bool errorOnNotFound
) const
{
    faceAreaWeightAMI<SourcePatch, TargetPatch>::setNextFaces
    (
        startSeedI,
        srcFaceI,
        tgtFaceI,
        mapFlag,
        seedFaces,
        visitedFaces,
        false // no error on not found
    );
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class SourcePatch, class TargetPatch>
CML::partialFaceAreaWeightAMI<SourcePatch, TargetPatch>::
partialFaceAreaWeightAMI
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
    faceAreaWeightAMI<SourcePatch, TargetPatch>
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
CML::partialFaceAreaWeightAMI<SourcePatch, TargetPatch>::
~partialFaceAreaWeightAMI()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class SourcePatch, class TargetPatch>
bool CML::partialFaceAreaWeightAMI<SourcePatch, TargetPatch>::conformal() const
{
    return false;
}


template<class SourcePatch, class TargetPatch>
void CML::partialFaceAreaWeightAMI<SourcePatch, TargetPatch>::calculate
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
    List<DynamicList<scalar> > srcWght(srcAddr.size());
    List<DynamicList<label> > tgtAddr(this->tgtPatch_.size());
    List<DynamicList<scalar> > tgtWght(tgtAddr.size());

    faceAreaWeightAMI<SourcePatch, TargetPatch>::calcAddressing
    (
        srcAddr,
        srcWght,
        tgtAddr,
        tgtWght,
        srcFaceI,
        tgtFaceI
    );

    // transfer data to persistent storage
    forAll(srcAddr, i)
    {
        srcAddress[i].transfer(srcAddr[i]);
        srcWeights[i].transfer(srcWght[i]);
    }
    forAll(tgtAddr, i)
    {
        tgtAddress[i].transfer(tgtAddr[i]);
        tgtWeights[i].transfer(tgtWght[i]);
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
