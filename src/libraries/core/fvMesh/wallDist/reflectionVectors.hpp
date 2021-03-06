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
    CML::reflectionVectors

Description
    Container for reflection vectors (= unit normal of nearest wall)
SourceFiles
    reflectionVectors.cpp

\*---------------------------------------------------------------------------*/

#ifndef reflectionVectors_H
#define reflectionVectors_H

#include "volFields.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class reflectionVectors Declaration
\*---------------------------------------------------------------------------*/

class reflectionVectors
{

protected:

    // Protected data

        volVectorField n_;


private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        reflectionVectors(const reflectionVectors&);

        //- Disallow default bitwise assignment
        void operator=(const reflectionVectors&);


public:

    // Constructors

        //- Construct from mesh
        reflectionVectors(const fvMesh& mesh);


    // Member Functions

        const volVectorField& n() const
        {
            return n_;
        }


        //- Correct for mesh geom/topo changes
        void correct();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
