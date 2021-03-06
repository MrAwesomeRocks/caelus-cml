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

\*---------------------------------------------------------------------------*/

#include "trapz.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

defineTypeNameAndDebug(trapz, 0);
addToRunTimeSelectionTable(postProcessingWaves, trapz, postProcessingWaves);

// * * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * //


void trapz::evaluateScalar()
{
    Info << "        - Trapezoidal integration" << endl;

    List<scalarField> input = readScalarFields(indices_);
    scalarField time = readIOScalarField(callName_ + "_time");

    scalarField trapezoidal(input.size(), 0.0);

    forAll (input, I)
    {
        const scalarField& field(input[I]);

        for (int i=1; i < field.size(); i++)
        {
            trapezoidal[I] += 0.5*(time[i] - time[i - 1])
                *(field[i] + field[i - 1]);
        }
    }

    writeScalar(trapezoidal);
}


void trapz::writeScalar
(
    const scalarField& trapezoidal
)
{
    Info << "        - Writing trapezoidal integral: " << directDir_.c_str()
         << this->type() << endl;

    mkDir(directDir_ + this->type());

    autoPtr<OFstream> spectrumPtr_;

    spectrumPtr_.reset
    (
        new OFstream
        (
            directDir_ + "/" + this->type() + "/" + callName_ + "_trapz.dat"
        )
    );

    forAll (indices_, indexi)
    {
        spectrumPtr_() << indices_[indexi] << tab << trapezoidal[indexi]
                       << endl;
    }
}


void trapz::evaluateVector()
{
    Info << "        - Trapezoidal integration" << endl;

    List<vectorField> input = readVectorFields(indices_);
    scalarField time = readIOScalarField(callName_ + "_time");

    vectorField trapezoidal(input.size(), vector::zero);

    forAll (input, I)
    {
        const vectorField& field(input[I]);

        for (int i=1; i < field.size(); i++)
        {
            trapezoidal[I] += 0.5*(time[i] - time[i - 1])
                *(field[i] + field[i - 1]);
        }
    }

    writeVector(trapezoidal);
}


void trapz::writeVector
(
    const vectorField& trapezoidal
)
{
    Info << "        - Writing trapezoidal integral: " << directDir_.c_str()
         << this->type() << endl;

    mkDir(directDir_ + this->type());

    autoPtr<OFstream> spectrumPtr_;

    spectrumPtr_.reset
    (
        new OFstream
        (
            directDir_ + "/" + this->type() + "/" + callName_ + "_trapz.dat"
        )
    );

    forAll (indices_, indexi)
    {
        spectrumPtr_() << indices_[indexi]
                       << tab << trapezoidal[indexi].x()
                       << tab << trapezoidal[indexi].y()
                       << tab << trapezoidal[indexi].z() << endl;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //


trapz::trapz
(
    const Time& rT,
    const dictionary& actionProp,
    const word& action
)
:
    postProcessingWaves( rT, actionProp, action ),

#   include "../../dataDict.hpp"
{
    readIndices(dataDict_, indices_);
}


trapz::~trapz()
{
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


void trapz::evaluate()
{
    if (dataType() == "scalar")
    {
        evaluateScalar();
    }
    else if (dataType() == "vector")
    {
        evaluateVector();
    }
    else
    {
        NotImplemented;
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
