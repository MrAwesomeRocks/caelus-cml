/*---------------------------------------------------------------------------*\
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
    CML::twoPhaseMixture

Description
    A two-phase incompressible transportModel

SourceFiles
    twoPhaseMixture.cpp

\*---------------------------------------------------------------------------*/

#ifndef twoPhaseMixture_H
#define twoPhaseMixture_H

#include "transportModel.hpp"
#include "viscosityModel.hpp"
#include "dimensionedScalar.hpp"
#include "volFields.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                      Class twoPhaseMixture Declaration
\*---------------------------------------------------------------------------*/

class twoPhaseMixture
:
    public transportModel
{
protected:

    // Protected data

        word phase1Name_;
        word phase2Name_;

        autoPtr<viscosityModel> nuModel1_;
        autoPtr<viscosityModel> nuModel2_;

        dimensionedScalar rho1_;
        dimensionedScalar rho2_;

        const volVectorField& U_;
        const surfaceScalarField& phi_;

        volScalarField alpha1_;

        volScalarField nu_;


    // Private Member Functions

        //- Calculate and return the laminar viscosity
        void calcNu();


public:

    // Constructors

        //- Construct from components
        twoPhaseMixture
        (
            const volVectorField& U,
            const surfaceScalarField& phi,
            const word& alpha1Name = "alpha1"
        );


    //- Destructor
    ~twoPhaseMixture()
    {}


    // Member Functions

        const word& phase1Name() const
        {
            return phase1Name_;
        }

        const word& phase2Name() const
        {
            return phase2Name_;
        }

        //- Return the phase-fraction of phase 1
        const volScalarField& alpha1() const
        {
            return alpha1_;
        }

        //- Return the phase-fraction of phase 1
        volScalarField& alpha1()
        {
            return alpha1_;
        }

        //- Return const-access to phase1 viscosityModel
        const viscosityModel& nuModel1() const
        {
            return nuModel1_();
        }

        //- Return const-access to phase2 viscosityModel
        const viscosityModel& nuModel2() const
        {
            return nuModel2_();
        }

        //- Return const-access to phase1 density
        const dimensionedScalar& rho1() const
        {
            return rho1_;
        }

        //- Return const-access to phase2 density
        const dimensionedScalar& rho2() const
        {
            return rho2_;
        };

        //- Return the dynamic laminar viscosity
        virtual tmp<volScalarField> mu() const;

        //- Return the face-interpolated dynamic laminar viscosity
        virtual tmp<surfaceScalarField> muf() const;

        //- Return the kinematic laminar viscosity
        virtual tmp<volScalarField> nu() const
        {
            return nu_;
        }

        //- Return the face-interpolated kinematic laminar viscosity
        virtual tmp<surfaceScalarField> nuf() const;

        //- Correct the laminar viscosity
        virtual void correct()
        {
            calcNu();
        }

        //- Read base transportProperties dictionary
        virtual bool read();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
