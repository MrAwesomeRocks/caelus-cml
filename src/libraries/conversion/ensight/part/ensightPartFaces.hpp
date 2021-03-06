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
    CML::ensightPartFaces

Description
    An implementation of ensightPart to hold volume mesh faces.

SourceFiles
    ensightPartFaces.C

\*---------------------------------------------------------------------------*/

#ifndef ensightPartFaces_H
#define ensightPartFaces_H

#include "ensightPart.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                     Class ensightPartFaces Declaration
\*---------------------------------------------------------------------------*/

class ensightPartFaces
:
    public ensightPart
{
    // Private Member Functions

        //- Disallow default bitwise assignment
        void operator=(const ensightPartFaces&);

        //- track points used
        virtual localPoints calcLocalPoints() const;

        //- element connectivity
        virtual void writeConnectivity
        (
            ensightGeoFile&,
            const word& key,
            const labelUList& idList,
            const labelUList& pointMap
        ) const;


protected:

        //- addressable ensight element types
        enum elemType
        {
            tria3Elements,
            quad4Elements,
            nsidedElements
        };


    // Static data members

        static const List<word> elemTypes_;


    // Protected data

        //- faces referenced
        const faceList& faces_;

        //- Can skip local point renumbering when points are contiguous
        const bool contiguousPoints_;


    // Protected Member Functions

        //- Classify the face shapes, set elemLists.
        void classify(const faceList&);

        //- Helper: write connectivity
        void writeConnectivity
        (
            ensightGeoFile&,
            const word& key,
            const faceList&,
            const labelUList& idList,
            const labelUList& pointMap
        ) const;


public:

    //- Runtime type information
    TypeName("ensightFaces");

    // Constructors

        //- Construct empty part with number and description
        ensightPartFaces(label partNumber, const string& partDescription);

        //- Construct part with number, description, points and faces
        //  Can skip local point renumbering when points are contiguous
        ensightPartFaces
        (
            label partNumber,
            const string& partDescription,
            const pointField&,
            const faceList&,
            const bool contiguousPoints = false
        );

        //- Construct from polyMesh and polyPatch
        ensightPartFaces
        (
            label partNumber,
            const polyMesh&,
            const polyPatch&
        );

        //- Construct as copy
        ensightPartFaces(const ensightPartFaces&);

        //- Reconstruct part characteristics (eg, element types) from Istream
        //  A part reconstructed in this manner can be used when writing fields,
        //  but cannot be used to write a new geometry
        //  \sa CML::ensightPart::reconstruct
        ensightPartFaces(Istream&);

        //- Reconstruct part characteristics on freestore from Istream
        static autoPtr<ensightPartFaces> New(Istream& is)
        {
            return autoPtr<ensightPartFaces>(new ensightPartFaces(is));
        }


    //- Destructor
    virtual ~ensightPartFaces();


    // Member Functions

        //- write geometry
        virtual void writeGeometry(ensightGeoFile&) const;

        //- static listing of the element types
        virtual const List<word>& elementTypes() const
        {
            return elemTypes_;
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
