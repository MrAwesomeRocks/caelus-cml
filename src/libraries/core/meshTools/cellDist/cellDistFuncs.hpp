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
    CML::cellDistFuncs

Description
    Collection of functions used in wall distance calculation.

SourceFiles
    cellDistFuncs.cpp
    cellDistFuncsTemplates.cpp

\*---------------------------------------------------------------------------*/

#ifndef cellDistFuncs_H
#define cellDistFuncs_H

#include "scalarField.hpp"
#include "HashSet.hpp"
#include "Map.hpp"
#include "wordReList.hpp"
#include "scalarField.hpp"
#include "point.hpp"
#include "primitivePatch.hpp"
#include "className.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes
class polyMesh;
class polyPatch;
class polyBoundaryMesh;

/*---------------------------------------------------------------------------*\
                           Class cellDistFuncs Declaration
\*---------------------------------------------------------------------------*/

class cellDistFuncs
{
    // Private Member Data

        //- Reference to mesh
        const polyMesh& mesh_;


    // Private Member Functions

        //- Search for element in first n elements of labelList. Return index
        //  or -1.
        static label findIndex(const label n, const labelList&, const label);


        //- Disallow default bitwise copy construct
        cellDistFuncs(const cellDistFuncs&);

        //- Disallow default bitwise assignment
        void operator=(const cellDistFuncs&);

public:

    ClassName("cellDistFuncs");

    // Constructors

        //- Construct from mesh
        cellDistFuncs(const polyMesh& mesh);


    // Member Functions

        //- Access mesh
        const polyMesh& mesh() const
        {
            return mesh_;
        }

        //- Return the set of patch IDs corresponding to the given names
        labelHashSet getPatchIDs(const wordReList& patchNames) const;

        //- Get patchIDs of/derived off certain type (e.g. 'processorPolyPatch')
        //  Uses isA, not isType
        template<class Type>
        labelHashSet getPatchIDs() const;

        //- Calculate smallest true distance (and face index)
        //  from pt to faces wallFaces.
        //  For efficiency reasons we still pass in patch instead of extracting
        //  it from mesh_
        scalar smallestDist
        (
            const point& p,
            const polyPatch& patch,
            const label nWallFaces,
            const labelList& wallFaces,
            label& meshFaceI
        ) const;

        //- Get faces sharing point with face on patch
        label getPointNeighbours
        (
            const primitivePatch&,
            const label patchFaceI,
            labelList&
        ) const;

        //- Size of largest patch (out of supplied subset of patches)
        label maxPatchSize(const labelHashSet& patchIDs) const;

        //- Sum of patch sizes (out of supplied subset of patches).
        //  Used in sizing arrays.
        label sumPatchSize(const labelHashSet& patchIDs) const;

        //- Correct all cells connected to boundary (via face). Sets values in
        //  wallDistCorrected. Sets nearest wallface in nearestFace.
        void correctBoundaryFaceCells
        (
            const labelHashSet& patchIDs,
            scalarField& wallDistCorrected,
            Map<label>& nearestFace
        ) const;


        //- Correct all cells connected to wall (via point). Sets values in
        //  wallDistCorrected. Uses/sets nearest wallFace in nearestFace.
        void correctBoundaryPointCells
        (
            const labelHashSet& patchIDs,
            scalarField& wallDistCorrected,
            Map<label>& nearestFace
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "polyMesh.hpp"
#include "polyBoundaryMesh.hpp"


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
CML::labelHashSet CML::cellDistFuncs::getPatchIDs() const
{
    const polyBoundaryMesh& bMesh = mesh().boundaryMesh();

    labelHashSet patchIDs(bMesh.size());

    forAll(bMesh, patchI)
    {
        if (isA<Type>(bMesh[patchI]))
        {
            patchIDs.insert(patchI);
        }
    }
    return patchIDs;
}



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
