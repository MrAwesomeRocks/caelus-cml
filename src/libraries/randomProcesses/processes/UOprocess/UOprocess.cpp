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

\*---------------------------------------------------------------------------*/

#include "error.hpp"

#include "UOprocess.hpp"
#include "Kmesh.hpp"
#include "dictionary.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

complexVector UOprocess::WeinerProcess()
{
    return RootDeltaT*complexVector
    (
        complex(GaussGen.scalarNormal(), GaussGen.scalarNormal()),
        complex(GaussGen.scalarNormal(), GaussGen.scalarNormal()),
        complex(GaussGen.scalarNormal(), GaussGen.scalarNormal())
    );
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// from components
UOprocess::UOprocess
(
    const Kmesh& kmesh,
    const scalar deltaT,
    const dictionary& UOdict
)
:
    GaussGen(label(0)),
    Mesh(kmesh),
    DeltaT(deltaT),
    RootDeltaT(sqrt(DeltaT)),
    UOfield(Mesh.size()),

    Alpha(readScalar(UOdict.lookup("UOalpha"))),
    Sigma(readScalar(UOdict.lookup("UOsigma"))),
    Kupper(readScalar(UOdict.lookup("UOKupper"))),
    Klower(readScalar(UOdict.lookup("UOKlower"))),
    Scale((Kupper - Klower)*pow(scalar(Mesh.size()), 1.0/vector::dim))
{
    const vectorField& K = Mesh;

    scalar sqrKupper = sqr(Kupper);
    scalar sqrKlower = sqr(Klower) + SMALL;
    scalar sqrK;

    forAll(UOfield, i)
    {
        if ((sqrK = magSqr(K[i])) < sqrKupper && sqrK > sqrKlower)
        {
            UOfield[i] = Scale*Sigma*WeinerProcess();
        }
        else
        {
            UOfield[i] = complexVector
            (
                complex(0, 0),
                complex(0, 0),
                complex(0, 0)
            );
        }
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const complexVectorField& UOprocess::newField()
{
    const vectorField& K = Mesh;

    label count = 0;
    scalar sqrKupper = sqr(Kupper);
    scalar sqrKlower = sqr(Klower) + SMALL;
    scalar sqrK;

    forAll(UOfield, i)
    {
        if ((sqrK = magSqr(K[i])) < sqrKupper && sqrK > sqrKlower)
        {
            count++;
            UOfield[i] =
                (1.0 - Alpha*DeltaT)*UOfield[i]
              + Scale*Sigma*WeinerProcess();
        }
    }

    Info<< "    Number of forced K = " << count << nl;

    return UOfield;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //

