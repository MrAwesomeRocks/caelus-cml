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
    CML::cyclicPointPatchField

Description
    Cyclic front and back plane patch field

SourceFiles
    cyclicPointPatchField.cpp

\*---------------------------------------------------------------------------*/

#ifndef cyclicPointPatchField_H
#define cyclicPointPatchField_H

#include "coupledPointPatchField.hpp"
#include "cyclicPointPatch.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                    Class cyclicPointPatchField Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class cyclicPointPatchField
:
    public coupledPointPatchField<Type>
{
    // Private data

        //- Local reference cast into the cyclic patch
        const cyclicPointPatch& cyclicPatch_;


public:

    //- Runtime type information
    TypeName(cyclicPointPatch::typeName_());


    // Constructors

        //- Construct from patch and internal field
        cyclicPointPatchField
        (
            const pointPatch&,
            const DimensionedField<Type, pointMesh>&
        );

        //- Construct from patch, internal field and dictionary
        cyclicPointPatchField
        (
            const pointPatch&,
            const DimensionedField<Type, pointMesh>&,
            const dictionary&
        );

        //- Construct by mapping given patchField<Type> onto a new patch
        cyclicPointPatchField
        (
            const cyclicPointPatchField<Type>&,
            const pointPatch&,
            const DimensionedField<Type, pointMesh>&,
            const pointPatchFieldMapper&
        );

        //- Construct and return a clone
        virtual autoPtr<pointPatchField<Type> > clone() const
        {
            return autoPtr<pointPatchField<Type> >
            (
                new cyclicPointPatchField<Type>
                (
                    *this
                )
            );
        }

        //- Construct as copy setting internal field reference
        cyclicPointPatchField
        (
            const cyclicPointPatchField<Type>&,
            const DimensionedField<Type, pointMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual autoPtr<pointPatchField<Type> > clone
        (
            const DimensionedField<Type, pointMesh>& iF
        ) const
        {
            return autoPtr<pointPatchField<Type> >
            (
                new cyclicPointPatchField<Type>
                (
                    *this, iF
                )
            );
        }


    // Member functions

        //- Constraint handling

            //- Return the constraint type this pointPatchField implements
            virtual const word& constraintType() const
            {
                return cyclicPointPatch::typeName;
            }

        //- Cyclic coupled interface functions

            //- Does the patch field perform the transfromation
            virtual bool doTransform() const
            {
                return !(cyclicPatch_.parallel() || pTraits<Type>::rank == 0);
            }

            //- Return face transformation tensor
            virtual const tensorField& forwardT() const
            {
                return cyclicPatch_.forwardT();
            }

            //- Return neighbour-cell transformation tensor
            virtual const tensorField& reverseT() const
            {
                return cyclicPatch_.reverseT();
            }


        // Evaluation functions

            //- Evaluate the patch field
            virtual void evaluate
            (
                const Pstream::commsTypes commsType=Pstream::blocking
            )
            {}

            //- Complete swap of patch point values and add to local values
            virtual void swapAddSeparated
            (
                const Pstream::commsTypes commsType,
                Field<Type>&
            ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "Swap.hpp"
#include "transformField.hpp"
#include "pointFields.hpp"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
CML::cyclicPointPatchField<Type>::cyclicPointPatchField
(
    const pointPatch& p,
    const DimensionedField<Type, pointMesh>& iF
)
:
    coupledPointPatchField<Type>(p, iF),
    cyclicPatch_(refCast<const cyclicPointPatch>(p))
{}


template<class Type>
CML::cyclicPointPatchField<Type>::cyclicPointPatchField
(
    const pointPatch& p,
    const DimensionedField<Type, pointMesh>& iF,
    const dictionary& dict
)
:
    coupledPointPatchField<Type>(p, iF, dict),
    cyclicPatch_(refCast<const cyclicPointPatch>(p))
{
    if (!isType<cyclicPointPatch>(p))
    {
        FatalIOErrorInFunction(dict)
            << "patch " << this->patch().index() << " not cyclic type. "
            << "Patch type = " << p.type()
            << exit(FatalIOError);
    }
}


template<class Type>
CML::cyclicPointPatchField<Type>::cyclicPointPatchField
(
    const cyclicPointPatchField<Type>& ptf,
    const pointPatch& p,
    const DimensionedField<Type, pointMesh>& iF,
    const pointPatchFieldMapper& mapper
)
:
    coupledPointPatchField<Type>(ptf, p, iF, mapper),
    cyclicPatch_(refCast<const cyclicPointPatch>(p))
{
    if (!isType<cyclicPointPatch>(this->patch()))
    {
        FatalErrorInFunction
            << "Field type does not correspond to patch type for patch "
            << this->patch().index() << "." << endl
            << "Field type: " << typeName << endl
            << "Patch type: " << this->patch().type()
            << exit(FatalError);
    }
}


template<class Type>
CML::cyclicPointPatchField<Type>::cyclicPointPatchField
(
    const cyclicPointPatchField<Type>& ptf,
    const DimensionedField<Type, pointMesh>& iF
)
:
    coupledPointPatchField<Type>(ptf, iF),
    cyclicPatch_(ptf.cyclicPatch_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void CML::cyclicPointPatchField<Type>::swapAddSeparated
(
    const Pstream::commsTypes,
    Field<Type>& pField
) const
{
    // Get neighbouring pointPatch
    const cyclicPointPatch& nbrPatch = cyclicPatch_.neighbPatch();

    if (cyclicPatch_.cyclicPatch().owner())
    {
        // We inplace modify pField. To prevent the other side (which gets
        // evaluated at a later date) using already changed values we do
        // all swaps on the side that gets evaluated first.

        // Get neighbouring pointPatchField	
		const GeometricField<Type, pointPatchField, pointMesh>& fld =
            refCast<const GeometricField<Type, pointPatchField, pointMesh> >
            (
                this->dimensionedInternalField()
            );	
		
        const cyclicPointPatchField<Type>& nbr =
            refCast<const cyclicPointPatchField<Type> >
            (
                fld.boundaryField()[nbrPatch.index()]
            );


        Field<Type> pf(this->patchInternalField(pField));
        Field<Type> nbrPf(nbr.patchInternalField(pField));

        const edgeList& pairs = cyclicPatch_.transformPairs();

        if (doTransform())
        {
            // Transform both sides.
            forAll(pairs, pairi)
            {
                label pointi = pairs[pairi][0];
                label nbrPointi = pairs[pairi][1];

                Type tmp = pf[pointi];
                pf[pointi] = transform(forwardT()[0], nbrPf[nbrPointi]);
                nbrPf[nbrPointi] = transform(reverseT()[0], tmp);
            }
        }
        else
        {
            forAll(pairs, pairi)
            {
                Swap(pf[pairs[pairi][0]], nbrPf[pairs[pairi][1]]);
            }
        }
        this->addToInternalField(pField, pf);
        nbr.addToInternalField(pField, nbrPf);
    }
}



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
