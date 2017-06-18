/*---------------------------------------------------------------------------*\
Copyright (C) 2016 OpenFOAM Foundation
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

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Cmpt>
inline CML::CompactSpatialTensor<Cmpt>::CompactSpatialTensor()
{}


template<class Cmpt>
inline CML::CompactSpatialTensor<Cmpt>::CompactSpatialTensor
(
    const CML::zero
)
:
    CompactSpatialTensor::msType(Zero)
{}


template<class Cmpt>
inline CML::CompactSpatialTensor<Cmpt>::CompactSpatialTensor
(
    const typename CompactSpatialTensor::msType& ms
)
:
    CompactSpatialTensor::msType(ms)
{}


template<class Cmpt>
inline CML::CompactSpatialTensor<Cmpt>::CompactSpatialTensor
(
    const Cmpt& t00, const Cmpt& t01, const Cmpt& t02,
    const Cmpt& t10, const Cmpt& t11, const Cmpt& t12,
    const Cmpt& t20, const Cmpt& t21, const Cmpt& t22,
    const Cmpt& t30, const Cmpt& t31, const Cmpt& t32,
    const Cmpt& t40, const Cmpt& t41, const Cmpt& t42,
    const Cmpt& t50, const Cmpt& t51, const Cmpt& t52
)
{
    this->v_[0] = t00;
    this->v_[1] = t01;
    this->v_[2] = t02;

    this->v_[6 + 0] = t10;
    this->v_[6 + 1] = t11;
    this->v_[6 + 2] = t12;

    this->v_[12 + 0] = t20;
    this->v_[12 + 1] = t21;
    this->v_[12 + 2] = t22;

    this->v_[18 + 0] = t30;
    this->v_[18 + 1] = t31;
    this->v_[18 + 2] = t32;

    this->v_[24 + 0] = t40;
    this->v_[24 + 1] = t41;
    this->v_[24 + 2] = t42;

    this->v_[30 + 0] = t50;
    this->v_[30 + 1] = t51;
    this->v_[30 + 2] = t52;
}


template<class Cmpt>
inline CML::CompactSpatialTensor<Cmpt>::CompactSpatialTensor(Istream& is)
:
    CompactSpatialTensor::msType(is)
{}


// ************************************************************************* //