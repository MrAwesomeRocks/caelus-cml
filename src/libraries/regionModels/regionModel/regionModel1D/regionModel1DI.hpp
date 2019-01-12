/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2015 OpenFOAM Foundation
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

#include "regionModel1D.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

inline const CML::labelListList&
CML::regionModels::regionModel1D::boundaryFaceFaces() const
{
    return boundaryFaceFaces_;
}


inline const CML::labelListList&
CML::regionModels::regionModel1D::boundaryFaceCells() const
{
    return boundaryFaceCells_;
}


inline const CML::labelList&
CML::regionModels::regionModel1D::boundaryFaceOppositeFace() const
{
    return boundaryFaceOppositeFace_;
}


inline const CML::surfaceScalarField&
CML::regionModels::regionModel1D::nMagSf() const
{
    if (!nMagSfPtr_.valid())
    {
        FatalErrorInFunction
         << "Face normal areas not available" << abort(FatalError);
    }

    return nMagSfPtr_();
}


inline CML::label CML::regionModels::regionModel1D::nLayers() const
{
    return nLayers_;
}


// ************************************************************************* //
