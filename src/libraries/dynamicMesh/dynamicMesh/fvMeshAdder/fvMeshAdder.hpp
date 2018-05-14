/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2016 OpenFOAM Foundation
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
    CML::fvMeshAdder

Description
    Adds two fvMeshes without using any polyMesh morphing.
    Uses polyMeshAdder.

SourceFiles
    fvMeshAdder.cpp

\*---------------------------------------------------------------------------*/

#ifndef fvMeshAdder_H
#define fvMeshAdder_H

#include "polyMeshAdder.hpp"
#include "volFieldsFwd.hpp"
#include "fvPatchFieldsFwd.hpp"
#include "fvsPatchFieldsFwd.hpp"
#include "fvPatchFieldMapper.hpp"
#include "DimensionedField.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes
class IOobject;
class faceCoupleInfo;
class IOobjectList;
class fvMesh;
class volMesh;
class surfaceMesh;
class mapAddedPolyMesh;

/*---------------------------------------------------------------------------*\
                           Class fvMeshAdder Declaration
\*---------------------------------------------------------------------------*/

class fvMeshAdder
:
    public polyMeshAdder
{

private:

    // Private Member Functions

        //- Calculate map from new patch faces to old patch faces. -1 where
        //  could not map.
        static labelList calcPatchMap
        (
            const label oldStart,
            const label oldSize,
            const labelList& oldToNew,
            const polyPatch& newPatch,
            const label unmappedIndex
        );

        //- Update single volField.
        template<class Type>
        static void MapVolField
        (
            const mapAddedPolyMesh& meshMap,

            GeometricField<Type, fvPatchField, volMesh>& fld,
            const GeometricField<Type, fvPatchField, volMesh>& fldToAdd
        );

        //- Update single surfaceField.
        template<class Type>
        static void MapSurfaceField
        (
            const mapAddedPolyMesh& meshMap,

            GeometricField<Type, fvsPatchField, surfaceMesh>& fld,
            const GeometricField<Type, fvsPatchField, surfaceMesh>& fldToAdd
        );

        //- Update single dimensionedField.
        template<class Type>
        static void MapDimField
        (
            const mapAddedPolyMesh& meshMap,

            DimensionedField<Type, volMesh>& fld,
            const DimensionedField<Type, volMesh>& fldToAdd
        );

public:

    // Declare name of the class and its debug switch
    ClassName("fvMeshAdder");


    // Member Functions

        //- Inplace add mesh to fvMesh. Maps all stored fields. Returns map.
        static autoPtr<mapAddedPolyMesh> add
        (
            fvMesh& mesh0,
            const fvMesh& mesh1,
            const faceCoupleInfo& coupleInfo,
            const bool validBoundary = true
        );

        //- Map all volFields of Type
        template<class Type>
        static void MapVolFields
        (
            const mapAddedPolyMesh&,
            const fvMesh& mesh,
            const fvMesh& meshToAdd
        );

        //- Map all surfaceFields of Type
        template<class Type>
        static void MapSurfaceFields
        (
            const mapAddedPolyMesh&,
            const fvMesh& mesh,
            const fvMesh& meshToAdd
        );

        //- Map all DimensionedFields of Type
        template<class Type>
        static void MapDimFields
        (
            const mapAddedPolyMesh&,
            const fvMesh& mesh,
            const fvMesh& meshToAdd
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

#include "volFields.hpp"
#include "surfaceFields.hpp"
#include "emptyFvPatchField.hpp"
#include "directFvPatchFieldMapper.hpp"
// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void CML::fvMeshAdder::MapVolField
(
    const mapAddedPolyMesh& meshMap,

    GeometricField<Type, fvPatchField, volMesh>& fld,
    const GeometricField<Type, fvPatchField, volMesh>& fldToAdd
)
{
    const fvMesh& mesh = fld.mesh();

    // Internal field
    // ~~~~~~~~~~~~~~

    {
        // Store old internal field
        Field<Type> oldInternalField(fld.internalField());

        // Modify internal field
        Field<Type>& intFld = fld.internalField();

        intFld.setSize(mesh.nCells());

        intFld.rmap(oldInternalField, meshMap.oldCellMap());
        intFld.rmap(fldToAdd.internalField(), meshMap.addedCellMap());
    }


    // Patch fields from old mesh
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~

    typename GeometricField<Type, fvPatchField, volMesh>::
    GeometricBoundaryField& bfld = fld.boundaryField();

    {
        const labelList& oldPatchMap = meshMap.oldPatchMap();
        const labelList& oldPatchStarts = meshMap.oldPatchStarts();
        const labelList& oldPatchSizes = meshMap.oldPatchSizes();

        // Reorder old patches in order of new ones. Put removed patches at end.

        label unusedPatchi = 0;

        forAll(oldPatchMap, patchi)
        {
            label newPatchi = oldPatchMap[patchi];

            if (newPatchi != -1)
            {
                unusedPatchi++;
            }
        }

        label nUsedPatches = unusedPatchi;

        // Reorder list for patchFields
        labelList oldToNew(oldPatchMap.size());

        forAll(oldPatchMap, patchi)
        {
            label newPatchi = oldPatchMap[patchi];

            if (newPatchi != -1)
            {
                oldToNew[patchi] = newPatchi;
            }
            else
            {
                oldToNew[patchi] = unusedPatchi++;
            }
        }


        // Sort deleted ones last so is now in newPatch ordering
        bfld.reorder(oldToNew);
        // Extend to covers all patches
        bfld.setSize(mesh.boundaryMesh().size());
        // Delete unused patches
        for
        (
            label newPatchi = nUsedPatches;
            newPatchi < bfld.size();
            newPatchi++
        )
        {
            bfld.set(newPatchi, NULL);
        }


        // Map old values
        // ~~~~~~~~~~~~~~

        forAll(oldPatchMap, patchi)
        {
            label newPatchi = oldPatchMap[patchi];

            if (newPatchi != -1)
            {
                labelList newToOld
                (
                    calcPatchMap
                    (
                        oldPatchStarts[patchi],
                        oldPatchSizes[patchi],
                        meshMap.oldFaceMap(),
                        mesh.boundaryMesh()[newPatchi],
                        -1              // unmapped value
                    )
                );

                directFvPatchFieldMapper patchMapper(newToOld);


                // Create new patchField with same type as existing one.
                // Note:
                // - boundaryField already in new order so access with newPatchi
                // - fld.boundaryField()[newPatchi] both used for type and old
                //   value
                // - hope that field mapping allows aliasing since old and new
                //   are same memory!
                bfld.set
                (
                    newPatchi,
                    fvPatchField<Type>::New
                    (
                        bfld[newPatchi],                // old field
                        mesh.boundary()[newPatchi],     // new fvPatch
                        fld.dimensionedInternalField(), // new internal field
                        patchMapper                     // mapper (new to old)
                    )
                );
            }
        }
    }



    // Patch fields from added mesh
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    {
        const labelList& addedPatchMap = meshMap.addedPatchMap();

        // Add addedMesh patches
        forAll(addedPatchMap, patchi)
        {
            label newPatchi = addedPatchMap[patchi];

            if (newPatchi != -1)
            {
                const polyPatch& newPatch = mesh.boundaryMesh()[newPatchi];
                const polyPatch& oldPatch =
                    fldToAdd.mesh().boundaryMesh()[patchi];

                if (!bfld(newPatchi))
                {
                    // First occurrence of newPatchi. Map from existing
                    // patchField

                    // From new patch faces to patch faces on added mesh.
                    labelList newToAdded
                    (
                        calcPatchMap
                        (
                            oldPatch.start(),
                            oldPatch.size(),
                            meshMap.addedFaceMap(),
                            newPatch,
                            -1          // unmapped values
                        )
                    );

                    directFvPatchFieldMapper patchMapper(newToAdded);

                    bfld.set
                    (
                        newPatchi,
                        fvPatchField<Type>::New
                        (
                            fldToAdd.boundaryField()[patchi], // added field
                            mesh.boundary()[newPatchi],       // new fvPatch
                            fld.dimensionedInternalField(),   // new int. field
                            patchMapper                       // mapper
                        )
                    );
                }
                else
                {
                    // PatchField will have correct size already. Just slot in
                    // my elements.

                    labelList addedToNew(oldPatch.size(), -1);
                    forAll(addedToNew, i)
                    {
                        label addedFacei = oldPatch.start()+i;
                        label newFacei = meshMap.addedFaceMap()[addedFacei];
                        label patchFacei = newFacei-newPatch.start();
                        if (patchFacei >= 0 && patchFacei < newPatch.size())
                        {
                            addedToNew[i] = patchFacei;
                        }
                    }

                    bfld[newPatchi].rmap
                    (
                        fldToAdd.boundaryField()[patchi],
                        addedToNew
                    );
                }
            }
        }
    }
}


template<class Type>
void CML::fvMeshAdder::MapVolFields
(
    const mapAddedPolyMesh& meshMap,
    const fvMesh& mesh,
    const fvMesh& meshToAdd
)
{
    HashTable<const GeometricField<Type, fvPatchField, volMesh>*> fields
    (
        mesh.objectRegistry::lookupClass
        <GeometricField<Type, fvPatchField, volMesh> >
        ()
    );

    HashTable<const GeometricField<Type, fvPatchField, volMesh>*> fieldsToAdd
    (
        meshToAdd.objectRegistry::lookupClass
        <GeometricField<Type, fvPatchField, volMesh> >
        ()
    );

    // It is necessary to enforce that all old-time fields are stored
    // before the mapping is performed.  Otherwise, if the
    // old-time-level field is mapped before the field itself, sizes
    // will not match.

    for
    (
        typename HashTable<const GeometricField<Type, fvPatchField, volMesh>*>::
            iterator fieldIter = fields.begin();
        fieldIter != fields.end();
        ++fieldIter
    )
    {
        if (debug)
        {
            Pout<< "MapVolFields : Storing old time for " << fieldIter()->name()
                << endl;
        }

        const_cast<GeometricField<Type, fvPatchField, volMesh>*>(fieldIter())
            ->storeOldTimes();
    }


    for
    (
        typename HashTable<const GeometricField<Type, fvPatchField, volMesh>*>::
            iterator fieldIter = fields.begin();
        fieldIter != fields.end();
        ++fieldIter
    )
    {
        GeometricField<Type, fvPatchField, volMesh>& fld =
            const_cast<GeometricField<Type, fvPatchField, volMesh>&>
            (
                *fieldIter()
            );

        if (fieldsToAdd.found(fld.name()))
        {
            const GeometricField<Type, fvPatchField, volMesh>& fldToAdd =
                *fieldsToAdd[fld.name()];

            if (debug)
            {
                Pout<< "MapVolFields : mapping " << fld.name()
                    << " and " << fldToAdd.name() << endl;
            }

            MapVolField<Type>(meshMap, fld, fldToAdd);
        }
        else
        {
            WarningIn("fvMeshAdder::MapVolFields(..)")
                << "Not mapping field " << fld.name()
                << " since not present on mesh to add"
                << endl;
        }
    }
}


template<class Type>
void CML::fvMeshAdder::MapSurfaceField
(
    const mapAddedPolyMesh& meshMap,

    GeometricField<Type, fvsPatchField, surfaceMesh>& fld,
    const GeometricField<Type, fvsPatchField, surfaceMesh>& fldToAdd
)
{
    const fvMesh& mesh = fld.mesh();
    const labelList& oldPatchStarts = meshMap.oldPatchStarts();

    typename GeometricField<Type, fvsPatchField, surfaceMesh>::
    GeometricBoundaryField& bfld = fld.boundaryField();

    // Internal field
    // ~~~~~~~~~~~~~~

    // Store old internal field
    {
        Field<Type> oldField(fld);

        // Modify internal field
        Field<Type>& intFld = fld.internalField();

        intFld.setSize(mesh.nInternalFaces());

        intFld.rmap(oldField, meshMap.oldFaceMap());
        intFld.rmap(fldToAdd, meshMap.addedFaceMap());


        // Faces that were boundary faces but are not anymore.
        // Use owner value (so lowest numbered cell, i.e. from 'old' not 'added'
        // mesh)
        forAll(bfld, patchi)
        {
            const fvsPatchField<Type>& pf = bfld[patchi];

            label start = oldPatchStarts[patchi];

            forAll(pf, i)
            {
                label newFacei = meshMap.oldFaceMap()[start + i];

                if (newFacei >= 0 && newFacei < mesh.nInternalFaces())
                {
                    intFld[newFacei] = pf[i];
                }
            }
        }
    }


    // Patch fields from old mesh
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~

    {
        const labelList& oldPatchMap = meshMap.oldPatchMap();
        const labelList& oldPatchSizes = meshMap.oldPatchSizes();

        // Reorder old patches in order of new ones. Put removed patches at end.

        label unusedPatchi = 0;

        forAll(oldPatchMap, patchi)
        {
            label newPatchi = oldPatchMap[patchi];

            if (newPatchi != -1)
            {
                unusedPatchi++;
            }
        }

        label nUsedPatches = unusedPatchi;

        // Reorder list for patchFields
        labelList oldToNew(oldPatchMap.size());

        forAll(oldPatchMap, patchi)
        {
            label newPatchi = oldPatchMap[patchi];

            if (newPatchi != -1)
            {
                oldToNew[patchi] = newPatchi;
            }
            else
            {
                oldToNew[patchi] = unusedPatchi++;
            }
        }


        // Sort deleted ones last so is now in newPatch ordering
        bfld.reorder(oldToNew);
        // Extend to covers all patches
        bfld.setSize(mesh.boundaryMesh().size());
        // Delete unused patches
        for
        (
            label newPatchi = nUsedPatches;
            newPatchi < bfld.size();
            newPatchi++
        )
        {
            bfld.set(newPatchi, NULL);
        }


        // Map old values
        // ~~~~~~~~~~~~~~

        forAll(oldPatchMap, patchi)
        {
            label newPatchi = oldPatchMap[patchi];

            if (newPatchi != -1)
            {
                labelList newToOld
                (
                    calcPatchMap
                    (
                        oldPatchStarts[patchi],
                        oldPatchSizes[patchi],
                        meshMap.oldFaceMap(),
                        mesh.boundaryMesh()[newPatchi],
                        -1      // unmapped value
                    )
                );

                directFvPatchFieldMapper patchMapper(newToOld);

                // Create new patchField with same type as existing one.
                // Note:
                // - boundaryField already in new order so access with newPatchi
                // - bfld[newPatchi] both used for type and old
                //   value
                // - hope that field mapping allows aliasing since old and new
                //   are same memory!
                bfld.set
                (
                    newPatchi,
                    fvsPatchField<Type>::New
                    (
                        bfld[newPatchi],                // old field
                        mesh.boundary()[newPatchi],     // new fvPatch
                        fld.dimensionedInternalField(), // new internal field
                        patchMapper                     // mapper (new to old)
                    )
                );
            }
        }
    }



    // Patch fields from added mesh
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    {
        const labelList& addedPatchMap = meshMap.addedPatchMap();

        // Add addedMesh patches
        forAll(addedPatchMap, patchi)
        {
            label newPatchi = addedPatchMap[patchi];

            if (newPatchi != -1)
            {
                const polyPatch& newPatch = mesh.boundaryMesh()[newPatchi];
                const polyPatch& oldPatch =
                    fldToAdd.mesh().boundaryMesh()[patchi];

                if (!bfld(newPatchi))
                {
                    // First occurrence of newPatchi. Map from existing
                    // patchField

                    // From new patch faces to patch faces on added mesh.
                    labelList newToAdded
                    (
                        calcPatchMap
                        (
                            oldPatch.start(),
                            oldPatch.size(),
                            meshMap.addedFaceMap(),
                            newPatch,
                            -1                  // unmapped values
                        )
                    );

                    directFvPatchFieldMapper patchMapper(newToAdded);

                    bfld.set
                    (
                        newPatchi,
                        fvsPatchField<Type>::New
                        (
                            fldToAdd.boundaryField()[patchi],// added field
                            mesh.boundary()[newPatchi],      // new fvPatch
                            fld.dimensionedInternalField(),  // new int. field
                            patchMapper                      // mapper
                        )
                    );
                }
                else
                {
                    // PatchField will have correct size already. Just slot in
                    // my elements.

                    labelList addedToNew(oldPatch.size(), -1);
                    forAll(addedToNew, i)
                    {
                        label addedFacei = oldPatch.start()+i;
                        label newFacei = meshMap.addedFaceMap()[addedFacei];
                        label patchFacei = newFacei-newPatch.start();
                        if (patchFacei >= 0 && patchFacei < newPatch.size())
                        {
                            addedToNew[i] = patchFacei;
                        }
                    }

                    bfld[newPatchi].rmap
                    (
                        fldToAdd.boundaryField()[patchi],
                        addedToNew
                    );
                }
            }
        }
    }
}


template<class Type>
void CML::fvMeshAdder::MapSurfaceFields
(
    const mapAddedPolyMesh& meshMap,
    const fvMesh& mesh,
    const fvMesh& meshToAdd
)
{
    typedef GeometricField<Type, fvsPatchField, surfaceMesh> fldType;

    HashTable<const fldType*> fields
    (
        mesh.objectRegistry::lookupClass<fldType>()
    );

    HashTable<const fldType*> fieldsToAdd
    (
        meshToAdd.objectRegistry::lookupClass<fldType>()
    );

    // It is necessary to enforce that all old-time fields are stored
    // before the mapping is performed.  Otherwise, if the
    // old-time-level field is mapped before the field itself, sizes
    // will not match.

    for
    (
        typename HashTable<const fldType*>::
            iterator fieldIter = fields.begin();
        fieldIter != fields.end();
        ++fieldIter
    )
    {
        if (debug)
        {
            Pout<< "MapSurfaceFields : Storing old time for "
                << fieldIter()->name() << endl;
        }

        const_cast<fldType*>(fieldIter())->storeOldTimes();
    }


    for
    (
        typename HashTable<const fldType*>::
            iterator fieldIter = fields.begin();
        fieldIter != fields.end();
        ++fieldIter
    )
    {
        fldType& fld = const_cast<fldType&>(*fieldIter());

        if (fieldsToAdd.found(fld.name()))
        {
            const fldType& fldToAdd = *fieldsToAdd[fld.name()];

            if (debug)
            {
                Pout<< "MapSurfaceFields : mapping " << fld.name()
                    << " and " << fldToAdd.name() << endl;
            }

            MapSurfaceField<Type>(meshMap, fld, fldToAdd);
        }
        else
        {
            WarningIn("fvMeshAdder::MapSurfaceFields(..)")
                << "Not mapping field " << fld.name()
                << " since not present on mesh to add"
                << endl;
        }
    }
}


template<class Type>
void CML::fvMeshAdder::MapDimField
(
    const mapAddedPolyMesh& meshMap,

    DimensionedField<Type, volMesh>& fld,
    const DimensionedField<Type, volMesh>& fldToAdd
)
{
    const fvMesh& mesh = fld.mesh();

    // Store old field
    Field<Type> oldField(fld);

    fld.setSize(mesh.nCells());

    fld.rmap(oldField, meshMap.oldCellMap());
    fld.rmap(fldToAdd, meshMap.addedCellMap());
}


template<class Type>
void CML::fvMeshAdder::MapDimFields
(
    const mapAddedPolyMesh& meshMap,
    const fvMesh& mesh,
    const fvMesh& meshToAdd
)
{
    typedef DimensionedField<Type, volMesh> fldType;

    // Note: use strict flag on lookupClass to avoid picking up
    //       volFields
    HashTable<const fldType*> fields
    (
        mesh.objectRegistry::lookupClass<fldType>(true)
    );

    HashTable<const fldType*> fieldsToAdd
    (
        meshToAdd.objectRegistry::lookupClass<fldType>(true)
    );

    for
    (
        typename HashTable<const fldType*>::
            iterator fieldIter = fields.begin();
        fieldIter != fields.end();
        ++fieldIter
    )
    {
        fldType& fld = const_cast<fldType&>(*fieldIter());

        if (fieldsToAdd.found(fld.name()))
        {
            const fldType& fldToAdd = *fieldsToAdd[fld.name()];

            if (debug)
            {
                Pout<< "MapDimFields : mapping " << fld.name()
                    << " and " << fldToAdd.name() << endl;
            }

            MapDimField<Type>(meshMap, fld, fldToAdd);
        }
        else
        {
            WarningIn("fvMeshAdder::MapDimFields(..)")
                << "Not mapping field " << fld.name()
                << " since not present on mesh to add"
                << endl;
        }
    }
}

#endif

// ************************************************************************* //
