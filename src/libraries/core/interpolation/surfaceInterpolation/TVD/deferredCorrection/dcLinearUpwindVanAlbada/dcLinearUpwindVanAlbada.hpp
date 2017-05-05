/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
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
    CML::dcLinearUpwindVanAlbada

Description
    A TVD property staisfying dcLinear upwind scheme based on the VanAlbada 
    limiter to prevent unwanted oscillations. Implementation suitable for
    scalar fields.

SourceFiles
    dcLinearUpwindVanAlbada.cpp

References

    [1] "A comparative study of computational methods in cosmic gas dynamics",
         G.D., Van Albada, Astronomy and Astrophysics 108, pp. 76–84

Author
    Aleksandar Jemcov

\*---------------------------------------------------------------------------*/

#ifndef dcLinearUpwindVanAlbada_H
#define dcLinearUpwindVanAlbada_H

#include "upwind.hpp"
#include "gaussGrad.hpp"

namespace CML
{

template<class Type>
class dcLinearUpwindVanAlbada
:
    public upwind<Type>
{
    // Private Data

        word gradSchemeName_;
        tmp<fv::gradScheme<Type> > gradScheme_;

    // Private Member Functions

        //- Disallow default bitwise copy construct
        dcLinearUpwindVanAlbada(const dcLinearUpwindVanAlbada&);

        //- Disallow default bitwise assignment
        void operator=(const dcLinearUpwindVanAlbada&);


public:

    //- Runtime type information
    TypeName("dcLinearUpwindVanAlbada");


    // Constructors

        //- Construct from faceFlux
        dcLinearUpwindVanAlbada
        (
            const fvMesh& mesh,
            const surfaceScalarField& faceFlux
        )
        :
            upwind<Type>(mesh, faceFlux),
            gradSchemeName_("grad"),
            gradScheme_
            (
                new fv::gaussGrad<Type>(mesh)
            )
        {}

        //- Construct from Istream.
        //  The name of the flux field is read from the Istream and looked-up
        //  from the mesh objectRegistry
        dcLinearUpwindVanAlbada
        (
            const fvMesh& mesh,
            Istream& schemeData
        )
        :
            upwind<Type>(mesh, schemeData),
            gradSchemeName_(schemeData),
            gradScheme_
            (
                fv::gradScheme<Type>::New
                (
                    mesh,
                    mesh.gradScheme(gradSchemeName_)
                )
            )
        {}

        //- Construct from faceFlux and Istream
        dcLinearUpwindVanAlbada
        (
            const fvMesh& mesh,
            const surfaceScalarField& faceFlux,
            Istream& schemeData
        )
        :
            upwind<Type>(mesh, faceFlux, schemeData),
            gradSchemeName_(schemeData),
            gradScheme_
            (
                fv::gradScheme<Type>::New
                (
                    mesh,
                    mesh.gradScheme(gradSchemeName_)
                )
            )
        {}

    // Member Functions

        //- Return false for deferred correction
        virtual bool corrected() const
        {
            return false;
        }

        //- Return the explicit correction to the face-interpolate
        virtual tmp<GeometricField<Type, fvsPatchField, surfaceMesh> >
        correction
        (
            const GeometricField<Type, fvPatchField, volMesh>&
        ) const;

};

}

#endif

