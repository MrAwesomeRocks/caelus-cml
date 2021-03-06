/*---------------------------------------------------------------------------*\
Copyright (C) 2013 OpenFOAM Foundation
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
    CML::cyclicACMIGAMGInterfaceField

Description
    GAMG agglomerated cyclic interface for Arbitrarily Coupled Mesh Interface
    (ACMI) fields.

SourceFiles
    cyclicACMIGAMGInterfaceField.C

\*---------------------------------------------------------------------------*/

#ifndef cyclicACMIGAMGInterfaceField_H
#define cyclicACMIGAMGInterfaceField_H

#include "GAMGInterfaceField.hpp"
#include "cyclicACMIGAMGInterface.hpp"
#include "cyclicACMILduInterfaceField.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                 Class cyclicACMIGAMGInterfaceField Declaration
\*---------------------------------------------------------------------------*/

class cyclicACMIGAMGInterfaceField
:
    public GAMGInterfaceField,
    virtual public cyclicACMILduInterfaceField
{
    // Private data

        //- Local reference cast into the cyclic interface
        const cyclicACMIGAMGInterface& cyclicACMIInterface_;

        //- Is the transform required
        bool doTransform_;

        //- Rank of component for transformation
        int rank_;


    // Private Member Functions

        //- Disallow default bitwise copy construct
        cyclicACMIGAMGInterfaceField(const cyclicACMIGAMGInterfaceField&);

        //- Disallow default bitwise assignment
        void operator=(const cyclicACMIGAMGInterfaceField&);


public:

    //- Runtime type information
    TypeName("cyclicACMI");


    // Constructors

        //- Construct from GAMG interface and fine level interface field
        cyclicACMIGAMGInterfaceField
        (
            const GAMGInterface& GAMGCp,
            const lduInterfaceField& fineInterfaceField
        );

        //- Construct from GAMG interface and fine level interface field
        cyclicACMIGAMGInterfaceField
        (
            const GAMGInterface& GAMGCp,
            const bool doTransform,
            const int rank
        );


    //- Destructor
    virtual ~cyclicACMIGAMGInterfaceField();


    // Member Functions

        // Access

            //- Return size
            label size() const
            {
                return cyclicACMIInterface_.size();
            }


        // Interface matrix update

            //- Update result field based on interface functionality
            virtual void updateInterfaceMatrix
            (
                const scalarField& psiInternal,
                scalarField& result,
                const lduMatrix&,
                const scalarField& coeffs,
                const direction cmpt,
                const Pstream::commsTypes commsType
            ) const;


        //- Cyclic interface functions

            //- Does the interface field perform the transfromation
            virtual bool doTransform() const
            {
                return doTransform_;
            }

            //- Return face transformation tensor
            virtual const tensorField& forwardT() const
            {
                return cyclicACMIInterface_.forwardT();
            }

            //- Return neighbour-cell transformation tensor
            virtual const tensorField& reverseT() const
            {
                return cyclicACMIInterface_.reverseT();
            }

            //- Return rank of component for transform
            virtual int rank() const
            {
                return rank_;
            }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
