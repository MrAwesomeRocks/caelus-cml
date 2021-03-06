/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2015 OpenFOAM Foundation
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

#include "pyrolysisModel.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace regionModels
{
namespace pyrolysisModels
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(pyrolysisModel, 0);
defineRunTimeSelectionTable(pyrolysisModel, mesh);
defineRunTimeSelectionTable(pyrolysisModel, dictionary);

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void pyrolysisModel::readPyrolysisControls()
{}


bool pyrolysisModel::read()
{
    if (regionModel1D::read())
    {
        readPyrolysisControls();
        return true;
    }
    else
    {
        return false;
    }
}


bool pyrolysisModel::read(const dictionary& dict)
{
    if (regionModel1D::read(dict))
    {
        readPyrolysisControls();
        return true;
    }
    else
    {
        return false;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

pyrolysisModel::pyrolysisModel(const fvMesh& mesh, const word& regionType)
:
    regionModel1D(mesh, regionType)
{}


pyrolysisModel::pyrolysisModel
(
    const word& modelType,
    const fvMesh& mesh,
    const word& regionType
)
:
    regionModel1D(mesh, regionType, modelType)
{
    if (active_)
    {
        read();
    }
}


pyrolysisModel::pyrolysisModel
(
    const word& modelType,
    const fvMesh& mesh,
    const dictionary& dict,
    const word& regionType
)
:
    regionModel1D(mesh, regionType, modelType, dict)
{
    if (active_)
    {
        read(dict);
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

pyrolysisModel::~pyrolysisModel()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

scalar pyrolysisModel::addMassSources
(
    const label patchi,
    const label facei
)
{
    return 0.0;
}


scalar pyrolysisModel::solidRegionDiffNo() const
{
    return VSMALL;
}


scalar pyrolysisModel::maxDiff() const
{
    return GREAT;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace pyrolysisModels
} // End namespace regionModels
} // End namespace CML

// ************************************************************************* //
