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
    CML::mappedVariableThicknessWallPolyPatch

Description
    CML::mappedVariableThicknessWallPolyPatch

SourceFiles
    mappedVariableThicknessWallPolyPatch.cpp

\*---------------------------------------------------------------------------*/

#ifndef mappedVariableThicknessWallPolyPatch_H
#define mappedVariableThicknessWallPolyPatch_H

#include "wallPolyPatch.hpp"
#include "mappedWallPolyPatch.hpp"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

class polyMesh;

/*---------------------------------------------------------------------------*\
           Class mappedVariableThicknessWallPolyPatch Declaration
\*---------------------------------------------------------------------------*/

class mappedVariableThicknessWallPolyPatch
:
    public mappedWallPolyPatch
{

    // Private data

        //- Thickness
        scalarList thickness_;


public:

    //- Runtime type information
    TypeName("mappedWallVariableThickness");


    // Constructors

        //- Construct from components
        mappedVariableThicknessWallPolyPatch
        (
            const word& name,
            const label size,
            const label start,
            const label index,
            const polyBoundaryMesh& bm,
            const word& patchType
        );

        //- Construct from components
        mappedVariableThicknessWallPolyPatch
        (
            const word& name,
            const label size,
            const label start,
            const label index,
            const word& sampleRegion,
            const mappedPatchBase::sampleMode mode,
            const word& samplePatch,
            const vectorField& offset,
            const polyBoundaryMesh& bm
        );

        //- Construct from components. Uniform offset.
        mappedVariableThicknessWallPolyPatch
        (
            const word& name,
            const label size,
            const label start,
            const label index,
            const word& sampleRegion,
            const mappedPatchBase::sampleMode mode,
            const word& samplePatch,
            const vector& offset,
            const polyBoundaryMesh& bm
        );

        //- Construct from dictionary
        mappedVariableThicknessWallPolyPatch
        (
            const word& name,
            const dictionary& dict,
            const label index,
            const polyBoundaryMesh& bm,
            const word& patchType
        );

        //- Construct as copy, resetting the boundary mesh
        mappedVariableThicknessWallPolyPatch
        (
            const mappedVariableThicknessWallPolyPatch&,
            const polyBoundaryMesh&
        );

        //- Construct given the original patch and resetting the
        //  face list and boundary mesh information
        mappedVariableThicknessWallPolyPatch
        (
            const mappedVariableThicknessWallPolyPatch& pp,
            const polyBoundaryMesh& bm,
            const label index,
            const label newSize,
            const label newStart
        );

        //- Construct given the original patch and a map
        mappedVariableThicknessWallPolyPatch
        (
            const mappedVariableThicknessWallPolyPatch& pp,
            const polyBoundaryMesh& bm,
            const label index,
            const labelUList& mapAddressing,
            const label newStart
        );

        //- Construct and return a clone, resetting the boundary mesh
        virtual autoPtr<polyPatch> clone(const polyBoundaryMesh& bm) const
        {
            return autoPtr<polyPatch>
            (
                new mappedVariableThicknessWallPolyPatch(*this, bm)
            );
        }

        //- Construct and return a clone, resetting the face list
        //  and boundary mesh
        virtual autoPtr<polyPatch> clone
        (
            const polyBoundaryMesh& bm,
            const label index,
            const label newSize,
            const label newStart
        ) const
        {
            return autoPtr<polyPatch>
            (
                new mappedVariableThicknessWallPolyPatch
                (
                    *this,
                    bm,
                    index,
                    newSize,
                    newStart
                )
            );
        }

        //- Construct and return a clone, resetting the face list
        //  and boundary mesh
        virtual autoPtr<polyPatch> clone
        (
            const polyBoundaryMesh& bm,
            const label index,
            const labelUList& mapAddressing,
            const label newStart
        ) const
        {
            return autoPtr<polyPatch>
            (
                new mappedVariableThicknessWallPolyPatch
                (
                    *this,
                    bm,
                    index,
                    mapAddressing,
                    newStart
                )
            );
        }


    //- Destructor
    virtual ~mappedVariableThicknessWallPolyPatch();


    // Member functions

        //- Return non const thickness
        scalarList& thickness()
        {
            return thickness_;
        }


        //- Return const thickness
        const scalarList& thickness() const
        {
            return thickness_;
        }


        //- Write the polyPatch data as a dictionary
        void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
