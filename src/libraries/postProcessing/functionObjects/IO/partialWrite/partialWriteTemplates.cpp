/*---------------------------------------------------------------------------*\
Copyright (C) 2013 OpenFOAM Foundation
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

#include "partialWrite.hpp"
#include "volFields.hpp"
#include "surfaceFields.hpp"
#include "Time.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
void CML::partialWrite::loadField
(
    const word& fieldName,
    UPtrList<GeometricField<Type, fvPatchField, volMesh> >& vflds,
    UPtrList<GeometricField<Type, fvsPatchField, surfaceMesh> >& sflds
) const
{
    typedef GeometricField<Type, fvPatchField, volMesh> vfType;
    typedef GeometricField<Type, fvsPatchField, surfaceMesh> sfType;

    if (obr_.foundObject<vfType>(fieldName))
    {
        if (debug)
        {
            Info<< "partialWrite : Field "
                << fieldName << " found in database" << endl;
        }

        vfType& vField =
            const_cast<vfType&>
            (
                obr_.lookupObject<vfType>(fieldName)
            );


        const unsigned int sz = vflds.size();
        vflds.setSize(sz + 1);
        vflds.set(sz, &vField);
    }
    else if (obr_.foundObject<sfType>(fieldName))
    {
        if (debug)
        {
            Info<< "partialWrite : Field " << fieldName
                << " found in database" << endl;
        }

         sfType& sField =
            const_cast<sfType&>
            (
                obr_.lookupObject<sfType>(fieldName)
            );


        const unsigned int sz = sflds.size();
        sflds.setSize(sz + 1);
        sflds.set(sz, &sField);
    }
}


template<class Type>
void CML::partialWrite::changeWriteOptions
(
    UPtrList<GeometricField<Type, fvPatchField, volMesh> >& vflds,
    UPtrList<GeometricField<Type, fvsPatchField, surfaceMesh> >& sflds,
    const IOobject::writeOption wOption
) const
{
    forAll(vflds , i)
    {
        vflds[i].writeOpt() = wOption;
    }

    forAll(sflds , i)
    {
        sflds[i].writeOpt() = wOption;
    }
}


// ************************************************************************* //
