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
    executeIfObjectExistsFunctionObject

Description
    This proxy-functionObject only executs its childrens if a certain
    object exists (or does not exist) in the registry

SourceFiles
    executeIfObjectExistsFunctionObject.cpp

Contributors/Copyright:
    2011, 2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef executeIfObjectExistsFunctionObject_H
#define executeIfObjectExistsFunctionObject_H

#include "conditionalFunctionObjectListProxy.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class executeIfObjectExistsFunctionObject Declaration
\*---------------------------------------------------------------------------*/

class executeIfObjectExistsFunctionObject
:
    public conditionalFunctionObjectListProxy
{
    // Private Member Functions

    //- Disallow default bitwise copy construct
    executeIfObjectExistsFunctionObject(const executeIfObjectExistsFunctionObject&);
    
    //- Disallow default bitwise assignment
    void operator=(const executeIfObjectExistsFunctionObject&);

    //- name of the object to look for 
    word objectName_;

    //- should the type be checked or ist the name sufficient
    bool checkType_;

    //- type of the object
    word objectType_;

    //- should the object exist or not
    bool objectShouldExist_;

    void readParameters(const dictionary &);
protected:
    
    //- the actual conition that determines whether the FO should be executed
    virtual bool condition();

public:

    //- Runtime type information
    TypeName("executeIfObjectExists");


    // Constructors

    //- Construct from components
    executeIfObjectExistsFunctionObject
    (
        const word&,
        const Time&,
        const dictionary&
    );

    bool read(const dictionary& dict);

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
