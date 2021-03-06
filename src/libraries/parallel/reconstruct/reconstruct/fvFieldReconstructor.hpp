/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2015 OpenFOAM Foundation
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
    CML::fvFieldReconstructor

Description
    Finite volume reconstructor for volume and surface fields.

SourceFiles
    fvFieldReconstructor.cpp

\*---------------------------------------------------------------------------*/

#ifndef fvFieldReconstructor_H
#define fvFieldReconstructor_H

#include "PtrList.hpp"
#include "fvMesh.hpp"
#include "IOobjectList.hpp"
#include "fvPatchFieldMapper.hpp"
#include "labelIOList.hpp"
#include "Time.hpp"
#include "PtrList.hpp"
#include "fvPatchFields.hpp"
#include "emptyFvPatch.hpp"
#include "emptyFvPatchField.hpp"
#include "emptyFvsPatchField.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{


/*---------------------------------------------------------------------------*\
                    Class fvFieldReconstructor Declaration
\*---------------------------------------------------------------------------*/

class fvFieldReconstructor
{
    // Private data

        //- Reconstructed mesh reference
        fvMesh& mesh_;

        //- List of processor meshes
        const PtrList<fvMesh>& procMeshes_;

        //- List of processor face addressing lists
        const PtrList<labelIOList>& faceProcAddressing_;

        //- List of processor cell addressing lists
        const PtrList<labelIOList>& cellProcAddressing_;

        //- List of processor boundary addressing lists
        const PtrList<labelIOList>& boundaryProcAddressing_;

        //- Number of fields reconstructed
        label nReconstructed_;


    // Private Member Functions

        //- Disallow default bitwise copy construct
        fvFieldReconstructor(const fvFieldReconstructor&);

        //- Disallow default bitwise assignment
        void operator=(const fvFieldReconstructor&);


public:

        //- Mapper for sizing only - does not do any actual mapping.
        class fvPatchFieldReconstructor
        :
            public fvPatchFieldMapper
        {
            label size_;

        public:

            // Constructors

                //- Construct given size
                fvPatchFieldReconstructor(const label size)
                :
                    size_(size)
                {}


            // Member functions

                label size() const
                {
                    return size_;
                }

                bool direct() const
                {
                    return true;
                }

                bool hasUnmapped() const
                {
                    return false;
                }

                const labelUList& directAddressing() const
                {
                    return labelUList::null();
                }
        };


    // Constructors

        //- Construct from components
        fvFieldReconstructor
        (
            fvMesh& mesh,
            const PtrList<fvMesh>& procMeshes,
            const PtrList<labelIOList>& faceProcAddressing,
            const PtrList<labelIOList>& cellProcAddressing,
            const PtrList<labelIOList>& boundaryProcAddressing
        );


    // Member Functions

        //- Return number of fields reconstructed
        label nReconstructed() const
        {
            return nReconstructed_;
        }

        //- Reconstruct volume internal field
        template<class Type>
        tmp<DimensionedField<Type, volMesh> >
        reconstructFvVolumeInternalField
        (
            const IOobject& fieldIoObject,
            const PtrList<DimensionedField<Type, volMesh> >& procFields
        ) const;

        //- Read and reconstruct volume internal field
        template<class Type>
        tmp<DimensionedField<Type, volMesh> >
        reconstructFvVolumeInternalField(const IOobject& fieldIoObject) const;


        //- Reconstruct volume field
        template<class Type>
        tmp<GeometricField<Type, fvPatchField, volMesh> >
        reconstructFvVolumeField
        (
            const IOobject& fieldIoObject,
            const PtrList<GeometricField<Type, fvPatchField, volMesh> >&
        ) const;

        //- Read and reconstruct volume field
        template<class Type>
        tmp<GeometricField<Type, fvPatchField, volMesh> >
        reconstructFvVolumeField(const IOobject& fieldIoObject) const;


        //- Reconstruct surface field
        template<class Type>
        tmp<GeometricField<Type, fvsPatchField, surfaceMesh> >
        reconstructFvSurfaceField
        (
            const IOobject& fieldIoObject,
            const PtrList<GeometricField<Type, fvsPatchField, surfaceMesh> >&
        ) const;

        //- Read and reconstruct surface field
        template<class Type>
        tmp<GeometricField<Type, fvsPatchField, surfaceMesh> >
        reconstructFvSurfaceField(const IOobject& fieldIoObject) const;

        //- Read, reconstruct and write all/selected volume internal fields
        template<class Type>
        void reconstructFvVolumeInternalFields
        (
            const IOobjectList& objects,
            const HashSet<word>& selectedFields
        );

        //- Read, reconstruct and write all/selected volume fields
        template<class Type>
        void reconstructFvVolumeFields
        (
            const IOobjectList& objects,
            const HashSet<word>& selectedFields
        );

        //- Read, reconstruct and write all/selected surface fields
        template<class Type>
        void reconstructFvSurfaceFields
        (
            const IOobjectList& objects,
            const HashSet<word>& selectedFields
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
CML::tmp<CML::DimensionedField<Type, CML::volMesh> >
CML::fvFieldReconstructor::reconstructFvVolumeInternalField
(
    const IOobject& fieldIoObject,
    const PtrList<DimensionedField<Type, volMesh> >& procFields
) const
{
    // Create the internalField
    Field<Type> internalField(mesh_.nCells());

    forAll(procMeshes_, procI)
    {
        const DimensionedField<Type, volMesh>& procField = procFields[procI];

        // Set the cell values in the reconstructed field
        internalField.rmap
        (
            procField.field(),
            cellProcAddressing_[procI]
        );
    }

    return tmp<DimensionedField<Type, volMesh> >
    (
        new DimensionedField<Type, volMesh>
        (
            fieldIoObject,
            mesh_,
            procFields[0].dimensions(),
            internalField
        )
    );
}


template<class Type>
CML::tmp<CML::DimensionedField<Type, CML::volMesh> >
CML::fvFieldReconstructor::reconstructFvVolumeInternalField
(
    const IOobject& fieldIoObject
) const
{
    // Read the field for all the processors
    PtrList<DimensionedField<Type, volMesh> > procFields
    (
        procMeshes_.size()
    );

    forAll(procMeshes_, procI)
    {
        procFields.set
        (
            procI,
            new DimensionedField<Type, volMesh>
            (
                IOobject
                (
                    fieldIoObject.name(),
                    procMeshes_[procI].time().timeName(),
                    procMeshes_[procI],
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                ),
                procMeshes_[procI]
            )
        );
    }


    return reconstructFvVolumeInternalField
    (
        IOobject
        (
            fieldIoObject.name(),
            mesh_.time().timeName(),
            mesh_,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        procFields
    );
}


template<class Type>
CML::tmp<CML::GeometricField<Type, CML::fvPatchField, CML::volMesh> >
CML::fvFieldReconstructor::reconstructFvVolumeField
(
    const IOobject& fieldIoObject,
    const PtrList<GeometricField<Type, fvPatchField, volMesh> >& procFields
) const
{
    // Create the internalField
    Field<Type> internalField(mesh_.nCells());

    // Create the patch fields
    PtrList<fvPatchField<Type> > patchFields(mesh_.boundary().size());

    forAll(procFields, procI)
    {
        const GeometricField<Type, fvPatchField, volMesh>& procField =
            procFields[procI];

        // Set the cell values in the reconstructed field
        internalField.rmap
        (
            procField.internalField(),
            cellProcAddressing_[procI]
        );

        // Set the boundary patch values in the reconstructed field
        forAll(boundaryProcAddressing_[procI], patchI)
        {
            // Get patch index of the original patch
            const label curBPatch = boundaryProcAddressing_[procI][patchI];

            // Get addressing slice for this patch
            const labelList::subList cp =
                procField.mesh().boundary()[patchI].patchSlice
                (
                    faceProcAddressing_[procI]
                );

            // check if the boundary patch is not a processor patch
            if (curBPatch >= 0)
            {
                // Regular patch. Fast looping

                if (!patchFields(curBPatch))
                {
                    patchFields.set
                    (
                        curBPatch,
                        fvPatchField<Type>::New
                        (
                            procField.boundaryField()[patchI],
                            mesh_.boundary()[curBPatch],
                            DimensionedField<Type, volMesh>::null(),
                            fvPatchFieldReconstructor
                            (
                                mesh_.boundary()[curBPatch].size()
                            )
                        )
                    );
                }

                const label curPatchStart =
                    mesh_.boundaryMesh()[curBPatch].start();

                labelList reverseAddressing(cp.size());

                forAll(cp, faceI)
                {
                    // Check
                    if (cp[faceI] <= 0)
                    {
                        FatalErrorInFunction
                            << "Processor " << procI
                            << " patch "
                            << procField.mesh().boundary()[patchI].name()
                            << " face " << faceI
                            << " originates from reversed face since "
                            << cp[faceI]
                            << exit(FatalError);
                    }

                    // Subtract one to take into account offsets for
                    // face direction.
                    reverseAddressing[faceI] = cp[faceI] - 1 - curPatchStart;
                }


                patchFields[curBPatch].rmap
                (
                    procField.boundaryField()[patchI],
                    reverseAddressing
                );
            }
            else
            {
                const Field<Type>& curProcPatch =
                    procField.boundaryField()[patchI];

                // In processor patches, there's a mix of internal faces (some
                // of them turned) and possible cyclics. Slow loop
                forAll(cp, faceI)
                {
                    // Subtract one to take into account offsets for
                    // face direction.
                    label curF = cp[faceI] - 1;

                    // Is the face on the boundary?
                    if (curF >= mesh_.nInternalFaces())
                    {
                        label curBPatch = mesh_.boundaryMesh().whichPatch(curF);

                        if (!patchFields(curBPatch))
                        {
                            patchFields.set
                            (
                                curBPatch,
                                fvPatchField<Type>::New
                                (
                                    mesh_.boundary()[curBPatch].type(),
                                    mesh_.boundary()[curBPatch],
                                    DimensionedField<Type, volMesh>::null()
                                )
                            );
                        }

                        // add the face
                        label curPatchFace =
                            mesh_.boundaryMesh()
                                [curBPatch].whichFace(curF);

                        patchFields[curBPatch][curPatchFace] =
                            curProcPatch[faceI];
                    }
                }
            }
        }
    }

    forAll(mesh_.boundary(), patchI)
    {
        // add empty patches
        if
        (
            isType<emptyFvPatch>(mesh_.boundary()[patchI])
         && !patchFields(patchI)
        )
        {
            patchFields.set
            (
                patchI,
                fvPatchField<Type>::New
                (
                    emptyFvPatchField<Type>::typeName,
                    mesh_.boundary()[patchI],
                    DimensionedField<Type, volMesh>::null()
                )
            );
        }
    }


    // Now construct and write the field
    // setting the internalField and patchFields
    return tmp<GeometricField<Type, fvPatchField, volMesh> >
    (
        new GeometricField<Type, fvPatchField, volMesh>
        (
            fieldIoObject,
            mesh_,
            procFields[0].dimensions(),
            internalField,
            patchFields
        )
    );
}


template<class Type>
CML::tmp<CML::GeometricField<Type, CML::fvPatchField, CML::volMesh> >
CML::fvFieldReconstructor::reconstructFvVolumeField
(
    const IOobject& fieldIoObject
) const
{
    // Read the field for all the processors
    PtrList<GeometricField<Type, fvPatchField, volMesh> > procFields
    (
        procMeshes_.size()
    );

    forAll(procMeshes_, procI)
    {
        procFields.set
        (
            procI,
            new GeometricField<Type, fvPatchField, volMesh>
            (
                IOobject
                (
                    fieldIoObject.name(),
                    procMeshes_[procI].time().timeName(),
                    procMeshes_[procI],
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                ),
                procMeshes_[procI]
            )
        );
    }

    return reconstructFvVolumeField
    (
        IOobject
        (
            fieldIoObject.name(),
            mesh_.time().timeName(),
            mesh_,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        procFields
    );
}


template<class Type>
CML::tmp<CML::GeometricField<Type, CML::fvsPatchField, CML::surfaceMesh> >
CML::fvFieldReconstructor::reconstructFvSurfaceField
(
    const IOobject& fieldIoObject,
    const PtrList<GeometricField<Type, fvsPatchField, surfaceMesh> >& procFields
) const
{
    // Create the internalField
    Field<Type> internalField(mesh_.nInternalFaces());

    // Create the patch fields
    PtrList<fvsPatchField<Type> > patchFields(mesh_.boundary().size());


    forAll(procMeshes_, procI)
    {
        const GeometricField<Type, fvsPatchField, surfaceMesh>& procField =
            procFields[procI];

        // Set the face values in the reconstructed field

        // It is necessary to create a copy of the addressing array to
        // take care of the face direction offset trick.
        //
        {
            const labelList& faceMap = faceProcAddressing_[procI];

            // Correctly oriented copy of internal field
            Field<Type> procInternalField(procField.internalField());
            // Addressing into original field
            labelList curAddr(procInternalField.size());

            forAll(procInternalField, addrI)
            {
                curAddr[addrI] = mag(faceMap[addrI])-1;
                if (faceMap[addrI] < 0)
                {
                    procInternalField[addrI] = -procInternalField[addrI];
                }
            }

            // Map
            internalField.rmap(procInternalField, curAddr);
        }

        // Set the boundary patch values in the reconstructed field
        forAll(boundaryProcAddressing_[procI], patchI)
        {
            // Get patch index of the original patch
            const label curBPatch = boundaryProcAddressing_[procI][patchI];

            // Get addressing slice for this patch
            const labelList::subList cp =
                procMeshes_[procI].boundary()[patchI].patchSlice
                (
                    faceProcAddressing_[procI]
                );

            // check if the boundary patch is not a processor patch
            if (curBPatch >= 0)
            {
                // Regular patch. Fast looping

                if (!patchFields(curBPatch))
                {
                    patchFields.set
                    (
                        curBPatch,
                        fvsPatchField<Type>::New
                        (
                            procField.boundaryField()[patchI],
                            mesh_.boundary()[curBPatch],
                            DimensionedField<Type, surfaceMesh>::null(),
                            fvPatchFieldReconstructor
                            (
                                mesh_.boundary()[curBPatch].size()
                            )
                        )
                    );
                }

                const label curPatchStart =
                    mesh_.boundaryMesh()[curBPatch].start();

                labelList reverseAddressing(cp.size());

                forAll(cp, faceI)
                {
                    // Subtract one to take into account offsets for
                    // face direction.
                    reverseAddressing[faceI] = cp[faceI] - 1 - curPatchStart;
                }

                patchFields[curBPatch].rmap
                (
                    procField.boundaryField()[patchI],
                    reverseAddressing
                );
            }
            else
            {
                const Field<Type>& curProcPatch =
                    procField.boundaryField()[patchI];

                // In processor patches, there's a mix of internal faces (some
                // of them turned) and possible cyclics. Slow loop
                forAll(cp, faceI)
                {
                    label curF = cp[faceI] - 1;

                    // Is the face turned the right side round
                    if (curF >= 0)
                    {
                        // Is the face on the boundary?
                        if (curF >= mesh_.nInternalFaces())
                        {
                            label curBPatch =
                                mesh_.boundaryMesh().whichPatch(curF);

                            if (!patchFields(curBPatch))
                            {
                                patchFields.set
                                (
                                    curBPatch,
                                    fvsPatchField<Type>::New
                                    (
                                        mesh_.boundary()[curBPatch].type(),
                                        mesh_.boundary()[curBPatch],
                                        DimensionedField<Type, surfaceMesh>
                                           ::null()
                                    )
                                );
                            }

                            // add the face
                            label curPatchFace =
                                mesh_.boundaryMesh()
                                [curBPatch].whichFace(curF);

                            patchFields[curBPatch][curPatchFace] =
                                curProcPatch[faceI];
                        }
                        else
                        {
                            // Internal face
                            internalField[curF] = curProcPatch[faceI];
                        }
                    }
                }
            }
        }
    }

    forAll(mesh_.boundary(), patchI)
    {
        // add empty patches
        if
        (
            isType<emptyFvPatch>(mesh_.boundary()[patchI])
         && !patchFields(patchI)
        )
        {
            patchFields.set
            (
                patchI,
                fvsPatchField<Type>::New
                (
                    emptyFvsPatchField<Type>::typeName,
                    mesh_.boundary()[patchI],
                    DimensionedField<Type, surfaceMesh>::null()
                )
            );
        }
    }


    // Now construct and write the field
    // setting the internalField and patchFields
    return tmp<GeometricField<Type, fvsPatchField, surfaceMesh> >
    (
        new GeometricField<Type, fvsPatchField, surfaceMesh>
        (
            fieldIoObject,
            mesh_,
            procFields[0].dimensions(),
            internalField,
            patchFields
        )
    );
}


template<class Type>
CML::tmp<CML::GeometricField<Type, CML::fvsPatchField, CML::surfaceMesh> >
CML::fvFieldReconstructor::reconstructFvSurfaceField
(
    const IOobject& fieldIoObject
) const
{
    // Read the field for all the processors
    PtrList<GeometricField<Type, fvsPatchField, surfaceMesh> > procFields
    (
        procMeshes_.size()
    );

    forAll(procMeshes_, procI)
    {
        procFields.set
        (
            procI,
            new GeometricField<Type, fvsPatchField, surfaceMesh>
            (
                IOobject
                (
                    fieldIoObject.name(),
                    procMeshes_[procI].time().timeName(),
                    procMeshes_[procI],
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                ),
                procMeshes_[procI]
            )
        );
    }

    return reconstructFvSurfaceField
    (
        IOobject
        (
            fieldIoObject.name(),
            mesh_.time().timeName(),
            mesh_,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        procFields
    );
}


template<class Type>
void CML::fvFieldReconstructor::reconstructFvVolumeInternalFields
(
    const IOobjectList& objects,
    const HashSet<word>& selectedFields
)
{
    const word& fieldClassName = DimensionedField<Type, volMesh>::typeName;

    IOobjectList fields = objects.lookupClass(fieldClassName);

    if (fields.size())
    {
        Info<< "    Reconstructing " << fieldClassName << "s\n" << endl;

        forAllConstIter(IOobjectList, fields, fieldIter)
        {
            if
            (
                selectedFields.empty()
             || selectedFields.found(fieldIter()->name())
            )
            {
                Info<< "        " << fieldIter()->name() << endl;

                reconstructFvVolumeInternalField<Type>(*fieldIter())().write();

                nReconstructed_++;
            }
        }
        Info<< endl;
    }
}


template<class Type>
void CML::fvFieldReconstructor::reconstructFvVolumeFields
(
    const IOobjectList& objects,
    const HashSet<word>& selectedFields
)
{
    const word& fieldClassName =
        GeometricField<Type, fvPatchField, volMesh>::typeName;

    IOobjectList fields = objects.lookupClass(fieldClassName);

    if (fields.size())
    {
        Info<< "    Reconstructing " << fieldClassName << "s\n" << endl;

        forAllConstIter(IOobjectList, fields, fieldIter)
        {
            if
            (
                selectedFields.empty()
             || selectedFields.found(fieldIter()->name())
            )
            {
                Info<< "        " << fieldIter()->name() << endl;

                reconstructFvVolumeField<Type>(*fieldIter())().write();

                nReconstructed_++;
            }
        }
        Info<< endl;
    }
}


template<class Type>
void CML::fvFieldReconstructor::reconstructFvSurfaceFields
(
    const IOobjectList& objects,
    const HashSet<word>& selectedFields
)
{
    const word& fieldClassName =
        GeometricField<Type, fvsPatchField, surfaceMesh>::typeName;

    IOobjectList fields = objects.lookupClass(fieldClassName);

    if (fields.size())
    {
        Info<< "    Reconstructing " << fieldClassName << "s\n" << endl;

        forAllConstIter(IOobjectList, fields, fieldIter)
        {
            if
            (
                selectedFields.empty()
             || selectedFields.found(fieldIter()->name())
            )
            {
                Info<< "        " << fieldIter()->name() << endl;

                reconstructFvSurfaceField<Type>(*fieldIter())().write();

                nReconstructed_++;
            }
        }
        Info<< endl;
    }
}


#endif

// ************************************************************************* //
