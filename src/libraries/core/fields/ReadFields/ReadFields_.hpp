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

Global
    CML::ReadFields

Description
    Helper routine to read fields

SourceFiles
    ReadFields.cpp

\*---------------------------------------------------------------------------*/

#ifndef ReadFields__H
#define ReadFields__H

#include "PtrList.hpp"
#include "wordList.hpp"
#include "GeometricField.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

class IOobjectList;


//- Get names of fields of type in parallel consistent order
wordList fieldNames(const IOobjectList& objects, const bool syncPar);

//- Helper routine to read Geometric fields
template<class Type, template<class> class PatchField, class GeoMesh>
wordList ReadFields
(
    const typename GeoMesh::Mesh& mesh,
    const IOobjectList& objects,
    PtrList<GeometricField<Type, PatchField, GeoMesh> >& fields,
    const bool syncPar = true,
    const bool readOldTime = false
);

//- Helper routine to read other mesh fields
template<class GeoField, class Mesh>
wordList ReadFields
(
    const Mesh& mesh,
    const IOobjectList& objects,
    PtrList<GeoField>& fields,
    const bool syncPar = true
);

//- Helper routine to read non-mesh fields (e.g. uniformDimensionedField like
//  'g')
template<class GeoField>
wordList ReadFields
(
    const IOobjectList& objects,
    PtrList<GeoField>& fields,
    const bool syncPar = true
);

//- Helper routine to read GeometricFields. The fieldsCache is per time
//  an objectRegistry of all stored fields
template<class GeoField>
static void ReadFields
(
    const word& fieldName,
    const typename GeoField::Mesh& mesh,
    const wordList& timeNames,
    objectRegistry& fieldsCache
);

//- Helper routine to read GeometricFields. The fieldsCache is per time
//  an objectRegistry of all stored fields
template<class GeoField>
static void ReadFields
(
    const word& fieldName,
    const typename GeoField::Mesh& mesh,
    const wordList& timeNames,
    const word& registryName = "fieldsCache"
);

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "HashSet.hpp"
#include "Pstream.hpp"
#include "IOobjectList.hpp"

// * * * * * * * * * * * * * * * Global Functions  * * * * * * * * * * * * * //

// Read all GeometricFields of type. Returns names of fields read. Guarantees
// all processors to read fields in same order.
template<class Type, template<class> class PatchField, class GeoMesh>
CML::wordList CML::ReadFields
(
    const typename GeoMesh::Mesh& mesh,
    const IOobjectList& objects,
    PtrList<GeometricField<Type, PatchField, GeoMesh> >& fields,
    const bool syncPar,
    const bool readOldTime
)
{
    typedef GeometricField<Type, PatchField, GeoMesh> GeoField;

    // Search list of objects for wanted type
    IOobjectList fieldObjects(objects.lookupClass(GeoField::typeName));

    const wordList masterNames(fieldNames(fieldObjects, syncPar));

    fields.setSize(masterNames.size());

    // Make sure to read in masterNames order.

    forAll(masterNames, i)
    {
        Info<< "Reading " << GeoField::typeName << ' ' << masterNames[i]
            << endl;

        const IOobject& io = *fieldObjects[masterNames[i]];

        fields.set
        (
            i,
            new GeoField
            (
                IOobject
                (
                    io.name(),
                    io.instance(),
                    io.local(),
                    io.db(),
                    IOobject::MUST_READ,
                    IOobject::AUTO_WRITE,
                    io.registerObject()
                ),
                mesh,
                readOldTime
            )
        );
    }
    return masterNames;
}


// Read all fields of type. Returns names of fields read. Guarantees all
// processors to read fields in same order.
template<class GeoField, class Mesh>
CML::wordList CML::ReadFields
(
    const Mesh& mesh,
    const IOobjectList& objects,
    PtrList<GeoField>& fields,
    const bool syncPar
)
{
    // Search list of objects for wanted type
    IOobjectList fieldObjects(objects.lookupClass(GeoField::typeName));

    const wordList masterNames(fieldNames(fieldObjects, syncPar));

    fields.setSize(masterNames.size());

    // Make sure to read in masterNames order.

    forAll(masterNames, i)
    {
        Info<< "Reading " << GeoField::typeName << ' ' << masterNames[i]
            << endl;

        const IOobject& io = *fieldObjects[masterNames[i]];

        fields.set
        (
            i,
            new GeoField
            (
                IOobject
                (
                    io.name(),
                    io.instance(),
                    io.local(),
                    io.db(),
                    IOobject::MUST_READ,
                    IOobject::AUTO_WRITE,
                    io.registerObject()
                ),
                mesh
            )
        );
    }
    return masterNames;
}


// Read all (non-mesh) fields of type. Returns names of fields read. Guarantees
// all processors to read fields in same order.
template<class GeoField>
CML::wordList CML::ReadFields
(
    const IOobjectList& objects,
    PtrList<GeoField>& fields,
    const bool syncPar
)
{
    // Search list of objects for wanted type
    IOobjectList fieldObjects(objects.lookupClass(GeoField::typeName));

    const wordList masterNames(fieldNames(fieldObjects, syncPar));

    fields.setSize(masterNames.size());

    // Make sure to read in masterNames order.

    forAll(masterNames, i)
    {
        Info<< "Reading " << GeoField::typeName << ' ' << masterNames[i]
            << endl;

        const IOobject& io = *fieldObjects[masterNames[i]];

        fields.set
        (
            i,
            new GeoField
            (
                IOobject
                (
                    io.name(),
                    io.instance(),
                    io.local(),
                    io.db(),
                    IOobject::MUST_READ,
                    IOobject::AUTO_WRITE,
                    io.registerObject()
                )
            )
        );
    }
    return masterNames;
}


template<class GeoField>
void CML::ReadFields
(
    const word& fieldName,
    const typename GeoField::Mesh& mesh,
    const wordList& timeNames,
    objectRegistry& fieldsCache
)
{
    // Collect all times that are no longer used
    {
        HashSet<word> usedTimes(timeNames);

        DynamicList<word> unusedTimes(fieldsCache.size());

        forAllIter(objectRegistry, fieldsCache, timeIter)
        {
            const word& tm = timeIter.key();
            if (!usedTimes.found(tm))
            {
                unusedTimes.append(tm);
            }
        }

        //Info<< "Unloading times " << unusedTimes << endl;

        forAll(unusedTimes, i)
        {
            objectRegistry& timeCache = const_cast<objectRegistry&>
            (
                fieldsCache.lookupObject<objectRegistry>(unusedTimes[i])
            );
            fieldsCache.checkOut(timeCache);
        }
    }


    // Load any new fields
    forAll(timeNames, i)
    {
        const word& tm = timeNames[i];

        // Create if not found
        if (!fieldsCache.found(tm))
        {
            //Info<< "Creating registry for time " << tm << endl;

            // Create objectRegistry if not found
            objectRegistry* timeCachePtr = new objectRegistry
            (
                IOobject
                (
                    tm,
                    tm,
                    fieldsCache,
                    IOobject::NO_READ,
                    IOobject::NO_WRITE
                )
            );
            timeCachePtr->store();
        }

        // Obtain cache for current time
        const objectRegistry& timeCache =
            fieldsCache.lookupObject<objectRegistry>
            (
                tm
            );

        // Store field if not found
        if (!timeCache.found(fieldName))
        {
            //Info<< "Loading field " << fieldName
            //    << " for time " << tm << endl;

            GeoField loadedFld
            (
                IOobject
                (
                    fieldName,
                    tm,
                    mesh.thisDb(),
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE,
                    false
                ),
                mesh
            );

            // Transfer to timeCache (new objectRegistry and store flag)
            GeoField* fldPtr = new GeoField
            (
                IOobject
                (
                    fieldName,
                    tm,
                    timeCache,
                    IOobject::NO_READ,
                    IOobject::NO_WRITE
                ),
                loadedFld
            );
            fldPtr->store();
        }
    }
}


template<class GeoField>
void CML::ReadFields
(
    const word& fieldName,
    const typename GeoField::Mesh& mesh,
    const wordList& timeNames,
    const word& registryName
)
{
    ReadFields<GeoField>
    (
        fieldName,
        mesh,
        timeNames,
        const_cast<objectRegistry&>
        (
            mesh.thisDb().subRegistry(registryName, true)
        )
    );
}

#endif

// ************************************************************************* //
