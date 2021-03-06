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
    CML::edgeMesh

Description
    Points connected by edges.

SourceFiles
    edgeMeshI.hpp
    edgeMesh.cpp
    edgeMeshIO.cpp

\*---------------------------------------------------------------------------*/

#ifndef edgeMesh_H
#define edgeMesh_H

#include "pointField.hpp"
#include "edgeList.hpp"
#include "edgeFormatsCore.hpp"
#include "runTimeSelectionTables.hpp"
#include "memberFunctionSelectionTables.hpp"
#include "HashSet.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes
class Istream;
class Ostream;

// Forward declaration of friend functions and operators
class edgeMesh;
Istream& operator>>(Istream&, edgeMesh&);
Ostream& operator<<(Ostream&, const edgeMesh&);


/*---------------------------------------------------------------------------*\
                           Class edgeMesh Declaration
\*---------------------------------------------------------------------------*/

class edgeMesh
:
    public fileFormats::edgeFormatsCore
{
    // Private data

        //- Vertices of the edges
        pointField points_;

        //- The edges defining the boundary
        edgeList edges_;

        //- From point to edges
        mutable autoPtr<labelListList> pointEdgesPtr_;

    // Private Member Functions

        //- Calculate point-edge addressing (inverse of edges)
        void calcPointEdges() const;


protected:

    // Protected Member Functions

        //- Non-const access to global points
        inline pointField& storedPoints();

        //- Non-const access to the edges
        inline edgeList& storedEdges();


public:

        //- Runtime type information
        TypeName("edgeMesh");


    // Static

        //- Can we read this file format?
        static bool canRead(const fileName&, const bool verbose=false);

        //- Can we read this file format?
        static bool canReadType(const word& ext, const bool verbose=false);

        //- Can we write this file format type?
        static bool canWriteType(const word& ext, const bool verbose=false);

        static wordHashSet readTypes();
        static wordHashSet writeTypes();


    // Constructors

        //- Construct null
        edgeMesh();

        //- Construct from components
        edgeMesh(const pointField&, const edgeList&);

        //- Construct by transferring components (points, edges).
        edgeMesh
        (
            const Xfer<pointField>&,
            const Xfer<edgeList>&
        );

        //- Construct as copy
        edgeMesh(const edgeMesh&);

        //- Construct from file name (uses extension to determine type)
        edgeMesh(const fileName&);

        //- Construct from file name (uses extension to determine type)
        edgeMesh(const fileName&, const word& ext);

        //- Construct from Istream
        edgeMesh(Istream&);


    // Declare run-time constructor selection table

        declareRunTimeSelectionTable
        (
            autoPtr,
            edgeMesh,
            fileExtension,
            (
                const fileName& name
            ),
            (name)
        );


    // Selectors

        //- Select constructed from filename (explicit extension)
        static autoPtr<edgeMesh> New
        (
            const fileName&,
            const word& ext
        );

        //- Select constructed from filename (implicit extension)
        static autoPtr<edgeMesh> New(const fileName&);


    //- Destructor
    virtual ~edgeMesh();


    // Member Function Selectors

        declareMemberFunctionSelectionTable
        (
            void,
            edgeMesh,
            write,
            fileExtension,
            (
                const fileName& name,
                const edgeMesh& mesh
            ),
            (name, mesh)
        );

        //- Write to file
        static void write(const fileName&, const edgeMesh&);


    // Member Functions

        //- Transfer the contents of the argument and annul the argument
        void transfer(edgeMesh&);

        //- Transfer contents to the Xfer container
        Xfer<edgeMesh > xfer();

    // Read

        //- Read from file. Chooses reader based on explicit extension
        bool read(const fileName&, const word& ext);

        //- Read from file. Chooses reader based on detected extension
        virtual bool read(const fileName&);


    // Access

        //- Return points
        inline const pointField& points() const;

        //- Return edges
        inline const edgeList& edges() const;

        //- Return edges
        inline const labelListList& pointEdges() const;

        //- Find connected regions. Set region number per edge.
        //  Returns number of regions.
        label regions(labelList& edgeRegion) const;


    // Edit

        //- Clear all storage
        virtual void clear();

        //- Reset primitive data (points, edges)
        //  Note, optimized to avoid overwriting data (with Xfer::null)
        virtual void reset
        (
            const Xfer<pointField>& points,
            const Xfer<edgeList>& edges
        );

        //- Scale points. A non-positive factor is ignored
        virtual void scalePoints(const scalar);

        //- Merge common points (points within mergeDist)
        void mergePoints(const scalar mergeDist);


    // Write

        void writeStats(Ostream&) const;

        //- Generic write routine. Chooses writer based on extension.
        virtual void write(const fileName& name) const
        {
            write(name, *this);
        }


    // Member Operators

        inline void operator=(const edgeMesh&);

        // Ostream Operator

            friend Ostream& operator<<(Ostream&, const edgeMesh&);
            friend Istream& operator>>(Istream&, edgeMesh&);

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "edgeMeshI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
