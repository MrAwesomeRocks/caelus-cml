/*---------------------------------------------------------------------------*\
Copyright (C) 2012 OpenFOAM Foundation
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
    CML::regionToFace

Description
    A topoSetSource to select faces belonging to topological connected region
    (that contains given point)

SourceFiles
    regionToFace.cpp

\*---------------------------------------------------------------------------*/

#ifndef regionToFace_H
#define regionToFace_H

#include "topoSetSource.hpp"
#include "PackedBoolList.hpp"
#include "indirectPrimitivePatch.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                        Class regionToFace Declaration
\*---------------------------------------------------------------------------*/

class regionToFace
:
    public topoSetSource
{
    // Private data

        //- Add usage string
        static addToUsageTable usage_;

        //- Name of set to use
        word setName_;

        //- Coordinate that is nearest/on connected region
        point nearPoint_;

    // Private Member Functions

        //- Walk edge-face-edge
        void markZone
        (
            const indirectPrimitivePatch& patch,
            const label procI,
            const label faceI,
            const label zoneI,
            labelList& faceZone
        ) const;

        void combine(topoSet& set, const bool add) const;

public:

    //- Runtime type information
    TypeName("regionToFace");

    // Constructors

        //- Construct from components
        regionToFace
        (
            const polyMesh& mesh,
            const word& setName,
            const point& nearPoint
        );

        //- Construct from dictionary
        regionToFace
        (
            const polyMesh& mesh,
            const dictionary& dict
        );

        //- Construct from Istream
        regionToFace
        (
            const polyMesh& mesh,
            Istream&
        );


    //- Destructor
    virtual ~regionToFace();


    // Member Functions

        virtual sourceType setType() const
        {
            return FACESETSOURCE;
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
