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

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

inline CML::powerSeriesReactionRate::powerSeriesReactionRate
(
    const scalar A,
    const scalar beta,
    const scalar Ta,
    const FixedList<scalar, nCoeff_> coeffs
)
:
    A_(A),
    beta_(beta),
    Ta_(Ta),
    coeffs_(coeffs)
{}


inline CML::powerSeriesReactionRate::powerSeriesReactionRate
(
    const speciesTable&,
    Istream& is
)
:
    A_(readScalar(is.readBegin("powerSeriesReactionRate(Istream&)"))),
    beta_(readScalar(is)),
    Ta_(readScalar(is)),
    coeffs_(is)
{
    is.readEnd("powerSeriesReactionRate(Istream&)");
}


inline CML::powerSeriesReactionRate::powerSeriesReactionRate
(
    const speciesTable&,
    const dictionary& dict
)
:
    A_(readScalar(dict.lookup("A"))),
    beta_(readScalar(dict.lookup("beta"))),
    Ta_(readScalar(dict.lookup("Ta"))),
    coeffs_(dict.lookup("coeffs"))
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

inline CML::scalar CML::powerSeriesReactionRate::operator()
(
    const scalar T,
    const scalar,
    const scalarField&
) const
{
    scalar lta = A_;

    if (mag(beta_) > VSMALL)
    {
        lta *= pow(T, beta_);
    }

    scalar expArg = 0.0;

    forAll(coeffs_, n)
    {
        expArg += coeffs_[n]/pow(T, n + 1);
    }

    lta *= exp(expArg);

    return lta;
}


inline void CML::powerSeriesReactionRate::write(Ostream& os) const
{
    os.writeKeyword("A") << A_ << token::END_STATEMENT << nl;
    os.writeKeyword("beta") << beta_ << token::END_STATEMENT << nl;
    os.writeKeyword("Ta") << Ta_ << token::END_STATEMENT << nl;
    os.writeKeyword("coeffs") << coeffs_ << token::END_STATEMENT << nl;
}


inline CML::Ostream& CML::operator<<
(
    Ostream& os,
    const powerSeriesReactionRate& psrr
)
{
    os  << token::BEGIN_LIST
        << psrr.A_ << token::SPACE << psrr.beta_ << token::SPACE << psrr.Ta_;

    for (int n=0; n<powerSeriesReactionRate::nCoeff_; n++)
    {
        os  << token::SPACE << psrr.coeffs_[n];
    }

    os << token::END_LIST;

    return os;
}


// ************************************************************************* //
