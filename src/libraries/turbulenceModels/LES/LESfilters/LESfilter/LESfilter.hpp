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
    CML::LESfilter

Description
    Abstract class for LES filters

SourceFiles
    LESfilter.cpp
    newFilter.cpp

\*---------------------------------------------------------------------------*/

#ifndef LESfilter_H
#define LESfilter_H

#include "volFields.hpp"
#include "typeInfo.hpp"
#include "autoPtr.hpp"
#include "runTimeSelectionTables.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

class fvMesh;

/*---------------------------------------------------------------------------*\
                           Class LESfilter Declaration
\*---------------------------------------------------------------------------*/

class LESfilter
{
    // Private data

        const fvMesh& mesh_;


    // Private Member Functions

        // Disallow default bitwise copy construct and assignment
        LESfilter(const LESfilter&);
        void operator=(const LESfilter&);


public:

    //- Runtime type information
    TypeName("LESfilter");


    // Declare run-time constructor selection table

        declareRunTimeSelectionTable
        (
            autoPtr,
            LESfilter,
            dictionary,
            (
                const fvMesh& mesh,
                const dictionary& LESfilterDict
            ),
            (mesh, LESfilterDict)
        );


    // Constructors

        //- Construct from components
        LESfilter(const fvMesh& mesh)
        :
            mesh_(mesh)
        {}


    // Selectors

        //- Return a reference to the selected LES filter
        static autoPtr<LESfilter> New
        (
            const fvMesh&,
            const dictionary&
        );


    //- Destructor
    virtual ~LESfilter()
    {}


    // Member Functions

        //- Return mesh reference
        const fvMesh& mesh() const
        {
            return mesh_;
        }

        //- Read the LESfilter dictionary
        virtual void read(const dictionary&) = 0;


    // Member Operators

        virtual tmp<volScalarField> operator()
        (
            const tmp<volScalarField>&
        ) const = 0;

        virtual tmp<volVectorField> operator()
        (
            const tmp<volVectorField>&
        ) const = 0;

        virtual tmp<volSymmTensorField> operator()
        (
            const tmp<volSymmTensorField>&
        ) const = 0;

        virtual tmp<volTensorField> operator()
        (
            const tmp<volTensorField>&
        ) const = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
