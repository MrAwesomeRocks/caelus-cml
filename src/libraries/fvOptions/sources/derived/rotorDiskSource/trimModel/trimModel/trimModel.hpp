/*---------------------------------------------------------------------------*\
Copyright (C) 2012-2013 OpenFOAM Foundation
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

Class
    CML::trimModel

Description
    Trim model base class

SourceFiles
    trimModel.cpp

\*---------------------------------------------------------------------------*/

#ifndef trimModel_HPP
#define trimModel_HPP

#include "rotorDiskSource.hpp"
#include "dictionary.hpp"
#include "runTimeSelectionTables.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                         Class trimModel Declaration
\*---------------------------------------------------------------------------*/

class trimModel
{

protected:

    // Protected data

        //- Reference to the rotor source model
        const fv::rotorDiskSource& rotor_;

        //- Name of model
        const word name_;

        //- Coefficients dictionary
        dictionary coeffs_;


public:

    //- Run-time type information
    TypeName("trimModel");


    // Declare runtime constructor selection table

        declareRunTimeSelectionTable
        (
            autoPtr,
            trimModel,
            dictionary,
            (
                const fv::rotorDiskSource& rotor,
                const dictionary& dict
            ),
            (rotor, dict)
        );


    // Constructors

        //- Construct from components
        trimModel
        (
            const fv::rotorDiskSource& rotor,
            const dictionary& dict,
            const word& name
        );


    // Selectors

        //- Return a reference to the selected trim model
        static autoPtr<trimModel> New
        (
            const fv::rotorDiskSource& rotor,
            const dictionary& dict
        );


    //- Destructor
    virtual ~trimModel();


    // Member functions

        //- Read
        virtual void read(const dictionary& dict);

        //- Return the geometric angle of attack [rad]
        virtual tmp<scalarField> thetag() const = 0;

        //- Correct the model
        virtual void correct
        (
            const vectorField& U,
            vectorField& force
        ) = 0;

        //- Correct the model for compressible flow
        virtual void correct
        (
            const volScalarField rho,
            const vectorField& U,
            vectorField& force
        ) = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
