/*---------------------------------------------------------------------------*\
Copyright (C) 2015 Applied CCM
Copyright (C) 2011 OpenCFD
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

Global
    cfl.hpp

Description
    Compute the compressible CFL number

Author
    Aleksandar Jemcov

\*---------------------------------------------------------------------------*/
scalar maxDeltaT
(
    pimple.dict().lookupOrDefault<scalar>("maxDeltaT", GREAT)
);

volScalarField rDeltaT
(
    IOobject
    (
        "rDeltaT",
        runTime.timeName(),
        mesh,
        IOobject::NO_READ,
        IOobject::AUTO_WRITE
    ),
    mesh,
    1/dimensionedScalar("maxDeltaT", dimTime, maxDeltaT),
    zeroGradientFvPatchScalarField::typeName
);

