/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
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
    CML::surfaceInterpolation

Description
    Cell to surface interpolation scheme. Included in fvMesh.

SourceFiles
    surfaceInterpolation.cpp

\*---------------------------------------------------------------------------*/

#ifndef surfaceInterpolation_H
#define surfaceInterpolation_H

#include "tmp.hpp"
#include "scalar.hpp"
#include "volFieldsFwd.hpp"
#include "surfaceFieldsFwd.hpp"
#include "className.hpp"
#include "FieldField.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                     Class surfaceInterpolation Declaration
\*---------------------------------------------------------------------------*/

class surfaceInterpolation
{
    // Private data

        // Reference to fvMesh
        const fvMesh& mesh_;

        // Demand-driven data

            //- Linear difference weighting factors
            mutable surfaceScalarField* weights_;

            //- Cell-centre difference coefficients
            mutable surfaceScalarField* deltaCoeffs_;

            //- Non-orthogonal cell-centre difference coefficients
            mutable surfaceScalarField* nonOrthDeltaCoeffs_;

            //- Non-orthogonality correction vectors
            mutable surfaceVectorField* nonOrthCorrectionVectors_;
			
			//- Boundary values of correction vectors for non-orthogonality with fixedValue condition
			mutable FieldField<fvsPatchField, vector>* fvNonOrthCorrectionVectors_;


    // Private Member Functions

        //- Construct central-differencing weighting factors
        void makeWeights() const;

        //- Construct face-gradient difference factors
        void makeDeltaCoeffs() const;

        //- Construct face-gradient difference factors
        void makeNonOrthDeltaCoeffs() const;

        //- Construct non-orthogonality correction vectors
        void makeNonOrthCorrectionVectors() const;


protected:

    // Protected Member Functions

        // Storage management

            //- Clear all geometry and addressing
            void clearOut();


public:

    // Declare name of the class and its debug switch
    ClassName("surfaceInterpolation");


    // Constructors

        //- Construct given an fvMesh
        explicit surfaceInterpolation(const fvMesh&);


    //- Destructor
    ~surfaceInterpolation();


    // Member functions

        //- Return reference to linear difference weighting factors
        const surfaceScalarField& weights() const;

        //- Return reference to cell-centre difference coefficients
        const surfaceScalarField& deltaCoeffs() const;

        //- Return reference to non-orthogonal cell-centre difference
        //  coefficients
        const surfaceScalarField& nonOrthDeltaCoeffs() const;

        //- Return reference to non-orthogonality correction vectors
        const surfaceVectorField& nonOrthCorrectionVectors() const;
		
		//- Return reference to non-orthogonality correction vectors at boundaries
        //	for fixed value conditions
        const FieldField<fvsPatchField, vector>& fvNonOrthCorrectionVectors() const;

        //- Do what is necessary if the mesh has moved
        bool movePoints();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
