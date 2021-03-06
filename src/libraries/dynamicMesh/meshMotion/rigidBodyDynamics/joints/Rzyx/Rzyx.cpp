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

#include "Rzyx.hpp"
#include "rigidBodyModel.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
namespace RBD
{
namespace joints
{
    defineTypeNameAndDebug(Rzyx, 0);

    addToRunTimeSelectionTable
    (
        joint,
        Rzyx,
        dictionary
    );
}
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::RBD::joints::Rzyx::Rzyx()
:
    joint(3)
{
    S_[0] = spatialVector(0, 0, 1, 0, 0, 0);
    S_[1] = spatialVector(0, 1, 0, 0, 0, 0);
    S_[2] = spatialVector(1, 0, 0, 0, 0, 0);
}


CML::RBD::joints::Rzyx::Rzyx(const dictionary& dict)
:
    joint(3)
{
    S_[0] = spatialVector(0, 0, 1, 0, 0, 0);
    S_[1] = spatialVector(0, 1, 0, 0, 0, 0);
    S_[2] = spatialVector(1, 0, 0, 0, 0, 0);
}


CML::autoPtr<CML::RBD::joint> CML::RBD::joints::Rzyx::clone() const
{
    return autoPtr<joint>(new Rzyx(*this));
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::RBD::joints::Rzyx::~Rzyx()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void CML::RBD::joints::Rzyx::jcalc
(
    joint::XSvc& J,
    const scalarField& q,
    const scalarField& qDot
) const
{
    vector qj(q.block<vector>(qIndex_));

    scalar s0 = sin(qj.x());
    scalar c0 = cos(qj.x());
    scalar s1 = sin(qj.y());
    scalar c1 = cos(qj.y());
    scalar s2 = sin(qj.z());
    scalar c2 = cos(qj.z());

    J.X.E() = tensor
    (
        c0*c1,            s0*c1,           -s1,
        c0*s1*s2 - s0*c2, s0*s1*s2 + c0*c2, c1*s2,
        c0*s1*c2 + s0*s2, s0*s1*c2 - c0*s2, c1*c2
    );
    J.X.r() = Zero;

    J.S = Zero;
    J.S.xx() = -s1;
    J.S.xz() = 1;
    J.S.yx() = c1*s2;
    J.S.yy() = c2;
    J.S.zx() = c1*c2;
    J.S.zy() = -s2;

    vector qDotj(qDot.block<vector>(qIndex_));
    J.v = J.S & qDotj;

    J.c = spatialVector
    (
       -c1*qDotj.x()*qDotj.y(),

       -s1*s2*qDotj.x()*qDotj.y()
      + c1*c2*qDotj.x()*qDotj.z()
      - s2*qDotj.y()*qDotj.z(),

       -s1*c2*qDotj.x()*qDotj.y()
      - c1*s2*qDotj.x()*qDotj.z()
      - c2*qDotj.y()*qDotj.z(),

        0,
        0,
        0
    );
}


// ************************************************************************* //
