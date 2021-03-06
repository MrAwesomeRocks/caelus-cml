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
    CML::fileFormats::OBJedgeFormat

Description
    Provide a means of reading/writing Alias/Wavefront OBJ format.

    Does not handle negative vertex indices.

SourceFiles
    OBJedgeFormat.cpp

\*---------------------------------------------------------------------------*/

#ifndef OBJedgeFormat_H
#define OBJedgeFormat_H

#include "edgeMesh.hpp"
#include "IFstream.hpp"
#include "Ostream.hpp"
#include "OFstream.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace fileFormats
{

/*---------------------------------------------------------------------------*\
                        Class OBJedgeFormat Declaration
\*---------------------------------------------------------------------------*/

class OBJedgeFormat
:
    public edgeMesh
{
    // Private Member Functions

        //- Disallow default bitwise copy construct
        OBJedgeFormat(const OBJedgeFormat&);

        //- Disallow default bitwise assignment
        void operator=(const OBJedgeFormat&);


public:

    // Constructors

        //- Construct from file name
        OBJedgeFormat(const fileName&);


    // Selectors

        //- Read file and return surface
        static autoPtr<edgeMesh> New(const fileName& name)
        {
            return autoPtr<edgeMesh>
            (
                new OBJedgeFormat(name)
            );
        }


    //- Destructor
    virtual ~OBJedgeFormat()
    {}


    // Member Functions

        //- Write surface mesh components by proxy
        static void write(const fileName&, const edgeMesh&);

        //- Read from file
        virtual bool read(const fileName&);

        //- Write object file
        virtual void write(const fileName& name) const
        {
            write(name, *this);
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace fileFormats
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
