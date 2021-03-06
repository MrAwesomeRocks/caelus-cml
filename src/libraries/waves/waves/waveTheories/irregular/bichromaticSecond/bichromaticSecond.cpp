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

#include "bichromaticSecond.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace waveTheories
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(bichromaticSecond, 0);
addToRunTimeSelectionTable(waveTheory, bichromaticSecond, dictionary);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //


bichromaticSecond::bichromaticSecond
(
    const word& subDictName,
    const fvMesh& mesh_
)
:
    waveTheory(subDictName, mesh_),
    Hn_(readScalar(coeffDict_.lookup("height1"))),
    Hm_(readScalar(coeffDict_.lookup("height2"))),
    h_(readScalar(coeffDict_.lookup("depth"))),
    omega1n_(readScalar(coeffDict_.lookup("omega1"))),
    omega1m_(readScalar(coeffDict_.lookup("omega2"))),
    periodn_(2*PI_/omega1n_),
    periodm_(2*PI_/omega1m_),
    phin_(readScalar(coeffDict_.lookup("phi1"))),
    phim_(readScalar(coeffDict_.lookup("phi2"))),
    kn_(vector(coeffDict_.lookup("waveNumber1"))),
    km_(vector(coeffDict_.lookup("waveNumber2"))),

    Tsoft_
    (
        coeffDict_.lookupOrDefault<scalar>
        (
            "Tsoft",
            CML::max(periodn_,periodm_)
        )
    )
{
    setCoeffs();
    printCoeffs();

    checkWaveDirection(kn_);
    checkWaveDirection(km_);
}


void bichromaticSecond::printCoeffs()
{
    if (false)
    {
        Info << "Loading wave theory: " << typeName << endl;

        // The values are taken from table 1 in the referenced paper.
        Info << "Name: " << tab << "OF-value:" << tab << "From paper:" << endl;
        printCoeffs("kappaN:", kappan_, 0.10737);
        printCoeffs("kappaM:", kappam_, 0.06514);
        printCoeffs("kappaNMP:", kappanmp_, 0.17004);
        printCoeffs("kappaNMM:", kappanmm_, 0.05125);
        printCoeffs("FN:",Fn_ , -6.5784 );
        printCoeffs("FM:",Fm_ , -13.2958);
        printCoeffs("G2N:", G2n_ , 2.5773);
        printCoeffs("G2m:", G2m_ , 4.6356);
        printCoeffs("GNMP:", Gnmp_ ,3.1320 );
        printCoeffs("GNMM:", Gnmm_, -1.4060);
        printCoeffs("F2N:", F2n_ , -2.4552);
        printCoeffs("F2m:", F2m_ , -19.0648);
        printCoeffs("FNMP:", Fnmp_ , -6.4505 );
        printCoeffs("FNMM:", Fnmm_, 32.3669);
    }
}


void bichromaticSecond::printCoeffs
(
    const word name,
    const scalar model,
    const scalar theory
) const
{
    Info << name << tab << model << tab << theory << endl;
}


void bichromaticSecond::setCoeffs()
{
    // First order coefficients
    an_ = Hn_/2.0;
    am_ = Hm_/2.0;

    kappan_ = CML::mag(kn_);
    kappam_ = CML::mag(km_);

    Fn_ = - omega1n_/(kappan_*CML::sinh(kappan_*h_));
    Fm_ = - omega1m_/(kappam_*CML::sinh(kappam_*h_));

    // Second order coefficients
    Anm_ = 1.0/h_*an_*am_;
    A2n_ = 1.0/(2.0*h_)*CML::sqr(an_);
    A2m_ = 1.0/(2.0*h_)*CML::sqr(am_);

    kappanmp_ = CML::mag(kn_ + km_);
    kappanmm_ = CML::mag(kn_ - km_);

    G2n_  = 0.5*h_*kappan_*(2.0 + CML::cosh(2*h_*kappan_));
    G2n_ /= (CML::tanh(h_*kappan_)*CML::sqr(CML::sinh(h_*kappan_)));

    G2m_  = 0.5*h_*kappam_*(2.0 + CML::cosh(2*h_*kappam_));
    G2m_ /= (CML::tanh(h_*kappam_)*CML::sqr(CML::sinh(h_*kappam_)));

    Gnmp_ = Lambda2(omega1n_, kn_, kappan_, omega1m_, km_, kappam_, kappanmp_);
    Gnmm_ = Lambda2
        (
            omega1n_,
            kn_,
            kappan_,
            -omega1m_,
            -km_,
            kappam_,
            kappanmm_
        );

    F2n_  = - 3.0/4.0*h_*omega1n_/CML::pow(CML::sinh(h_*kappan_), 4.0);
    F2m_  = - 3.0/4.0*h_*omega1m_/CML::pow(CML::sinh(h_*kappam_), 4.0);

    Fnmp_ = Gamma2(omega1n_, kn_, kappan_, omega1m_, km_, kappam_, kappanmp_);
    Fnmm_ = Gamma2
        (
            omega1n_,
            kn_,
            kappan_,
            -omega1m_,
            -km_,
            kappam_,
            kappanmm_
        );

    // Velocity direction for the different interactions.
    // dirn_ == dir2n_ so not defined!
    dirn_   = kn_/kappan_;
    dirm_   = km_/kappam_;
    dirnmp_ = (kn_ + km_)/kappanmp_;
    dirnmm_ = (kn_ - km_)/kappanmm_;
}


scalar bichromaticSecond::betanm
(
    const scalar& omega0,
    const scalar& omega1,
    const scalar& kappa01
) const
{
    scalar res = 0.0;

    res += CML::sqr(omega0 + omega1)*CML::cosh(h_*kappa01);
    res -= CML::mag(g_)*kappa01*sinh(h_*kappa01);
    res *= (2.0*omega0*omega1);

    return res;
}


scalar bichromaticSecond::Lambda2
(
    const scalar& omega0,
    const vector& k0,
    const scalar& kappa0,
    const scalar& omega1,
    const vector& k1,
    const scalar& kappa1,
    const scalar& kappa01
) const
{
    scalar res0 = 0, res1 = 0;

    res0 += omega0*(CML::sqr(kappa1) + (k0 & k1))
        + omega1*(CML::sqr(kappa0) + (k0 & k1));
    res0 *= CML::mag(g_)*h_/betanm(omega0, omega1, kappa01)*(omega0 + omega1)
        *CML::cosh(h_*kappa01);

    res1 += CML::sqr(CML::mag(g_))*(k0 & k1);
    res1 += CML::sqr(omega0)*CML::sqr(omega1);
    res1 -= omega0*omega1*CML::sqr(omega0 + omega1);
    res1 *= (h_*kappa01)/betanm(omega0, omega1, kappa01)
        *CML::sinh(h_*kappa01);

    return res0 + res1;
}


scalar bichromaticSecond::Gamma2
(
    const scalar& omega0,
    const vector& k0,
    const scalar& kappa0,
    const scalar& omega1,
    const vector& k1,
    const scalar& kappa1,
    const scalar& kappa01
) const
{
    scalar res0 = 0, res1 = 0;

    res0 += omega0*omega1*(omega0 + omega1);
    res0 *= (CML::sqr(omega0 + omega1) - omega0*omega1);
    res0 *= h_/betanm(omega0, omega1, kappa01);

    res1 += omega0*(CML::sqr(kappa1) + 2*(k0 & k1));
    res1 += omega1*(CML::sqr(kappa0) + 2*(k0 & k1));
    res1 *= (h_*CML::sqr(CML::mag(g_)))/betanm(omega0, omega1, kappa01);

    return res0 - res1;
}


scalar bichromaticSecond::argn
(
    const point& x,
    const scalar& time
) const
{
    return omega1n_*time - (kn_ & x) + phin_;
}


scalar bichromaticSecond::argm
(
    const point& x,
    const scalar& time
) const
{
    return omega1m_*time - (km_ & x) + phim_;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


scalar bichromaticSecond::factor(const scalar& time) const
{
    scalar factor(1.0);
    if (Tsoft_ > 0.0)
    {
        factor = CML::sin(2*PI_/(4.0*Tsoft_)*CML::min(Tsoft_, time));
    }

    return factor;
}


scalar bichromaticSecond::eta
(
    const point& x,
    const scalar& time
) const
{
    scalar eta = 0.0;
    scalar thetan = argn(x, time);
    scalar thetam = argm(x, time);

    // First order contribution
    eta += an_*CML::cos(thetan) + am_*CML::cos(thetam);

    eta += Gnmm_*Anm_*CML::cos(thetan - thetam)
        + Gnmp_*Anm_*CML::cos(thetan - thetam)
        + G2n_*A2n_*CML::cos(2*thetan)
        + G2m_*A2m_*CML::cos(2*thetam);

    return eta*factor(time) + seaLevel_;
}


scalar bichromaticSecond::ddxPd
(
    const point& x,
    const scalar& time,
    const vector& unitVector
) const
{
    scalar ddxPd(0);

    return ddxPd;
}


vector bichromaticSecond::U
(
    const point& x,
    const scalar& time
) const
{
    scalar Z(returnZ(x));
    scalar thetan = argn(x, time);
    scalar thetam = argm(x, time);

    vector res = vector::zero;

    // HORIZONTAL VELOCITY CONTRIBUTIONS
    // The horizontal velocity is thought of being derived from the velocity
    // potential independently for each interaction, so a differentiation along
    // a local coordinate system pointing in e.g. dirnmp_ gives a scalar
    // velocity, which is subsequently given the correct direction by
    // multiplication with dirnmp_. This avoids the need to check which
    // direction is the vertical one when evaluating the derivative of the
    // velocity potential.

    // First order contributions
    res += - dirn_*Fn_*CML::cosh(kappan_*(Z + h_))*an_*kappan_
        *CML::cos(thetan)
        - dirm_*Fm_*CML::cosh(kappam_*(Z + h_))*am_*kappam_
        *CML::cos(thetam);

    // Second order self-interactions
    res += - dirn_*F2n_*CML::cosh(2.0*kappan_*(Z + h_))*A2n_*2.0*kappan_
        *CML::cos( 2.0*thetan )
        - dirn_*F2m_*CML::cosh(2.0*kappam_*(Z + h_))*A2m_*2.0*kappam_
        *CML::cos( 2.0*thetam );

    // Second order super- and sub-harmonics
    res += - dirnmp_*Fnmp_*CML::cosh(kappanmp_*(Z + h_))*Anm_*kappanmp_
        *CML::cos(thetan + thetam)
        - dirnmm_*Fnmm_*CML::cosh(kappanmm_*(Z + h_))*Anm_*kappanmm_
        *CML::cos(thetan - thetam);

    // VERTICAL VELOCITY CONTRIBUTIONS
    // Note "-=" to correct to the sign on direction_!
    res -= ( Fn_*kappan_*CML::sinh(kappan_*(Z + h_))*an_*CML::sin(thetan)
        // First order
        + Fm_*kappam_*CML::sinh(kappam_*(Z + h_))*am_*CML::sin(thetam)
        // Second order self-interaction
        + F2n_*2.0*kappan_*CML::sinh(2.0*kappan_*(Z + h_))
        *A2n_*CML::sin(2.0*thetan)
        // Second order self-interaction
        + F2m_*2.0*kappam_*CML::sinh(2.0*kappam_*(Z + h_))
        *A2m_*CML::sin(2.0*thetam)
        // Second order sub-harmonic
        + Fnmm_*kappanmm_*CML::sinh(kappanmm_*(Z + h_))
        *Anm_*CML::sin(thetan - thetam)
        // Second order super-harmonic
        + Fnmp_*kappanmp_*CML::sinh(kappanmp_*(Z + h_))
        *Anm_*CML::sin(thetan + thetam)
        )*direction_;

    return res*factor(time);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace waveTheories
} // End namespace CML

// ************************************************************************* //
