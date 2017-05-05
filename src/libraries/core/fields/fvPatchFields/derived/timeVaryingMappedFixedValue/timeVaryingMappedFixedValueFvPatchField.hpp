/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
Copyright (C) 2011-2012 OpenFOAM Foundation
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

Class
    CML::timeVaryingMappedFixedValueFvPatchField

Description
    CML::timeVaryingMappedFixedValueFvPatchField

    Interpolates from a set of supplied points in space and time. Supplied
    data in constant/boundaryData/<patchname>:
        - points : pointField with locations
        - ddd    : supplied values at time ddd
    Points need to be more or less on a plane since get triangulated in 2D.

    At startup this bc does the triangulation and determines linear
    interpolation (triangle it is in and weights to the 3 vertices)
    for every face centre. Interpolates linearly inbetween times.

    @verbatim
        inlet
        {
            type            timeVaryingMappedFixedValue;

            // Maintain average to that of the supplied values
            setAverage      false;

            // Optional: change perturbation (default 1E-5) to avoid any ties
            // in triangulating regular geometries.
            //perturb       0.0;

            // Optional: use name instead of patchname for location of data
            //fieldTableName samples;
        }
    @endverbatim

    Switch on debug flag to have it dump the triangulation (in transformed
    space) and transform face centres.

SourceFiles
    timeVaryingMappedFixedValueFvPatchField.cpp

\*---------------------------------------------------------------------------*/

#ifndef timeVaryingMappedFixedValueFvPatchField_H
#define timeVaryingMappedFixedValueFvPatchField_H

#include "fixedValueFvPatchFields.hpp"
#include "FixedList.hpp"
#include "instantList.hpp"
#include "pointToPointPlanarInterpolation.hpp"
#include "DataEntry.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
              Class timeVaryingMappedFixedValueFvPatch Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class timeVaryingMappedFixedValueFvPatchField
:
    public fixedValueFvPatchField<Type>
{
    // Private data

        //- Name of the field data table, defaults to the name of the field
        word fieldTableName_;

        //- If true adjust the mapped field to maintain average value
        bool setAverage_;

        //- Fraction of perturbation (fraction of bounding box) to add
        scalar perturb_;

        //- 2D interpolation
        autoPtr<pointToPointPlanarInterpolation> mapperPtr_;

        //- List of boundaryData time directories
        instantList sampleTimes_;

        //- Current starting index in sampleTimes
        label startSampleTime_;

        //- Interpolated values from startSampleTime
        Field<Type> startSampledValues_;

        //- If setAverage: starting average value
        Type startAverage_;

        //- Current end index in sampleTimes
        label endSampleTime_;

        //- Interpolated values from endSampleTime
        Field<Type> endSampledValues_;

        //- If setAverage: end average value
        Type endAverage_;

        //- Time varying offset values to interpolated data
        autoPtr<DataEntry<Type> > offset_;


public:

    //- Runtime type information
    TypeName("timeVaryingMappedFixedValue");


    // Constructors

        //- Construct from patch and internal field
        timeVaryingMappedFixedValueFvPatchField
        (
            const fvPatch&,
            const DimensionedField<Type, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        timeVaryingMappedFixedValueFvPatchField
        (
            const fvPatch&,
            const DimensionedField<Type, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given timeVaryingMappedFixedValueFvPatchField
        //  onto a new patch
        timeVaryingMappedFixedValueFvPatchField
        (
            const timeVaryingMappedFixedValueFvPatchField<Type>&,
            const fvPatch&,
            const DimensionedField<Type, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        timeVaryingMappedFixedValueFvPatchField
        (
            const timeVaryingMappedFixedValueFvPatchField<Type>&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchField<Type> > clone() const
        {
            return tmp<fvPatchField<Type> >
            (
                new timeVaryingMappedFixedValueFvPatchField<Type>(*this)
            );
        }

        //- Construct as copy setting internal field reference
        timeVaryingMappedFixedValueFvPatchField
        (
            const timeVaryingMappedFixedValueFvPatchField<Type>&,
            const DimensionedField<Type, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchField<Type> > clone
        (
            const DimensionedField<Type, volMesh>& iF
        ) const
        {
            return tmp<fvPatchField<Type> >
            (
                new timeVaryingMappedFixedValueFvPatchField<Type>(*this, iF)
            );
        }


    // Member functions

        // Access

            //- Return startSampledValues
            const Field<Type> startSampledValues()
            {
                 return startSampledValues_;
            }


        // Mapping functions

            //- Map (and resize as needed) from self given a mapping object
            virtual void autoMap
            (
                const fvPatchFieldMapper&
            );

            //- Reverse map the given fvPatchField onto this fvPatchField
            virtual void rmap
            (
                const fvPatchField<Type>&,
                const labelList&
            );


        // Utility functions

            //- Find boundary data inbetween current time and interpolate
            void checkTable();


        // Evaluation functions

            //- Update the coefficients associated with the patch field
            virtual void updateCoeffs();


        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "Time.hpp"
#include "AverageIOField.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
timeVaryingMappedFixedValueFvPatchField<Type>::
timeVaryingMappedFixedValueFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF
)
:
    fixedValueFvPatchField<Type>(p, iF),
    fieldTableName_(iF.name()),
    setAverage_(false),
    perturb_(0),
    mapperPtr_(NULL),
    sampleTimes_(0),
    startSampleTime_(-1),
    startSampledValues_(0),
    startAverage_(pTraits<Type>::zero),
    endSampleTime_(-1),
    endSampledValues_(0),
    endAverage_(pTraits<Type>::zero),
    offset_()
{}


template<class Type>
timeVaryingMappedFixedValueFvPatchField<Type>::
timeVaryingMappedFixedValueFvPatchField
(
    const timeVaryingMappedFixedValueFvPatchField<Type>& ptf,
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchField<Type>(ptf, p, iF, mapper),
    fieldTableName_(ptf.fieldTableName_),
    setAverage_(ptf.setAverage_),
    perturb_(ptf.perturb_),
    mapperPtr_(NULL),
    sampleTimes_(0),
    startSampleTime_(-1),
    startSampledValues_(0),
    startAverage_(pTraits<Type>::zero),
    endSampleTime_(-1),
    endSampledValues_(0),
    endAverage_(pTraits<Type>::zero),
    offset_
    (
        ptf.offset_.valid()
      ? ptf.offset_().clone().ptr()
      : NULL
    )
{}


template<class Type>
timeVaryingMappedFixedValueFvPatchField<Type>::
timeVaryingMappedFixedValueFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchField<Type>(p, iF),
    fieldTableName_(iF.name()),
    setAverage_(readBool(dict.lookup("setAverage"))),
    perturb_(dict.lookupOrDefault("perturb", 1e-5)),
    mapperPtr_(NULL),
    sampleTimes_(0),
    startSampleTime_(-1),
    startSampledValues_(0),
    startAverage_(pTraits<Type>::zero),
    endSampleTime_(-1),
    endSampledValues_(0),
    endAverage_(pTraits<Type>::zero),
    offset_(DataEntry<Type>::New("offset", dict))
{
    dict.readIfPresent("fieldTableName", fieldTableName_);

    if (dict.found("value"))
    {
        fvPatchField<Type>::operator==(Field<Type>("value", dict, p.size()));
    }
    else
    {
        updateCoeffs();
    }
}


template<class Type>
timeVaryingMappedFixedValueFvPatchField<Type>::
timeVaryingMappedFixedValueFvPatchField
(
    const timeVaryingMappedFixedValueFvPatchField<Type>& ptf
)
:
    fixedValueFvPatchField<Type>(ptf),
    fieldTableName_(ptf.fieldTableName_),
    setAverage_(ptf.setAverage_),
    perturb_(ptf.perturb_),
    mapperPtr_(NULL),
    sampleTimes_(ptf.sampleTimes_),
    startSampleTime_(ptf.startSampleTime_),
    startSampledValues_(ptf.startSampledValues_),
    startAverage_(ptf.startAverage_),
    endSampleTime_(ptf.endSampleTime_),
    endSampledValues_(ptf.endSampledValues_),
    endAverage_(ptf.endAverage_),
    offset_
    (
        ptf.offset_.valid()
      ? ptf.offset_().clone().ptr()
      : NULL
    )
{}


template<class Type>
timeVaryingMappedFixedValueFvPatchField<Type>::
timeVaryingMappedFixedValueFvPatchField
(
    const timeVaryingMappedFixedValueFvPatchField<Type>& ptf,
    const DimensionedField<Type, volMesh>& iF
)
:
    fixedValueFvPatchField<Type>(ptf, iF),
    fieldTableName_(ptf.fieldTableName_),
    setAverage_(ptf.setAverage_),
    perturb_(ptf.perturb_),
    mapperPtr_(NULL),
    sampleTimes_(ptf.sampleTimes_),
    startSampleTime_(ptf.startSampleTime_),
    startSampledValues_(ptf.startSampledValues_),
    startAverage_(ptf.startAverage_),
    endSampleTime_(ptf.endSampleTime_),
    endSampledValues_(ptf.endSampledValues_),
    endAverage_(ptf.endAverage_),
    offset_
    (
        ptf.offset_.valid()
      ? ptf.offset_().clone().ptr()
      : NULL
    )
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void timeVaryingMappedFixedValueFvPatchField<Type>::autoMap
(
    const fvPatchFieldMapper& m
)
{
    fixedValueFvPatchField<Type>::autoMap(m);
    if (startSampledValues_.size())
    {
        startSampledValues_.autoMap(m);
        endSampledValues_.autoMap(m);
    }
    // Clear interpolator
    mapperPtr_.clear();
    startSampleTime_ = -1;
    endSampleTime_ = -1;
}


template<class Type>
void timeVaryingMappedFixedValueFvPatchField<Type>::rmap
(
    const fvPatchField<Type>& ptf,
    const labelList& addr
)
{
    fixedValueFvPatchField<Type>::rmap(ptf, addr);

    const timeVaryingMappedFixedValueFvPatchField<Type>& tiptf =
        refCast<const timeVaryingMappedFixedValueFvPatchField<Type> >(ptf);

    startSampledValues_.rmap(tiptf.startSampledValues_, addr);
    endSampledValues_.rmap(tiptf.endSampledValues_, addr);

    // Clear interpolator
    mapperPtr_.clear();
    startSampleTime_ = -1;
    endSampleTime_ = -1;
}


template<class Type>
void timeVaryingMappedFixedValueFvPatchField<Type>::checkTable()
{
    // Initialise
    if (mapperPtr_.empty())
    {
        pointIOField samplePoints
        (
            IOobject
            (
                "points",
                this->db().time().constant(),
                "boundaryData"/this->patch().name(),
                this->db(),
                IOobject::MUST_READ,
                IOobject::AUTO_WRITE,
                false
            )
        );

        const fileName samplePointsFile = samplePoints.filePath();

        if (debug)
        {
            Info<< "timeVaryingMappedFixedValueFvPatchField :"
                << " Read " << samplePoints.size() << " sample points from "
                << samplePointsFile << endl;
        }

        // Allocate the interpolator
        mapperPtr_.reset
        (
            new pointToPointPlanarInterpolation
            (
                samplePoints,
                 this->patch().patch().faceCentres(),
                perturb_
            )
        );

        // Read the times for which data is available
        const fileName samplePointsDir = samplePointsFile.path();
        sampleTimes_ = Time::findTimes(samplePointsDir);

        if (debug)
        {
            Info<< "timeVaryingMappedFixedValueFvPatchField : In directory "
                << samplePointsDir << " found times "
                << pointToPointPlanarInterpolation::timeNames(sampleTimes_)
                << endl;
        }
    }


    // Find current time in sampleTimes
    label lo = -1;
    label hi = -1;

    bool foundTime = mapperPtr_().findTime
    (
        sampleTimes_,
        startSampleTime_,
        this->db().time().value(),
        lo,
        hi
    );

    if (!foundTime)
    {
        FatalErrorIn
        (
            "timeVaryingMappedFixedValueFvPatchField<Type>::checkTable()"
        )   << "Cannot find starting sampling values for current time "
            << this->db().time().value() << nl
            << "Have sampling values for times "
            << pointToPointPlanarInterpolation::timeNames(sampleTimes_) << nl
            << "In directory "
            <<  this->db().time().constant()/"boundaryData"/this->patch().name()
            << "\n    on patch " << this->patch().name()
            << " of field " << fieldTableName_
            << exit(FatalError);
    }


    // Update sampled data fields.

    if (lo != startSampleTime_)
    {
        startSampleTime_ = lo;

        if (startSampleTime_ == endSampleTime_)
        {
            // No need to reread since are end values
            if (debug)
            {
                Pout<< "checkTable : Setting startValues to (already read) "
                    <<   "boundaryData"
                        /this->patch().name()
                        /sampleTimes_[startSampleTime_].name()
                    << endl;
            }
            startSampledValues_ = endSampledValues_;
            startAverage_ = endAverage_;
        }
        else
        {
            if (debug)
            {
                Pout<< "checkTable : Reading startValues from "
                    <<   "boundaryData"
                        /this->patch().name()
                        /sampleTimes_[lo].name()
                    << endl;
            }


            // Reread values and interpolate
            AverageIOField<Type> vals
            (
                IOobject
                (
                    fieldTableName_,
                    this->db().time().constant(),
                    "boundaryData"
                   /this->patch().name()
                   /sampleTimes_[startSampleTime_].name(),
                    this->db(),
                    IOobject::MUST_READ,
                    IOobject::AUTO_WRITE,
                    false
                )
            );

            if (vals.size() != mapperPtr_().sourceSize())
            {
                FatalErrorIn
                (
                    "timeVaryingMappedFixedValueFvPatchField<Type>::"
                    "checkTable()"
                )   << "Number of values (" << vals.size()
                    << ") differs from the number of points ("
                    <<  mapperPtr_().sourceSize()
                    << ") in file " << vals.objectPath() << exit(FatalError);
            }

            startAverage_ = vals.average();
            startSampledValues_ = mapperPtr_().interpolate(vals);
        }
    }

    if (hi != endSampleTime_)
    {
        endSampleTime_ = hi;

        if (endSampleTime_ == -1)
        {
            // endTime no longer valid. Might as well clear endValues.
            if (debug)
            {
                Pout<< "checkTable : Clearing endValues" << endl;
            }
            endSampledValues_.clear();
        }
        else
        {
            if (debug)
            {
                Pout<< "checkTable : Reading endValues from "
                    <<   "boundaryData"
                        /this->patch().name()
                        /sampleTimes_[endSampleTime_].name()
                    << endl;
            }

            // Reread values and interpolate
            AverageIOField<Type> vals
            (
                IOobject
                (
                    fieldTableName_,
                    this->db().time().constant(),
                    "boundaryData"
                   /this->patch().name()
                   /sampleTimes_[endSampleTime_].name(),
                    this->db(),
                    IOobject::MUST_READ,
                    IOobject::AUTO_WRITE,
                    false
                )
            );

            if (vals.size() != mapperPtr_().sourceSize())
            {
                FatalErrorIn
                (
                    "timeVaryingMappedFixedValueFvPatchField<Type>::"
                    "checkTable()"
                )   << "Number of values (" << vals.size()
                    << ") differs from the number of points ("
                    <<  mapperPtr_().sourceSize()
                    << ") in file " << vals.objectPath() << exit(FatalError);
            }

            endAverage_ = vals.average();
            endSampledValues_ = mapperPtr_().interpolate(vals);
        }
    }
}


template<class Type>
void timeVaryingMappedFixedValueFvPatchField<Type>::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }


    checkTable();

    // Interpolate between the sampled data

    Type wantedAverage;

    if (endSampleTime_ == -1)
    {
        // only start value
        if (debug)
        {
            Pout<< "updateCoeffs : Sampled, non-interpolated values"
                << " from start time:"
                << sampleTimes_[startSampleTime_].name() << nl;
        }

        this->operator==(startSampledValues_);
        wantedAverage = startAverage_;
    }
    else
    {
        scalar start = sampleTimes_[startSampleTime_].value();
        scalar end = sampleTimes_[endSampleTime_].value();

        scalar s = (this->db().time().value() - start)/(end - start);

        if (debug)
        {
            Pout<< "updateCoeffs : Sampled, interpolated values"
                << " between start time:"
                << sampleTimes_[startSampleTime_].name()
                << " and end time:" << sampleTimes_[endSampleTime_].name()
                << " with weight:" << s << endl;
        }

        this->operator==((1 - s)*startSampledValues_ + s*endSampledValues_);
        wantedAverage = (1 - s)*startAverage_ + s*endAverage_;
    }

    // Enforce average. Either by scaling (if scaling factor > 0.5) or by
    // offsetting.
    if (setAverage_)
    {
        const Field<Type>& fld = *this;

        Type averagePsi =
            gSum(this->patch().magSf()*fld)
           /gSum(this->patch().magSf());

        if (debug)
        {
            Pout<< "updateCoeffs :"
                << " actual average:" << averagePsi
                << " wanted average:" << wantedAverage
                << endl;
        }

        if (mag(averagePsi) < VSMALL)
        {
            // Field too small to scale. Offset instead.
            const Type offset = wantedAverage - averagePsi;
            if (debug)
            {
                Pout<< "updateCoeffs :"
                    << " offsetting with:" << offset << endl;
            }
            this->operator==(fld + offset);
        }
        else
        {
            const scalar scale = mag(wantedAverage)/mag(averagePsi);

            if (debug)
            {
                Pout<< "updateCoeffs :"
                    << " scaling with:" << scale << endl;
            }
            this->operator==(scale*fld);
        }
    }

    // apply offset to mapped values
    const scalar t = this->db().time().timeOutputValue();
    this->operator==(*this + offset_->value(t));

    if (debug)
    {
        Pout<< "updateCoeffs : set fixedValue to min:" << gMin(*this)
            << " max:" << gMax(*this) << endl;
    }

    fixedValueFvPatchField<Type>::updateCoeffs();
}


template<class Type>
void timeVaryingMappedFixedValueFvPatchField<Type>::write(Ostream& os) const
{
    fvPatchField<Type>::write(os);
    os.writeKeyword("setAverage") << setAverage_ << token::END_STATEMENT << nl;
    os.writeKeyword("perturb") << perturb_ << token::END_STATEMENT << nl;

    if (fieldTableName_ != this->dimensionedInternalField().name())
    {
        os.writeKeyword("fieldTableName") << fieldTableName_
            << token::END_STATEMENT << nl;
    }

    offset_->writeData(os);

    this->writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
