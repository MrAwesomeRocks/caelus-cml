/*---------------------------------------------------------------------------*\
Copyright (C) 2011 OpenFOAM Foundation
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

\*---------------------------------------------------------------------------*/

#include "exponentialDiffusivity.hpp"
#include "surfaceFields.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(exponentialDiffusivity, 0);

    addToRunTimeSelectionTable
    (
        motionDiffusivity,
        exponentialDiffusivity,
        Istream
    );
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::exponentialDiffusivity::exponentialDiffusivity
(
    const fvMesh& mesh,
    Istream& mdData
)
:
    motionDiffusivity(mesh),
    alpha_(readScalar(mdData)),
    basicDiffusivityPtr_(motionDiffusivity::New(mesh, mdData))
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::exponentialDiffusivity::~exponentialDiffusivity()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::tmp<CML::surfaceScalarField>
CML::exponentialDiffusivity::operator()() const
{
    return exp(-alpha_/basicDiffusivityPtr_->operator()());
}


void CML::exponentialDiffusivity::correct()
{
    basicDiffusivityPtr_->correct();
}


// ************************************************************************* //
