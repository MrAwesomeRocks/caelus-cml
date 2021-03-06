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

InNamespace
    CML::fvc

Description
    Area-weighted average a surfaceField creating a volField


\*---------------------------------------------------------------------------*/


#ifndef fvcAverage_H
#define fvcAverage_H

#include "volFieldsFwd.hpp"
#include "surfaceFieldsFwd.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                     Namespace fvc functions Declaration
\*---------------------------------------------------------------------------*/

namespace fvc
{
    //- Area-weighted average a surfaceField creating a volField
    template<class Type>
    tmp<GeometricField<Type, fvPatchField, volMesh> > average
    (
        const GeometricField<Type, fvsPatchField, surfaceMesh>&
    );

    //- Area-weighted average a surfaceField creating a volField
    template<class Type>
    tmp<GeometricField<Type, fvPatchField, volMesh> > average
    (
        const tmp<GeometricField<Type, fvsPatchField, surfaceMesh> >&
    );


    //- Area-weighted average a surfaceField creating a volField
    template<class Type>
    tmp<GeometricField<Type, fvPatchField, volMesh> > average
    (
        const GeometricField<Type, fvPatchField, volMesh>&
    );

    //- Area-weighted average a surfaceField creating a volField
    template<class Type>
    tmp<GeometricField<Type, fvPatchField, volMesh> > average
    (
        const tmp<GeometricField<Type, fvPatchField, volMesh> >&
    );
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "fvcSurfaceIntegrate.hpp"
#include "fvMesh.hpp"
#include "linear.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace fvc
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
tmp<GeometricField<Type, fvPatchField, volMesh> >
average
(
    const GeometricField<Type, fvsPatchField, surfaceMesh>& ssf
)
{
    const fvMesh& mesh = ssf.mesh();

    tmp<GeometricField<Type, fvPatchField, volMesh> > taverage
    (
        new GeometricField<Type, fvPatchField, volMesh>
        (
            IOobject
            (
                "average("+ssf.name()+')',
                ssf.instance(),
                mesh,
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            mesh,
            ssf.dimensions()
        )
    );

    GeometricField<Type, fvPatchField, volMesh>& av = taverage();

    av.internalField() =
    (
        surfaceSum(mesh.magSf()*ssf)/surfaceSum(mesh.magSf())
    )().internalField();

    typename GeometricField<Type, fvPatchField, volMesh>::
    GeometricBoundaryField& bav = av.boundaryField();

    forAll(bav, patchi)
    {
        bav[patchi] = ssf.boundaryField()[patchi];
    }

    av.correctBoundaryConditions();

    return taverage;
}


template<class Type>
tmp<GeometricField<Type, fvPatchField, volMesh> >
average
(
    const tmp<GeometricField<Type, fvsPatchField, surfaceMesh> >& tssf
)
{
    tmp<GeometricField<Type, fvPatchField, volMesh> > taverage
    (
        fvc::average(tssf())
    );
    tssf.clear();
    return taverage;
}


template<class Type>
tmp<GeometricField<Type, fvPatchField, volMesh> >
average
(
    const GeometricField<Type, fvPatchField, volMesh>& vtf
)
{
    return fvc::average(linearInterpolate(vtf));
}


template<class Type>
tmp<GeometricField<Type, fvPatchField, volMesh> >
average
(
    const tmp<GeometricField<Type, fvPatchField, volMesh> >& tvtf
)
{
    tmp<GeometricField<Type, fvPatchField, volMesh> > taverage
    (
        fvc::average(tvtf())
    );
    tvtf.clear();
    return taverage;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace fvc

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
