/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
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

Class
    CML::TimeState

Description
    The time value with time-stepping information, user-defined remapping, etc.

SourceFiles
    TimeState.cpp

\*---------------------------------------------------------------------------*/

#ifndef TimeState_H
#define TimeState_H

#include "dimensionedScalar.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class TimeState Declaration
\*---------------------------------------------------------------------------*/


class TimeState
:
    public dimensionedScalar
{

protected:

        label timeIndex_;
        scalar deltaT_;
        scalar deltaTSave_;
        scalar deltaT0_;
        bool deltaTchanged_;

        label outputTimeIndex_;
        label secondaryOutputTimeIndex_;
        bool  outputTime_;

public:

    // Constructors

        TimeState();


    //- Destructor
    virtual ~TimeState();


    // Member functions

        // Access

            //- Convert the user-time (e.g. CA deg) to real-time (s).
            virtual scalar userTimeToTime(const scalar theta) const;

            //- Convert the real-time (s) into user-time (e.g. CA deg)
            virtual scalar timeToUserTime(const scalar t) const;

            //- Return current time value
            scalar timeOutputValue() const;

            //- Return current time index
            label timeIndex() const;

            //- Return time step value
            inline scalar deltaTValue() const
            {
                return deltaT_;
            }

            //- Return old time step value
            inline scalar deltaT0Value() const
            {
                return deltaT0_;
            }

            //- Return time step
            dimensionedScalar deltaT() const;

            //- Return old time step
            dimensionedScalar deltaT0() const;


        // Check

            //- Return true if this is an output time
            bool outputTime() const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
