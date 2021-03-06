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
    2010-2011, 2013-2014 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#include "patchExpressionFunctionObject.hpp"
#include "addToRunTimeSelectionTable.hpp"

#include "volFields.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(patchExpressionFunctionObject, 0);

    addToRunTimeSelectionTable
    (
        functionObject,
        patchExpressionFunctionObject,
        dictionary
    );



// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

patchExpressionFunctionObject::patchExpressionFunctionObject
(
    const word& name,
    const Time& t,
    const dictionary& dict
)
:
    patchFunctionObject(name,t,dict),
    expression_(
        dict.lookup("expression"),
        dict
    ),
    data_(dict),
    accumulations_(
        NumericAccumulationNamedEnum::readAccumulations(
            dict,"accumulations"
        )
    )
{
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

word patchExpressionFunctionObject::dirName()
{
    return typeName;
//     string expr="";
//     forAll(expression_,i) {
//         const char c=expression_[i];
//         switch (c) {
//             case '/':
//                 expr+="_DIV_";
//                 break;
//             case '*':
//                 expr+="_MULT_";
//                 break;
//             case '&':
//                 expr+="_IPROD_";
//                 break;
//             default:
//                 expr+=c;
//         }
//     }

//     return typeName+"_"+expr;
}

wordList patchExpressionFunctionObject::fileNames()
{
    return patchNames_;
}

stringList patchExpressionFunctionObject::columnNames()
{
    stringList result(accumulations_.size());

    forAll(accumulations_,i) {
        result[i]=NumericAccumulationNamedEnum::toString(
            accumulations_[i]
        );
    }

    return result;
}

void patchExpressionFunctionObject::write()
{
    forAll(patchIndizes_,i) {
        if(patchIndizes_[i]<0) {
            continue;
        }
        PatchValueExpressionDriver &driver=drivers_[i];

        if(verbose()) {
            Info << "Expression " << name() << " on " << patchNames_[i] << ": ";
        }

        driver.clearVariables();
        driver.parse(expression_);
        word rType=driver.getResultType();

        if(rType==pTraits<scalar>::typeName) {
            writeTheData<scalar>(patchNames_[i],driver);
        } else if(rType==pTraits<vector>::typeName) {
            writeTheData<vector>(patchNames_[i],driver);
        } else if(rType==pTraits<tensor>::typeName) {
            writeTheData<tensor>(patchNames_[i],driver);
        } else if(rType==pTraits<symmTensor>::typeName) {
            writeTheData<symmTensor>(patchNames_[i],driver);
        } else if(rType==pTraits<sphericalTensor>::typeName) {
            writeTheData<sphericalTensor>(patchNames_[i],driver);
        }

        if(verbose()) {
            Info << endl;
        }

        // make sure that the stored Variables are consistently written
        driver.tryWrite();
    }
}

bool patchExpressionFunctionObject::start()
{
    const fvMesh &mesh=refCast<const fvMesh>(obr_);

    bool result=patchFunctionObject::start();

    drivers_.clear();
    drivers_.resize(patchIndizes_.size());

    forAll(drivers_,i) {
        drivers_.set(
            i,
            new PatchValueExpressionDriver(
                data_,
                mesh.boundary()[patchIndizes_[i]]
            )
        );
        drivers_[i].createWriterAndRead(name()+"_"+mesh.boundary()[patchIndizes_[i]].name()+"_"+type());
    }
    return result;
}

} // namespace CML

// ************************************************************************* //
