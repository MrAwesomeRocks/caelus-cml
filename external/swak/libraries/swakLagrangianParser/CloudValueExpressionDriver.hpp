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
    CML::CloudValueExpressionDriver

Description

SourceFiles
    CloudValueExpressionDriverI.hpp
    CloudValueExpressionDriver.cpp

Contributors/Copyright:
    2010-2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef CloudValueExpressionDriver_H
#define CloudValueExpressionDriver_H

#include "volFields.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "ExpressionResult.hpp"

#include "CommonValueExpressionDriver.hpp"

#include "cloud_.hpp"

namespace CML {
    class CloudValueExpressionDriver;
}

using CML::CloudValueExpressionDriver;

#include "CloudValueExpressionParser.tab.hh"

namespace parserCloud {
    class location;
    class CloudValueExpressionParser;
}

namespace CML
{

class CloudProxy;
class ReaderParticleCloud;

/*---------------------------------------------------------------------------*\
                           Class CloudValueExpressionDriver Declaration
\*---------------------------------------------------------------------------*/

class CloudValueExpressionDriver
:
    public CommonValueExpressionDriver
{
    // Private data

    //- the cloud
    const cloud &cloud_;

    //- the actual class dealing with the particle cloud
    autoPtr<CloudProxy> proxy_;

    static const cloud& getCloud(
        const dictionary &dict,
        const fvMesh &mesh,
        bool keep=true
    );

    static const cloud& getCloud(
        const word &name,
        const fvMesh &mesh,
        bool keep=true
    );

    //- the interpolation schemes
    dictionary interpolationSchemes_;

    // Private Member Functions

    void writeProxyInfo();

    // used to make sure that the same cloud info is printed only once
    static HashTable<string,word> cloudInfos_;

protected:

    // Handling the scanner
    void scan_begin();
    void scan_end();

    //- set the state of the lexer to scanning vector components
    void startVectorComponent();

    //- set the state of the lexer to scanning tensor components
    void startTensorComponent();

    //- set the state to "eat characters"
    void startEatCharacters();

    //- weights of the entries if it is not a point field

    //- parametert size is the expected size of the weight field and
    //- will be used to determine whether this is correct.
    virtual tmp<scalarField> weightsNonPoint(
        label size
    ) const;

    //- get field values from the continous phase
    template <typename Type>
    tmp<Field<Type> > getFluidField(
        const word &name
    );
public:

     //- Runtime type information
    TypeName("CloudValueExpressionDriver");

    // Constructors

    // //- Construct from components
    // CloudValueExpressionDriver(
    //     bool autoInterpolate=false,
    //     bool warnAutoInterpolate=true
    // );

    //- Copy construct
    CloudValueExpressionDriver(const CloudValueExpressionDriver&);

    //- Construct from a dictionary
    CloudValueExpressionDriver(
        const dictionary&,
        const fvMesh&
    );

    //- Construct from an id
    CloudValueExpressionDriver(
        const word&,
        const fvMesh&
    );

    //- construct from a cloud
    CloudValueExpressionDriver(
        const ReaderParticleCloud& c,
        const word &defaultInterpolation
    );

    //- Construct from a dictionary and cloud
    CloudValueExpressionDriver(
        const dictionary&,
        const ReaderParticleCloud& c
    );

    autoPtr<CommonValueExpressionDriver> clone() const
        {
            return autoPtr<CommonValueExpressionDriver>(
                new CloudValueExpressionDriver(*this)
            );
        }

    // Destructor

    ~CloudValueExpressionDriver();

    // Member Functions

    tmp<vectorField> makePositionField() const;

    tmp<scalarField> makeIdField();

    tmp<Field<bool> > makeCellSetField(const word &name);
    tmp<Field<bool> > makeCellZoneField(const word &name);

    const CloudProxy &proxy() const
        { return proxy_(); }

    const cloud &theCloud() const
        { return cloud_; }

//     template<class Type>
//     Field<Type> *getField(const string &name);

    bool isScalarField(const word &name);
    bool isVectorField(const word &name);
    bool isTensorField(const word &name);
    bool isSymmTensorField(const word &name);
    bool isSphericalTensorField(const word &name);
    bool isBoolField(const word &name);

    tmp<Field<scalar> > getScalarField(
        const word &name
    );

    tmp<Field<vector> > getVectorField(
        const word &name
    );

    tmp<Field<tensor> > getTensorField(
        const word &name
    );

    tmp<Field<symmTensor> > getSymmTensorField(
        const word &name
    );

    tmp<Field<sphericalTensor> > getSphericalTensorField(
        const word &name
    );

    tmp<Field<bool> > getBoolField(
        const word &name
    );

    //- return a new plugin-function
    virtual autoPtr<CommonPluginFunction> newPluginFunction(
        const word &name
    );

    //- tests for a plugin-function
    virtual bool existsPluginFunction(
        const word &name
    );

    virtual label size() const;

    virtual label pointSize() const;

    const fvMesh &mesh() const;

    // Handling the parser
    void parseInternal(int startToken);

    //- execute a plugin-function
    template<class T>
    autoPtr<Field<T> > evaluatePluginFunction(
        const word &name,
        const parserCloud::location &loc,
        int &scanned,
        bool isPoint=false
    );

    static const SymbolTable<CloudValueExpressionDriver> &symbolTable();

    int startupSymbol(const word &name);

//     template<class Type>
//     virtual Field<Type> *toFace(const Field<Type> &)=0;

//     template<class Type>
//     virtual Field<Type> *toPoint(const Field<Type> &)=0;

    // allow access to protected methods
    friend class parserCloud::CloudValueExpressionParser;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "CloudValueExpressionDriverI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
