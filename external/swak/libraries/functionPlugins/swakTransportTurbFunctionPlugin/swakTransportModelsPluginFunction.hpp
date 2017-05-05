/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright (C) 1991-2008 OpenCFD Ltd.
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
    CML::swakTransportModelsPluginFunction

Description

SourceFiles
    swakTransportModelsPluginFunction.cpp

Contributors/Copyright:
    2012-2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef swakTransportModelsPluginFunction_H
#define swakTransportModelsPluginFunction_H

#include "FieldValuePluginFunction.hpp"

#include "runTimeSelectionTables.hpp"

#include "transportModel.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class swakTransportModelsPluginFunction Declaration
\*---------------------------------------------------------------------------*/
class swakTransportModelsPluginFunction
:
    public FieldValuePluginFunction
{
    //- Disallow default bitwise assignment
    void operator=(const swakTransportModelsPluginFunction &);

    swakTransportModelsPluginFunction(const swakTransportModelsPluginFunction &);

protected:
    static const transportModel &transportInternal(const fvMesh &reg);

    const transportModel &transport();

public:
    swakTransportModelsPluginFunction(
        const FieldValueExpressionDriver &parentDriver,
        const word &name,
        const word &returnValueType
    );

    virtual ~swakTransportModelsPluginFunction() {}

    TypeName("swakTransportModelsPluginFunction");

};



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
