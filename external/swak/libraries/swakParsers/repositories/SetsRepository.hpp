/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright (C) 1991-2008 OpenCFD Ltd.
-------------------------------------------------------------------------------
License
    This file is based on CAELUS.

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
    CML::SetsRepository

Description

SourceFiles
    SetsRepositoryI.hpp
    SetsRepository.cpp
    SetsRepositoryIO.cpp

Contributors/Copyright:
    2012-2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef SetsRepository_H
#define SetsRepository_H

#include "PtrMap.hpp"
#include "sampledSet.hpp"
#include "RepositoryBase.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class SetsRepository Declaration
\*---------------------------------------------------------------------------*/

class SetsRepository
:
    public RepositoryBase
{
    // Private data

    //- the actual sets
    HashPtrTable<sampledSet,word> sets_;

    //- names of the formats to be written
    HashTable<word,word> formatNames_;

    HashPtrTable<meshSearch,word> meshSearches_;

    // Constructors

    //- Construct null
    SetsRepository(const IOobject &);

    //- The only instance of this repository
    static SetsRepository *repositoryInstance;

    //- get a meshSearch using the mesh (construct new if missing)
    meshSearch &getSearch(
        const polyMesh &mesh
    );

public:
     //- Runtime type information
    TypeName("SetsRepository");

    //- Destructor

    virtual ~SetsRepository();

    //- Get the Singleton of this class
    static SetsRepository &getRepository(const objectRegistry &mesh);

    //- get a set using a dictionary (set will be constructed if not existing)
    sampledSet &getSet(
        const dictionary &dict,
        const polyMesh &mesh
    );

    //- get a set using its name (set must exist)
    sampledSet &getSet(
        const word &name,
        const polyMesh &mesh
    );

    //- Make sure that this class is not abstract. Write sets
    bool writeData(Ostream &) const;

    //- update repo at the start of a time-step
    void updateRepo();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
