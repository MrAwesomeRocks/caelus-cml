/*---------------------------------------------------------------------------*\
Copyright Niels G. Jacobsen and Sopheak Seng Technical University of Denmark
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

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

#include "sampledSurfaceElevation.hpp"
#include "dictionary.hpp"
#include "Time.hpp"
#include "volFields.hpp"
#include "ListListOps.hpp"
#include "SortableList.hpp"
#include "volPointInterpolation.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(sampledSurfaceElevation, 0);
}

bool CML::sampledSurfaceElevation::verbose_ = false;


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


bool CML::sampledSurfaceElevation::checkFieldTypes()
{
    wordList fieldTypes(fieldNames_.size());

    // check files for a particular time
    if (loadFromFiles_)
    {
        forAll (fieldNames_, fieldi)
        {
            IOobject io
            (
                fieldNames_[fieldi],
                mesh_.time().timeName(),
                mesh_,
                IOobject::MUST_READ,
                IOobject::NO_WRITE,
                false
            );

            if (io.headerOk())
            {
                fieldTypes[fieldi] = io.headerClassName();
            }
            else
            {
                fieldTypes[fieldi] = "(notFound)";
            }
        }
    }
    else
    {
        // check objectRegistry
        forAll (fieldNames_, fieldi)
        {
            objectRegistry::const_iterator iter =
                mesh_.find(fieldNames_[fieldi]);

            if (iter != mesh_.objectRegistry::end())
            {
                fieldTypes[fieldi] = iter()->type();
            }
            else
            {
                fieldTypes[fieldi] = "(notFound)";
            }
        }
    }

    label nFields = 0;

    // classify fieldTypes
    nFields += grep(scalarFields_, fieldTypes);

    if (Pstream::master())
    {
        if (debug)
        {
            Pout<< "timeName = " << mesh_.time().timeName() << nl
                << "scalarFields    " << scalarFields_ << nl;
        }
    }

    return nFields > 0;
}


void CML::sampledSurfaceElevation::combineSampledSets
(
    PtrList<coordSet>& masterSampledSets,
    labelListList& indexSets
)
{
    // Combine sampleSets from processors. Sort by curveDist. Return
    // ordering in indexSets.
    // Note: only master results are valid

    masterSampledSets_.clear();
    masterSampledSets_.setSize(size());
    indexSets_.setSize(size());

    const PtrList<sampledSet>& sampledSets = *this;

    forAll (sampledSets, seti)
    {
        const sampledSet& samplePts = sampledSets[seti];

        // Collect data from all processors
        List<List<point> > gatheredPts(Pstream::nProcs());
        gatheredPts[Pstream::myProcNo()] = samplePts;
        Pstream::gatherList(gatheredPts);

        List<labelList> gatheredSegments(Pstream::nProcs());
        gatheredSegments[Pstream::myProcNo()] = samplePts.segments();
        Pstream::gatherList(gatheredSegments);

        List<scalarList> gatheredDist(Pstream::nProcs());
        gatheredDist[Pstream::myProcNo()] = samplePts.curveDist();
        Pstream::gatherList(gatheredDist);

        // Combine processor lists into one big list.
        List<point> allPts
        (
            ListListOps::combine<List<point> >
            (
                gatheredPts, accessOp<List<point> >()
            )
        );
        labelList allSegments
        (
            ListListOps::combine<labelList>
            (
                gatheredSegments, accessOp<labelList>()
            )
        );
        scalarList allCurveDist
        (
            ListListOps::combine<scalarList>
            (
                gatheredDist, accessOp<scalarList>()
            )
        );

        // Sort curveDist and use to fill masterSamplePts
        SortableList<scalar> sortedDist(allCurveDist);
        indexSets[seti] = sortedDist.indices();

        masterSampledSets.set
        (
            seti,
            new coordSet
            (
                samplePts.name(),
                samplePts.axis(),
                List<point>(UIndirectList<point>(allPts, indexSets[seti])),
                allCurveDist
            )
        );
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::sampledSurfaceElevation::sampledSurfaceElevation
(
    const word& name,
    const objectRegistry& obr,
    const dictionary& dict,
    const bool loadFromFiles
)
:
    PtrList<sampledSet>(),
    name_(name),
    mesh_(refCast<const fvMesh>(obr)),
    loadFromFiles_(loadFromFiles),
    outputPath_(fileName::null),
    searchEngine_(mesh_),
    fieldNames_(),
    interpolationScheme_(word::null),
    writeFormat_(word::null),
    surfaceElevationFilePtr_(nullptr)
{
    if (Pstream::parRun())
    {
        outputPath_ = mesh_.time().path()/".."/name_;
    }
    else
    {
        outputPath_ = mesh_.time().path()/name_;
    }
    if (mesh_.name() != fvMesh::defaultRegion)
    {
        outputPath_ = outputPath_/mesh_.name();
    }

    read(dict);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //


CML::sampledSurfaceElevation::~sampledSurfaceElevation()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


void CML::sampledSurfaceElevation::verbose(const bool verbosity)
{
    verbose_ = verbosity;
}


void CML::sampledSurfaceElevation::execute()
{
    // Do nothing - only valid on write
}


void CML::sampledSurfaceElevation::end()
{
    // Do nothing - only valid on write
}


void CML::sampledSurfaceElevation::write()
{
    if (size() && checkFieldTypes())
    {
        sampleIntegrateAndWrite(scalarFields_);
    }
}


void CML::sampledSurfaceElevation::sampleIntegrateAndWrite
(
    fieldGroup<scalar>& fields
)
{
    if (fields.size())
    {
        scalarField result(0);
        result.setSize(0);

        bool interpolate = interpolationScheme_ != "cell";

        // Create or use existing writer
        if (fields.formatter.empty())
        {
            fields.formatter = writer<scalar>::New(writeFormat_);
        }

        // Storage for interpolated values
        PtrList<volFieldSampler<scalar> > sampledFields(fields.size());

        forAll (fields, fieldi)
        {
            if (Pstream::master() && verbose_)
            {
                Pout<< "sampledSets::sampleAndWrite: "
                    << fields[fieldi] << endl;
            }

            if (loadFromFiles_)
            {
                GeometricField<scalar, fvPatchField, volMesh> vf
                (
                    IOobject
                    (
                        fields[fieldi],
                        mesh_.time().timeName(),
                        mesh_,
                        IOobject::MUST_READ,
                        IOobject::NO_WRITE,
                        false
                    ),
                    mesh_
                );

                if (interpolate)
                {
                    sampledFields.set
                    (
                        fieldi,
                        new volFieldSampler<scalar>
                        (
                            interpolationScheme_,
                            vf,
                            *this
                        )
                    );
                }
                else
                {
                    sampledFields.set
                    (
                        fieldi,
                        new volFieldSampler<scalar>(vf, *this)
                    );
                }
            }
            else
            {
                if (interpolate)
                {
                    sampledFields.set
                    (
                        fieldi,
                        new volFieldSampler<scalar>
                        (
                            interpolationScheme_,
                            mesh_.lookupObject
                            <GeometricField<scalar, fvPatchField, volMesh> >
                            (fields[fieldi]),
                            *this
                        )
                    );
                }
                else
                {
                    sampledFields.set
                    (
                        fieldi,
                        new volFieldSampler<scalar>
                        (
                            mesh_.lookupObject
                            <GeometricField<scalar, fvPatchField, volMesh> >
                            (fields[fieldi]),
                            *this
                        )
                    );
                }
            }
        }

        // Combine sampled fields from processors.
        // Note: only master results are valid
        PtrList<volFieldSampler<scalar> > masterFields(sampledFields.size());
        combineSampledValues(sampledFields, indexSets_, masterFields);

        result.setSize(masterSampledSets_.size(), 0.0);

        if (Pstream::master())
        {
            forAll (masterSampledSets_, seti)
            {
                const coordSet & cs( masterSampledSets_[seti] );

                List< const Field<scalar>*> valueSets(masterFields.size());
                valueSets[0] = &masterFields[0][seti];

                List<const Field<scalar>*> columns(valueSets.size());
                columns[0] = valueSets[0];

                const Field<scalar>& alpha = *columns[0];
                scalar tolerance(0.0001);

                if (alpha.size() < 2)
                {
                    result[seti] = -GREAT;
                }
                else if
                (
                    (alpha[0] < tolerance && alpha[alpha.size()-1] < tolerance)
                    ||
                    (
                        alpha[0] > 1.0 - tolerance &&
                        alpha[alpha.size()-1] > 1.0 - tolerance
                    )
                )
                {
                    result[seti] = -GREAT;
                }
                else
                {
                    scalar value(0);
                    scalar minScalarCoord(cs.scalarCoord(0));

                    for (int pointi=0; pointi < alpha.size() - 1; pointi++)
                    {
                        value +=
                            (
                                cs.scalarCoord(pointi + 1)
                              - cs.scalarCoord(pointi)
                             )
                             *( alpha[pointi + 1] + alpha[pointi] );

                        minScalarCoord =
                            CML::min
                            (
                                minScalarCoord,
                                cs.scalarCoord(pointi + 1)
                            );
                    }
                    value *= 0.5;

                    result[seti] = value + minScalarCoord;
                }
            }

            if (surfaceElevationFilePtr_.valid())
            {
                // Write time
                surfaceElevationFilePtr_() << mesh_.time().value();

                // Write the surface elevation
                forAll (result, seti)
                {
                    surfaceElevationFilePtr_() << tab << result[seti];
                }

                surfaceElevationFilePtr_() << endl;
            }
        }
    }
}


void CML::sampledSurfaceElevation::read(const dictionary& dict)
{
    dict_ = dict;

    fieldNames_ = wordList(dict_.lookup("fields"));

    interpolationScheme_ = "cell";
    dict_.readIfPresent("interpolationScheme", interpolationScheme_);

    dict_.lookup("setFormat") >> writeFormat_;

    scalarFields_.clear();

    PtrList<sampledSet> newList
    (
        dict_.lookup("sets"),
        sampledSet::iNew(mesh_, searchEngine_)
    );
    transfer(newList);
    combineSampledSets(masterSampledSets_, indexSets_);

    if (Pstream::master() && debug)
    {
        Pout<< "sample fields:" << fieldNames_ << nl
            << "sample sets:" << nl << "(" << nl;

        forAll (*this, si)
        {
            Pout << "  " << operator[](si) << endl;
        }
        Pout << ")" << endl;
    }

    // Create the output file
    if (Pstream::master())
    {
        // create file if not already there, notice: this shall be
        // done on master node only
        if (surfaceElevationFilePtr_.empty())
        {
            // Initialise the file
            mkDir(outputPath_ + "/" + mesh_.time().timeName());
            surfaceElevationFilePtr_.reset
            (
                new OFstream
                (
                    outputPath_ + "/" + mesh_.time().timeName()
                  + "/surfaceElevation.dat"
                )
            );

            // Write header
            if (surfaceElevationFilePtr_.valid())
            {
                surfaceElevationFilePtr_() << "Time";

                forAll (masterSampledSets_, seti)
                {
                    surfaceElevationFilePtr_() << tab
                        << masterSampledSets_[seti].name();
                }
                surfaceElevationFilePtr_() << endl;

                for (int coordi = 0; coordi < 3; coordi++)
                {
                    surfaceElevationFilePtr_() << -1 - coordi;

                    forAll (masterSampledSets_, seti)
                    {
                        surfaceElevationFilePtr_() << tab
                             << masterSampledSets_[seti][0].component(coordi);
                    }
                    surfaceElevationFilePtr_() << endl;
                }
            }
            else
            {
                FatalErrorIn
                (
                   "void CML::sampledSurfaceElevation::sampleIntegrateAndWrite( ... )"
                )
                << "Output file could not be opened in " << outputPath_
                << "/" << mesh_.time().timeName() << endl << endl
                << exit(FatalError);
            }
        }
    }
}


void CML::sampledSurfaceElevation::correct()
{
    // Reset interpolation
	// These two lines make the moving mesh algorithms crash
	// (tested: velocityLaplacian)
	// NGJ: 16/03/2015
//    pointMesh::Delete(mesh_);
//    volPointInterpolation::Delete(mesh_);

    searchEngine_.correct();

    // A quick test has shown that this takes a lot of time on moving meshes
    // Potentially return to improve - if possible.
    // NGJ: 16/03/2015.
    PtrList<sampledSet> newList
    (
        dict_.lookup("sets"),
        sampledSet::iNew(mesh_, searchEngine_)
    );

    transfer(newList);

    combineSampledSets(masterSampledSets_, indexSets_);
}


void CML::sampledSurfaceElevation::updateMesh(const mapPolyMesh&)
{
    correct();
}

void CML::sampledSurfaceElevation::movePoints(const pointField&)
{
    correct();
}

bool CML::sampledSurfaceElevation::timeSet()
{
    // Do nothing
    return true;
}


void CML::sampledSurfaceElevation::readUpdate
(
    const polyMesh::readUpdateState state
)
{
    if (state != polyMesh::UNCHANGED)
    {
        correct();
    }
}


// ************************************************************************* //
