/*---------------------------------------------------------------------------*\
Copyright (C) 2012 OpenFOAM Foundation
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

Description
    Generic internal field mapper for dimensioned fields.  For "real" mapping,
    add template specialisations for mapping of internal fields depending on
    mesh type.

\*---------------------------------------------------------------------------*/

#ifndef MapDimensionedFields_H
#define MapDimensionedFields_H

#include "polyMesh.hpp"
#include "MapFvVolField.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

template<class Type, class MeshMapper, class GeoMesh>
void MapDimensionedFields(const MeshMapper& mapper)
{
    typedef DimensionedField<Type, GeoMesh> FieldType;
    typedef HashTable<const FieldType*> TableType;

    TableType fields(mapper.thisDb().size());

    forAllConstIter(objectRegistry, mapper.thisDb(), iter)
    {
        if (isType<FieldType>(*iter()))
        {
            fields.insert
            (
                iter()->name(),
                dynamic_cast<const FieldType*>(iter())
            );
        }
    }

    forAllConstIter(typename TableType, fields, fieldIter)
    {
        FieldType& field = const_cast<FieldType&>(*fieldIter());

        if (&field.mesh() == &mapper.mesh())
        {
            if (polyMesh::debug)
            {
                Info<< "Mapping " << field.typeName << ' ' << field.name()
                    << endl;
            }

            MapInternalField<Type, MeshMapper, GeoMesh>()(field, mapper);

            field.instance() = field.time().timeName();
        }
        else if (polyMesh::debug)
        {
            Info<< "Not mapping " << field.typeName << ' ' << field.name()
                << " since originating mesh differs from that of mapper."
                << endl;
        }
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
