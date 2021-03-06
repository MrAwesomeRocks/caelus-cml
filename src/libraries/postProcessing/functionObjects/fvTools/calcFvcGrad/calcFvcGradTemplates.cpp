/*---------------------------------------------------------------------------*\
Copyright (C) 2012-2013 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

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

\*---------------------------------------------------------------------------*/

#include "fvMesh.hpp"
#include "fvcGrad.hpp"

template<class Type>
CML::GeometricField
<
    typename CML::outerProduct<CML::vector, Type>::type,
    CML::fvPatchField,
    CML::volMesh
>&
CML::calcFvcGrad::gradField(const word& gradName, const dimensionSet& dims)
{
    typedef typename outerProduct<vector, Type>::type gradType;
    typedef GeometricField<gradType, fvPatchField, volMesh> vfGradType;

    const fvMesh& mesh = refCast<const fvMesh>(obr_);

    if (!mesh.foundObject<vfGradType>(gradName))
    {
        vfGradType* gradFieldPtr
        (
            new vfGradType
            (
                IOobject
                (
                    gradName,
                    mesh.time().timeName(),
                    mesh,
                    IOobject::NO_READ,
                    IOobject::NO_WRITE
                ),
                mesh,
                dimensioned<gradType>
                (
                    "zero",
                    dims/dimLength,
                    pTraits<gradType>::zero
                )
            )
        );

        mesh.objectRegistry::store(gradFieldPtr);
    }

    const vfGradType& field = mesh.lookupObject<vfGradType>(gradName);

    return const_cast<vfGradType&>(field);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void CML::calcFvcGrad::calcGrad
(
    const word& fieldName,
    const word& resultName,
    bool& processed
)
{
    typedef GeometricField<Type, fvPatchField, volMesh> vfType;
    typedef GeometricField<Type, fvsPatchField, surfaceMesh> sfType;

    typedef typename outerProduct<vector, Type>::type gradType;
    typedef GeometricField<gradType, fvPatchField, volMesh> vfGradType;

    const fvMesh& mesh = refCast<const fvMesh>(obr_);


    if (mesh.foundObject<vfType>(fieldName))
    {
        const vfType& vf = mesh.lookupObject<vfType>(fieldName);

        vfGradType& field = gradField<Type>(resultName, vf.dimensions());

        field = fvc::grad(vf);

        processed = true;
    }
    else if (mesh.foundObject<sfType>(fieldName))
    {
        const sfType& sf = mesh.lookupObject<sfType>(fieldName);

        vfGradType& field = gradField<Type>(resultName, sf.dimensions());

        field = fvc::grad(sf);

        processed = true;
    }
}


// ************************************************************************* //
