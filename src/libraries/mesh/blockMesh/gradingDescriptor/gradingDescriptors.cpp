/*---------------------------------------------------------------------------*\
Copyright (C) 2015 OpenFOAM Foundation
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

#include "gradingDescriptors.hpp"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::gradingDescriptors::gradingDescriptors()
:
    List<gradingDescriptor>(1, gradingDescriptor())
{}


CML::gradingDescriptors::gradingDescriptors(const gradingDescriptor& gd)
:
    List<gradingDescriptor>(1, gd)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::gradingDescriptors CML::gradingDescriptors::inv() const
{
    gradingDescriptors ret(*this);

    forAll(ret, i)
    {
        ret[i] = operator[](i).inv();
    }

    return ret;
}


// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //

CML::Istream& CML::operator>>(Istream& is, gradingDescriptors& gds)
{
    // Examine next token
    token t(is);

    if (t.isNumber())
    {
        gds = gradingDescriptors(gradingDescriptor(t.number()));
    }
    else
    {
        is.putBack(t);

        // Read the list for gradingDescriptors
        is >> static_cast<List<gradingDescriptor>& >(gds);

        // Check state of Istream
        is.check("operator>>(Istream&, gradingDescriptor&)");

        // Normalize the blockFractions and nDivFractions
        // of the list of gradingDescriptors

        scalar sumBlockFraction = 0;
        scalar sumNDivFraction = 0;

        forAll(gds, i)
        {
            sumBlockFraction += gds[i].blockFraction_;
            sumNDivFraction += gds[i].nDivFraction_;
        }

        forAll(gds, i)
        {
            gds[i].blockFraction_ /= sumBlockFraction;
            gds[i].nDivFraction_ /= sumNDivFraction;
        }
    }

    return is;
}


// ************************************************************************* //
