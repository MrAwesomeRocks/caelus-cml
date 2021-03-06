/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright  held by original author
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
    2008-2014 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#include "distributionFunctionObject.hpp"
#include "addToRunTimeSelectionTable.hpp"

#include "polyMesh.hpp"
#include "Time.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(distributionFunctionObject, 0);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

distributionFunctionObject::distributionFunctionObject
(
    const word &name,
    const Time& t,
    const dictionary& dict
)
:
    timelineFunctionObject(name,t,dict),
    writeTimeline_(
        readBool(dict.lookup("writeTimeline"))
    ),
    writeDistribution_(
        readBool(dict.lookup("writeDistribution"))
    ),
    startup_(false),
    startTime_("none")
{
    if(writeTimeline_ && writeDistribution_) {
        WarningInFunction
            << "Both 'writeTimeline' and 'writeDistribution' set in " << name
                << ". "
                << "This is usually not a good idea because of the amount of data written"
                << endl;
    }
    if(!writeTimeline_ && writeDistribution_) {
        WarningInFunction
            << "Both 'writeTimeline' and 'writeDistribution' not set. "
                << "This makes calculating the distribution futile"
                << endl;
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void distributionFunctionObject::clearDistributions() {
    distScalar_.clear();
    distVector_.clear();
    distTensor_.clear();
    distSymmTensor_.clear();
    distSphericalTensor_.clear();
}

bool distributionFunctionObject::start() {
    startup_=true;
    startTime_=time().timeName();

    return true;
}

void distributionFunctionObject::flush() {

    if(writeTimeline()) {
        timelineFunctionObject::flush();
    }
}

fileName distributionFunctionObject::dataDir()
{
    if(startup_) {
        return baseDir()/startTime_;
    } else {
        return timelineFunctionObject::dataDir();
    }
}

void distributionFunctionObject::write() {
    clearDistributions();

    getDistribution();

    if(writeTimeline() && startup_) {
        timelineFunctionObject::start();
    }
    startup_=false;

    bool zeroDistribution=false;

    if(
        distScalar_.valid()
        &&
        distScalar_->maxNrBins()<=0
    ) {
        zeroDistribution=true;
    }
    if(
        distVector_.valid()
        &&
        distVector_->maxNrBins()<=0
    ) {
        zeroDistribution=true;
    }
    if(
        distTensor_.valid()
        &&
        distTensor_->maxNrBins()<=0
    ) {
        zeroDistribution=true;
    }
    if(
        distSymmTensor_.valid()
        &&
        distSymmTensor_->maxNrBins()<=0
    ) {
        zeroDistribution=true;
    }
    if(
        distSphericalTensor_.valid()
        &&
        distSphericalTensor_->maxNrBins()<=0
    ) {
        zeroDistribution=true;
    }

    if(zeroDistribution) {
        WarningInFunction
            << "Distribution for " << name() << " has size 0. "
                << "Doing nothing"
                << endl;
        return;
    }
    if(Pstream::master()) {
        if(writeTimeline()) {
            writeATimeline(distScalar_);
            writeATimeline(distVector_);
            writeATimeline(distTensor_);
            writeATimeline(distSymmTensor_);
            writeATimeline(distSphericalTensor_);
        }

        if(writeDistribution()) {
            writeADistribution(distScalar_);
            writeADistribution(distVector_);
            writeADistribution(distTensor_);
            writeADistribution(distSymmTensor_);
            writeADistribution(distSphericalTensor_);
        }

        if(verbose()) {
            reportADistribution(distScalar_);
            reportADistribution(distVector_);
            reportADistribution(distTensor_);
            reportADistribution(distSymmTensor_);
            reportADistribution(distSphericalTensor_);
        }
    }
}

wordList distributionFunctionObject::fileNames() {
    stringList onames(this->componentNames());
    wordList names(onames.size());
    forAll(names,i) {
        names[i]=this->baseName()+"_"+onames[i];
    }
    return names;
}

stringList distributionFunctionObject::componentNames() {
    const char * const *names=nullptr;
    direction nComp=0;

    if(distScalar_.valid()) {
        names=pTraits<scalar>::componentNames;
        nComp=pTraits<scalar>::nComponents;
    } else if(distVector_.valid()) {
        names=pTraits<vector>::componentNames;
        nComp=pTraits<vector>::nComponents;
    } else if(distTensor_.valid()) {
        names=pTraits<tensor>::componentNames;
        nComp=pTraits<tensor>::nComponents;
    } else if(distSymmTensor_.valid()) {
        names=pTraits<symmTensor>::componentNames;
        nComp=pTraits<symmTensor>::nComponents;
    } else if(distSphericalTensor_.valid()) {
        names=pTraits<sphericalTensor>::componentNames;
        nComp=pTraits<sphericalTensor>::nComponents;
    } else {
        WarningInFunction
            << "Can't determine type"
                << endl;

    }

    if(nComp==0) {
        return stringList();
    }
    stringList result(nComp);
    for(direction i=0;i<nComp;i++) {
        result[i]=string(names[i]);
    }
    return result;
}

stringList distributionFunctionObject::columnNames() {
    DynamicList<string> names;
    names.append("mean");
    names.append("median");
    names.append("min");
    names.append("max");
    names.append("totalWeight");

    return stringList(names);
}

} // namespace CML

// ************************************************************************* //
