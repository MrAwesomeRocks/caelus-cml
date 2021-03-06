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
    CML::diagonalSolver

Description
    CML::diagonalSolver

SourceFiles
    diagonalSolver.cpp

\*---------------------------------------------------------------------------*/

#ifndef diagonalSolver_H
#define diagonalSolver_H

#include "lduMatrix.hpp"

namespace CML
{

class diagonalSolver
:
    public lduMatrix::solver
{
    // Private Member Functions

        //- Disallow default bitwise copy construct
        diagonalSolver(const diagonalSolver&);

        //- Disallow default bitwise assignment
        void operator=(const diagonalSolver&);


public:

    //- Runtime type information
    TypeName("diagonal");


    // Constructors

        //- Construct from matrix and solver controls
        diagonalSolver
        (
            const word& fieldName,
            const lduMatrix& matrix,
            const FieldField<Field, scalar>& interfaceBouCoeffs,
            const FieldField<Field, scalar>& interfaceIntCoeffs,
            const lduInterfaceFieldPtrsList& interfaces,
            const dictionary& solverControls
        );


    // Member Functions

        //- Read and reset the solver parameters from the given stream
        void read(const dictionary&)
        {}

        //- Solve the matrix with this solver
        solverPerformance solve
        (
            scalarField& x,
            const scalarField& b,
            const direction cmpt=0
        ) const;
};


}

#endif

