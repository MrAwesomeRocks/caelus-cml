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

Class
    CML::domainDecomposition

Description
    Automatic domain decomposition class for finite-volume meshes

SourceFiles
    domainDecomposition.cpp
    decomposeMesh.cpp

\*---------------------------------------------------------------------------*/

#ifndef domainDecomposition_H
#define domainDecomposition_H

#include "fvMesh.hpp"
#include "labelList.hpp"
#include "SLList.hpp"
#include "PtrList.hpp"
#include "point.hpp"
#include "Time.hpp"
#include "volFields.hpp"


namespace CML
{

/*---------------------------------------------------------------------------*\
                     Class domainDecomposition Declaration
\*---------------------------------------------------------------------------*/

class domainDecomposition
:
    public fvMesh
{
    // Private data

        //- Optional: points at the facesInstance
        autoPtr<pointIOField> facesInstancePointsPtr_;

        //- Mesh decomposition control dictionary
        IOdictionary decompositionDict_;

        //- Number of processors in decomposition
        label nProcs_;

        //- Is the decomposition data to be distributed for each processor
        bool distributed_;

        //- Processor label for each cell
        labelList cellToProc_;

        //- Labels of points for each processor
        labelListList procPointAddressing_;

        //- Labels of faces for each processor
        // Note: Face turning index is stored as the sign on addressing
        // Only the processor boundary faces are affected: if the sign of the
        // index is negative, the processor face is the reverse of the
        // original face. In order to do this properly, all face
        // indices will be incremented by 1 and the decremented as
        // necessary to avoid the problem of face number zero having no
        // sign.
        List<DynamicList<label> > procFaceAddressing_;

        //- Labels of cells for each processor
        labelListList procCellAddressing_;

        //- Sizes for processor mesh patches
        // Excludes inter-processor boundaries
        labelListList procPatchSize_;

        //- Start indices for processor patches
        // Excludes inter-processor boundaries
        labelListList procPatchStartIndex_;


        // Per inter-processor patch information

        //- Neighbour processor ID for inter-processor boundaries
        labelListList procNeighbourProcessors_;

        //- Sizes for inter-processor patches
        labelListList procProcessorPatchSize_;

        //- Start indices (in procFaceAddressing_) for inter-processor patches
        labelListList procProcessorPatchStartIndex_;

        //- Sub patch IDs for inter-processor patches
        List<labelListList> procProcessorPatchSubPatchIDs_;

        //- Sub patch sizes for inter-processor patches
        List<labelListList> procProcessorPatchSubPatchStarts_;

    // Private Member Functions

        void distributeCells();

        //- Mark all elements with value or -2 if occur twice
        static void mark
        (
            const labelList& zoneElems,
            const label zoneI,
            labelList& elementToZone
        );

        //- Append single element to list
        static void append(labelList&, const label);

        //- Add face to inter-processor patch
        void addInterProcFace
        (
            const label facei,
            const label ownerProc,
            const label nbrProc,

            List<Map<label> >&,
            List<DynamicList<DynamicList<label> > >&
        ) const;

        //- Generate sub patch info for processor cyclics
        template <class BinaryOp>
        void processInterCyclics
        (
            const polyBoundaryMesh& patches,
            List<DynamicList<DynamicList<label> > >& interPatchFaces,
            List<Map<label> >& procNbrToInterPatch,
            List<labelListList>& subPatchIDs,
            List<labelListList>& subPatchStarts,
            bool owner,
            BinaryOp bop
        ) const;


public:

    // Constructors

        //- Construct from IOobject
        domainDecomposition(const IOobject& io);


    //- Destructor
    ~domainDecomposition();


    // Member Functions

        //- Number of processor in decomposition
        label nProcs() const
        {
            return nProcs_;
        }

        //- Is the decomposition data to be distributed for each processor
        bool distributed() const
        {
            return distributed_;
        }

        //- Decompose mesh.
        void decomposeMesh();

        //- Write decomposition
        bool writeDecomposition();

        //- Cell-processor decomposition labels
        const labelList& cellToProc() const
        {
            return cellToProc_;
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "cyclicPolyPatch.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template <class BinaryOp>
void CML::domainDecomposition::processInterCyclics
(
    const polyBoundaryMesh& patches,
    List<DynamicList<DynamicList<label> > >& interPatchFaces,
    List<Map<label> >& procNbrToInterPatch,
    List<labelListList>& subPatchIDs,
    List<labelListList>& subPatchStarts,
    bool owner,
    BinaryOp bop
) const
{
    // Processor boundaries from split cyclics
    forAll(patches, patchi)
    {
        if (isA<cyclicPolyPatch>(patches[patchi]))
        {
            const cyclicPolyPatch& pp = refCast<const cyclicPolyPatch>
            (
                patches[patchi]
            );

            if (pp.owner() != owner)
            {
                continue;
            }

            // cyclic: check opposite side on this processor
            const labelUList& patchFaceCells = pp.faceCells();
            const labelUList& nbrPatchFaceCells =
                pp.neighbPatch().faceCells();

            // Store old sizes. Used to detect which inter-proc patches
            // have been added to.
            labelListList oldInterfaceSizes(nProcs_);
            forAll(oldInterfaceSizes, procI)
            {
                labelList& curOldSizes = oldInterfaceSizes[procI];

                curOldSizes.setSize(interPatchFaces[procI].size());
                forAll(curOldSizes, interI)
                {
                    curOldSizes[interI] =
                        interPatchFaces[procI][interI].size();
                }
            }

            // Add faces with different owner and neighbour processors
            forAll(patchFaceCells, facei)
            {
                const label ownerProc = cellToProc_[patchFaceCells[facei]];
                const label nbrProc = cellToProc_[nbrPatchFaceCells[facei]];
                if (bop(ownerProc, nbrProc))
                {
                    // inter - processor patch face found.
                    addInterProcFace
                    (
                        pp.start()+facei,
                        ownerProc,
                        nbrProc,
                        procNbrToInterPatch,
                        interPatchFaces
                    );
                }
            }

            // 1. Check if any faces added to existing interfaces
            forAll(oldInterfaceSizes, procI)
            {
                const labelList& curOldSizes = oldInterfaceSizes[procI];

                forAll(curOldSizes, interI)
                {
                    label oldSz = curOldSizes[interI];
                    if (interPatchFaces[procI][interI].size() > oldSz)
                    {
                        // Added faces to this interface. Add an entry
                        append(subPatchIDs[procI][interI], patchi);
                        append(subPatchStarts[procI][interI], oldSz);
                    }
                }
            }

            // 2. Any new interfaces
            forAll(subPatchIDs, procI)
            {
                label nIntfcs = interPatchFaces[procI].size();
                subPatchIDs[procI].setSize(nIntfcs, labelList(1, patchi));
                subPatchStarts[procI].setSize(nIntfcs, labelList(1, label(0)));
            }
        }
    }
}





#endif

// ************************************************************************* //
