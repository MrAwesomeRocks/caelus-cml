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

Class
    CML::SampledSetValueExpressionDriver

Description

SourceFiles
    SampledSetValueExpressionDriverI.hpp
    SampledSetValueExpressionDriver.cpp

Contributors/Copyright:
    2011-2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef SampledSetValueExpressionDriver_H
#define SampledSetValueExpressionDriver_H

#include "volFields.hpp"

#include "sampledSet.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "SubsetValueExpressionDriver.hpp"

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class SampledSetValueExpressionDriver Declaration
\*---------------------------------------------------------------------------*/

class SampledSetValueExpressionDriver
:
    public SubsetValueExpressionDriver
{
    static word driverName_;

    // Private data
    const sampledSet &theSet_;

    // Private Member Functions

    template<class Type,class volField,class surfField>
    tmp<Field<Type> >
    sampleOrInterpolateInternal(const word &name,bool oldTime=false);

    static void setDebug();

protected:
    //- update stuff the set
    virtual bool update();

    //- should we interpolate?
    bool interpolate_;

    //- what kind of interpolation
    word interpolationType_;

    //- weights of the entries if it is not a point field

    //- parametert size is the expected size of the weight field and
    //- will be used to determine whether this is correct.
    virtual tmp<scalarField> weightsNonPoint(
        label size
    ) const;
public:

     //- Runtime type information
    TypeName("SampledSetValueExpressionDriver");

    //- entity this driver operates on
    typedef sampledSet EntityType;

    static const word &driverName() { return driverName_; }

    // Constructors

    //- Construct from components
    SampledSetValueExpressionDriver(
        sampledSet &,
        bool autoInterpolate=false,
        bool warnAutoInterpolate=true
    );

    //- Construct from an id
    SampledSetValueExpressionDriver(const word& id,const fvMesh&mesh);

    //- Construct from a dictionary
    SampledSetValueExpressionDriver(const dictionary& dict,const fvMesh&mesh);

//     //- Copy construct
    SampledSetValueExpressionDriver(const sampledSet &,const SampledSetValueExpressionDriver&);

    autoPtr<CommonValueExpressionDriver> clone() const
        {
            return autoPtr<CommonValueExpressionDriver>(
                new SampledSetValueExpressionDriver(theSet_,*this)
            );
        }

    // Destructor

    ~SampledSetValueExpressionDriver();


    // Member Functions

    virtual tmp<vectorField> makePositionField() const;

    virtual tmp<scalarField> makeFaceAreaMagField() const;

    virtual tmp<scalarField> makeCellVolumeField() const;

    //     virtual tmp<vectorField> makePointField()=0;

    virtual tmp<scalarField> makeFaceFlipField() const;

    virtual tmp<vectorField> makeFaceNormalField() const;

    virtual tmp<vectorField> makeFaceAreaField() const;

//     template<class Type>
//     tmp<Field<Type> > getField(const string &name);

    virtual bool isCell() { return false; }

    virtual tmp<Field<scalar> > getScalarField(
        const word &name,bool oldTime=false
    );

    virtual tmp<Field<vector> > getVectorField(
        const word &name,bool oldTime=false
    );

    virtual tmp<Field<tensor> > getTensorField(
        const word &name,bool oldTime=false
    );

    virtual tmp<Field<symmTensor> > getSymmTensorField(
        const word &name,bool oldTime=false
    );

    virtual tmp<Field<sphericalTensor> > getSphericalTensorField(
        const word &name,bool oldTime=false
    );

    // Handling the parser
    //    void parse(const std::string& f);

    virtual label size() const;

    inline virtual label pointSize() const;

    inline virtual const fvMesh &mesh() const;

//     template<class Type>
//     virtual tmp<Field<Type> > toFace(const Field<Type> &)=0;

//     template<class Type>
//     virtual tmp<Field<Type> > toPoint(const Field<Type> &)=0;

    //- return a new plugin-function
    virtual autoPtr<CommonPluginFunction> newPluginFunction(
        const word &name
    );

    //- tests for a plugin-function
    virtual bool existsPluginFunction(
        const word &name
    );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "SampledSetValueExpressionDriverI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
