/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright  held by original author
-------------------------------------------------------------------------------
License
    This file is based on CAELUS.

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
    readGravitationFunctionObject

Description
    Read the gravitational acceleration. This is done to satisfy boundary
    conditions that rely on it for solvers/utilities that don't provide it

SourceFiles
    readGravitationFunctionObject.cpp

Contributors/Copyright:
    2008-2011, 2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef readGravitationFunctionObject_H
#define readGravitationFunctionObject_H

#include "simpleFunctionObject.hpp"
#include "uniformDimensionedFields.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class readGravitationFunctionObject Declaration
\*---------------------------------------------------------------------------*/

class readGravitationFunctionObject
:
    public simpleFunctionObject
{
    // Private Member Functions

    //- Disallow default bitwise copy construct
    readGravitationFunctionObject(const readGravitationFunctionObject&);
    
    //- Disallow default bitwise assignment
    void operator=(const readGravitationFunctionObject&);

    //- the actual field
    uniformDimensionedVectorField g_;

public:

    //- Runtime type information
    TypeName("readGravitation");


    // Constructors

    //- Construct from components
    readGravitationFunctionObject
    (
        const word&,
        const Time&,
        const dictionary&
    );

    void write();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
