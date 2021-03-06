/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2016 OpenFOAM Foundation
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

Description
    Reads a char from an input stream, for a given version
    number and File format. If an ascii File is being read, then the line
    numbers are counted and an erroneous read is reported.

\*---------------------------------------------------------------------------*/

#include "error.hpp"

#include "char.hpp"
#include "IOstreams.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

char CML::readChar(Istream& is)
{
   char c;
   is.read(c);
   return c;
}


CML::Istream& CML::operator>>(Istream& is, char& c)
{
    is.read(c);
    is.check("Istream& operator>>(Istream&, char&)");
    return is;
}


CML::Ostream& CML::operator<<(Ostream& os, const char c)
{
    os.write(c);
    os.check("Ostream& operator<<(Ostream&, const char)");
    return os;
}


CML::Ostream& CML::operator<<(Ostream& os, const char* s)
{
    os.write(s);
    os.check("Ostream& operator<<(Ostream&, const char*)");
    return os;
}


// ************************************************************************* //
