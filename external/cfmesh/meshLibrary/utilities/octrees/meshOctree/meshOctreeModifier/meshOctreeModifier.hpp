/*---------------------------------------------------------------------------*\
Copyright (C) Creative Fields, Ltd.
-------------------------------------------------------------------------------
License
    This file is part of cfMesh.

    cfMesh is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 3 of the License, or (at your
    option) any later version.

    cfMesh is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with cfMesh.  If not, see <http://www.gnu.org/licenses/>.

Class
    meshOctreeModifier

Description
    Octree for mesh generation

Author: Franjo Juretic (franjo.juretic@c-fields.com)

SourceFiles
    meshOctreeModifier.cpp

\*---------------------------------------------------------------------------*/

#ifndef meshOctreeModifier_HPP
#define meshOctreeModifier_HPP

#include "meshOctree.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                    Class meshOctreeModifier Declaration
\*---------------------------------------------------------------------------*/

class meshOctreeModifier
{
    // Private data
        //- Reference to the octree
        meshOctree& octree_;

    // Private member functions
        //- correct refinement such that it produces 1-irregular octree
        void ensureCorrectRegularity(labelList& refineBox);

        //- add additional refinement with the following rule. If a son
        //- of a father box is refined, then refine all other sons
        bool ensureCorrectRegularitySons(labelList& refineBox);

    // Private copy constructor
        //- Disallow default bitwise copy construct
        meshOctreeModifier(const meshOctreeModifier&);

        //- Disallow default bitwise assignment
        void operator=(const meshOctreeModifier&);

public:

    // Constructors

        //- Construct from octree
        meshOctreeModifier(meshOctree&);

    // Destructor

        ~meshOctreeModifier();


    // Member Functions
        //- return octree
        inline const meshOctree& octree() const;

        //- return neighbour processors
        inline labelList& neiProcsAccess();

        //- return rootBox
        inline boundBox& rootBoxAccess();

        //- return isRootInitialised_
        inline bool& isRootInitialisedAccess();

        //- return search range
        inline scalar& searchRangeAccess();

        //- return the range of leaves at each processor
        inline List<Pair<meshOctreeCubeCoordinates> >& neiRangeAccess();

        //- return initial cube
        inline meshOctreeCube& initialCubeAccess();

        //- return octree slots
        inline List<meshOctreeSlot>& dataSlotsAccess();

        //- return the pointer to the meshOctreeCube at the given position
        inline meshOctreeCube* findCubeForPosition
        (
            const meshOctreeCubeCoordinates&
        ) const;

        //- find leaves contained in a given boundBox
        inline void findLeavesContainedInBox
        (
            const boundBox&,
            DynList<const meshOctreeCube*, 256>&
        ) const;

        //- return leaves
        inline LongList<meshOctreeCube*>& leavesAccess();

        //- create leaves
        inline void createListOfLeaves();

        //- mark additional layers around the leaves selected for refinement
        void markAdditionalLayers
        (
            labelList& refineBox,
            const label nLayers = 1
        ) const;

        //- mark additional layers around the leaves selected for refinement
        void markAdditionalLayersOfFaceNeighbours
        (
            labelList& refineBox,
            const label nLayers = 1
        ) const;

        //- mark additional layers around the leaves selected for refinement
        //- given on a box-by-box basis
        //- returns the number of boxes selected for refinement
        label markAdditionalLayers
        (
            labelList& refineBox,
            labelList& nLayers,
            List<direction>& targetRefLevel
        ) const;

        //- refine leaves marked for refinement
        //- hexRefinement is activated when it is required to refine all
        //- sons of the same father, if a single son gets marked for refinement
        void refineSelectedBoxes
        (
            labelList& refineBox,
            const bool hexRefinement = false
        );

        //- refine selected boxes and the boxes within the given range
        void refineSelectedBoxesAndAdditionalLayers
        (
            labelList& refineBox,
            const scalarList& refThickness
        );

    // functions for parallel runs
        //- distribute leaves of the initial octree to processors
        //- each processor creates a list of neighbouring processors
        void distributeLeavesToProcessors();

        //- move octree cubes from one processor to another
        void loadDistribution(const direction usedType = 0);

        //- refine the tree to add cubes transferred from other processors
        void refineTreeForCoordinates
        (
            const meshOctreeCubeCoordinates&,
            const short procNo = Pstream::myProcNo(),
            const direction cubeType = meshOctreeCubeBasic::UNKNOWN
        );

        void refineTreeForCoordinates
        (
            const meshOctreeCubeCoordinates&,
            const labelList& containedTriangles,
            const labelList& containedEdges,
            const short procNo = Pstream::myProcNo(),
            const direction cubeType = meshOctreeCubeBasic::UNKNOWN

        );

        //- update the communication pattern between the processors
        //- this function is used for load balancing
        void updateCommunicationPattern();

        //- add an additional layer of boxes which belongs to the neighbouring
        //- processors. This simplifies the process of extracting the mesh
        //- template from the octree in a parallel run
        void addLayerFromNeighbouringProcessors();

        //- reduce memory consumption by deleting unnecessary data
        void reduceMemoryConsumption();
};


inline const meshOctree& meshOctreeModifier::octree() const
{
    return octree_;
}

inline labelList& meshOctreeModifier::neiProcsAccess()
{
    return octree_.neiProcs_;
}

inline boundBox& meshOctreeModifier::rootBoxAccess()
{
    return octree_.rootBox_;
}

inline bool& meshOctreeModifier::isRootInitialisedAccess()
{
    return octree_.isRootInitialised_;
}

inline scalar& meshOctreeModifier::searchRangeAccess()
{
    return octree_.searchRange_;
}

inline List<Pair<meshOctreeCubeCoordinates> >&
meshOctreeModifier::neiRangeAccess()
{
    return octree_.neiRange_;
}

inline meshOctreeCube& meshOctreeModifier::initialCubeAccess()
{
    return *octree_.initialCubePtr_;
}

inline List<meshOctreeSlot>& meshOctreeModifier::dataSlotsAccess()
{
    return octree_.dataSlots_;
}

inline meshOctreeCube* meshOctreeModifier::findCubeForPosition
(
    const meshOctreeCubeCoordinates& cc
) const
{
    return octree_.findCubeForPosition(cc);
}

inline void meshOctreeModifier::findLeavesContainedInBox
(
    const boundBox& bb,
    DynList<const meshOctreeCube*, 256>& containedLeaves
) const
{
    containedLeaves.clear();
    octree_.findLeavesContainedInBox(bb, containedLeaves);
}

inline LongList<meshOctreeCube*>& meshOctreeModifier::leavesAccess()
{
    return octree_.leaves_;
}

inline void meshOctreeModifier::createListOfLeaves()
{
    octree_.leaves_.clear();

    octree_.initialCubePtr_->findLeaves(octree_.leaves_);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
