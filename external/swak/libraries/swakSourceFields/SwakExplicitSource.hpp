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

Class
    CML::SwakExplicitSource

Description
    Sets an explicit source term according to an expression

SourceFiles
    SwakExplicitSource.cpp

Contributors/Copyright:
    2010, 2012-2014 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef SwakExplicitSource_H
#define SwakExplicitSource_H

#include "SwakBasicSourceCommon.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace fv
{

/*---------------------------------------------------------------------------*\
                           Class SwakExplicitSource Declaration
\*---------------------------------------------------------------------------*/

template<class T>
class SwakExplicitSource
:
    public SwakBasicSourceCommon<T>
{
    // Private data

    SwakExplicitSource(const SwakExplicitSource&);

public:

    //- Runtime type information
    TypeName("SwakExplicitSource");

    // Constructors

        //- Construct from a dictionary
        SwakExplicitSource
        (
            const word& name,
            const word& modelType,
            const dictionary& dict,
            const fvMesh& mesh
        );

    // Destructor

        virtual ~SwakExplicitSource();

        // Evaluation

            //- Add explicit contribution to equation
            virtual void addSup(fvMatrix<T>& eqn, const label fieldI);


};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace fv
} // End namespace CML

#include "polyMesh.hpp"
#include "cellSet.hpp"
#include "fvMatrix.hpp"

#include "FieldValueExpressionDriver.hpp"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from dictionary
template<class T>
CML::fv::SwakExplicitSource<T>::SwakExplicitSource
(
    const word& name,
    const word& modelType,
    const dictionary& dict,
    const fvMesh& mesh
)
:
    SwakBasicSourceCommon<T>(name, modelType, dict, mesh)
{
    this->read(dict);

    this->driver().createWriterAndRead(
        dict.name().name()+"_"+this->type()+"<"+
        pTraits<T>::typeName+">"
    );

    if(this->verbose_) {
        WarningInFunction
            << "Adding source term to the fields " << this->fieldNames_
            << " to the values " << this->expressions_
            << " will be verbose. To switch this off set the "
            << "parameter 'verbose' to false" << endl;
    }

}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class T>
CML::fv::SwakExplicitSource<T>::~SwakExplicitSource()
{}


//- Add explicit contribution to equation
template<class T>
void CML::fv::SwakExplicitSource<T>::addSup(fvMatrix<T>& eqn, const label fieldI)
{
    if (debug)
    {
        Info<< "SwakExplicitSource<"<< pTraits<T>::typeName
            << ">::addSup for source " << this->name_ << endl;
    }

    this->driver().clearVariables();
    this->driver().parse(this->expressions_[fieldI]);

    if(
        !this->driver().
        FieldValueExpressionDriver::template resultIsTyp<typename SwakExplicitSource<T>::resultField>()
    ) {
        FatalErrorInFunction
            << "Result of " << this->expressions_[fieldI] << " is not a "
                << pTraits<T>::typeName
                << endl
                << exit(FatalError);
    }

    typename SwakExplicitSource<T>::resultField result(
        this->driver().
        FieldValueExpressionDriver::template getResult<typename SwakExplicitSource<T>::resultField>()
    );
    result.dimensions().reset(this->dimensions_[fieldI]);
    typename SwakExplicitSource<T>::resultField usedResult(result*0);
    forAll(this->cells_,i) {
        label cellI=this->cells_[i];
        usedResult[cellI]=result[cellI];
    }
    eqn+=usedResult;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
