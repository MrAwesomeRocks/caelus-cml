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
    CML::CommonPluginFunction

Description

SourceFiles
    CommonPluginFunctionI.hpp
    CommonPluginFunction.cpp
    CommonPluginFunctionIO.cpp

Contributors/Copyright:
    2012-2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef CommonPluginFunction_H
#define CommonPluginFunction_H

#include "wordList.hpp"
#include "stringList.hpp"

#include "typeInfo.hpp"

#include "ExpressionResult.hpp"

#include "vector.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

    class CommonValueExpressionDriver;

    class fvMesh;

/*---------------------------------------------------------------------------*\
                           Class CommonPluginFunction Declaration
\*---------------------------------------------------------------------------*/
class CommonPluginFunction {
    // //- the driver that calls this plugin
    const CommonValueExpressionDriver &parentDriver_;

    //- name of the function
    word name_;

    //- type of data it returns (textual representation)
    word returnType_;

    //- names of the arguments
    wordList argumentNames_;

    //- which parser can read the arguments
    wordList argumentParsers_;

    //- expected types of the arguments
    wordList argumentTypes_;

    //- the actual expression result. To be set by doEvaluation
    ExpressionResult result_;

    //- Disallow default bitwise assignment
    void operator=(const CommonPluginFunction &);

    CommonPluginFunction(const CommonPluginFunction &);

    void evaluateInternal(
        const string &content,
        int &consumed
    );

protected:
    //- wrapper for argument reading
    label readArgument(
        label index,
        const string &content,
        const word &type
    );

    //- set a primitive argument
    virtual void setArgument(
        label index,
        const string &value
    ) {
        // should be overwritten by the function itself
        NotImplemented;
    }

    //- set a primitive argument
    virtual void setArgument(
        label index,
        const word &value
    ) {
        // should be overwritten by the function itself
        NotImplemented;
    }

    //- set a primitive argument
    virtual void setArgument(
        label index,
        const scalar &value
    ) {
        // should be overwritten by the function itself
        NotImplemented;
    }

    //- set a primitive argument
    virtual void setArgument(
        label index,
        const label &value
    ) {
        // should be overwritten by the function itself
        NotImplemented;
    }

    //- set a primitive argument
    virtual void setArgument(
        label index,
        const vector &value
    ) {
        // should be overwritten by the function itself
        NotImplemented;
    }

    //- set a primitive argument
    virtual void setArgument(
        label index,
        const bool &value
    ) {
        // should be overwritten by the function itself
        NotImplemented;
    }

    //- wrapper for argument reading
    label readArgument(
        label index,
        const string &content,
        CommonValueExpressionDriver &driver
    );

    //- read an argument that is to be parsed
    virtual void setArgument(
        label index,
        const string &content,
        const CommonValueExpressionDriver &driver
    ) {
        // should be overwritten by the function itself
        NotImplemented;
    }

    //- get an addtional ID for the parser of argument i
    virtual word getID(
        label index
    ) {
        return "method_getID_NotImplemented";
    }

    //- do the actual evaluation
    virtual void doEvaluation() = 0;

    //- scan if no values are bein read
    label scanEmpty(const string &content,word sym);

    inline CommonValueExpressionDriver &parentDriver();

    inline const fvMesh &mesh() const;

    inline ExpressionResult &result();

public:
    CommonPluginFunction(
        const CommonValueExpressionDriver &parentDriver,
        const word &name,
        const word &returnType,
        const string &argumentSpecificationString
    );

    virtual ~CommonPluginFunction() {}

    TypeName("CommonPluginFunction");

    const word &name() const { return name_; }

    const word &returnType() const { return returnType_; }

    word &returnType() { return returnType_; }

    //- return a help text constructed from parameters
    string helpText() const;

    //- evaluate the string and report how many characters were consumed in total
    template <typename T>
    tmp<T> evaluate(
        const string &content,
        int &consumed
    );

    template <typename T>
    tmp<Field<T> > evaluate(
        const string &content,
        int &consumed,
        bool isPoint
    );
};



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "CommonPluginFunctionI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
