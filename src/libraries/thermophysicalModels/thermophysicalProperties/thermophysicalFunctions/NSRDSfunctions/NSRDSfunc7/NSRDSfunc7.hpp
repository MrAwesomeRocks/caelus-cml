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
    CML::NSRDSfunc7

Description
    NSRDS-AICHE function number 107

    Source:
    \verbatim
                      NSRDS - AICHE
                 Data Compilation Tables
                    of Properties of
                     Pure Compounds

        Design Institute for Physical Property Data
          American Institute of Chemical Engineers
                  345 East 47th Street
                New York, New York 10017

         National Standard Reference Data System
         American Institute of Chemical Engineers

          T.E. Daubert       -       R.P. Danner

            Department of Chemical Engineering
            The Pennsylvania State University
                University Park, PA 16802
    \endverbatim

\*---------------------------------------------------------------------------*/

#ifndef NSRDSfunc7_HPP
#define NSRDSfunc7_HPP

#include "thermophysicalFunction.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class NSRDSfunc7 Declaration
\*---------------------------------------------------------------------------*/

class NSRDSfunc7
:
    public thermophysicalFunction
{

    // NSRDS function 107 coefficients
    scalar a_, b_, c_, d_, e_;


public:

    //- Runtime type information
    TypeName("NSRDSfunc7");


    //- Construct from components
    NSRDSfunc7
    (
        const scalar a,
        const scalar b,
        const scalar c,
        const scalar d,
        const scalar e
    );

    //- Construct from dictionary
    NSRDSfunc7(const dictionary& dict);


    // Member Functions

    //- Evaluate the function and return the result
    scalar f(scalar, scalar T) const
    {
        return a_ + b_*sqr((c_/T)/sinh(c_/T)) + d_*sqr((e_/T)/cosh(e_/T));
    }

    //- Write the function coefficients
    void writeData(Ostream& os) const
    {
        os  << a_ << token::SPACE
            << b_ << token::SPACE
            << c_ << token::SPACE
            << d_ << token::SPACE
            << e_;
    }


    // Ostream Operator
    friend Ostream& operator<<(Ostream& os, const NSRDSfunc7& f)
    {
        f.writeData(os);
        return os;
    }

};


} // End namespace CML


#endif
