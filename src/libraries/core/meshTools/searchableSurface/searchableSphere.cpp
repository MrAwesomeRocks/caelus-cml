/*---------------------------------------------------------------------------*\
Copyright (C) 2011 OpenFOAM Foundation
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

#include "searchableSphere.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{

defineTypeNameAndDebug(searchableSphere, 0);
addToRunTimeSelectionTable(searchableSurface, searchableSphere, dict);

}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

CML::pointIndexHit CML::searchableSphere::findNearest
(
    const point& sample,
    const scalar nearestDistSqr
) const
{
    pointIndexHit info(false, sample, -1);

    const vector n(sample - centre_);
    scalar magN = mag(n);

    if (nearestDistSqr >= sqr(magN - radius_))
    {
        if (magN < ROOTVSMALL)
        {
            info.rawPoint() = centre_ + vector(1,0,0)*radius_;
        }
        else
        {
            info.rawPoint() = centre_ + n/magN*radius_;
        }
        info.setHit();
        info.setIndex(0);
    }

    return info;
}


// From Graphics Gems - intersection of sphere with ray
void CML::searchableSphere::findLineAll
(
    const point& start,
    const point& end,
    pointIndexHit& near,
    pointIndexHit& far
) const
{
    near.setMiss();
    far.setMiss();

    vector dir(end-start);
    scalar magSqrDir = magSqr(dir);

    if (magSqrDir > ROOTVSMALL)
    {
        const vector toCentre(centre_-start);
        scalar magSqrToCentre = magSqr(toCentre);

        dir /= CML::sqrt(magSqrDir);

        scalar v = (toCentre & dir);

        scalar disc = sqr(radius_) - (magSqrToCentre - sqr(v));

        if (disc >= 0)
        {
            scalar d = CML::sqrt(disc);

            scalar nearParam = v-d;

            if (nearParam >= 0 && sqr(nearParam) <= magSqrDir)
            {
                near.setHit();
                near.setPoint(start + nearParam*dir);
                near.setIndex(0);
            }

            scalar farParam = v+d;

            if (farParam >= 0 && sqr(farParam) <= magSqrDir)
            {
                far.setHit();
                far.setPoint(start + farParam*dir);
                far.setIndex(0);
            }
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::searchableSphere::searchableSphere
(
    const IOobject& io,
    const point& centre,
    const scalar radius
)
:
    searchableSurface(io),
    centre_(centre),
    radius_(radius)
{
    bounds() = boundBox
    (
        centre_ - radius_*vector::one,
        centre_ + radius_*vector::one
    );
}


CML::searchableSphere::searchableSphere
(
    const IOobject& io,
    const dictionary& dict
)
:
    searchableSurface(io),
    centre_(dict.lookup("centre")),
    radius_(readScalar(dict.lookup("radius")))
{
    bounds() = boundBox
    (
        centre_ - radius_*vector::one,
        centre_ + radius_*vector::one
    );
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::searchableSphere::~searchableSphere()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool CML::searchableSphere::overlaps(const boundBox& bb) const
{
    return bb.overlaps(centre_, sqr(radius_));
}


const CML::wordList& CML::searchableSphere::regions() const
{
    if (regions_.empty())
    {
        regions_.setSize(1);
        regions_[0] = "region0";
    }
    return regions_;
}



void CML::searchableSphere::boundingSpheres
(
    pointField& centres,
    scalarField& radiusSqr
) const
{
    centres.setSize(1);
    centres[0] = centre_;

    radiusSqr.setSize(1);
    radiusSqr[0] = CML::sqr(radius_);

    // Add a bit to make sure all points are tested inside
    radiusSqr += CML::sqr(SMALL);
}


void CML::searchableSphere::findNearest
(
    const pointField& samples,
    const scalarField& nearestDistSqr,
    List<pointIndexHit>& info
) const
{
    info.setSize(samples.size());

    forAll(samples, i)
    {
        info[i] = findNearest(samples[i], nearestDistSqr[i]);
    }
}


void CML::searchableSphere::findLine
(
    const pointField& start,
    const pointField& end,
    List<pointIndexHit>& info
) const
{
    info.setSize(start.size());

    pointIndexHit b;

    forAll(start, i)
    {
        // Pick nearest intersection. If none intersected take second one.
        findLineAll(start[i], end[i], info[i], b);
        if (!info[i].hit() && b.hit())
        {
            info[i] = b;
        }
    }
}


void CML::searchableSphere::findLineAny
(
    const pointField& start,
    const pointField& end,
    List<pointIndexHit>& info
) const
{
    info.setSize(start.size());

    pointIndexHit b;

    forAll(start, i)
    {
        // Discard far intersection
        findLineAll(start[i], end[i], info[i], b);
        if (!info[i].hit() && b.hit())
        {
            info[i] = b;
        }
    }
}


void CML::searchableSphere::findLineAll
(
    const pointField& start,
    const pointField& end,
    List<List<pointIndexHit> >& info
) const
{
    info.setSize(start.size());

    forAll(start, i)
    {
        pointIndexHit near, far;
        findLineAll(start[i], end[i], near, far);

        if (near.hit())
        {
            if (far.hit())
            {
                info[i].setSize(2);
                info[i][0] = near;
                info[i][1] = far;
            }
            else
            {
                info[i].setSize(1);
                info[i][0] = near;
            }
        }
        else
        {
            if (far.hit())
            {
                info[i].setSize(1);
                info[i][0] = far;
            }
            else
            {
                info[i].clear();
            }
        }
    }
}


void CML::searchableSphere::getRegion
(
    const List<pointIndexHit>& info,
    labelList& region
) const
{
    region.setSize(info.size());
    region = 0;
}


void CML::searchableSphere::getNormal
(
    const List<pointIndexHit>& info,
    vectorField& normal
) const
{
    normal.setSize(info.size());
    normal = vector::zero;

    forAll(info, i)
    {
        if (info[i].hit())
        {
            normal[i] = info[i].hitPoint() - centre_;

            normal[i] /= mag(normal[i])+VSMALL;
        }
        else
        {
            // Set to what?
        }
    }
}


void CML::searchableSphere::getVolumeType
(
    const pointField& points,
    List<volumeType>& volType
) const
{
    volType.setSize(points.size());
    volType = volumeType::INSIDE;

    forAll(points, pointI)
    {
        const point& pt = points[pointI];

        if (magSqr(pt - centre_) <= sqr(radius_))
        {
            volType[pointI] = volumeType::INSIDE;
        }
        else
        {
            volType[pointI] = volumeType::OUTSIDE;
        }
    }
}


// ************************************************************************* //
