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
    momentOfInertia

Description
    Calculates the inertia tensor and principal axes and moments of a
    polyhedra/cells/triSurfaces.  Inertia can either be of the solid body or
    of a thin shell.

SourceFiles
    momentOfInertia.cpp

\*---------------------------------------------------------------------------*/

#ifndef momentOfInertia_H
#define momentOfInertia_H

#include "tetPointRef.hpp"
#include "triFaceList.hpp"
#include "triSurface.hpp"
#include "polyMesh.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                     Class momentOfInertia Declaration
\*---------------------------------------------------------------------------*/

class momentOfInertia
{

public:

    static void massPropertiesSolid
    (
        const pointField& pts,
        const triFaceList& triFaces,
        scalar density,
        scalar& mass,
        vector& cM,
        tensor& J
    );

    static void massPropertiesShell
    (
        const pointField& pts,
        const triFaceList& triFaces,
        scalar density,
        scalar& mass,
        vector& cM,
        tensor& J
    );

    static void massPropertiesSolid
    (
        const triSurface& surf,
        scalar density,
        scalar& mass,
        vector& cM,
        tensor& J
    );

    static void massPropertiesShell
    (
        const triSurface& surf,
        scalar density,
        scalar& mass,
        vector& cM,
        tensor& J
    );

    static tensor applyParallelAxisTheorem
    (
        scalar mass,
        const vector& cM,
        const tensor& J,
        const vector& refPt
    );

    // Calculate the inertia tensor for all cells in the mesh
    static tmp<tensorField> meshInertia
    (
        const polyMesh& mesh
    );

    // Calculate the inertia tensor the given cell
    static tensor meshInertia
    (
        const polyMesh& mesh,
        label cellI
    );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
