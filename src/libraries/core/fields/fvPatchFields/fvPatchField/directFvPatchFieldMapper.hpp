/*---------------------------------------------------------------------------*\
Copyright (C) 2013 OpenFOAM Foundation
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
    CML::directFvPatchFieldMapper

Description
    direct fvPatchFieldMapper

\*---------------------------------------------------------------------------*/

#ifndef directFvPatchFieldMapper_H
#define directFvPatchFieldMapper_H

#include "fvPatchFieldMapper.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                       Class directFvPatchFieldMapper Declaration
\*---------------------------------------------------------------------------*/

class directFvPatchFieldMapper
:
    public fvPatchFieldMapper
{

    //- Addressing from new back to old
    const labelUList& directAddressing_;

    //- Does map contain any unmapped values
    bool hasUnmapped_;


public:

    // Constructors

        //- Construct given addressing
        directFvPatchFieldMapper(const labelUList& directAddressing)
        :
            directAddressing_(directAddressing),
            hasUnmapped_(false)
        {
            if (directAddressing_.size() && min(directAddressing_) < 0)
            {
                hasUnmapped_ = true;
            }
        }

    //- Destructor
    virtual ~directFvPatchFieldMapper()
    {}


    // Member Functions

        label size() const
        {
            return directAddressing_.size();
        }

        bool direct() const
        {
            return true;
        }

        bool hasUnmapped() const
        {
            return hasUnmapped_;
        }

        const labelUList& directAddressing() const
        {
            return directAddressing_;
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
