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
    CML::fileFormats::edgeFormatsCore

Description
    A collection of helper functions for reading/writing edge formats.

SourceFiles
    edgeFormatsCore.C

\*---------------------------------------------------------------------------*/

#ifndef edgeFormatsCore_H
#define edgeFormatsCore_H

#include "Map.hpp"
#include "HashSet.hpp"
#include "labelList.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes

class IFstream;
class Time;

namespace fileFormats
{

/*---------------------------------------------------------------------------*\
                       Class edgeFormatsCore Declaration
\*---------------------------------------------------------------------------*/

class edgeFormatsCore
{
protected:

    // Protected Member Functions

        //- Read non-comment line
        static string getLineNoComment(IFstream&);

public:

    // Static Data

        //- The file extension corresponding to 'native' edge format
        //  Normally "eMesh" (edge-mesh)
        static word nativeExt;

    // Static Member Functions

        static bool checkSupport
        (
            const wordHashSet& available,
            const word& ext,
            const bool verbose,
            const word& functionName
        );

       // //- Return the local file name (within time directory)
       // //  NEEDS FIXING
       // static fileName localMeshFileName(const word& edgeName="");

       // //- Find instance with edgeName
       // //  NEEDS FIXING
       // static fileName findMeshInstance
       // (
       //     const Time&,
       //     const word& edgeName=""
       // );

       // //- Find mesh file with edgeName
       // //  NEEDS FIXING
       // static fileName findMeshFile(const Time&, const word& edgeName="");


    // Constructors

        //- Construct null
        edgeFormatsCore();


    //- Destructor
    virtual ~edgeFormatsCore();

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace fileFormats
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
