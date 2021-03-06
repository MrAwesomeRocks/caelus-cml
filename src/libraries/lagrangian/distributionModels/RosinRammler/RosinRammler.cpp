/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2018 OpenFOAM Foundation
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

#include "RosinRammler.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
namespace distributionModels
{
    defineTypeNameAndDebug(RosinRammler, 0);
    addToRunTimeSelectionTable(distributionModel, RosinRammler, dictionary);
}
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::distributionModels::RosinRammler::RosinRammler
(
    const dictionary& dict,
    Random& rndGen
)
:
    distributionModel(typeName, dict, rndGen),
    minValue_(readScalar(distributionModelDict_.lookup("minValue"))),
    maxValue_(readScalar(distributionModelDict_.lookup("maxValue"))),
    d_(readScalar(distributionModelDict_.lookup("d"))),
    n_(readScalar(distributionModelDict_.lookup("n")))
{
    check();
}


CML::distributionModels::RosinRammler::RosinRammler(const RosinRammler& p)
:
    distributionModel(p),
    minValue_(p.minValue_),
    maxValue_(p.maxValue_),
    d_(p.d_),
    n_(p.n_)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::distributionModels::RosinRammler::~RosinRammler()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::scalar CML::distributionModels::RosinRammler::sample() const
{
    scalar K = 1.0 - exp(-pow((maxValue_ - minValue_)/d_, n_));
    scalar y = rndGen_.sample01<scalar>();
    scalar x = minValue_ + d_*::pow(-log(1.0 - y*K), 1.0/n_);
    return x;
}


CML::scalar CML::distributionModels::RosinRammler::minValue() const
{
    return minValue_;
}


CML::scalar CML::distributionModels::RosinRammler::maxValue() const
{
    return maxValue_;
}


CML::scalar CML::distributionModels::RosinRammler::meanValue() const
{
    return d_;
}


// ************************************************************************* //
