/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright  held by original author
-------------------------------------------------------------------------------
License
    This file is based on CAELUS.

    CAELUS is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    CAELUS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with CAELUS; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Class
    SimpleDistribution

Description
    Encapsulates the regular Distribution-class to allow fast usage with fields
    and parallelize

SourceFiles
    SimpleDistribution.cpp

Contributors/Copyright:
    2013-2014 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef SimpleDistribution_H
#define SimpleDistribution_H

#include "Distribution.hpp"

#include "Tuple2.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

template<class Type>
class SimpleDistribution;

template<class Type>
Istream& operator>>(Istream&, SimpleDistribution<Type>&);

template<class Type>
Ostream& operator<<(Ostream&, const SimpleDistribution<Type>&);

template<class Type>
SimpleDistribution<Type> operator+
(
    const SimpleDistribution<Type>&,
    const SimpleDistribution<Type>&
);

/*---------------------------------------------------------------------------*\
                           Class SimpleDistribution Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class SimpleDistribution
:
    protected Distribution<Type>
{
    // Private data

   // Private Member Functions

   //- Disallow default bitwise copy construct
    //    SimpleDistribution(const SimpleDistribution&);

    //- Disallow default bitwise assignment
    //    void operator=(const SimpleDistribution&);

    //- store the valid limits
    List<Pair<label> > validLimits_;

    void recalcLimits();

    bool hasInvalidValue_;

    Type invalidValue_;

    void setInvalidValue(const Type &val);

protected:
    //- Returns the indices of the first and last non-zero entries
    Pair<label> validLimits(direction cmpt) const;

    //- round value down to the next nearst power of 10
    Type smaller10Power(const Type value);

    //- minimum for distributions that can handle it
    List<Field<scalar> > minimum_;
    //- maximum for distributions that can handle it
    List<Field<scalar> > maximum_;
    //- number of samples for distributions that can handle it
    List<Field<label> > nSamples_;

public:

    //- Construct null
    SimpleDistribution();

     //- setting a size
    SimpleDistribution(const scalar binSize);

    //- setting using a span plus the bin number
    SimpleDistribution(const Type min,const Type max,const label binNr);

    //- from a base class
    SimpleDistribution(const Distribution<Type> &);

    //- direct copy
    SimpleDistribution(const SimpleDistribution<Type> &);

    void calc(
        const Field<Type> &values,
        const Field<Type> &weights
    );

    void calc(
        const Field<Type> &values,
        const Field<Type> &weights,
        const Field<bool> &mask
    );

   void calcScalarWeight(
        const Field<Type> &values,
        const Field<scalar> &weights
    );

   void calcScalarWeight(
        const Field<Type> &values,
        const Field<scalar> &weights,
        const Field<bool> &mask
    );

   void calcMinimumMaximum(
        const Field<Type> &values,
        const Field<scalar> &weights,
        const Field<bool> &mask
    );

   void calcMinimumMaximum(
        const Field<Type> &values,
        const Field<scalar> &weights
    );

    void divideByDistribution(
        const SimpleDistribution<Type> &weightSum,
        const Type &valueIfZero
    );

    //- Return the distribution of the total bin weights - adapted to allow negative weights
    List< List < Pair<scalar> > > rawNegative() const;

    template<class FType>
    List< List < Tuple2<scalar,FType> > > rawField(const List<Field<FType> > &f) const;

    Type mean() const { return Distribution<Type>::mean(); }

    Type median() const { return Distribution<Type>::median(); }

    scalar totalWeight(direction cmpt) const
        { return Distribution<Type>::totalWeight(cmpt); }

    Type min() const;

    Type max() const;

    //- return the value where a fraction of the distribution is smaller
    Type quantile(scalar frac) const;

    //- return the fraction of the distribution that is smaller than the value
    Type smaller(scalar value) const;

    void write(const fileName& filePrefix) const {
        Distribution<Type>::write(filePrefix);
    }

    label maxNrBins() const;

    //- Write the distribution to file: only key and raw.
    //  Produces a separate file for each component.
    void writeRaw(const fileName& filePrefix) const;

    // Member Operators

    void operator=(const SimpleDistribution<Type>&);

    friend Ostream& operator<< <Type>
    (
        Ostream&,
        const SimpleDistribution<Type>&
    );

    friend Istream& operator>> <Type>
    (
        Istream& is,
        SimpleDistribution<Type>& d
    );

   friend SimpleDistribution<Type> operator+ <Type>
    (
        const SimpleDistribution<Type>& d1,
        const SimpleDistribution<Type>& d2
    );
};

    //template <typename T>
    //inline bool contiguous<SimpleDistribution<T> >() { return false; }

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
#include <cassert>

namespace CML {

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
SimpleDistribution<Type>::SimpleDistribution()
:
    Distribution<Type>(),
    hasInvalidValue_(false),
    invalidValue_(-GREAT*pTraits<Type>::one)
{
}

template<class Type>
SimpleDistribution<Type>::SimpleDistribution(const scalar binSize)
:
    Distribution<Type>(
        pTraits<Type>::one*binSize
    ),
    hasInvalidValue_(false),
    invalidValue_(-GREAT*pTraits<Type>::one)
{
    if(binSize<SMALL) {
        FatalErrorInFunction
            << "Specified bin size " << binSize << " is smaller than " << SMALL
                << endl
                << exit(FatalError);
    }
}

template<class Type>
void SimpleDistribution<Type>::setInvalidValue(const Type &value)
{
    hasInvalidValue_=true;
    invalidValue_=value;
}

template<class Type>
Type SimpleDistribution<Type>::smaller10Power(const Type value)
{
    Type result=value;
    for(direction i=0;i<pTraits<Type>::nComponents;i++) {
        const scalar v=component(value,i);
        if(v>SMALL) {
            setComponent(result,i)=
                pow(
                    10,
                    floor(
                        log10(v)
                    )
                );
        } else {
            setComponent(result,i)=SMALL;
        }
    }

    return result;
}

template<class Type>
SimpleDistribution<Type>::SimpleDistribution(
    const Type mini,
    const Type maxi,
    const label binNr
)
:
    Distribution<Type>(
        CML::max(
            smaller10Power(
                (maxi-mini)/binNr
            ),
            // avoid overflow if span=0 but mini/maxi is big
            1e3*pTraits<Type>::one*(
                CML::max(
                    mag(maxi),mag(mini)
                )/scalar(pTraits<label>::max)
            )
        )
    ),
    hasInvalidValue_(false),
    invalidValue_(-GREAT*pTraits<Type>::one)
{
}

template<class Type>
SimpleDistribution<Type>::SimpleDistribution(const Distribution<Type> &o)
:
    Distribution<Type>(o),
    hasInvalidValue_(false),
    invalidValue_(-GREAT*pTraits<Type>::one)
{
    recalcLimits();
}

template<class Type>
SimpleDistribution<Type>::SimpleDistribution(const SimpleDistribution<Type> &o)
:
    Distribution<Type>(o),
    hasInvalidValue_(o.hasInvalidValue_),
    invalidValue_(o.invalidValue_),
    minimum_(o.minimum_),
    maximum_(o.maximum_),
    nSamples_(o.nSamples_)
{

    recalcLimits();
}

template <typename Type>
label SimpleDistribution<Type>::maxNrBins() const
{
    label maxBin=0;
    for(direction i=0;i<pTraits<Type>::nComponents;i++) {
        Pair<label> lim=this->validLimits(i);
        label spread=lim.second()-lim.first();
        if(spread>maxBin) {
            maxBin=spread;
        }
    }

    return maxBin;
}

template <typename Type>
Type SimpleDistribution<Type>::min() const
{
    Type val(pTraits<Type>::zero);
    for(direction i=0;i<pTraits<Type>::nComponents;i++) {
        label index=this->validLimits(i).first();
        label key=this->keys(i)[index];
        setComponent(val,i)=component(this->binWidth(),i)*key;
    }

    return val;
}

template <typename Type>
Type SimpleDistribution<Type>::max() const
{
    Type val(pTraits<Type>::zero);
    for(direction i=0;i<pTraits<Type>::nComponents;i++) {
        label index=this->validLimits(i).second();
        label key=this->keys(i)[index];
        setComponent(val,i)=component(this->binWidth(),i)*(key+1);
    }

    return val;
}

template<class Type>
void SimpleDistribution<Type>::calcScalarWeight(
    const Field<Type> &values,
    const Field<scalar> &weights
)
{
    if(values.size()!=weights.size()) {
        FatalErrorInFunction
            << "Size " << values.size() << " of the values and size "
                << weights.size() << " of the weights differ"
                << endl
                << exit(FatalError);
    }

    this->clear();
    forAll(values,i) {
        this->add(
            values[i],
            pTraits<Type>::one*weights[i]
        );
    }

    // TODO: This does not properly work for weights that are 0
    recalcLimits();
}

template <typename Type>
void SimpleDistribution<Type>::calcMinimumMaximum(
        const Field<Type> &values,
        const Field<scalar> &weights,
        const Field<bool> &mask
) {
    minimum_.resize(pTraits<Type>::nComponents);
    maximum_.resize(pTraits<Type>::nComponents);
    nSamples_.resize(pTraits<Type>::nComponents);

    for (direction cmpt = 0; cmpt < pTraits<Type>::nComponents; cmpt++)
    {
        minimum_[cmpt]=List<scalar>((*this)[cmpt].size(), HUGE_VAL);
        maximum_[cmpt]=List<scalar>((*this)[cmpt].size(),-HUGE_VAL);
        nSamples_[cmpt]=List<label>((*this)[cmpt].size(), 0);
    }

    forAll(mask,i) {
        if(mask[i]) {
            const Type &val=values[i];
            const scalar &wei=weights[i];

            for (direction cmpt = 0; cmpt < pTraits<Type>::nComponents; cmpt++)
            {
                scalar binWidth=component(this->binWidth(), cmpt);
                label n =
                    label(component(val, cmpt)/binWidth)
                    - label(neg(component(val, cmpt)/binWidth));

                label listIndex = this->index(cmpt, n);

                scalar &minVal=minimum_[cmpt][listIndex];
                scalar &maxVal=maximum_[cmpt][listIndex];

                minVal=CML::min(minVal,wei);
                maxVal=CML::max(maxVal,wei);
                nSamples_[cmpt][listIndex]++;
            }
        }
    }
}

template <typename Type>
void SimpleDistribution<Type>::calcMinimumMaximum(
    const Field<Type> &values,
    const Field<scalar> &weights
) {
    Field<bool> mask(values.size(),true);

    calcMinimumMaximum(
        values,
        weights,
        mask
    );
}

template <typename Type>
void SimpleDistribution<Type>::recalcLimits()
{
    validLimits_=List<Pair<label> >(
        pTraits<Type>::nComponents,
        Pair<label>(-1,-1)
    );

    for (direction cmpt = 0; cmpt < pTraits<Type>::nComponents; cmpt++)
    {
        const List<scalar> &vals=(*this)[cmpt];
        Pair<label> &limits=validLimits_[cmpt];
        const scalar invalid=component(invalidValue_,cmpt);

        forAll(vals,i) {
            if(
                (!hasInvalidValue_ && mag(vals[i])>VSMALL)
                ||
                ( hasInvalidValue_ && mag(vals[i]-invalid)>VSMALL)
            ) {
                if (limits.first() == -1)
                {
                    limits.first() = i;
                    limits.second() = i;
                }
                else
                {
                    limits.second() = i;
                }
            }
        }
    }
}

template<class Type>
void SimpleDistribution<Type>::calcScalarWeight(
    const Field<Type> &values,
    const Field<scalar> &weights,
    const Field<bool> &mask
)
{
    if(
        values.size()!=weights.size()
        ||
        values.size()!=mask.size()
    ) {
        FatalErrorInFunction
            << "Size " << values.size() << " of the values and size "
                << weights.size() << " of the weights or the size of the mask "
                << mask.size() << " differ"
                << endl
                << exit(FatalError);
    }

    this->clear();
    forAll(values,i) {
        if(mask[i]) {
            this->add(
                values[i],
                pTraits<Type>::one*weights[i]
            );
        }
    }

    recalcLimits();
}

template<class Type>
void SimpleDistribution<Type>::calc(
    const Field<Type> &values,
    const Field<Type> &weights
)
{
    if(values.size()!=weights.size()) {
        FatalErrorInFunction
            << "Size " << values.size() << " of the values and size "
                << weights.size() << " of the weights differ"
                << endl
                << exit(FatalError);
    }

    this->clear();
    forAll(values,i) {
        this->add(
            values[i],
            weights[i]
        );
    }

    reduce(*this,plusOp<SimpleDistribution<Type> >());

    recalcLimits();
}

template<class Type>
void SimpleDistribution<Type>::calc(
    const Field<Type> &values,
    const Field<Type> &weights,
    const Field<bool> &mask
)
{
    if(
        values.size()!=weights.size()
        ||
        values.size()!=mask.size()
    ) {
        FatalErrorInFunction
            << "Size " << values.size() << " of the values and size "
                << weights.size() << " of the weights or the size of the mask "
                << mask.size() << " differ"
                << endl
                << exit(FatalError);
    }

    this->clear();
    forAll(values,i) {
        if(mask[i]) {
            this->add(
                values[i],
                weights[i]
            );
        }
    }

    reduce(*this,plusOp<SimpleDistribution<Type> >());

    recalcLimits();
}

template<class Type>
void SimpleDistribution<Type>::divideByDistribution(
    const SimpleDistribution<Type> &weightSum,
    const Type &valueIfZero
)
{
    if(this->size()!=weightSum.size()) {
        FatalErrorInFunction
            << "Number of components " << this->size() << " differs from "
                << "number of components in divisor " << weightSum.size()
                << endl
                << exit(FatalError);
    }

    setInvalidValue(valueIfZero);

    validLimits_=List<Pair<label> >(
        pTraits<Type>::nComponents,
        Pair<label>(-1,-1)
    );

    for (direction cmpt = 0; cmpt < pTraits<Type>::nComponents; cmpt++)
    {
        List<scalar> &vals=(*this)[cmpt];
        const List<scalar> &weights=weightSum[cmpt];
        const scalar zero=component(valueIfZero,cmpt);
        Pair<label> &limits=validLimits_[cmpt];

        if(vals.size()!=weights.size()) {
            FatalErrorInFunction
                << "For component " << cmpt << " the size " << vals.size()
                    << " of the data and " << weights.size() << " of the weights "
                    << "differ"
                    << endl
                    << exit(FatalError);
        }

        forAll(vals,i) {
            if(mag(weights[i])<VSMALL) {
                vals[i]=zero;
            } else {
                vals[i]/=weights[i];
                if (limits.first() == -1)
                {
                    limits.first() = i;
                    limits.second() = i;
                }
                else
                {
                    limits.second() = i;
                }
            }
        }
    }
}

template<class Type>
Pair<label> SimpleDistribution<Type>::validLimits
(
    direction cmpt
) const
{
    if(validLimits_.size()<=label(cmpt)) {
        return Distribution<Type>::validLimits(cmpt);
    } else {
        return validLimits_[cmpt];
    }
}

template<class Type>
void SimpleDistribution<Type>::operator=(const SimpleDistribution<Type>&other)
{
    Distribution<Type>::operator=(other);

    invalidValue_=other.invalidValue_;
    hasInvalidValue_=other.hasInvalidValue_;

    minimum_.clear();
    minimum_=other.minimum_;
    maximum_.clear();
    maximum_=other.maximum_;
    nSamples_.clear();
    nSamples_=other.nSamples_;

    recalcLimits();
}

    // generalization of the median-method from Distribution
template<class Type>
Type SimpleDistribution<Type>::quantile(scalar frac) const
{
    Type quantileValue(pTraits<Type>::zero);

    List< List < Pair<scalar> > > normDistribution = this->normalised();

    for (direction cmpt = 0; cmpt < pTraits<Type>::nComponents; cmpt++)
    {
        List< Pair<scalar> >& normDist = normDistribution[cmpt];

        if (normDist.size())
        {
            if (normDist.size() == 1)
            {
                setComponent(quantileValue, cmpt) = normDist[0].first();
            }
            else if
            (
                normDist.size() > 1
                && normDist[0].second()*component(this->binWidth(), cmpt) > frac
            )
            {
                scalar xk =
                    normDist[0].first()
                  + 0.5*component(this->binWidth(), cmpt);

                scalar xkm1 =
                    normDist[0].first()
                  - 0.5*component(this->binWidth(), cmpt);

                scalar Sk = (normDist[0].second())*component(this->binWidth(), cmpt);

                setComponent(quantileValue, cmpt) = frac*(xk - xkm1)/(Sk) + xkm1;
            }
            else
            {
                label previousNonZeroIndex = 0;

                scalar cumulative = 0.0;
                bool done=false;

                forAll(normDist, nD)
                {
                    if
                    (
                        cumulative
                      + (normDist[nD].second()*component(this->binWidth(), cmpt))
                      > frac
                    )
                    {
                        scalar xk =
                            normDist[nD].first()
                          + 0.5*component(this->binWidth(), cmpt);

                        scalar xkm1 =
                            normDist[previousNonZeroIndex].first()
                          + 0.5*component(this->binWidth(), cmpt);

                        scalar Sk =
                            cumulative
                          + (normDist[nD].second()*component(this->binWidth(), cmpt));

                        scalar Skm1 = cumulative;

                        setComponent(quantileValue, cmpt) =
                            (frac - Skm1)*(xk - xkm1)/(Sk - Skm1) + xkm1;

                        done=true;
                        break;
                    }
                    else if (mag(normDist[nD].second()) > VSMALL)
                    {
                        cumulative +=
                            normDist[nD].second()*component(this->binWidth(), cmpt);

                        previousNonZeroIndex = nD;
                    }
                }
                if(!done) {
                    // the quantile is in the last bin or on its edge
                    scalar xk =
                        normDist[normDist.size()-1].first()
                        + 0.5*component(this->binWidth(), cmpt);

                    scalar xkm1 =
                        normDist[previousNonZeroIndex].first()
                        - 0.5*component(this->binWidth(), cmpt);

                    scalar Sk =
                        cumulative
                        + (
                            normDist[normDist.size()-1].second()
                            *
                            component(this->binWidth(), cmpt)
                        );

                    scalar Skm1 = cumulative;

                    setComponent(quantileValue, cmpt) =
                        (frac - Skm1)*(xk - xkm1)/(Sk - Skm1) + xkm1;
                }
            }
        }

    }

    return quantileValue;
}

template<class Type>
Type SimpleDistribution<Type>::smaller(scalar value) const
{
    Type smallerValue(-1*pTraits<Type>::one);

    List< List < Pair<scalar> > > normDistribution = this->normalised();

    for (direction cmpt = 0; cmpt < pTraits<Type>::nComponents; cmpt++)
    {
        List< Pair<scalar> >& normDist = normDistribution[cmpt];

        if (normDist.size())
        {
            scalar firstValue=
                normDist[0].first()
                -0.5*component(this->binWidth(), cmpt);
            scalar lastValue=
                normDist[normDist.size()-1].first()
                +0.5*component(this->binWidth(), cmpt);

            if( value < firstValue ) {
                setComponent(smallerValue, cmpt) = 0;
            } else if( value > lastValue ) {
                setComponent(smallerValue, cmpt) = 1;
            }
            else if (normDist.size() == 1)
            {
                // fraction of the only bin
                setComponent(smallerValue, cmpt) =
                    (value-firstValue)
                    /
                    (component(this->binWidth(), cmpt)+SMALL);
            }
            else if
            (
                normDist.size() > 1
                && (firstValue+component(this->binWidth(), cmpt))>value
            )
            {
                // fraction of the bin
                setComponent(smallerValue, cmpt) =
                    normDist[0].second()
                    *
                    (value-firstValue);
            }
            else
            {
                scalar cumulative = 0.0;
                bool done=false;

                forAll(normDist, nD)
                {
                    if
                    (
                        value
                        <
                        (
                            normDist[nD].first()
                            +
                            0.5*component(this->binWidth(), cmpt)
                        )
                    )
                    {
                        setComponent(smallerValue, cmpt) =
                            cumulative
                            +
                            normDist[nD].second()
                            *
                            (
                                value
                                -
                                (
                                    normDist[nD].first()
                                    -
                                    0.5*component(this->binWidth(), cmpt)
                                )
                            );
                        done=true;
                        break;
                    }
                    else if (mag(normDist[nD].second()) > VSMALL)
                    {
                        cumulative +=
                            normDist[nD].second()*component(this->binWidth(), cmpt);
                    }
                }
                // shouldn't be needed. Just in case
                if(!done) {
                    setComponent(smallerValue, cmpt) = 1;
                }
            }
        }

    }

    return smallerValue;
}


template<class Type>
template<class FType>
List< List < Tuple2<scalar,FType> > > SimpleDistribution<Type>::rawField(
    const List<Field<FType> > &f
) const
{
    if(f.size()!=pTraits<Type>::nComponents) {
        FatalErrorInFunction
            << "Field size" << f.size() << " does not fit number of components "
                << pTraits<Type>::nComponents
                << endl
                << exit(FatalError);
    }

    const_cast<SimpleDistribution<Type>&>(*this).recalcLimits();

    List< List < Tuple2<scalar,FType> > > rawF(pTraits<Type>::nComponents);

    for (direction cmpt = 0; cmpt < pTraits<Type>::nComponents; cmpt++)
    {
        const List<FType>& cmptF = f[cmpt];

        if (cmptF.empty())
        {
            continue;
        }

        List<label> cmptKeys = this->keys(cmpt);
        List< Tuple2<scalar,FType> >& rawDist = rawF[cmpt];

        Pair<label> limits = validLimits(cmpt);

        rawDist.setSize(limits.second() - limits.first() + 1);
        // if(rawDist.size()!=cmptF.size()) {
        //     FatalErrorInFunction
        //         << "For component " << cmpt << " the field size "
        //             << cmptF.size() << " does not fit the distribution size "
        //             << rawDist.size()
        //             << endl
        //             << exit(FatalError);
        // }

        for
        (
            label k = limits.first(), i = 0;
            k <= limits.second();
            k++, i++
        )
        {
            label key = cmptKeys[k];

            rawDist[i].first() = (0.5 + scalar(key))*component(
                this->binWidth(), cmpt
            );

            rawDist[i].second() = cmptF[k];
        }
    }

    return rawF;
}

template<class Type>
List< List < Pair<scalar> > > SimpleDistribution<Type>::rawNegative() const
{
    const_cast<SimpleDistribution<Type>&>(*this).recalcLimits();

    // Copy paste from Distribution::raw to use our validLimits
    List< List < Pair<scalar> > > rawDistribution(pTraits<Type>::nComponents);

    for (direction cmpt = 0; cmpt < pTraits<Type>::nComponents; cmpt++)
    {
        const List<scalar>& cmptDistribution = (*this)[cmpt];

        if (cmptDistribution.empty())
        {
            continue;
        }

        List<label> cmptKeys = this->keys(cmpt);

        List< Pair<scalar> >& rawDist = rawDistribution[cmpt];

        Pair<label> limits = validLimits(cmpt);

        rawDist.setSize(limits.second() - limits.first() + 1);

        for
        (
            label k = limits.first(), i = 0;
            k <= limits.second();
            k++, i++
        )
        {
            label key = cmptKeys[k];

            rawDist[i].first() = (0.5 + scalar(key))*component(
                this->binWidth(), cmpt
            );

            rawDist[i].second() = cmptDistribution[k];
        }
    }

    return rawDistribution;
}

template<class Type>
void SimpleDistribution<Type>::writeRaw(const fileName& filePrefix) const
{
    List< List < Pair<scalar> > > rawDistribution = rawNegative();
    List< List < Tuple2<scalar,scalar> > > rawMin;
    List< List < Tuple2<scalar,scalar> > > rawMax;
    List< List < Tuple2<scalar,label> > > rawnSamp;

    if(minimum_.size()>0 && maximum_.size()>0) {
        rawMin=rawField(minimum_);
        rawMax=rawField(maximum_);
        rawnSamp=rawField(nSamples_);
    }

    for (direction cmpt = 0; cmpt < pTraits<Type>::nComponents; cmpt++)
    {
        const List< Pair<scalar> >& rawPairs = rawDistribution[cmpt];

        OFstream os(filePrefix + '_' + pTraits<Type>::componentNames[cmpt]);

//        Pair<label> limits = validLimits(cmpt);
//        Pair<label> limits2 = Distribution<Type>::validLimits(cmpt);

        os  << "# key raw";
        if(minimum_.size()>cmpt && maximum_.size()>cmpt) {
            os << " min max nSamples";
        }
        os << endl;

        forAll(rawPairs, i)
        {
            os  << rawPairs[i].first()
                << ' ' << rawPairs[i].second();
            if(minimum_.size()>cmpt && maximum_.size()>cmpt) {
                os  << ' ' << rawMin[cmpt][i].second()
                    << ' ' << rawMax[cmpt][i].second()
                    << ' ' << rawnSamp[cmpt][i].second();
            }
            os  << nl;
        }
    }
}

template<class Type>
Istream& operator>>
(
    Istream& is,
    SimpleDistribution<Type>& d
) {
    is  >>  static_cast<Distribution<Type> &>(d);
    is  >>  d.hasInvalidValue_;
    is  >>  d.invalidValue_;
    is  >>  d.minimum_;
    is  >>  d.maximum_;
    is  >>  d.nSamples_;

    d.recalcLimits();
    return is;
}

template<class Type>
Ostream& operator<<
(
    Ostream& os,
    const SimpleDistribution<Type>& d
)
{
    os  <<  static_cast<const Distribution<Type> &>(d);
    os  <<  d.hasInvalidValue_;
    os  <<  d.invalidValue_;
    os  <<  d.minimum_;
    os  <<  d.maximum_;
    os  <<  d.nSamples_;

    return os;
}

    // Copy/paste from Distribution.C with two changed lines
template<class Type>
SimpleDistribution<Type> operator+
(
    const SimpleDistribution<Type>& d1,
    const SimpleDistribution<Type>& d2
)
{
    // The coarsest binWidth is the sensible choice
    SimpleDistribution<Type> d(max(d1.binWidth(), d2.binWidth()));
    List< List< List < Pair<scalar> > > > rawDists(2);
    bool doMinMax=false;
    if(
        d1.minimum_.size()>0 || d1.maximum_.size()>0
        ||
        d2.minimum_.size()>0 || d2.maximum_.size()>0
    ) {
        const label nComp=pTraits<Type>::nComponents;

        if(
            d1.minimum_.size()==nComp && d1.maximum_.size()==nComp
            &&
            d2.minimum_.size()==nComp && d2.maximum_.size()==nComp
        ) {
            doMinMax=true;
        } else {
            FatalErrorInFunction
                << "Sizes of minimum and maximum in operand inconsitent"
                    << " " << d1.minimum_.size()
                    << " " << d1.maximum_.size()
                    << " " << d2.minimum_.size()
                    << " " << d2.maximum_.size()
                    << endl
                    << exit(FatalError);
        }
    }

    // the changed lines
    rawDists[0] = d1.rawNegative();
    rawDists[1] = d2.rawNegative();

    forAll(rawDists, rDI)
    {
        for (direction cmpt = 0; cmpt < pTraits<Type>::nComponents; cmpt++)
        {
            List<scalar>& cmptSimpleDistribution = d[cmpt];

            const List < Pair<scalar> >& cmptRaw = rawDists[rDI][cmpt];

            forAll(cmptRaw, rI)
            {
                scalar valueToAdd = cmptRaw[rI].first();
                scalar cmptWeight = cmptRaw[rI].second();

                label n =
                label
                (
                    component(valueToAdd, cmpt)
                   /component(d.binWidth(), cmpt)
                )
                - label
                (
                    neg(component(valueToAdd, cmpt)
                   /component(d.binWidth(), cmpt))
                );

                label listIndex = d.index(cmpt, n);

                cmptSimpleDistribution[listIndex] += cmptWeight;
            }
        }
    }

    if(doMinMax) {
        d.minimum_.resize(pTraits<Type>::nComponents);
        d.maximum_.resize(pTraits<Type>::nComponents);
        d.nSamples_.resize(pTraits<Type>::nComponents);

        for (direction cmpt = 0; cmpt < pTraits<Type>::nComponents; cmpt++)
        {
            d.minimum_[cmpt]=List<scalar>(d[cmpt].size(), HUGE_VAL);
            d.maximum_[cmpt]=List<scalar>(d[cmpt].size(),-HUGE_VAL);
            d.nSamples_[cmpt]=List<label>(d[cmpt].size(), 0);
        }
        List< List< List < Tuple2<scalar,scalar> > > > rawMin(2);
        List< List< List < Tuple2<scalar,scalar> > > > rawMax(2);
        List< List< List < Tuple2<scalar,label> > > > rawSamples(2);
        rawMin[0] = d1.rawField(d1.minimum_);
        rawMin[1] = d2.rawField(d2.minimum_);
        rawMax[0] = d1.rawField(d1.maximum_);
        rawMax[1] = d2.rawField(d2.maximum_);
        rawSamples[0] = d1.rawField(d1.nSamples_);
        rawSamples[1] = d2.rawField(d2.nSamples_);

        forAll(rawDists, rDI)
        {
            for (direction cmpt = 0; cmpt < pTraits<Type>::nComponents; cmpt++)
            {
                List<scalar>& cmptMin = d.minimum_[cmpt];
                List<scalar>& cmptMax = d.maximum_[cmpt];
                List<label>& cmptSamples = d.nSamples_[cmpt];

                const List < Tuple2<scalar,scalar> >& rMin = rawMin[rDI][cmpt];
                const List < Tuple2<scalar,scalar> >& rMax = rawMax[rDI][cmpt];
                const List < Tuple2<scalar,label> >& rSamples = rawSamples[rDI][cmpt];

                forAll(rMin, rI)
                {
                    scalar valueToAdd = rMin[rI].first();
                    scalar minToAdd = rMin[rI].second();
                    scalar maxToAdd = rMax[rI].second();
                    assert(valueToAdd==rMax[rI].first());

                    label n =
                        label
                        (
                            component(valueToAdd, cmpt)
                            /component(d.binWidth(), cmpt)
                        )
                        - label
                        (
                            neg(component(valueToAdd, cmpt)
                            /component(d.binWidth(), cmpt))
                        );

                    label listIndex = d.index(cmpt, n);

                    cmptMin[listIndex] = min(cmptMin[listIndex],minToAdd);
                    cmptMax[listIndex] = max(cmptMax[listIndex],maxToAdd);
                    cmptSamples[listIndex]+=rSamples[rI].second();
                }
            }
        }
    }

    d.recalcLimits();

    return d;
}


}


#endif

// ************************************************************************* //
