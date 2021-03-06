/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2017 OpenFOAM Foundation
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

#include "solidParticle.hpp"
#include "IOstreams.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

const std::size_t CML::solidParticle::sizeofFields_
(
    sizeof(solidParticle) - sizeof(particle)
);


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::solidParticle::solidParticle
(
    const polyMesh& mesh,
    Istream& is,
    bool readFields
)
:
    particle(mesh, is, readFields)
{
    if (readFields)
    {
        if (is.format() == IOstream::ASCII)
        {
            d_ = readScalar(is);
            is >> U_;
        }
        else
        {
            is.read(reinterpret_cast<char*>(&d_), sizeofFields_);
        }
    }

    // Check state of Istream
    is.check(FUNCTION_NAME);
}


void CML::solidParticle::readFields(Cloud<solidParticle>& c)
{
    if (!c.size())
    {
        return;
    }

    particle::readFields(c);

    IOField<scalar> d(c.fieldIOobject("d", IOobject::MUST_READ));
    c.checkFieldIOobject(c, d);

    IOField<vector> U(c.fieldIOobject("U", IOobject::MUST_READ));
    c.checkFieldIOobject(c, U);

    label i = 0;
    forAllIter(Cloud<solidParticle>, c, iter)
    {
        solidParticle& p = iter();

        p.d_ = d[i];
        p.U_ = U[i];
        i++;
    }
}


void CML::solidParticle::writeFields(const Cloud<solidParticle>& c)
{
    particle::writeFields(c);

    label np = c.size();

    IOField<scalar> d(c.fieldIOobject("d", IOobject::NO_READ), np);
    IOField<vector> U(c.fieldIOobject("U", IOobject::NO_READ), np);

    label i = 0;
    forAllConstIter(Cloud<solidParticle>, c, iter)
    {
        const solidParticle& p = iter();

        d[i] = p.d_;
        U[i] = p.U_;
        i++;
    }

    d.write();
    U.write();
}


// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

CML::Ostream& CML::operator<<(Ostream& os, const solidParticle& p)
{
    if (os.format() == IOstream::ASCII)
    {
        os  << static_cast<const particle&>(p)
            << token::SPACE << p.d_
            << token::SPACE << p.U_;
    }
    else
    {
        os  << static_cast<const particle&>(p);
        os.write
        (
            reinterpret_cast<const char*>(&p.d_),
            solidParticle::sizeofFields_
        );
    }

    // Check state of Ostream
    os.check("Ostream& operator<<(Ostream&, const solidParticle&)");

    return os;
}


// ************************************************************************* //
