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
    CML::maxDeltaxyz

Description
    maxDeltaxyz takes the maximum of the three dimensions per cell:
    max(hx, hy, hz). Valid for structures hexahedral cells only.


SourceFiles
    maxDeltaxyz.cpp

\*---------------------------------------------------------------------------*/

#ifndef maxDeltaxyzDelta_H
#define maxDeltaxyzDelta_H

#include "LESdelta.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class maxDeltaxyz Declaration
\*---------------------------------------------------------------------------*/

class maxDeltaxyz
:
    public LESdelta
{
    // Private data

        scalar deltaCoeff_; //


    // Private Member Functions

        //- Disallow default bitwise copy construct and assignment
        maxDeltaxyz(const maxDeltaxyz&);
        void operator=(const maxDeltaxyz&);

        // Calculate the delta values
        void calcDelta();


public:

    //- Runtime type information
    TypeName("maxDeltaxyz");


    // Constructors

        //- Construct from name, mesh and IOdictionary
        maxDeltaxyz
        (
            const word& name,
            const fvMesh& mesh,
            const dictionary&
        );


    //- Destructor
    virtual ~maxDeltaxyz()
    {}


    // Member Functions

        //- Read the LESdelta dictionary
        virtual void read(const dictionary&);

        // Correct values
        virtual void correct();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


#endif

// ************************************************************************* //
