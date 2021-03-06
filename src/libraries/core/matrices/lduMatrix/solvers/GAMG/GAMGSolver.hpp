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
    CML::GAMGSolver

Description
    Geometric agglomerated algebraic multigrid solver.

  Characteristics:
      - Requires positive definite, diagonally dominant matrix.
      - Agglomeration algorithm: selectable and optionally cached.
      - Restriction operator: summation.
      - Prolongation operator: injection.
      - Smoother: Gauss-Seidel.
      - Coarse matrix creation: central coefficient: summation of fine grid
        central coefficients with the removal of intra-cluster face;
        off-diagonal coefficient: summation of off-diagonal faces.
      - Coarse matrix scaling: performed by correction scaling, using steepest
        descent optimisation.
      - Type of cycle: V-cycle with optional pre-smoothing.
      - Coarsest-level matrix solved using ICCG or BICCG.

SourceFiles
    GAMGSolver.cpp
    GAMGSolverCalcAgglomeration.cpp
    GAMGSolverMakeCoarseMatrix.cpp
    GAMGSolverOperations.cpp
    GAMGSolverSolve.cpp

\*---------------------------------------------------------------------------*/

#ifndef GAMGSolver_H
#define GAMGSolver_H

#include "GAMGAgglomeration.hpp"
#include "lduMatrix.hpp"
#include "labelField.hpp"
#include "primitiveFields.hpp"
#include "LUscalarMatrix.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class GAMGSolver Declaration
\*---------------------------------------------------------------------------*/

class GAMGSolver
:
    public lduMatrix::solver
{
    // Private data

        bool cacheAgglomeration_;

        //- Number of pre-smoothing sweeps
        label nPreSweeps_;

        //- Number of post-smoothing sweeps
        label nPostSweeps_;

        //- Number of smoothing sweeps on finest mesh
        label nFinestSweeps_;

        //- Choose if the corrections should be scaled.
        //  By default corrections for symmetric matrices are scaled
        //  but not for asymmetric matrices.
        bool scaleCorrection_;

        //- Direct or iteratively solve the coarsest level
        bool directSolveCoarsest_;

        //- The agglomeration
        const GAMGAgglomeration& agglomeration_;

        //- Hierarchy of matrix levels
        PtrList<lduMatrix> matrixLevels_;

        //- Hierarchy of interfaces.
        //  Warning: Needs to be deleted explicitly.
        PtrList<lduInterfaceFieldPtrsList> interfaceLevels_;

        //- Hierarchy of interface boundary coefficients
        PtrList<FieldField<Field, scalar> > interfaceLevelsBouCoeffs_;

        //- Hierarchy of interface internal coefficients
        PtrList<FieldField<Field, scalar> > interfaceLevelsIntCoeffs_;

        //- LU decompsed coarsest matrix
        autoPtr<LUscalarMatrix> coarsestLUMatrixPtr_;


    // Private Member Functions

        //- Read control parameters from the control dictionary
        virtual void readControls();

        //- Simplified access to interface level
        const lduInterfaceFieldPtrsList& interfaceLevel
        (
            const label i
        ) const;

        //- Simplified access to matrix level
        const lduMatrix& matrixLevel(const label i) const;

        //- Simplified access to interface boundary coeffs level
        const FieldField<Field, scalar>& interfaceBouCoeffsLevel
        (
            const label i
        ) const;

        //- Simplified access to interface internal coeffs level
        const FieldField<Field, scalar>& interfaceIntCoeffsLevel
        (
            const label i
        ) const;

        //- Agglomerate coarse matrix
        void agglomerateMatrix(const label fineLevelIndex);

        //- Calculate and return the scaling factor from Acf, coarseSource
        //  and coarseField.
        //  At the same time do a Jacobi iteration on the coarseField using
        //  the Acf provided after the coarseField values are used for the
        //  scaling factor.
        scalar scalingFactor
        (
            scalarField& field,
            const scalarField& source,
            const scalarField& Acf,
            const scalarField& D
        ) const;

        //- Calculate Acf and calculate and return the scaling factor.
        scalar scalingFactor
        (
            scalarField& Acf,
            const lduMatrix& A,
            scalarField& field,
            const FieldField<Field, scalar>& interfaceLevelBouCoeffs,
            const lduInterfaceFieldPtrsList& interfaceLevel,
            const scalarField& source,
            const direction cmpt
        ) const;


        //- Initialise the data structures for the V-cycle
        void initVcycle
        (
            PtrList<scalarField>& coarseCorrFields,
            PtrList<scalarField>& coarseSources,
            PtrList<lduMatrix::smoother>& smoothers
        ) const;


        //- Perform a single GAMG V-cycle with pre, post and finest smoothing.
        void Vcycle
        (
            const PtrList<lduMatrix::smoother>& smoothers,
            scalarField& psi,
            const scalarField& source,
            scalarField& Apsi,
            scalarField& finestCorrection,
            scalarField& finestResidual,
            PtrList<scalarField>& coarseCorrFields,
            PtrList<scalarField>& coarseSources,
            const direction cmpt=0
        ) const;


        //- Solve the coarsest level with either an iterative or direct solver
        void solveCoarsestLevel
        (
            scalarField& coarsestCorrField,
            const scalarField& coarsestSource
        ) const;


public:

    friend class GAMGPreconditioner;

    //- Runtime type information
    TypeName("GAMG");


    // Constructors

        //- Construct from lduMatrix and solver controls
        GAMGSolver
        (
            const word& fieldName,
            const lduMatrix& matrix,
            const FieldField<Field, scalar>& interfaceBouCoeffs,
            const FieldField<Field, scalar>& interfaceIntCoeffs,
            const lduInterfaceFieldPtrsList& interfaces,
            const dictionary& solverControls
        );


    //- Destructor
    virtual ~GAMGSolver();


    // Member Functions

        //- Solve
        virtual solverPerformance solve
        (
            scalarField& psi,
            const scalarField& source,
            const direction cmpt=0
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
