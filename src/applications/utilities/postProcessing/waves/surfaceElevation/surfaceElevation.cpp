/*---------------------------------------------------------------------------*\
Copyright Niels Gjøl Jacobsen, Technical University of Denmark.
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    Caelus is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Description
    Sample field data with a choice of interpolation schemes, sampling options
    and write formats.

    Keywords:

    @param setFormat : set output format, choice of \n
      - xmgr
      - jplot
      - gnuplot
      - raw

    @param surfaceFormat : surface output format, choice of \n
      - null        : suppress output
      - foamFile    : separate points, faces and values file
      - dx          : DX scalar or vector format
      - vtk         : VTK ascii format
      - raw         : x y z value format for use with e.g. gnuplot 'splot'.
      - obj         : Wavefron stl. Does not contain values!
      - stl         : ascii stl. Does not contain values!

    @param interpolationScheme : interpolation scheme, choice of \n
      - cell          : use cell-centre value; constant over cells (default)
      - cellPoint     : use cell-centre and vertex values
      - cellPointFace : use cell-centre, vertex and face values. \n
        -# vertex values determined from neighbouring cell-centre values
        -# face values determined using the current face interpolation scheme
           for the field (linear, limitedLinear, etc.)

    @param fields : list of fields to sample

    @param sets : list of sets to sample, choice of \n
      - uniform            evenly distributed points on line
      - face               one point per face intersection
      - midPoint           one point per cell, inbetween two face intersections
      - midPointAndFace    combination of face and midPoint

      - curve              specified points, not necessary on line, uses
                           tracking
      - cloud              specified points, uses findCell

        Option axis: how to write point coordinate. Choice of
          - x/y/z: x/y/z coordinate only
          - xyz: three columns
            (probably does not make sense for anything but raw)
          - distance: distance from start of sampling line (if uses line)
            or distance from first specified sampling point

        Type specific options:
            uniform, face, midPoint, midPointAndFace : start and end coordinate
            uniform: extra number of sampling points
            curve, cloud: list of coordinates

    @param surfaces : list of surfaces to sample, choice of \n
      - plane : values on plane defined by point, normal.
      - patch : values on patch.

Notes
    Runs in parallel

\*---------------------------------------------------------------------------*/

#include "argList.hpp"
#include "timeSelector.hpp"
#include "IOsampledSurfaceElevation.hpp"
#include "sampledSurfaceElevation.hpp"
#include "uniformDimensionedFields.hpp"

using namespace CML;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
// Main program:

int main(int argc, char *argv[])
{
    timeSelector::addOptions();
#   include "addRegionOption.hpp"
#   include "setRootCase.hpp"
#   include "createTime.hpp"
    instantList timeDirs = timeSelector::select0(runTime, args);
#   include "createNamedMesh.hpp"

#   include "readGravitationalAcceleration.hpp"
#   include "readWaveProperties.hpp"

    fileName dict("system/surfaceElevationDict");

    IOsampledSurfaceElevation sSets
    (
        sampledSurfaceElevation::typeName,
        mesh,
        dict,
        IOobject::MUST_READ,
        true
    );

    forAll (timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI], timeI);
        Info<< "Time = " << runTime.timeName() << endl;

        // Handle geometry/topology changes
        polyMesh::readUpdateState state = mesh.readUpdate();

        sSets.readUpdate(state);

        sSets.write();

        Info<< endl;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
