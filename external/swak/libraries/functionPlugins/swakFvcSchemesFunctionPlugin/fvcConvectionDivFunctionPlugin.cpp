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

#include "fvcConvectionDivFunctionPlugin.hpp"
#include "FieldValueExpressionDriver.hpp"

#include "addToRunTimeSelectionTable.hpp"

#include "convectionScheme.hpp"

namespace CML {

defineTemplateTypeNameAndDebug(fvcConvectionDivFunctionPlugin<scalar>,1);
addNamedTemplateToRunTimeSelectionTable(FieldValuePluginFunction, fvcConvectionDivFunctionPlugin,scalar , name, fvcConvectionDivScalar);

defineTemplateTypeNameAndDebug(fvcConvectionDivFunctionPlugin<vector>,1);
addNamedTemplateToRunTimeSelectionTable(FieldValuePluginFunction, fvcConvectionDivFunctionPlugin,vector , name, fvcConvectionDivVector);

defineTemplateTypeNameAndDebug(fvcConvectionDivFunctionPlugin<tensor>,1);
addNamedTemplateToRunTimeSelectionTable(FieldValuePluginFunction, fvcConvectionDivFunctionPlugin,tensor , name, fvcConvectionDivTensor);

defineTemplateTypeNameAndDebug(fvcConvectionDivFunctionPlugin<symmTensor>,1);
addNamedTemplateToRunTimeSelectionTable(FieldValuePluginFunction, fvcConvectionDivFunctionPlugin,symmTensor , name, fvcConvectionDivSymmTensor);

defineTemplateTypeNameAndDebug(fvcConvectionDivFunctionPlugin<sphericalTensor>,1);
addNamedTemplateToRunTimeSelectionTable(FieldValuePluginFunction, fvcConvectionDivFunctionPlugin,sphericalTensor , name, fvcConvectionDivSphericalTensor);


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class T>
fvcConvectionDivFunctionPlugin<T>::fvcConvectionDivFunctionPlugin(
    const FieldValueExpressionDriver &parentDriver,
    const word &name
):
    FieldValuePluginFunction(
        parentDriver,
        name,
        word(pTraits<resultType>::typeName),
        string(
            "flow internalField surfaceScalarField,original internalField "
            +pTraits<originalType>::typeName
            +",specString primitive string"
        )
    )
{
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class T>
void fvcConvectionDivFunctionPlugin<T>::doEvaluation()
{
    IStringStream spec(specString_);

    tmp<fv::convectionScheme<T> > scheme(
        fv::convectionScheme<T>::New(
            mesh(),
            flow_(),
            spec
        )
    );

    autoPtr<resultType> pInterpol(
        new resultType(
            IOobject(
                "fvcInterpolated"+this->original_->name(),
                mesh().time().timeName(),
                mesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            scheme().fvcDiv(flow_(),original_())
        )
    );

    result().setObjectResult(pInterpol);
}

template<class T>
void fvcConvectionDivFunctionPlugin<T>::setArgument(
    label index,
    const string &content,
    const CommonValueExpressionDriver &driver
)
{
    assert(index==0 || index==1);
    if(index==1) {
        this->original_.set(
            new originalType(
                //                dynamicCast<const FieldValueExpressionDriver &>(
                dynamic_cast<const FieldValueExpressionDriver &>(
                    driver
                ).getResult<originalType>()
            )
        );
    } else {
        this->flow_.set(
            new surfaceScalarField(
                //                dynamicCast<const FieldValueExpressionDriver &>(
                dynamic_cast<const FieldValueExpressionDriver &>(
                    driver
                ).getResult<surfaceScalarField>()
            )
        );
    }
}

template <class T>
void fvcConvectionDivFunctionPlugin<T>::setArgument(
    label index,
    const string &value
)
{
    assert(index==2);

    specString_=value;
}

// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //

} // namespace

// ************************************************************************* //
