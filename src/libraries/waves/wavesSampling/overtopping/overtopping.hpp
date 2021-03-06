/*---------------------------------------------------------------------------*\
Copyright Niels G. Jacobsen Technical University of Denmark
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
    CML::overtopping

Description
    Computes the overtopping rates in the case of two phase flows.

SourceFiles
    overtopping.cpp

\*---------------------------------------------------------------------------*/

#ifndef overtopping_HPP
#define overtopping_HPP

#include "objectRegistry.hpp"
#include "fvMesh.hpp"
#include "surfaceFieldsFwd.hpp"
#include "volFieldsFwd.hpp"
#include "mapPolyMesh.hpp"
#include "OFstream.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                         Class faceSource Declaration
\*---------------------------------------------------------------------------*/

class overtopping
{



private:

    // Private member data
        word phiName_;

        word rhoPhiName_;

        scalar rho1_, rho2_, invRhoDiff_;

        const fvMesh& mesh_;

        autoPtr<OFstream> outputFilePtr_;

        autoPtr<OFstream> outputFileForcePtr_;

    // Private member functions
        bool operateOnZone(const faceZone&) const;

        void computeAndWriteBoundary
        (
            const label&,
            const surfaceScalarField&,
            const surfaceScalarField&,
            const surfaceScalarField&,
            const surfaceVectorField&,
            vector&,
            vector&
        ) const;

        void computeAndWrite
        (
            const faceZone&,
            const surfaceScalarField&,
            const surfaceScalarField&
        );


protected:

    // Protected data


    // Protected member functions

    // Functions to be over-ridden from IOoutputFilter class

        //- Make the output file
        virtual void makeFile();

        //- Write the output file header
        virtual void writeFileHeader();

        //- Update mesh
        virtual void updateMesh(const mapPolyMesh&);

        //- Move points
        virtual void movePoints(const pointField&);


public:

    //- Run-time type information
    TypeName("overtopping");


    //- Construct from components
    overtopping
    (
        const word& name,
        const objectRegistry& obr,
        const dictionary& dict,
        const bool loadFromFiles = false
    );

    virtual ~overtopping();

    // Function object functions

        //- Read from dictionary
        virtual void read(const dictionary&);

        //- Calculate and write
        virtual void write();

        //- Execute
        virtual void execute() {};

        //- Execute the at the final time-loop, currently does nothing
        virtual void end() {};

        virtual bool timeSet();

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
