/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
Copyright (C) 2011-2016 OpenFOAM Foundation
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
    CML::SphericalTensor

Description
    Templated 3D SphericalTensor derived from VectorSpace adding construction
    from 1 component, element access using th ii() member function and the
    inner-product (dot-product) and outer-product operators.

SourceFiles
    SphericalTensorI.hpp

\*---------------------------------------------------------------------------*/

#ifndef SphericalTensor__H
#define SphericalTensor__H

#include "VectorSpace.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class SphericalTensor Declaration
\*---------------------------------------------------------------------------*/

template<class Cmpt>
class SphericalTensor
:
    public VectorSpace<SphericalTensor<Cmpt>, Cmpt, 1>
{

public:

    //- Equivalent type of labels used for valid component indexing
    typedef SphericalTensor<label> labelType;


    // Member constants

        //- Rank of SphericalTensor is 2
        static const direction rank = 2;


    // Static data members

        static const SphericalTensor I;
        static const SphericalTensor oneThirdI;
        static const SphericalTensor twoThirdsI;


    //- Component labeling enumeration
    enum components { II };


    // Constructors

        //- Construct null
        inline SphericalTensor();

        //- Construct initialized to zero
        inline SphericalTensor(const CML::zero);

        //- Construct given VectorSpace
        template<class Cmpt2>
        inline SphericalTensor
        (
            const VectorSpace<SphericalTensor<Cmpt2>, Cmpt2, 1>&
        );

        //- Construct given the component
        inline SphericalTensor(const Cmpt& tii);

        //- Construct from Istream
        inline SphericalTensor(Istream&);


    // Member Functions

        // Access

            inline const Cmpt& ii() const;
            inline Cmpt& ii();


        //- Transpose
        inline const SphericalTensor<Cmpt>& T() const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Include inline implementations
#include "SphericalTensorI.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
