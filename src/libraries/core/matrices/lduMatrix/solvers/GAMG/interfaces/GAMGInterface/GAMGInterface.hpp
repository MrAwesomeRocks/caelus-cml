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
    CML::GAMGInterface

Description
     Abstract base class for GAMG agglomerated interfaces.

SourceFiles
    GAMGInterface.cpp
    newAmgInterface.cpp

\*---------------------------------------------------------------------------*/

#ifndef GAMGInterface_H
#define GAMGInterface_H

#include "autoPtr.hpp"
#include "lduInterfacePtrsList.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                      Class GAMGInterface Declaration
\*---------------------------------------------------------------------------*/

class GAMGInterface
:
    public lduInterface
{

protected:

    // Protected data

        //- My index in coarseInterfaces
        const label index_;

        //- All interfaces
        const lduInterfacePtrsList& coarseInterfaces_;

        //- Face-cell addressing
        labelField faceCells_;

        //- Face restrict addressing
        labelField faceRestrictAddressing_;


private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        GAMGInterface(const GAMGInterface&);

        //- Disallow default bitwise assignment
        void operator=(const GAMGInterface&);


public:

    //- Runtime type information
    TypeName("GAMGInterface");


    // Declare run-time constructor selection tables

        declareRunTimeSelectionTable
        (
            autoPtr,
            GAMGInterface,
            lduInterface,
            (
                const label index,
                const lduInterfacePtrsList& coarseInterfaces,
                const lduInterface& fineInterface,
                const labelField& localRestrictAddressing,
                const labelField& neighbourRestrictAddressing
            ),
            (
                index,
                coarseInterfaces,
                fineInterface,
                localRestrictAddressing,
                neighbourRestrictAddressing
            )
        );


    // Selectors

        //- Return a pointer to a new interface created on freestore given
        //  the fine interface
        static autoPtr<GAMGInterface> New
        (
            const label index,
            const lduInterfacePtrsList& coarseInterfaces,
            const lduInterface& fineInterface,
            const labelField& localRestrictAddressing,
            const labelField& neighbourRestrictAddressing
        );


    // Constructors

        //- Construct from fine-level interface,
        //  local and neighbour restrict addressing
        GAMGInterface
        (
            const label index,
            const lduInterfacePtrsList& coarseInterfaces,
            const lduInterface&,
            const labelField&,
            const labelField&
        )
        :
            index_(index),
            coarseInterfaces_(coarseInterfaces)
        {}


    // Member Functions

        // Access

            //- Return size
            virtual label size() const
            {
                return faceCells_.size();
            }

            virtual label index() const
            {
                return index_;
            }

            virtual const lduInterfacePtrsList& coarseInterfaces() const
            {
                return coarseInterfaces_;
            }

            //- Return faceCell addressing
            virtual const labelUList& faceCells() const
            {
                return faceCells_;
            }

            //- Return face restrict addressing
            virtual const labelList& faceRestrictAddressing() const
            {
                return faceRestrictAddressing_;
            }

            //- Return non-const access to face restrict addressing
            virtual labelList& faceRestrictAddressing()
            {
                return faceRestrictAddressing_;
            }

            //- Return the interface internal field of the given field
            template<class Type>
            tmp<Field<Type> > interfaceInternalField
            (
                const UList<Type>& internalData
            ) const;

            //- Return the values of the given internal data adjacent to
            //  the interface as a field
            virtual tmp<labelField> interfaceInternalField
            (
                const labelUList& internalData
            ) const;


        // Agglomeration

            //- Merge the next level with this level
            //  combining the face-restrict addressing
            //  and copying the face-cell addressing
            void combine(const GAMGInterface&);

            //- Agglomerating the given fine-level coefficients and return
            virtual tmp<scalarField> agglomerateCoeffs
            (
                const scalarField& fineCoeffs
            ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
CML::tmp<CML::Field<Type> > CML::GAMGInterface::interfaceInternalField
(
    const UList<Type>& iF
) const
{
    tmp<Field<Type> > tresult(new Field<Type>(size()));
    Field<Type>& result = tresult();

    forAll(result, elemI)
    {
        result[elemI] = iF[faceCells_[elemI]];
    }

    return tresult;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
