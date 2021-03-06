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
    CML::forceEquation

Description
    Force a fvMatrix to fixed values in specific places

SourceFiles
    forceEquation.cpp

Contributors/Copyright:
    2011, 2013-2014 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef forceEquation_H
#define forceEquation_H

#include "FieldValueExpressionDriver.hpp"

#include "fvMatrix.hpp"

#include "DynamicList.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class forceEquation Declaration
\*---------------------------------------------------------------------------*/

template<class T>
class forceEquation
:
    protected FieldValueExpressionDriver
{
    // Private data

    forceEquation(const forceEquation&);

    exprString valueExpression_;

    exprString maskExpression_;

    bool verbose_;

    bool getMask(DynamicList<label> &,const word &psi);

public:

    // Constructors

        //- Construct from a dictionary
        forceEquation
        (
            const dictionary& ,
            const fvMesh&
        );

    // Destructor

        virtual ~forceEquation();

    //- fix equations
    void operator()(fvMatrix<T> &);

    //- where are the equations fixed
    tmp<volScalarField> getMask();

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
