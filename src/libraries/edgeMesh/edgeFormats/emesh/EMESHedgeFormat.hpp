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
    CML::fileFormats::EMESHedgeFormat

Description
    Provide a means of reading/writing the single-file CAELUS edge format.

Note
   This class provides more methods than the regular edge format interface.

SourceFiles
    EMESHedgeFormat.cpp

\*---------------------------------------------------------------------------*/

#ifndef EMESHedgeFormat_H
#define EMESHedgeFormat_H

#include "edgeMesh.hpp"
#include "Ostream.hpp"
#include "OFstream.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace fileFormats
{

/*---------------------------------------------------------------------------*\
                      Class EMESHedgeFormat Declaration
\*---------------------------------------------------------------------------*/

class EMESHedgeFormat
:
    public edgeMesh
{
    // Private Member Functions

        //- Disallow default bitwise copy construct
        EMESHedgeFormat(const EMESHedgeFormat&);

        //- Disallow default bitwise assignment
        void operator=(const EMESHedgeFormat&);


protected:

    // Protected Member Functions

        //- Write header information
        static void writeHeader
        (
            Ostream&,
            const pointField&,
            const edgeList&
        );


public:

    // Constructors

        //- Construct from file name
        EMESHedgeFormat(const fileName&);


    // Selectors

        //- Read file and return edgeMesh
        static autoPtr<edgeMesh> New(const fileName& name)
        {
            return autoPtr<edgeMesh>
            (
                new EMESHedgeFormat(name)
            );
        }


    //- Destructor
    virtual ~EMESHedgeFormat()
    {}


    // Member Functions

        //- Read edgeMesh components from stream
        static bool read
        (
            Istream&,
            pointField&,
            edgeList&
        );

        //- Write edgeMesh components to stream
        static Ostream& write
        (
            Ostream&,
            const pointField&,
            const edgeList&
        );

        //- Write edgeMesh with a mimicked IOobject header
        static void write(const fileName&, const edgeMesh&);

        //- Read from file
        virtual bool read(const fileName&);

        //- Write object
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
