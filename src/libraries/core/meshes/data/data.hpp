/*---------------------------------------------------------------------------*\
Copyright (C) 2011 OpenFOAM Foundation
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
    CML::data

Description
    Database for solution data, solver performance and other reduced data.

    fvMesh is derived from data so that all fields have access to the data from
    the mesh reference they hold.

SourceFiles
    data.cpp

\*---------------------------------------------------------------------------*/

#ifndef data_H
#define data_H

#include "IOdictionary.hpp"
#include "lduMatrix.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                            Class data Declaration
\*---------------------------------------------------------------------------*/

class data
:
    public IOdictionary
{
    // Private data

        //- Previously used time-index, used for reset between iterations
        mutable label prevTimeIndex_;


    // Private Member Functions

        //- Disallow default bitwise copy construct
        data(const data&);

        //- Disallow default bitwise assignment
        void operator=(const data&);


public:

    //- Debug switch
    static int debug;


    // Constructors

        //- Construct for objectRegistry
        data(const objectRegistry& obr);


    // Member Functions

        // Access

            //- Return the dictionary of solver performance data
            //  which includes initial and final residuals for convergence
            //  checking
            const dictionary& solverPerformanceDict() const;

            //- Add/set the solverPerformance entry for the named field
            void setSolverPerformance
            (
                const word& name,
                const solverPerformance&
            ) const;

            //- Add/set the solverPerformance entry, using its fieldName
            void setSolverPerformance
            (
                const solverPerformance&
            ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
