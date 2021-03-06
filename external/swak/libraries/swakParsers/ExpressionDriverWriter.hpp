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
    CML::ExpressionDriverWriter


Description

SourceFiles
    ExpressionDriverWriter.cpp

Contributors/Copyright:
    2011, 2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef ExpressionDriverWriter_H
#define ExpressionDriverWriter_H

#include "regIOobject.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class ExpressionDriverWriter
 Declaration
\*---------------------------------------------------------------------------*/

class CommonValueExpressionDriver;

class ExpressionDriverWriter
:
    public regIOobject
{
    //- the driver whos data should be written
    CommonValueExpressionDriver &driver_;

    // Constructors

    //- Construct null
    ExpressionDriverWriter();

public:
     //- Runtime type information
    TypeName("ExpressionDriverWriter");

    //- constructor
    ExpressionDriverWriter(
        const word &name,
        CommonValueExpressionDriver &driver
    );

    //- Destructor

    virtual ~ExpressionDriverWriter();

    virtual bool writeData(Ostream &) const ;

    virtual bool readData(Istream &);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
