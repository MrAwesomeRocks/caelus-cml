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
    recalcThermoHFunctionObject

Description
    Recalculate enthalpy according to the current temperature

SourceFiles
    recalcThermoHFunctionObject.cpp

Contributors/Copyright:
    2008-2011, 2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef recalcThermoHFunctionObject_H
#define recalcThermoHFunctionObject_H

#include "updateSimpleFunctionObject.hpp"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class recalcThermoHFunctionObject Declaration
\*---------------------------------------------------------------------------*/

class recalcThermoHFunctionObject
:
    public updateSimpleFunctionObject
{
    // Private Member Functions

    //- Disallow default bitwise copy construct
    recalcThermoHFunctionObject(const recalcThermoHFunctionObject&);

    //- Disallow default bitwise assignment
    void operator=(const recalcThermoHFunctionObject&);

protected:
    //- do the actual calculation
    void recalc();

public:

    //- Runtime type information
    TypeName("recalcThermoH");

    // Constructors

    //- Construct from components
    recalcThermoHFunctionObject
    (
        const word&,
        const Time&,
        const dictionary&
    );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
