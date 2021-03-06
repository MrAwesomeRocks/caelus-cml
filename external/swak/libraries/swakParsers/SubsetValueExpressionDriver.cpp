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

Contributors/Copyright:
    2010-2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#include "SubsetValueExpressionDriver.hpp"

#include "Random.hpp"

namespace CML {

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(SubsetValueExpressionDriver, 0);

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //


SubsetValueExpressionDriver::SubsetValueExpressionDriver(const SubsetValueExpressionDriver& orig)
:
    CommonValueExpressionDriver(orig),
    autoInterpolate_(orig.autoInterpolate_),
    warnAutoInterpolate_(orig.warnAutoInterpolate_)
{}

SubsetValueExpressionDriver::SubsetValueExpressionDriver(const dictionary& dict)
:
    CommonValueExpressionDriver(dict),
    autoInterpolate_(dict.lookupOrDefault("autoInterpolate",false)),
    warnAutoInterpolate_(dict.lookupOrDefault("warnAutoInterpolate",true))
{}

SubsetValueExpressionDriver::SubsetValueExpressionDriver(
        bool autoInterpolate,
        bool warnAutoInterpolate
)
:
    CommonValueExpressionDriver(),
    autoInterpolate_(autoInterpolate),
    warnAutoInterpolate_(warnAutoInterpolate)
{}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

SubsetValueExpressionDriver::~SubsetValueExpressionDriver()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void SubsetValueExpressionDriver::setAutoInterpolate(
    bool autoInterpolate,
    bool warnAutoInterpolate
)
{
    autoInterpolate_=autoInterpolate;
    warnAutoInterpolate_=warnAutoInterpolate;
}

void SubsetValueExpressionDriver::parseInternal (int startToken)
{
    parserSubset::SubsetValueExpressionParser parser (
        scanner_,
        *this,
        startToken,
        0
    );
    parser.set_debug_level (traceParsing());
    parser.parse ();
}

tmp<scalarField> SubsetValueExpressionDriver::makeIdField()
{
    tmp<scalarField> ids(
        new scalarField(this->size())
    );
    forAll(ids(),i) {
        ids()[i]=i;
    }
    return ids;
}

tmp<vectorField> SubsetValueExpressionDriver::makePositionField() const
{
    NotImplemented;

    return tmp<vectorField>(new vectorField(0));
}

// tmp<vectorField> SubsetValueExpressionDriver::makePointField() const
// {
//     NotImplemented;
// }

tmp<vectorField> SubsetValueExpressionDriver::makeFaceNormalField() const
{
    NotImplemented;

    return tmp<vectorField>(new vectorField(0));
}

tmp<vectorField> SubsetValueExpressionDriver::makeFaceAreaField() const
{
    NotImplemented;

    return tmp<vectorField>(new vectorField(0));
}

template<>
SubsetValueExpressionDriver::SymbolTable<SubsetValueExpressionDriver>::SymbolTable()
:
StartupSymbols()
{
    // default value
    insert("",parserSubset::SubsetValueExpressionParser::token::START_DEFAULT);

    insert(
        "scalar_SC",
        parserSubset::SubsetValueExpressionParser::token::START_FACE_SCALAR_COMMA
    );
    insert(
        "scalar_CL",
        parserSubset::SubsetValueExpressionParser::token::START_FACE_SCALAR_CLOSE
    );
    insert(
        "point_scalar_SC",
        parserSubset::SubsetValueExpressionParser::token::START_POINT_SCALAR_COMMA
    );
    insert(
        "point_scalar_CL",
        parserSubset::SubsetValueExpressionParser::token::START_POINT_SCALAR_CLOSE
    );
    insert(
        "vector_SC",
        parserSubset::SubsetValueExpressionParser::token::START_FACE_VECTOR_COMMA
    );
    insert(
        "vector_CL",
        parserSubset::SubsetValueExpressionParser::token::START_FACE_VECTOR_CLOSE
    );
    insert(
        "point_vector_SC",
        parserSubset::SubsetValueExpressionParser::token::START_POINT_VECTOR_COMMA
    );
    insert(
        "point_vector_CL",
        parserSubset::SubsetValueExpressionParser::token::START_POINT_VECTOR_CLOSE
    );
    insert(
        "tensor_SC",
        parserSubset::SubsetValueExpressionParser::token::START_FACE_TENSOR_COMMA
    );
    insert(
        "tensor_CL",
        parserSubset::SubsetValueExpressionParser::token::START_FACE_TENSOR_CLOSE
    );
    insert(
        "point_tensor_SC",
        parserSubset::SubsetValueExpressionParser::token::START_POINT_TENSOR_COMMA
    );
    insert(
        "point_tensor_CL",
        parserSubset::SubsetValueExpressionParser::token::START_POINT_TENSOR_CLOSE
    );
    insert(
        "symmTensor_SC",
        parserSubset::SubsetValueExpressionParser::token::START_FACE_YTENSOR_COMMA
    );
    insert(
        "symmTensor_CL",
        parserSubset::SubsetValueExpressionParser::token::START_FACE_YTENSOR_CLOSE
    );
    insert(
        "point_symmTensor_SC",
        parserSubset::SubsetValueExpressionParser::token::START_POINT_YTENSOR_COMMA
    );
    insert(
        "point_symmTensor_CL",
        parserSubset::SubsetValueExpressionParser::token::START_POINT_YTENSOR_CLOSE
    );
    insert(
        "sphericalTensor_SC",
        parserSubset::SubsetValueExpressionParser::token::START_FACE_HTENSOR_COMMA
    );
    insert(
        "sphericalTensor_CL",
        parserSubset::SubsetValueExpressionParser::token::START_FACE_HTENSOR_CLOSE
    );
    insert(
        "point_sphericalTensor_SC",
        parserSubset::SubsetValueExpressionParser::token::START_POINT_HTENSOR_COMMA
    );
    insert(
        "point_sphericalTensor_CL",
        parserSubset::SubsetValueExpressionParser::token::START_POINT_HTENSOR_CLOSE
    );
    insert(
        "logical_SC",
        parserSubset::SubsetValueExpressionParser::token::START_FACE_LOGICAL_COMMA
    );
    insert(
        "logical_CL",
        parserSubset::SubsetValueExpressionParser::token::START_FACE_LOGICAL_CLOSE
    );
    insert(
        "point_logical_SC",
        parserSubset::SubsetValueExpressionParser::token::START_POINT_LOGICAL_COMMA
    );
    insert(
        "point_logical_CL",
        parserSubset::SubsetValueExpressionParser::token::START_POINT_LOGICAL_CLOSE
    );

    insert(
        "CL",
        parserSubset::SubsetValueExpressionParser::token::START_CLOSE_ONLY
    );
    insert(
        "SC",
        parserSubset::SubsetValueExpressionParser::token::START_COMMA_ONLY
    );
}

const SubsetValueExpressionDriver::SymbolTable<SubsetValueExpressionDriver> &SubsetValueExpressionDriver::symbolTable()
{
    static SymbolTable<SubsetValueExpressionDriver> actualTable;

    return actualTable;
}

int SubsetValueExpressionDriver::startupSymbol(const word &name) {
    return symbolTable()[name];
}

// ************************************************************************* //

} // namespace
