/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright  held by original author
-------------------------------------------------------------------------------
License
    This file is part of CAELUS.

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
    2014 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#include "executeFunctionObjectsFvOption.hpp"
#include "fvMatrices.hpp"
#include "DimensionedField.hpp"
#include "IFstream.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //

namespace CML
{
namespace fv
{
    defineTypeNameAndDebug(executeFunctionObjectsFvOption, 0);

    addToRunTimeSelectionTable
    (
        option,
        executeFunctionObjectsFvOption,
        dictionary
    );
}
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //



// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::fv::executeFunctionObjectsFvOption::executeFunctionObjectsFvOption
(
    const word& sourceName,
    const word& modelType,
    const dictionary& dict,
    const fvMesh& mesh
)
:
    option(sourceName, modelType, dict, mesh),
    functions_(
        mesh.time(),
        coeffs_
    ),
    verbose_(readBool(coeffs_.lookup("verbose"))),
    doCorrect_(readBool(coeffs_.lookup("doCorrect"))),
    doAddSup_(readBool(coeffs_.lookup("doAddSup"))),
    doSetValue_(readBool(coeffs_.lookup("doSetValue"))),
    doMakeRelative_(readBool(coeffs_.lookup("doMakeRelative"))),
    doMakeAbsolute_(readBool(coeffs_.lookup("doMakeAbsolute")))
{
    coeffs_.lookup("fieldNames") >> fieldNames_;
    applied_.setSize(fieldNames_.size(), false);

    if(!coeffs_.found("functions")) {
        FatalErrorInFunction
            << "No entry 'functions' in " << coeffs_.name()
                << endl
                << exit(FatalError);

    }

    if(verbose_) {
        Info << name() << " Starting functions" << endl;
    }
    functions_.start();
}

CML::fv::executeFunctionObjectsFvOption::~executeFunctionObjectsFvOption()
{
    if(verbose_) {
        Info << name() << " Stopping functions" << endl;
    }
    functions_.end();
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::fv::executeFunctionObjectsFvOption::executeFunctionObjects(
    const string &message
) {
    if(verbose_) {
        Info << name() << " executing: " << message.c_str() << endl;
    }
    functions_.execute();
    if(verbose_) {
        Info << name() << " ended" << endl;
    }
}

void CML::fv::executeFunctionObjectsFvOption::correct(volVectorField& U)
{
    if(doCorrect_) {
        executeFunctionObjects("correct(volVectorField& "+U.name()+")");
    }
}


void CML::fv::executeFunctionObjectsFvOption::addSup
(
    fvMatrix<vector>& eqn,
    const label fieldI
)
{
    if(doAddSup_) {
        executeFunctionObjects("addSup(fvMatrix<vector>& "+eqn.psi().name()+"/"+fieldNames_[fieldI]+")");
    }
}


void CML::fv::executeFunctionObjectsFvOption::setValue
(
    fvMatrix<vector>& eqn,
    const label fieldI
)
{
    if(doSetValue_) {
        executeFunctionObjects("setValue(fvMatrix<vector>& "+eqn.psi().name()+"/"+fieldNames_[fieldI]+")");
    }
}

void CML::fv::executeFunctionObjectsFvOption::correct(volScalarField& U)
{
    if(doCorrect_) {
        executeFunctionObjects("correct(volScalarField& "+U.name()+")");
    }
}


void CML::fv::executeFunctionObjectsFvOption::addSup
(
    fvMatrix<scalar>& eqn,
    const label fieldI
)
{
    if(doAddSup_) {
        executeFunctionObjects("addSup(fvMatrix<scalar>& "+eqn.psi().name()+"/"+fieldNames_[fieldI]+")");
    }
}


void CML::fv::executeFunctionObjectsFvOption::setValue
(
    fvMatrix<scalar>& eqn,
    const label fieldI
)
{
    if(doSetValue_) {
        executeFunctionObjects("setValue(fvMatrix<scalar>& "+eqn.psi().name()+"/"+fieldNames_[fieldI]+")");
    }
}

void CML::fv::executeFunctionObjectsFvOption::correct(volTensorField& U)
{
    if(doCorrect_) {
        executeFunctionObjects("correct(volTensorField& "+U.name()+")");
    }
}


void CML::fv::executeFunctionObjectsFvOption::addSup
(
    fvMatrix<tensor>& eqn,
    const label fieldI
)
{
    if(doAddSup_) {
        executeFunctionObjects("addSup(fvMatrix<tensor>& "+eqn.psi().name()+"/"+fieldNames_[fieldI]+")");
    }
}


void CML::fv::executeFunctionObjectsFvOption::setValue
(
    fvMatrix<tensor>& eqn,
    const label fieldI
)
{
    if(doSetValue_) {
        executeFunctionObjects("setValue(fvMatrix<tensor>& "+eqn.psi().name()+"/"+fieldNames_[fieldI]+")");
    }
}

void CML::fv::executeFunctionObjectsFvOption::correct(volSymmTensorField& U)
{
    if(doCorrect_) {
        executeFunctionObjects("correct(volSymmTensorField& "+U.name()+")");
    }
}


void CML::fv::executeFunctionObjectsFvOption::addSup
(
    fvMatrix<symmTensor>& eqn,
    const label fieldI
)
{
    if(doAddSup_) {
        executeFunctionObjects("addSup(fvMatrix<symmTensor>& "+eqn.psi().name()+"/"+fieldNames_[fieldI]+")");
    }
}


void CML::fv::executeFunctionObjectsFvOption::setValue
(
    fvMatrix<symmTensor>& eqn,
    const label fieldI
)
{
    if(doSetValue_) {
        executeFunctionObjects("setValue(fvMatrix<symmTensor>& "+eqn.psi().name()+"/"+fieldNames_[fieldI]+")");
    }
}

void CML::fv::executeFunctionObjectsFvOption::correct(volSphericalTensorField& U)
{
    if(doCorrect_) {
        executeFunctionObjects("correct(volSphericalTensorField& "+U.name()+")");
    }
}


void CML::fv::executeFunctionObjectsFvOption::addSup
(
    fvMatrix<sphericalTensor>& eqn,
    const label fieldI
)
{
    if(doAddSup_) {
        executeFunctionObjects("addSup(fvMatrix<sphericalTensor>& "+eqn.psi().name()+"/"+fieldNames_[fieldI]+")");
    }
}


void CML::fv::executeFunctionObjectsFvOption::setValue
(
    fvMatrix<sphericalTensor>& eqn,
    const label fieldI
)
{
    if(doSetValue_) {
        executeFunctionObjects("setValue(fvMatrix<sphericalTensor>& "+eqn.psi().name()+"/"+fieldNames_[fieldI]+")");
    }
}

void CML::fv::executeFunctionObjectsFvOption::makeRelative(
    surfaceScalarField& phi
) const
{
    if(doCorrect_) {
        const_cast<executeFunctionObjectsFvOption&>(*this).
            executeFunctionObjects("makeRelative(surfaceScalarField& "+phi.name()+")");
    }
}

void CML::fv::executeFunctionObjectsFvOption::makeRelative(
    const surfaceScalarField& rho,
    surfaceScalarField& phi
) const
{
    if(doCorrect_) {
        const_cast<executeFunctionObjectsFvOption&>(*this).
            executeFunctionObjects("makeRelative(const surfaceScalarField& "+rho.name()+",surfaceScalarField& "+phi.name()+")");
    }
}

void CML::fv::executeFunctionObjectsFvOption::makeRelative(
    FieldField<fvsPatchField, scalar>& phi
) const
{
    if(doCorrect_) {
        const_cast<executeFunctionObjectsFvOption&>(*this).
            executeFunctionObjects("makeRelative(FieldField<fvsPatchField, scalar>&)");
    }
}


void CML::fv::executeFunctionObjectsFvOption::makeAbsolute(
    surfaceScalarField& phi
) const
{
    if(doCorrect_) {
        const_cast<executeFunctionObjectsFvOption&>(*this).
            executeFunctionObjects("makeAbsolute(surfaceScalarField& "+phi.name()+")");
    }
}

void CML::fv::executeFunctionObjectsFvOption::makeAbsolute(
    const surfaceScalarField& rho,
    surfaceScalarField& phi
) const
{
    if(doCorrect_) {
        const_cast<executeFunctionObjectsFvOption&>(*this).
            executeFunctionObjects("makeAbsolute(const surfaceScalarField& "+rho.name()+",surfaceScalarField& "+phi.name()+")");
    }
}


// ************************************************************************* //
