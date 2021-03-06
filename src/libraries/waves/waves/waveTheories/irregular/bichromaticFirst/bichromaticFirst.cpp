/*---------------------------------------------------------------------------*\
Copyright Niels Gjøl Jacobsen, Technical University of Denmark.
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

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

#include "bichromaticFirst.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace waveTheories
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(bichromaticFirst, 0);
addToRunTimeSelectionTable(waveTheory, bichromaticFirst, dictionary);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //


bichromaticFirst::bichromaticFirst
(
    const word& subDictName,
    const fvMesh& mesh_
)
:
    waveTheory(subDictName, mesh_),
    H1_(readScalar(coeffDict_.lookup("height1"))),
    H2_(readScalar(coeffDict_.lookup("height2"))),
    h_(readScalar(coeffDict_.lookup("depth"))),
    omega1_(readScalar(coeffDict_.lookup("omega1"))),
    omega2_(readScalar(coeffDict_.lookup("omega2"))),
    period1_(2*PI_/omega1_),
    period2_(2*PI_/omega2_),
    phi1_(readScalar(coeffDict_.lookup("phi1"))),
    phi2_(readScalar(coeffDict_.lookup("phi2"))),
    k1_(vector(coeffDict_.lookup("waveNumber1"))),
    k2_(vector(coeffDict_.lookup("waveNumber2"))),
    K1_(mag(k1_)),
    K2_(mag(k2_)),

    Tsoft_
    (
        coeffDict_.lookupOrDefault<scalar>
        (
            "Tsoft",
            CML::max(period1_,period2_)
        )
    )
{
    checkWaveDirection(k1_);
    checkWaveDirection(k2_);
}


void bichromaticFirst::printCoeffs()
{
    Info << "Loading wave theory: " << typeName << endl;
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


scalar bichromaticFirst::factor(const scalar& time) const
{
    scalar factor(1.0);
    if (Tsoft_ > 0.0)
    {
        factor = CML::sin(2*PI_/(4.0*Tsoft_)*CML::min(Tsoft_, time));
    }

    return factor;
}


scalar bichromaticFirst::eta
(
    const point& x,
    const scalar& time
) const
{
    scalar eta = ( H1_/2.0*CML::cos(omega1_*time - (k1_ & x) + phi1_)
                 + H2_/2.0*CML::cos(omega2_*time - (k2_ & x) + phi2_)
                 )*factor(time)
                 + seaLevel_;
    return eta;
}


scalar bichromaticFirst::ddxPd
(
    const point& x,
    const scalar& time,
    const vector& unitVector
) const
{

    scalar Z(returnZ(x));
    scalar arg1(omega1_*time - (k1_ & x) + phi1_);
    scalar arg2(omega2_*time - (k2_ & x) + phi2_);

    scalar ddxPd(0);

    ddxPd =
        (
           rhoWater_*mag(g_)*K1_*H1_/2.0*CML::cosh(K1_*(Z + h_))
          /CML::cosh(K1_*h_)*CML::sin(arg1)
         + rhoWater_*mag(g_)*K2_*H2_/2.0*CML::cosh(K2_*(Z + h_))
          /CML::cosh(K2_*h_)*CML::sin(arg2)
        )*factor(time);

    return ddxPd;
}


vector bichromaticFirst::U
(
    const point& x,
    const scalar& time
) const
{
    scalar Z(returnZ(x));

    scalar Uhorz1 = PI_*H1_/period1_ *
        CML::cosh(K1_*(Z + h_))/CML::sinh(K1_*h_) *
        CML::cos(omega1_*time - (k1_ & x) + phi1_);

    scalar Uhorz2 = PI_*H2_/period2_ *
        CML::cosh(K2_*(Z + h_))/CML::sinh(K2_*h_) *
        CML::cos(omega2_*time - (k2_ & x) + phi2_);

    Uhorz1 *= factor(time);
    Uhorz2 *= factor(time);

    scalar Uvert1 = - PI_*H1_/period1_ *
        CML::sinh(K1_*(Z + h_))/CML::sinh(K1_*h_) *
        CML::sin(omega1_*time - (k1_ & x) + phi1_);

    scalar Uvert2 = - PI_*H2_/period2_ *
        CML::sinh(K2_*(Z + h_))/CML::sinh(K2_*h_) *
        CML::sin(omega2_*time - (k2_ & x) + phi2_);

    Uvert1 *= factor(time);
    Uvert2 *= factor(time);

    // Note "-" because of "g" working in the opposite direction
    return Uhorz1*k1_/K1_ + Uhorz2*k2_/K2_ - (Uvert1 + Uvert2)*direction_;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace waveTheories
} // End namespace CML

// ************************************************************************* //
