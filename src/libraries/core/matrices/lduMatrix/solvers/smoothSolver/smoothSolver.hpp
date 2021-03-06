/*---------------------------------------------------------------------------*\
Copyright (C) 2011 OpenFOAM Foundation
Copyright (C) 2014 Applied CCM
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
    CML::smoothSolver

Description
    Iterative solver for symmetric and asymmetric matrices which uses a
    run-time selected smoother e.g. GaussSeidel to converge the solution to
    the required tolerance.

    To improve efficiency, the residual is evaluated after every nSweeps
    smoothing iterations.

SourceFiles
    smoothSolver.cpp

\*---------------------------------------------------------------------------*/

#ifndef smoothSolver_H
#define smoothSolver_H

#include "lduMatrix.hpp"

namespace CML
{

class smoothSolver
:
    public lduMatrix::solver
{
protected:

    // Protected data

        //- Number of sweeps before the evaluation of residual
        label nSweeps_;

        //- Read the control parameters from the controlDict_
        virtual void readControls();

public:

    //- Runtime type information
    TypeName("smoothSolver");


    // Constructors

        //- Construct from matrix components and solver controls
        smoothSolver
        (
            const word& fieldName,
            const lduMatrix& matrix,
            const FieldField<Field, scalar>& interfaceBouCoeffs,
            const FieldField<Field, scalar>& interfaceIntCoeffs,
            const lduInterfaceFieldPtrsList& interfaces,
            const dictionary& solverControls
        );


    //- Destructor
    virtual ~smoothSolver()
    {}

    // Member Functions

        //- Solve the matrix with this solver
        virtual solverPerformance solve
        (
            scalarField& x,
            const scalarField& b,
            const direction cmpt=0
        ) const;
};

} // End namespace CML

#endif

