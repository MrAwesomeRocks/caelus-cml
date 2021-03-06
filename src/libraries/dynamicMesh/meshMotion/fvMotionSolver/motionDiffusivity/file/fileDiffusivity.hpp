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
    CML::fileDiffusivity

Description
    Motion diffusivity read from given file name.

SourceFiles
    fileDiffusivity.cpp

\*---------------------------------------------------------------------------*/

#ifndef fileDiffusivity_H
#define fileDiffusivity_H

#include "motionDiffusivity.hpp"
#include "surfaceFields.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class fileDiffusivity Declaration
\*---------------------------------------------------------------------------*/

class fileDiffusivity
:
    public motionDiffusivity
{

protected:

    // Protected data

        surfaceScalarField faceDiffusivity_;


private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        fileDiffusivity(const fileDiffusivity&);

        //- Disallow default bitwise assignment
        void operator=(const fileDiffusivity&);


public:

    //- Runtime type information
    TypeName("file");


    // Constructors

        //- Construct for the given fvMesh and data Istream
        fileDiffusivity(const fvMesh& mesh, Istream& mdData);


    //- Destructor
    virtual ~fileDiffusivity();


    // Member Functions

        //- Return diffusivity field
        virtual tmp<surfaceScalarField> operator()() const
        {
            return faceDiffusivity_;
        }

        //- Do not correct the motion diffusivity
        virtual void correct()
        {}
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
