/*---------------------------------------------------------------------------*\
Copyright (C) 2013-2016 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Caelus is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

Class
    CML::PatchInjectionBase

Description
    Base class for patch-based injection models.

    Class handles injecting at a random point adjacent to the patch faces to
    provide a more stochastic view of the injection process.  Patch faces are
    triangulated, and area fractions accumulated.  The fractional areas are
    then applied to determine across which face a parcel is to be injected.

SourceFiles
    PatchInjectionBase.cpp

\*---------------------------------------------------------------------------*/

#ifndef patchInjectionBase_HPP
#define patchInjectionBase_HPP

#include "word.hpp"
#include "labelList.hpp"
#include "scalarList.hpp"
#include "vectorList.hpp"
#include "faceList.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward class declarations
class polyMesh;
class fvMesh;
class Random;

/*---------------------------------------------------------------------------*\
                     Class patchInjectionBase Declaration
\*---------------------------------------------------------------------------*/

class patchInjectionBase
{
protected:

    // Protected data

        //- Patch name
        const word patchName_;

        //- Patch ID
        const label patchId_;

        //- Patch area - total across all processors
        scalar patchArea_;

        //- Patch face normal directions
        vectorList patchNormal_;

        //- List of cell labels corresponding to injector positions
        labelList cellOwners_;

        //- Decomposed patch faces as a list of triangles
        faceList triFace_;

        //- Addressing from per triangle to patch face
        labelList triToFace_;

        //- Cumulative triangle area per triangle face
        scalarList triCumulativeMagSf_;

        //- Cumulative area fractions per processor
        scalarList sumTriMagSf_;


public:

    // Constructors

        //- Construct from mesh and patch name
        patchInjectionBase(const polyMesh& mesh, const word& patchName);

        //- Copy constructor
        patchInjectionBase(const patchInjectionBase& pib);


    //- Destructor
    virtual ~patchInjectionBase();


    // Member Functions

        //- Update patch geometry and derived info for injection locations
        virtual void updateMesh(const polyMesh& mesh);

        //- Set the injection position and owner cell, tetFace and tetPt
        virtual void setPositionAndCell
        (
            const fvMesh& mesh,
            Random& rnd,
            vector& position,
            label& cellOwner,
            label& tetFacei,
            label& tetPti
        );
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // end namespace CML


#endif
