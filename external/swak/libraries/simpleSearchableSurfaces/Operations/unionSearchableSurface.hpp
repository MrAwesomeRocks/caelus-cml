/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright held by original author
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
    CML::unionSearchableSurface

Description
    Searching on rotated surface

SourceFiles
    unionSearchableSurface.cpp

Contributors/Copyright:
    2009, 2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#ifndef unionSearchableSurface_H
#define unionSearchableSurface_H

#include "binaryOperationSearchableSurface.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes

/*---------------------------------------------------------------------------*\
                           Class unionSearchableSurface Declaration
\*---------------------------------------------------------------------------*/

class unionSearchableSurface
:
    public binaryOperationSearchableSurface
{
private:
        //- Disallow default bitwise copy construct
        unionSearchableSurface(const unionSearchableSurface&);

        //- Disallow default bitwise assignment
        void operator=(const unionSearchableSurface&);

protected:

    virtual bool decidePoint(
        const hitWhom who,
        const bool inA,
        const bool inB
    ) const;

    //- Check whether the normal for a point on surface A should be reverted
    bool revertNormalA(const pointIndexHit&) const
        { return false; }

    //- Check whether the normal for a point on surface B should be reverted
    bool revertNormalB(const pointIndexHit&) const
        { return false; }

public:

    //- Runtime type information
    TypeName("union");


    // Constructors

        //- Construct from dictionary (used by transformationSearchableSurface)
        unionSearchableSurface
        (
            const IOobject& io,
            const dictionary& dict
        );

    // Destructor

        virtual ~unionSearchableSurface();

    void getVolumeType
    (
        const pointField& points,
        List<volumeType>& volType
    ) const;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
