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
    CML::snapParameters

Description
    Simple container to keep together snap specific information.

SourceFiles
    snapParameters.C

\*---------------------------------------------------------------------------*/

#ifndef snapParameters_H
#define snapParameters_H

#include "dictionary.hpp"
#include "scalar.hpp"
#include "Switch.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Class forward declarations

/*---------------------------------------------------------------------------*\
                           Class snapParameters Declaration
\*---------------------------------------------------------------------------*/

class snapParameters
{
    // Private data

        const label nSmoothPatch_;

        const scalar snapTol_;

        const label nSmoothDispl_;

        const label nSnap_;

        const label nFeatureSnap_;

        const Switch explicitFeatureSnap_;

        const Switch implicitFeatureSnap_;

        const Switch multiRegionFeatureSnap_;


    // Private Member Functions

        //- Disallow default bitwise copy construct
        snapParameters(const snapParameters&);

        //- Disallow default bitwise assignment
        void operator=(const snapParameters&);


public:

    // Constructors

        //- Construct from dictionary
        snapParameters(const dictionary& dict);


    // Member Functions

        // Access

            //- Number of patch smoothing iterations before finding
            //  correspondence to surface
            label nSmoothPatch() const
            {
                return nSmoothPatch_;
            }

            //- Relative distance for points to be attracted by surface
            //  feature point
            //  or edge. True distance is this factor times local
            //  maximum edge length.
            scalar snapTol() const
            {
                return snapTol_;
            }

            //- Number of mesh displacement smoothing iterations.
            label nSmoothDispl() const
            {
                return nSmoothDispl_;
            }

            //- Maximum number of snapping relaxation iterations. Should stop
            //  before upon reaching a correct mesh.
            label nSnap() const
            {
                return nSnap_;
            }

            label nFeatureSnap() const
            {
                return nFeatureSnap_;
            }

            Switch explicitFeatureSnap() const
            {
                return explicitFeatureSnap_;
            }

            Switch implicitFeatureSnap() const
            {
                return implicitFeatureSnap_;
            }

            Switch multiRegionFeatureSnap() const
            {
                return multiRegionFeatureSnap_;
            }

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
