/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2018 OpenFOAM Foundation
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
    CML::thermalBaffleModel

Description

SourceFiles
    thermalBaffleModel.C

\*---------------------------------------------------------------------------*/

#ifndef thermalBaffleModel_H
#define thermalBaffleModel_H

#include "runTimeSelectionTables.hpp"
#include "scalarIOField.hpp"
#include "autoPtr.hpp"
#include "volFieldsFwd.hpp"
#include "solidThermo.hpp"
#include "regionModel1D.hpp"
#include "radiationModel.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace regionModels
{
namespace thermalBaffleModels
{

/*---------------------------------------------------------------------------*\
                      Class thermalBaffleModel Declaration
\*---------------------------------------------------------------------------*/

class thermalBaffleModel
:
    public regionModel1D
{
private:

    // Private Member Functions

        //- Disallow default bitwise copy construct
        thermalBaffleModel(const thermalBaffleModel&);

        //- Disallow default bitwise assignment
        void operator=(const thermalBaffleModel&);

        //- Initialize thermal Baffle
        void init();


protected:

    // Protected Data

        //- Baffle physical thickness
        scalarField thickness_;

        //- Baffle mesh thickness
        dimensionedScalar delta_;

        //- Is it one dimension
        bool oneD_;

        //- Is thickness constant
        bool constantThickness_;


    // Protected Member Functions

        //- Read control parameters from IO dictionary
        virtual bool read();

        //- Read control parameters from dictionary
        virtual bool read(const dictionary&);


public:

    //- Runtime type information
    TypeName("thermalBaffleModel");


    // Declare runtime constructor selection tables

         declareRunTimeSelectionTable
         (
             autoPtr,
             thermalBaffleModel,
             mesh,
             (
                const word& modelType,
                const fvMesh& mesh
             ),
             (modelType, mesh)
         );

         declareRunTimeSelectionTable
         (
             autoPtr,
             thermalBaffleModel,
             dictionary,
             (
                const word& modelType,
                const fvMesh& mesh,
                const dictionary& dict
             ),
             (modelType, mesh, dict)
         );


    // Constructors

        //- Construct null from mesh
        thermalBaffleModel(const fvMesh& mesh);

        //- Construct from type name and mesh
        thermalBaffleModel(const word& modelType, const fvMesh& mesh);

        //- Construct from type name and mesh and dict
        thermalBaffleModel
        (
            const word& modelType,
            const fvMesh& mesh,
            const dictionary& dict
        );


    // Selectors

        //- Return a reference to the selected model
        static autoPtr<thermalBaffleModel> New(const fvMesh& mesh);

        //- Return a reference to the selected model using dictionary
        static autoPtr<thermalBaffleModel> New
        (
            const fvMesh& mesh,
            const dictionary& dict
        );


    //- Destructor
    virtual ~thermalBaffleModel();


    // Member Functions

        // Access

            //- Return solid thermo
            virtual const solidThermo& thermo() const = 0;

            //- Return thickness
            const scalarField& thickness() const
            {
                return thickness_;
            }

            //- Return geometrical thickness
            const dimensionedScalar& delta() const
            {
                return delta_;
            }

             //- Return if region is one dimensional
            bool oneD() const
            {
                return oneD_;
            }

             //- Return if region has constant thickness
            bool constantThickness() const
            {
                return constantThickness_;
            }


            // Fields

                //- Return density [kg/m3]
                virtual const volScalarField& rho() const = 0;

                //- Return const temperature [K]
                virtual const volScalarField& T() const = 0;

                //- Return specific heat capacity [J/kg/K]
                virtual const tmp<volScalarField> Cp() const = 0;

                //- Return the region absorptivity [1/m]
                virtual const volScalarField& kappaRad() const = 0;

                //- Return the region thermal conductivity [W/m/k]
                virtual const volScalarField& kappa() const = 0;


        // Evolution

            //- Pre-evolve region
            virtual void preEvolveRegion();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace thermalBaffleModels
} // End namespace regionModels
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
