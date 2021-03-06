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
    CML::mapPatchChange

Description
    Class containing mesh-to-mesh mapping information after a patch change
    operation.

SourceFiles

\*---------------------------------------------------------------------------*/

#ifndef mapPatchChange_H
#define mapPatchChange_H

#include "labelList.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class mapPatchChange Declaration
\*---------------------------------------------------------------------------*/

class mapPatchChange
{
    // Private data

        //- Old patches
        const label nOldPatches_;

        //- Patch mapping array
        const labelList patchMap_;

public:

    // Constructors

        //- Construct from components
        mapPatchChange(const label nOldPatches, const labelList& patchMap)
        :
            nOldPatches_(nOldPatches),
            patchMap_(patchMap)
        {}


    // Member Functions

        // Access

            //- Number of old patches
            label nOldPatches() const
            {
                return nOldPatches_;
            }

            //- Patch map. Size of current patches.
            //  -1  : patch was added
            //  >=0 : old position of patch
            //  any original patch which is not in the list has been deleted
            const labelList& patchMap() const
            {
                return patchMap_;
            }


        // Utility functions

            //- labels of added patches
            labelList addedPatches() const
            {
                labelList added(patchMap_.size());

                label addedI = 0;

                forAll(patchMap_, patchI)
                {
                    if (patchMap_[patchI] == -1)
                    {
                        added[addedI++] = patchI;
                    }
                }
                added.setSize(addedI);
                return added;
            }

            //- labels (on old mesh) of deleted patches
            labelList deletedPatches() const
            {
                labelList oldToNew(nOldPatches_, -1);

                // Mark all preserved patches
                forAll(patchMap_, patchI)
                {
                    if (patchMap_[patchI] != -1)
                    {
                        oldToNew[patchMap_[patchI]] = patchI;
                    }
                }

                // Extract -1 elements from oldToNew. These are the deleted
                // patches.
                label deletedI = 0;

                forAll(oldToNew, oldPatchI)
                {
                    if (oldToNew[oldPatchI] == -1)
                    {
                        oldToNew[deletedI++] = oldPatchI;
                    }
                }

                oldToNew.setSize(deletedI);

                return oldToNew;
            }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
