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
    CML::faceToPoint

Description
    A topoSetSource to select points based on usage in faces.

SourceFiles
    faceToPoint.cpp

\*---------------------------------------------------------------------------*/

#ifndef faceToPoint_H
#define faceToPoint_H

#include "topoSetSource.hpp"
#include "NamedEnum.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                        Class faceToPoint Declaration
\*---------------------------------------------------------------------------*/

class faceToPoint
:
    public topoSetSource
{

public:
        //- Enumeration defining the valid options
        enum faceAction
        {
            ALL
        };

private:

        //- Add usage string
        static addToUsageTable usage_;

        static const NamedEnum<faceAction, 1> faceActionNames_;

        //- Name of set to use
        word setName_;

        //- Option
        faceAction option_;


    // Private Member Functions

        //- Depending on face to cell option add to or delete from cellSet.
        void combine(topoSet& set, const bool add) const;


public:

    //- Runtime type information
    TypeName("faceToPoint");

    // Constructors

        //- Construct from components
        faceToPoint
        (
            const polyMesh& mesh,
            const word& setName,
            const faceAction option
        );

        //- Construct from dictionary
        faceToPoint
        (
            const polyMesh& mesh,
            const dictionary& dict
        );

        //- Construct from Istream
        faceToPoint
        (
            const polyMesh& mesh,
            Istream&
        );


    //- Destructor
    virtual ~faceToPoint();


    // Member Functions

        virtual sourceType setType() const
        {
            return POINTSETSOURCE;
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
