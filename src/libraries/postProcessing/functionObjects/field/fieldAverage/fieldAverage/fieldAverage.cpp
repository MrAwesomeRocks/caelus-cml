/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2015 OpenFOAM Foundation
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

#include "fieldAverage.hpp"
#include "volFields.hpp"
#include "Time.hpp"

#include "fieldAverageItem.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(fieldAverage, 0);
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::fieldAverage::resetFields()
{
    forAll(faItems_, i)
    {
        if (faItems_[i].mean())
        {
            if (obr_.found(faItems_[i].meanFieldName()))
            {
                obr_.checkOut(*obr_[faItems_[i].meanFieldName()]);
            }
        }

        if (faItems_[i].prime2Mean())
        {
            if (obr_.found(faItems_[i].prime2MeanFieldName()))
            {
                obr_.checkOut(*obr_[faItems_[i].prime2MeanFieldName()]);
            }
        }
    }
}


void CML::fieldAverage::initialize()
{
    resetFields();

    Info<< type() << " " << name_ << ":" << nl;

    // Add mean fields to the field lists
    forAll(faItems_, fieldI)
    {
        addMeanField<scalar>(fieldI);
        addMeanField<vector>(fieldI);
        addMeanField<sphericalTensor>(fieldI);
        addMeanField<symmTensor>(fieldI);
        addMeanField<tensor>(fieldI);
    }

    // Add prime-squared mean fields to the field lists
    forAll(faItems_, fieldI)
    {
        addPrime2MeanField<scalar, scalar>(fieldI);
        addPrime2MeanField<vector, symmTensor>(fieldI);
    }

    forAll(faItems_, fieldI)
    {
        if (!faItems_[fieldI].active())
        {
            WarningInFunction
                << "Field " << faItems_[fieldI].fieldName()
                << " not found in database for averaging";
        }
    }

    // ensure first averaging works unconditionally
    prevTimeIndex_ = -1;

    Info<< endl;

    initialised_ = true;
}


void CML::fieldAverage::calcAverages()
{
    if (!initialised_)
    {
        initialize();
    }

    const label currentTimeIndex =
        static_cast<const fvMesh&>(obr_).time().timeIndex();

    if (prevTimeIndex_ == currentTimeIndex)
    {
        return;
    }
    else
    {
        prevTimeIndex_ = currentTimeIndex;
    }

    Info<< type() << " " << name_ << " output:" << nl;

    Info<< "    Calculating averages" << nl;

    addMeanSqrToPrime2Mean<scalar, scalar>();
    addMeanSqrToPrime2Mean<vector, symmTensor>();

    calculateMeanFields<scalar>();
    calculateMeanFields<vector>();
    calculateMeanFields<sphericalTensor>();
    calculateMeanFields<symmTensor>();
    calculateMeanFields<tensor>();

    calculatePrime2MeanFields<scalar, scalar>();
    calculatePrime2MeanFields<vector, symmTensor>();

    forAll(faItems_, fieldI)
    {
        totalIter_[fieldI]++;
        totalTime_[fieldI] += obr_.time().deltaTValue();
    }
}


void CML::fieldAverage::writeAverages() const
{
    Info<< "    Writing average fields" << endl;

    writeFields<scalar>();
    writeFields<vector>();
    writeFields<sphericalTensor>();
    writeFields<symmTensor>();
    writeFields<tensor>();
}


void CML::fieldAverage::writeAveragingProperties() const
{
    IOdictionary propsDict
    (
        IOobject
        (
            "fieldAveragingProperties",
            obr_.time().timeName(),
            "uniform",
            obr_,
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            false
        )
    );

    forAll(faItems_, fieldI)
    {
        const word& fieldName = faItems_[fieldI].fieldName();
        propsDict.add(fieldName, dictionary());
        propsDict.subDict(fieldName).add("totalIter", totalIter_[fieldI]);
        propsDict.subDict(fieldName).add("totalTime", totalTime_[fieldI]);
    }

    propsDict.regIOobject::write();
}


void CML::fieldAverage::readAveragingProperties()
{
    totalIter_.clear();
    totalIter_.setSize(faItems_.size(), 1);

    totalTime_.clear();
    totalTime_.setSize(faItems_.size(), obr_.time().deltaTValue());

    if (resetOnRestart_ || resetOnOutput_)
    {
        Info<< "    Starting averaging at time " << obr_.time().timeName()
            << nl;
    }
    else
    {
        IOobject propsDictHeader
        (
            "fieldAveragingProperties",
            obr_.time().timeName(obr_.time().startTime().value()),
            "uniform",
            obr_,
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE,
            false
        );

        if (!propsDictHeader.headerOk())
        {
            Info<< "    Starting averaging at time " << obr_.time().timeName()
                << nl;
            return;
        }

        IOdictionary propsDict(propsDictHeader);

        Info<< "    Restarting averaging for fields:" << nl;
        forAll(faItems_, fieldI)
        {
            const word& fieldName = faItems_[fieldI].fieldName();
            if (propsDict.found(fieldName))
            {
                dictionary fieldDict(propsDict.subDict(fieldName));

                totalIter_[fieldI] = readLabel(fieldDict.lookup("totalIter"));
                totalTime_[fieldI] = readScalar(fieldDict.lookup("totalTime"));
                Info<< "        " << fieldName
                    << " iters = " << totalIter_[fieldI]
                    << " time = " << totalTime_[fieldI] << nl;
            }
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::fieldAverage::fieldAverage
(
    const word& name,
    const objectRegistry& obr,
    const dictionary& dict,
    const bool loadFromFiles
)
:
    name_(name),
    obr_(obr),
    active_(true),
    prevTimeIndex_(-1),
    resetOnRestart_(false),
    resetOnOutput_(false),
    initialised_(false),
    faItems_(),
    totalIter_(),
    totalTime_()
{
    // Only active if a fvMesh is available
    if (isA<fvMesh>(obr_))
    {
        read(dict);
    }
    else
    {
        active_ = false;
        WarningInFunction
            << "No fvMesh available, deactivating " << name_ << nl
            << endl;
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::fieldAverage::~fieldAverage()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::fieldAverage::read(const dictionary& dict)
{
    if (active_)
    {
        initialised_ = false;

        Info<< type() << " " << name_ << ":" << nl;

        dict.readIfPresent("resetOnRestart", resetOnRestart_);
        dict.readIfPresent("resetOnOutput", resetOnOutput_);
        dict.lookup("fields") >> faItems_;

        readAveragingProperties();

        Info<< endl;
    }
}


void CML::fieldAverage::execute()
{
    if (active_)
    {
        calcAverages();
        Info<< endl;
    }
}


void CML::fieldAverage::end()
{
    if (active_)
    {
        calcAverages();
        Info<< endl;
    }
}


void CML::fieldAverage::timeSet()
{}


void CML::fieldAverage::write()
{
    if (active_)
    {
        writeAverages();
        writeAveragingProperties();

        if (resetOnOutput_)
        {
            Info<< "    Restarting averaging at time " << obr_.time().timeName()
                << nl << endl;

            totalIter_.clear();
            totalIter_.setSize(faItems_.size(), 1);

            totalTime_.clear();
            totalTime_.setSize(faItems_.size(), obr_.time().deltaTValue());

            initialize();
        }

        Info<< endl;
    }
}


void CML::fieldAverage::updateMesh(const mapPolyMesh&)
{
    // Do nothing
}


void CML::fieldAverage::movePoints(const pointField&)
{
    // Do nothing
}


// ************************************************************************* //
