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
    CML::normalToFace

Description
    A topoSetSource to select faces based on normal.

SourceFiles
    normalToFace.cpp

\*---------------------------------------------------------------------------*/

#ifndef normalToFace_H
#define normalToFace_H

#include "topoSetSource.hpp"
#include "NamedEnum.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                       Class normalToFace Declaration
\*---------------------------------------------------------------------------*/

class normalToFace
:
    public topoSetSource
{

private:

        //- Add usage string
        static addToUsageTable usage_;

        //- (unit)vector to compare to
        vector normal_;

        //- Tolerance (i.e. cos of angle between normal_ and faceNormal)
        const scalar tol_;


    // Private Member Functions

        //- Normalize normal and check tolerance
        void setNormal();


public:

    //- Runtime type information
    TypeName("normalToFace");

    // Constructors

        //- Construct from components
        normalToFace
        (
            const polyMesh& mesh,
            const vector& normal,
            const scalar tol
        );

        //- Construct from dictionary
        normalToFace(const polyMesh& mesh, const dictionary& dict);

        //- Construct from Istream
        normalToFace(const polyMesh& mesh, Istream&);


    //- Destructor
    virtual ~normalToFace();


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
