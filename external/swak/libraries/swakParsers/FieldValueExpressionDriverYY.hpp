/*----------------------- -*- C++ -*- ---------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
-------------------------------------------------------------------------------
License
    This file is part of swak.

    swak is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    swak is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with swak.  If not, see <http://www.gnu.org/licenses/>.

Description


Contributors/Copyright:
    2006-2010, 2012-2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef VALUE_EXPRESSION_DRIVER_YY_H
#define VALUE_EXPRESSION_DRIVER_YY_H

#include "FieldValueExpressionDriver.hpp"

#define YY_DECL int parserFieldlex(parserField::FieldValueExpressionParser::semantic_type* yylval_param,parserField::location* yylloc,void *yyscanner,CML::FieldValueExpressionDriver &driver,int& start_token,int& numberOfFunctionChars)

YY_DECL;

#endif
