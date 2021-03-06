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

#include "fvcSnGradFunctionPlugin.hpp"
#include "FieldValueExpressionDriver.hpp"

#include "addToRunTimeSelectionTable.hpp"

#include "snGradScheme.hpp"

namespace CML {

defineTemplateTypeNameAndDebug(fvcSnGradFunctionPlugin<scalar>,1);
addNamedTemplateToRunTimeSelectionTable(FieldValuePluginFunction, fvcSnGradFunctionPlugin,scalar , name, fvcSnGradScalar);

defineTemplateTypeNameAndDebug(fvcSnGradFunctionPlugin<vector>,1);
addNamedTemplateToRunTimeSelectionTable(FieldValuePluginFunction, fvcSnGradFunctionPlugin,vector , name, fvcSnGradVector);

defineTemplateTypeNameAndDebug(fvcSnGradFunctionPlugin<tensor>,1);
addNamedTemplateToRunTimeSelectionTable(FieldValuePluginFunction, fvcSnGradFunctionPlugin,tensor , name, fvcSnGradTensor);

defineTemplateTypeNameAndDebug(fvcSnGradFunctionPlugin<symmTensor>,1);
addNamedTemplateToRunTimeSelectionTable(FieldValuePluginFunction, fvcSnGradFunctionPlugin,symmTensor , name, fvcSnGradSymmTensor);

defineTemplateTypeNameAndDebug(fvcSnGradFunctionPlugin<sphericalTensor>,1);
addNamedTemplateToRunTimeSelectionTable(FieldValuePluginFunction, fvcSnGradFunctionPlugin,sphericalTensor , name, fvcSnGradSphericalTensor);


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class T>
fvcSnGradFunctionPlugin<T>::fvcSnGradFunctionPlugin(
    const FieldValueExpressionDriver &parentDriver,
    const word &name
):
    FieldValuePluginFunction(
        parentDriver,
        name,
        word(pTraits<resultType>::typeName),
        string(
            "original internalField "
            +pTraits<originalType>::typeName
            +",specString primitive string"
        )
    )
{
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class T>
void fvcSnGradFunctionPlugin<T>::doEvaluation()
{
    IStringStream spec(specString_);

    tmp<fv::snGradScheme<T> > scheme(
        fv::snGradScheme<T>::New(
            mesh(),
            spec
        )
    );

    autoPtr<resultType> pInterpol(
        new resultType(
            IOobject(
                "fvcSnGrad"+this->original_->name(),
                mesh().time().timeName(),
                mesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            scheme().snGrad(original_())
        )
    );

    result().setObjectResult(pInterpol);
}

template<class T>
void fvcSnGradFunctionPlugin<T>::setArgument(
    label index,
    const string &content,
    const CommonValueExpressionDriver &driver
)
{
    assert(index==0);
    this->original_.set(
        new originalType(
            //            dynamicCast<const FieldValueExpressionDriver &>(
            dynamic_cast<const FieldValueExpressionDriver &>(
                driver
            ).getResult<originalType>()
        )
    );
}

template <class T>
void fvcSnGradFunctionPlugin<T>::setArgument(
    label index,
    const string &value
)
{
    assert(index==1);

    specString_=value;
}

// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //

} // namespace

// ************************************************************************* //
