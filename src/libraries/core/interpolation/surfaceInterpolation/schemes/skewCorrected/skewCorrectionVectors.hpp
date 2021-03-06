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
    CML::skewCorrectionVectors

Description
    Skew-correction vectors for the skewness-corrected interpolation scheme

SourceFiles
    skewCorrectionVectors.cpp

\*---------------------------------------------------------------------------*/

#ifndef skewCorrectionVectors_H
#define skewCorrectionVectors_H

#include "MeshObject.hpp"
#include "fvMesh.hpp"
#include "surfaceFieldsFwd.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

class fvMesh;

/*---------------------------------------------------------------------------*\
                    Class skewCorrectionVectors Declaration
\*---------------------------------------------------------------------------*/

class skewCorrectionVectors
:
    public MeshObject<fvMesh, skewCorrectionVectors>
{
    // Private data

        //- Is mesh skew
        mutable bool skew_;

        //- Skew correction vectors
        mutable surfaceVectorField* skewCorrectionVectors_;

        //- Construct skewness correction vectors
        void makeSkewCorrectionVectors() const;


public:

    TypeName("skewCorrectionVectors");


    // Constructors

        explicit skewCorrectionVectors(const fvMesh& mesh);


    //- Destructor
    virtual ~skewCorrectionVectors();


    // Member functions

        //- Return whether mesh is skew or not
        bool skew() const;

        //- Return reference to skew vectors array
        const surfaceVectorField& operator()() const;

        //- Delete the correction vectors when the mesh moves
        virtual bool movePoints();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
