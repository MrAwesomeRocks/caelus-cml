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
    CML::setsToFaceZone

Description
    A topoSetSource to select faces based on usage in a faceSet and cellSet

SourceFiles
    setsToFaceZone.cpp

\*---------------------------------------------------------------------------*/

#ifndef setsToFaceZone_H
#define setsToFaceZone_H

#include "topoSetSource.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                        Class setsToFaceZone Declaration
\*---------------------------------------------------------------------------*/

class setsToFaceZone
:
    public topoSetSource
{
    // Private data

        //- Add usage string
        static addToUsageTable usage_;

        //- Name of set to use
        word faceSetName_;

        //- Name of set to use
        word cellSetName_;

public:

    //- Runtime type information
    TypeName("setsToFaceZone");

    // Constructors

        //- Construct from components
        setsToFaceZone
        (
            const polyMesh& mesh,
            const word& faceSetName,
            const word& cellSetName
        );

        //- Construct from dictionary
        setsToFaceZone
        (
            const polyMesh& mesh,
            const dictionary& dict
        );

        //- Construct from Istream
        setsToFaceZone
        (
            const polyMesh& mesh,
            Istream&
        );


    //- Destructor
    virtual ~setsToFaceZone();


    // Member Functions

        virtual sourceType setType() const
        {
            return FACEZONESOURCE;
        }

        virtual void applyToSet
        (
            const topoSetSource::setAction action,
            topoSet&
        ) const;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
