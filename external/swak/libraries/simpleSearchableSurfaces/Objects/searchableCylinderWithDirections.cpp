/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright (C) 1991-2008 OpenCFD Ltd.
-------------------------------------------------------------------------------
License
    This file is based on CAELUS.

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

Contributors/Copyright:
    2009, 2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#include "searchableCylinderWithDirections.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{

defineTypeNameAndDebug(searchableCylinderWithDirections, 0);
addToRunTimeSelectionTable(searchableSurface, searchableCylinderWithDirections, dict);

}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::searchableCylinderWithDirections::recalcIndex(
    List<pointIndexHit>& info
) const
{
    scalar magDir=mag(point2_-point1_);
    vector unitDir=(point2_-point1_)/magDir;

    forAll(info, i)
    {
        if (info[i].hit())
        {
            vector v(info[i].hitPoint() - point1_);

            // Decompose sample-point1 into normal and parallel component
            scalar parallel = v & unitDir;

            if (mag(parallel) < SMALL)
            {
                info[i].setIndex(0);
            }
            else if (mag(parallel-magDir)<SMALL)
            {
                info[i].setIndex(1);
            }
            else
            {
                // Remove the parallel component
                info[i].setIndex(2);
            }
        }
    }
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::searchableCylinderWithDirections::searchableCylinderWithDirections
(
    const IOobject& io,
    const dictionary& dict
)
:
    searchableCylinder(
        io,
        point(dict.lookup("point1")),
        point(dict.lookup("point2")),
        readScalar(dict.lookup("radius"))
    ),
    point1_(dict.lookup("point1")),
    point2_(dict.lookup("point2"))
{
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::searchableCylinderWithDirections::~searchableCylinderWithDirections()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const CML::wordList& CML::searchableCylinderWithDirections::regions() const
{
    if (regions_.size() == 0)
    {
        regions_.setSize(3);
        regions_[0] = "top";
        regions_[1] = "bottom";
        regions_[2] = "hull";
    }
    return regions_;
}

CML::tmp<CML::pointField>
CML::searchableCylinderWithDirections::coordinates() const
{
    pointField result(3);

    result[0]=point1_;
    result[1]=point2_;
    result[2]=0.5*(point1_+point2_);

    return tmp<pointField>(new pointField(result));
}


void CML::searchableCylinderWithDirections::getRegion
(
    const List<pointIndexHit>& info,
    labelList& region
) const
{
    region.setSize(info.size());
    forAll(info,i) {
        region[i] = info[i].index();
    }
}

void CML::searchableCylinderWithDirections::findNearest
(
    const pointField& sample,
    const scalarField& nearestDistSqr,
    List<pointIndexHit>&info
) const
{
    searchableCylinder::findNearest(
        sample,
        nearestDistSqr,
        info
    );

    recalcIndex(info);
}

void CML::searchableCylinderWithDirections::findLine
(
    const pointField& start,
    const pointField& end,
    List<pointIndexHit>&info
) const
{
    searchableCylinder::findLine(
        start,
        end,
        info
    );

    recalcIndex(info);
}

void CML::searchableCylinderWithDirections::findLineAny
(
    const pointField& start,
    const pointField& end,
    List<pointIndexHit>& info
) const
{
    searchableCylinder::findLine(
        start,
        end,
        info
    );

    recalcIndex(info);
}

//- Get all intersections in order from start to end.
void CML::searchableCylinderWithDirections::findLineAll
(
    const pointField& start,
    const pointField& end,
    List<List<pointIndexHit> >&info
) const
{
    searchableCylinder::findLineAll(
        start,
        end,
        info
    );

    forAll(info,i) {
        recalcIndex(info[i]);
    }
}

// ************************************************************************* //
