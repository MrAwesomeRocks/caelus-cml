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

Contributors/Copyright:
    2008-2011, 2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#include "patchFieldFlowFunctionObject.hpp"
#include "addToRunTimeSelectionTable.hpp"

#include "volFields.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(patchFieldFlowFunctionObject, 0);

    addToRunTimeSelectionTable
    (
        functionObject,
        patchFieldFlowFunctionObject,
        dictionary
    );



// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

patchFieldFlowFunctionObject::patchFieldFlowFunctionObject
(
    const word& name,
    const Time& t,
    const dictionary& dict
)
:
    patchFieldFunctionObject(name,t,dict),
    solver_(obr_,dict)
{
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

word patchFieldFlowFunctionObject::dirName()
{
    return typeName;
}

scalarField patchFieldFlowFunctionObject::process(const word& fieldName,scalar preset)
{
    return fieldFlow(fieldName,preset);
}

Field<vector> patchFieldFlowFunctionObject::process(const word& fieldName,vector preset)
{
    return fieldFlow(fieldName,preset);
}

Field<sphericalTensor> patchFieldFlowFunctionObject::process(const word& fieldName,sphericalTensor preset)
{
    return fieldFlow(fieldName,preset);
}

Field<symmTensor> patchFieldFlowFunctionObject::process(const word& fieldName,symmTensor preset)
{
    return fieldFlow(fieldName,preset);
}

Field<tensor> patchFieldFlowFunctionObject::process(const word& fieldName,tensor preset)
{
    return fieldFlow(fieldName,preset);
}


} // namespace CML

// ************************************************************************* //
