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

#include "sampledSurfaces.hpp"
#include "volFields.hpp"
#include "dictionary.hpp"
#include "Time.hpp"
#include "IOmanip.hpp"
#include "volPointInterpolation.hpp"
#include "PatchTools.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(sampledSurfaces, 0);
}


bool CML::sampledSurfaces::verbose_ = false;

CML::scalar CML::sampledSurfaces::mergeTol_ = 1e-10;


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::sampledSurfaces::writeGeometry() const
{
    // Write to time directory under outputPath_
    // Skip surface without faces (eg, a failed cut-plane)

    const fileName outputDir = outputPath_/mesh_.time().timeName();

    forAll(*this, surfI)
    {
        const sampledSurface& s = operator[](surfI);

        if (Pstream::parRun())
        {
            if (Pstream::master() && mergeList_[surfI].faces.size())
            {
                formatter_->write
                (
                    outputDir,
                    s.name(),
                    mergeList_[surfI].points,
                    mergeList_[surfI].faces
                );
            }
        }
        else if (s.faces().size())
        {
            formatter_->write
            (
                outputDir,
                s.name(),
                s.points(),
                s.faces()
            );
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::sampledSurfaces::sampledSurfaces
(
    const word& name,
    const objectRegistry& obr,
    const dictionary& dict,
    const bool loadFromFiles
)
:
    PtrList<sampledSurface>(),
    name_(name),
    mesh_(refCast<const fvMesh>(obr)),
    loadFromFiles_(loadFromFiles),
    outputPath_(fileName::null),
    fieldSelection_(),
    interpolationScheme_(word::null),
    mergeList_(),
    formatter_(nullptr)
{
    if (Pstream::parRun())
    {
        outputPath_ = mesh_.time().path()/".."/"postProcessing"/name_;
    }
    else
    {
        outputPath_ = mesh_.time().path()/"postProcessing"/name_;
    }

    read(dict);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::sampledSurfaces::~sampledSurfaces()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::sampledSurfaces::verbose(const bool verbosity)
{
    verbose_ = verbosity;
}


void CML::sampledSurfaces::execute()
{
    // Do nothing - only valid on write
}


void CML::sampledSurfaces::end()
{
    // Do nothing - only valid on write
}


void CML::sampledSurfaces::timeSet()
{
    // Do nothing - only valid on write
}


void CML::sampledSurfaces::write()
{
    if (size())
    {
        // Finalize surfaces, merge points etc.
        update();

        const label nFields = classifyFields();

        if (Pstream::master())
        {
            if (debug)
            {
                Pout<< "Creating directory "
                    << outputPath_/mesh_.time().timeName() << nl << endl;

            }

            mkDir(outputPath_/mesh_.time().timeName());
        }

        // Write geometry first if required,
        // or when no fields would otherwise be written
        if (nFields == 0 || formatter_->separateGeometry())
        {
            writeGeometry();
        }

        const IOobjectList objects(mesh_, mesh_.time().timeName());

        sampleAndWrite<volScalarField>(objects);
        sampleAndWrite<volVectorField>(objects);
        sampleAndWrite<volSphericalTensorField>(objects);
        sampleAndWrite<volSymmTensorField>(objects);
        sampleAndWrite<volTensorField>(objects);

        sampleAndWrite<surfaceScalarField>(objects);
        sampleAndWrite<surfaceVectorField>(objects);
        sampleAndWrite<surfaceSphericalTensorField>(objects);
        sampleAndWrite<surfaceSymmTensorField>(objects);
        sampleAndWrite<surfaceTensorField>(objects);
    }
}


void CML::sampledSurfaces::read(const dictionary& dict)
{
    bool surfacesFound = dict.found("surfaces");

    if (surfacesFound)
    {
        dict.lookup("fields") >> fieldSelection_;

        dict.lookup("interpolationScheme") >> interpolationScheme_;
        const word writeType(dict.lookup("surfaceFormat"));

        // Define the surface formatter
        // Optionally defined extra controls for the output formats
        formatter_ = surfaceWriter::New
        (
            writeType,
            dict.subOrEmptyDict("formatOptions").subOrEmptyDict(writeType)
        );

        PtrList<sampledSurface> newList
        (
            dict.lookup("surfaces"),
            sampledSurface::iNew(mesh_)
        );
        transfer(newList);

        if (Pstream::parRun())
        {
            mergeList_.setSize(size());
        }

        // Ensure all surfaces and merge information are expired
        expire();

        if (this->size())
        {
            Info<< "Reading surface description:" << nl;
            forAll(*this, surfI)
            {
                Info<< "    " << operator[](surfI).name() << nl;
            }
            Info<< endl;
        }
    }

    if (Pstream::master() && debug)
    {
        Pout<< "sample fields:" << fieldSelection_ << nl
            << "sample surfaces:" << nl << "(" << nl;

        forAll(*this, surfI)
        {
            Pout<< "  " << operator[](surfI).name() << endl;
        }
        Pout<< ")" << endl;
    }
}


void CML::sampledSurfaces::updateMesh(const mapPolyMesh&)
{
    expire();

    // pointMesh and interpolation will have been reset in mesh.update
}


void CML::sampledSurfaces::movePoints(const pointField&)
{
    expire();
}


void CML::sampledSurfaces::readUpdate(const polyMesh::readUpdateState state)
{
    if (state != polyMesh::UNCHANGED)
    {
        expire();
    }
}


bool CML::sampledSurfaces::needsUpdate() const
{
    forAll(*this, surfI)
    {
        if (operator[](surfI).needsUpdate())
        {
            return true;
        }
    }

    return false;
}


bool CML::sampledSurfaces::expire()
{
    bool justExpired = false;

    forAll(*this, surfI)
    {
        if (operator[](surfI).expire())
        {
            justExpired = true;
        }

        // Clear merge information
        if (Pstream::parRun())
        {
            mergeList_[surfI].clear();
        }
    }

    // true if any surfaces just expired
    return justExpired;
}


bool CML::sampledSurfaces::update()
{
    bool updated = false;

    if (!needsUpdate())
    {
        return updated;
    }

    // Serial: quick and easy, no merging required
    if (!Pstream::parRun())
    {
        forAll(*this, surfI)
        {
            if (operator[](surfI).update())
            {
                updated = true;
            }
        }

        return updated;
    }

    // Dimension as fraction of mesh bounding box
    scalar mergeDim = mergeTol_ * mesh_.bounds().mag();

    if (Pstream::master() && debug)
    {
        Pout<< nl << "Merging all points within "
            << mergeDim << " metre" << endl;
    }

    forAll(*this, surfI)
    {
        sampledSurface& s = operator[](surfI);

        if (s.update())
        {
            updated = true;
        }
        else
        {
            continue;
        }

        PatchTools::gatherAndMerge
        (
            mergeDim,
            primitivePatch
            (
                SubList<face>(s.faces(), s.faces().size()),
                s.points()
            ),
            mergeList_[surfI].points,
            mergeList_[surfI].faces,
            mergeList_[surfI].pointsMap
        );
    }

    return updated;
}


// ************************************************************************* //
