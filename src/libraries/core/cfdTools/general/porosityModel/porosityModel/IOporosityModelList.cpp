/*---------------------------------------------------------------------------*\
Copyright (C) 2012 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Caelus is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "IOporosityModelList.hpp"
#include "fvMesh.hpp"
#include "Time.hpp"

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

CML::IOobject CML::IOporosityModelList::createIOobject
(
    const fvMesh& mesh
) const
{
    IOobject io
    (
        "porosityProperties",
        mesh.time().constant(),
        mesh,
        IOobject::MUST_READ,
        IOobject::NO_WRITE
    );

    if (io.headerOk())
    {
        Info<< "Creating porosity model list from " << io.name() << nl << endl;

        io.readOpt() = IOobject::MUST_READ_IF_MODIFIED;
        return io;
    }
    else
    {
        Info<< "No porosity models present" << nl << endl;

        io.readOpt() = IOobject::NO_READ;
        return io;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::IOporosityModelList::IOporosityModelList
(
    const fvMesh& mesh
)
:
    IOdictionary(createIOobject(mesh)),
    porosityModelList(mesh, *this)
{}


bool CML::IOporosityModelList::read()
{
    if (regIOobject::read())
    {
        porosityModelList::read(*this);
        return true;
    }
    else
    {
        return false;
    }
}


// ************************************************************************* //

