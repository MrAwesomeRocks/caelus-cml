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

Class
    CML::SRIFallOffFunction

Description
    The SRI fall-off function


\*---------------------------------------------------------------------------*/

#ifndef SRIFallOffFunction_HPP
#define SRIFallOffFunction_HPP

#include "scalar.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of friend functions and operators

class SRIFallOffFunction;
Ostream& operator<<(Ostream&, const SRIFallOffFunction&);


/*---------------------------------------------------------------------------*\
                    Class SRIFallOffFunction Declaration
\*---------------------------------------------------------------------------*/

class SRIFallOffFunction
{

    scalar a_, b_, c_, d_, e_;


public:


    //- Construct from components
    inline SRIFallOffFunction
    (
        const scalar a,
        const scalar b,
        const scalar c,
        const scalar d,
        const scalar e
    )
    :
        a_(a),
        b_(b),
        c_(c),
        d_(d),
        e_(e)
    {}

    //- Construct from dictionary
    inline SRIFallOffFunction(const dictionary& dict)
    :
        a_(readScalar(dict.lookup("a"))),
        b_(readScalar(dict.lookup("b"))),
        c_(readScalar(dict.lookup("c"))),
        d_(readScalar(dict.lookup("d"))),
        e_(readScalar(dict.lookup("e")))
    {}


    // Member Functions

    //- Return the type name
    static word type()
    {
        return "SRI";
    }

    inline scalar operator()
    (
        const scalar T,
        const scalar Pr
    ) const
    {
        scalar X = 1.0/(1 + sqr(log10(max(Pr, SMALL))));
        return d_*pow(a_*exp(-b_/T) + exp(-T/c_), X)*pow(T, e_);
    }

    inline scalar ddT
    (
        const scalar Pr,
        const scalar F,
        const scalar dPrdT,
        const scalar T
    ) const
    {
        scalar X = 1.0/(1 + sqr(log10(max(Pr, SMALL))));
        scalar dXdPr = -X*X*2*log10(Pr)/Pr/log(10.0);
        return
            F
           *(
                e_/T
              + X
               *(a_*b_*exp(-b_/T)/sqr(T) - exp(-T/c_)/c_)
               /(a_*exp(-b_/T) + exp(-T/c_))
              + dXdPr*dPrdT*log(a_*exp(-b_/T) + exp(-T/c_))
            );
    }

    inline scalar ddc
    (
        const scalar Pr,
        const scalar F,
        const scalar dPrdc,
        const scalar T
    ) const
    {
        scalar X = 1.0/(1 + sqr(log10(max(Pr, SMALL))));
        scalar dXdPr = -X*X*2*log10(Pr)/Pr/log(10.0);
        scalar dXdc = dXdPr*dPrdc;
        return F*dXdc*log(a_*exp(-b_/T) + exp(-T/c_));
    }

    //- Write to stream
    inline void write(Ostream& os) const
    {
        os.writeKeyword("a") << a_ << token::END_STATEMENT << nl;
        os.writeKeyword("b") << b_ << token::END_STATEMENT << nl;
        os.writeKeyword("c") << c_ << token::END_STATEMENT << nl;
        os.writeKeyword("d") << d_ << token::END_STATEMENT << nl;
        os.writeKeyword("e") << e_ << token::END_STATEMENT << nl;
    }


    // Ostream Operator

    friend Ostream& operator<<
    (
        Ostream& os,
        const SRIFallOffFunction& srifof
    )
    {
        srifof.write(os);
        return os;
    }

};


} // End namespace CML


#endif
