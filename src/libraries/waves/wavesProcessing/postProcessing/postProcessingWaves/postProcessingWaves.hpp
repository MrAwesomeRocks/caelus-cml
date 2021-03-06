/*---------------------------------------------------------------------------*\
Copyright Niels Gjøl Jacobsen, Technical University of Denmark.
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

Class
    CML::postProcessingWaves

Description
    Abstract base class for all sorts of different post-processing of
    logged/probed data, e.g. ensemble average, spectral analysis, reflection
    analysis, etc.

SourceFiles
    postProcessingWaves.cpp

Author
    Niels Gjøl Jacobsen, Technical University of Denmark.


\*---------------------------------------------------------------------------*/

#ifndef postProcessingWaves_HPP
#define postProcessingWaves_HPP

#include "fvMesh.hpp"
#include "fvCFD.hpp"
#include "volFields.hpp"
#include "IOdictionary.hpp"

#include "autoPtr.hpp"
#include "runTimeSelectionTables.hpp"

#include "complex.hpp"

#include "OFstream.hpp"

#include <fstream>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                         Class porosityCoefficient Declaration
\*---------------------------------------------------------------------------*/

class postProcessingWaves
{

protected:

    // Protected data
        const Time& rT_;
        const dictionary& actionProperties_;

        word callName_;
        word actionType_;
        fileName addDir_;
        fileName directDir_;

    // Protected member functions

        void getTimeDirs
        (
            const word&,
            wordList&
        );

        void writeNameDict
        (
            const scalar& dt,
            const wordList& names
        );

        void writeXYZDict
        (
            const scalar&,
            const scalarField&,
            const scalarField&,
            const scalarField&
        );

        void writeIOScalarField
        (
            const scalarField& field,
            const word& name
        );

        void writeIOVectorField
        (
            const vectorField& field,
            const word& name
        );

        scalarField readIOScalarField
        (
            const word& name
        );

        vectorField readIOVectorField
        (
            const word& name
        );

        word dataType();

        scalar readDeltaT
        (
            const dictionary&
        );

        void readIndices
        (
            const dictionary&,
            labelList&
        );

        List<scalarField> readScalarFields
        (
            const labelList& indices
        );

        List<vectorField> readVectorFields
        (
            const labelList& indices
        );

        scalarField equidistantTime
        (
            const List<std::pair<scalar, label > >& timeLabel,
            const dictionary&
        );

        void interpolationWeights
        (
            const List<std::pair<scalar, label> >&,
            const scalarField&,
            scalarField&,
            labelList&,
            labelList&
        );

private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        postProcessingWaves(const postProcessingWaves&);

        //- Disallow default bitwise assignment
        void operator=(const postProcessingWaves&);

public:

    //- Runtime type information
    TypeName("postProcessingWaves");

    // Declare run-time constructor selection table

        declareRunTimeSelectionTable
        (
            autoPtr,
            postProcessingWaves,
            postProcessingWaves,
            (
                const Time& rT,
                const dictionary& actionProp,
                const word& action
            ),
            (rT, actionProp, action)
        );

    // Constructors

        //- Construct from components
        postProcessingWaves
        (
            const Time&,
            const dictionary&,
            const word&
        );

    // Selectors

        //- Return a reference to the selected turbulence model
        static autoPtr<postProcessingWaves> New
        (
            const Time&,
            const dictionary&,
            const word&
        );

    // Destructor

        virtual ~postProcessingWaves();

    // Member Functions

        // Access
        virtual void evaluate() = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// The GCC compiler do not allow these to be overloaded for some reason,
// hence A and B
bool pairSortA(std::pair<scalar, label>, std::pair<scalar, label>);

bool paitSortB(std::pair<scalar, word>, std::pair<scalar, word>);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
