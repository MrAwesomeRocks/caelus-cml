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
    CML::linearUpwindMCV

Description
    A TVD property staisfying linear upwind scheme based on the
    monotonized central limiter to prevent unwanted oscillations. 
    Implementation suitable for vector fields.

SourceFiles
    linearUpwindMCV.cpp

References

    [1] "Towards the ultimate conservative difference scheme III. 
         Upstream-centered finite-difference schemes for ideal 
         compressible flow", B. Van Leer, J. Comp. Phys. 23 (3), 
         pp. 263–275, 1977

Author
    Aleksandar Jemcov

\*---------------------------------------------------------------------------*/

#ifndef linearUpwindMCV_H
#define linearUpwindMCV_H

#include "upwind.hpp"
#include "gaussGrad.hpp"

namespace CML
{

template<class Type>
class linearUpwindMCV
:
    public upwind<Type>
{
    // Private Data

        word gradSchemeName_;
        tmp<fv::gradScheme<Type> > gradScheme_;


    // Private Member Functions

        //- Disallow default bitwise copy construct
        linearUpwindMCV(const linearUpwindMCV&);

        //- Disallow default bitwise assignment
        void operator=(const linearUpwindMCV&);


public:

    //- Runtime type information
    TypeName("linearUpwindMCV");


    // Constructors

        //- Construct from faceFlux
        linearUpwindMCV
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
        linearUpwindMCV
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
        {
            if (!schemeData.eof())
            {
                IOWarningIn
                (
                    "linearUpwindMCV(const fvMesh&, Istream&)",
                    schemeData
                )   << "unexpected additional entries in stream." << nl
                    << "    Only the name of the gradient scheme in the"
                       " 'gradSchemes' dictionary should be specified."
                    << endl;
            }
        }

        //- Construct from faceFlux and Istream
        linearUpwindMCV
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
        {
            if (!schemeData.eof())
            {
                IOWarningIn
                (
                    "linearUpwindMCV(const fvMesh&, "
                    "const surfaceScalarField& faceFlux, Istream&)",
                    schemeData
                )   << "unexpected additional entries in stream." << nl
                    << "    Only the name of the gradient scheme in the"
                       " 'gradSchemes' dictionary should be specified."
                    << endl;
            }
        }


    // Member Functions

        //- Return true if this scheme uses an explicit correction
        virtual bool corrected() const
        {
            return true;
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

