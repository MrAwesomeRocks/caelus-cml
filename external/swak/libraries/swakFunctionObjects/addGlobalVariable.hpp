/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright  held by original author
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
    addGlobalVariable

Description
    Function object that adds a global variable

SourceFiles
    addGlobalVariable.cpp

Contributors/Copyright:
    2011, 2013-2014 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>
    2013 Bruno Santos <wyldckat@gmail.com>

\*---------------------------------------------------------------------------*/

#ifndef addGlobalVariable_H
#define addGlobalVariable_H

#include "primitiveFieldsFwd.hpp"
#include "pointFieldFwd.hpp"

#include "swakVersion.hpp"
#include "DebugOStream.hpp"

namespace CML
{
// Forward declaration of classes
class dictionary;
class objectRegistry;
class mapPolyMesh;
class polyMesh;

class addGlobalVariable
{
    //- Disallow default bitwise copy construct
    addGlobalVariable(const addGlobalVariable&);

    //- Disallow default bitwise assignment
    void operator=(const addGlobalVariable&);

    const objectRegistry& obr_;

public:

    //- Runtime type information
    TypeName("addGlobalVariable");


    // Constructors

    //- Construct for given objectRegistry and dictionary.
    //  Allow the possibility to load fields from files
    addGlobalVariable
    (
        const word& name,
        const objectRegistry&,
        const dictionary&,
        const bool loadFromFiles = false
    );


    //- Destructor
    virtual ~addGlobalVariable();


    // Member Functions

    //- Called when time was set at the end of the Time::operator++ 
    virtual void timeSet();         

    //- Read the forces data
    virtual void read(const dictionary&);

    //- Execute, currently does nothing
    virtual void execute();

    //- Execute at the final time-loop, currently does nothing
    virtual void end();

    //- Write the forces
    virtual void write();

    //- Update for changes of mesh
    virtual void updateMesh(const mapPolyMesh&)
        {}

    //- Update for changes of mesh
    virtual void movePoints(const pointField&)
        {}

    //- Clear all data
    void clearData();
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif
