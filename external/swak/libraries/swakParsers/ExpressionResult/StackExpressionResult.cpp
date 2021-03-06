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
    2012-2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#include "StackExpressionResult.hpp"
#include "vector.hpp"
#include "tensor.hpp"
#include "symmTensor.hpp"
#include "sphericalTensor.hpp"

#include "addToRunTimeSelectionTable.hpp"

namespace CML {

defineTypeNameAndDebug(StackExpressionResult,0);

addToRunTimeSelectionTable(ExpressionResult, StackExpressionResult, dictionary);
addToRunTimeSelectionTable(ExpressionResult, StackExpressionResult, nothing);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

StackExpressionResult::StackExpressionResult()
:
    ExpressionResult()
{
    // this has to be reset every timestep to work
    setNeedsReset(true);
}

StackExpressionResult::StackExpressionResult(
    const StackExpressionResult &rhs
)
:
    ExpressionResult(rhs)
{
    // this has to be reset every timestep to work
    setNeedsReset(true);
}

StackExpressionResult::StackExpressionResult(const dictionary &dict)
:
    ExpressionResult(dict)
{
    // this has to be reset every timestep to work
    setNeedsReset(true);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

StackExpressionResult::~StackExpressionResult()
{
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

ExpressionResult StackExpressionResult::pop()
{
    if(!(size()>0)) {
        FatalErrorInFunction
            << "Trying to pop result from a empty queue"
            << endl
            << abort(FatalError);

        return ExpressionResult();
    }

    if(valueType()==pTraits<scalar>::typeName) {
        return popInternal<scalar>();
    } else if(valueType()==pTraits<vector>::typeName) {
        return popInternal<vector>();
    } else if(valueType()==pTraits<tensor>::typeName) {
        return popInternal<tensor>();
    } else if(valueType()==pTraits<symmTensor>::typeName) {
        return popInternal<symmTensor>();
    } else if(valueType()==pTraits<sphericalTensor>::typeName) {
        return popInternal<sphericalTensor>();
    } else {
            FatalErrorInFunction
                << " Unsopported value type " << valueType()
                << endl
                << abort(FatalError);

            return ExpressionResult();
    }
}

void StackExpressionResult::push(ExpressionResult &atEnd)
{
    Dbug << "push(ExpressionResult &atEnd)" << endl;
    Dbug << "Pushing: " << atEnd << endl;

    if(!hasValue()) {
        // this is the first push
        //        static_cast<ExpressionResult>(*this)=atEnd;
        ExpressionResult::operator=(atEnd);
    } else {
        if(valueType()!=atEnd.valueType()) {
            FatalErrorInFunction
                << "Type of pushed value " << atEnd.valueType()
                << " is not the expected type " << valueType()
                << endl
                << abort(FatalError);
        }
        if(valueType()==pTraits<scalar>::typeName) {
            pushInternal<scalar>(atEnd);
        } else if(valueType()==pTraits<vector>::typeName) {
            pushInternal<vector>(atEnd);
        } else if(valueType()==pTraits<tensor>::typeName) {
            pushInternal<tensor>(atEnd);
        } else if(valueType()==pTraits<symmTensor>::typeName) {
            pushInternal<symmTensor>(atEnd);
        } else if(valueType()==pTraits<sphericalTensor>::typeName) {
            pushInternal<sphericalTensor>(atEnd);
        } else {
            FatalErrorInFunction
                << " Unsopported value type " << valueType()
                << endl
                << abort(FatalError);
        }
    }

    Dbug << "After push: " << *this << endl;
}

// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

void StackExpressionResult::operator=(const StackExpressionResult& rhs)
{
    Dbug << "operator=(const StackExpressionResult& rhs)"
        << endl;

    // Check for assignment to self
    if (this == &rhs)
    {
        FatalErrorInFunction
            << "Attempted assignment to self"
            << exit(FatalError);
    }

    static_cast<ExpressionResult&>(*this)=rhs;
}

void StackExpressionResult::operator=(const ExpressionResult& rhs)
{
    Dbug << "operator=(const ExpressionResult& rhs)" << endl;

    ExpressionResult last(
        rhs.getUniform(
            1,
            false // issue a warning if the other result is not really uniform
        )
    );

    this->push(
        last
    );
}

// * * * * * * * * * * * * * * * Friend Functions  * * * * * * * * * * * * * //

// Ostream & operator<<(Ostream &out,const StackExpressionResult &data)
// {
//     out << token::BEGIN_BLOCK << endl;

//     out.writeKeyword("name");
//     out << word(data.name_) << token::END_STATEMENT << nl;

//     out.writeKeyword("initialValue");
//     out << data.initialValueExpression_ << token::END_STATEMENT << nl;

//     out.writeKeyword("value");
//     out << static_cast<const ExpressionResult &>(data);

//     out << token::END_BLOCK << endl;

//     return out;
// }

// Istream & operator>>(Istream &in,StackExpressionResult &data)
// {
//     dictionary dict(in);

//     data=StackExpressionResult(dict);

//     return in;
// }

// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //

// bool operator!=(const StackExpressionResult &,const StackExpressionResult &)
// {
//     return false;
// }

} // namespace

// ************************************************************************* //
