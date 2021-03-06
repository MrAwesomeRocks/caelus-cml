/*---------------------------------------------------------------------------*\
Copyright (C) 2016 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Caelus is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "limitVelocity.hpp"
#include "volFields.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
namespace fv
{
    defineTypeNameAndDebug(limitVelocity, 0);
    addToRunTimeSelectionTable(option, limitVelocity, dictionary);
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::fv::limitVelocity::limitVelocity
(
    const word& name,
    const word& modelType,
    const dictionary& dict,
    const fvMesh& mesh
)
:
    option(name, modelType, dict, mesh),
    UName_(coeffs_.lookupOrDefault<word>("U", "U")),
    max_(readScalar(coeffs_.lookup("max")))
{
    fieldNames_.setSize(1, UName_);
    applied_.setSize(1, false);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool CML::fv::limitVelocity::read(const dictionary& dict)
{
    if (option::read(dict))
    {
        coeffs_.lookup("max") >> max_;

        return true;
    }
    else
    {
        return false;
    }
}


void CML::fv::limitVelocity::correct(volVectorField& U)
{
    const scalar maxSqrU = sqr(max_);

    vectorField& Uif = U.internalField();

    forAll(cells_, i)
    {
        const label celli = cells_[i];

        const scalar magSqrUi = magSqr(Uif[celli]);

        if (magSqrUi > maxSqrU)
        {
            Uif[celli] *= sqrt(maxSqrU/magSqrUi);
        }
    }

    // Handle boundaries in the case of 'all'
    if (selectionMode_ == smAll)
    {
        forAll(U.boundaryField(), patchi)
        {
            fvPatchVectorField& Up = U.boundaryField()[patchi];

            if (!Up.fixesValue())
            {
                forAll(Up, facei)
                {
                    const scalar magSqrUi = magSqr(Up[facei]);

                    if (magSqrUi > maxSqrU)
                    {
                        Up[facei] *= sqrt(maxSqrU/magSqrUi);
                    }
                }
            }
        }
    }
}


// ************************************************************************* //
