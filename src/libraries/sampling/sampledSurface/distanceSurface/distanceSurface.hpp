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
    CML::distanceSurface

Description
    A sampledSurface defined by a distance to a surface.

    Uses either isoSurfaceCell or isoSurface.

SourceFiles
    distanceSurface.cpp

\*---------------------------------------------------------------------------*/

#ifndef distanceSurface_H
#define distanceSurface_H

#include "sampledSurface.hpp"
#include "searchableSurface.hpp"
#include "isoSurfaceCell.hpp"
#include "isoSurface.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                       Class distanceSurface Declaration
\*---------------------------------------------------------------------------*/

class distanceSurface
:
    public sampledSurface
{
    // Private data

        //- Surface
        const autoPtr<searchableSurface> surfPtr_;

        //- distance value
        const scalar distance_;

        //- signed distance
        const bool signed_;

        //- Whether to use isoSurfaceCell or isoSurface
        const bool cell_;

        //- Whether to coarsen
        const Switch regularise_;

        //- Whether to recalculate cell values as average of point values
        const Switch average_;

        //- If restricted to zones, name of this zone or a regular expression
        keyType zoneKey_;

        //- Track if the surface needs an update
        mutable bool needsUpdate_;


        //- Distance to cell centres
        autoPtr<volScalarField> cellDistancePtr_;

        //- Distance to points
        scalarField pointDistance_;

        //- Constructed iso surface
        autoPtr<isoSurfaceCell> isoSurfCellPtr_;

        //- Constructed iso surface
        autoPtr<isoSurface> isoSurfPtr_;

        //- triangles converted to faceList
        mutable autoPtr<faceList> facesPtr_;


    // Private Member Functions

        //- Create iso surface
        void createGeometry();

        //- sample field on faces
        template<class Type>
        tmp<Field<Type> > sampleField
        (
            const GeometricField<Type, fvPatchField, volMesh>& vField
        ) const;


        template<class Type>
        tmp<Field<Type> >
        interpolateField(const interpolation<Type>&) const;


public:

    //- Runtime type information
    TypeName("distanceSurface");


    // Constructors

        //- Construct from dictionary
        distanceSurface
        (
            const word& name,
            const polyMesh& mesh,
            const dictionary& dict
        );

        //- Construct from components
        distanceSurface
        (
            const word& name,
            const polyMesh& mesh,
            const bool interpolate,
            const word& surfaceType,
            const word& surfaceName,
            const scalar distance,
            const bool signedDistance,
            const bool cell,
            const Switch regularise,
            const Switch average
        );


    //- Destructor
    virtual ~distanceSurface();


    // Member Functions

        //- Does the surface need an update?
        virtual bool needsUpdate() const;

        //- Mark the surface as needing an update.
        //  May also free up unneeded data.
        //  Return false if surface was already marked as expired.
        virtual bool expire();

        //- Update the surface as required.
        //  Do nothing (and return false) if no update was needed
        virtual bool update();

        //- Points of surface
        virtual const pointField& points() const
        {
            return surface().points();
        }

        //- Faces of surface
        virtual const faceList& faces() const
        {
            if (facesPtr_.empty())
            {
                const triSurface& s = surface();

                facesPtr_.reset(new faceList(s.size()));

                forAll(s, i)
                {
                    facesPtr_()[i] = s[i].triFaceFace();
                }
            }
            return facesPtr_;
        }

        const triSurface& surface() const
        {
            if (cell_)
            {
                return isoSurfCellPtr_();
            }
            else
            {
                return isoSurfPtr_();
            }
        }

        //- sample field on surface
        virtual tmp<scalarField> sample
        (
            const volScalarField&
        ) const;

        //- sample field on surface
        virtual tmp<vectorField> sample
        (
            const volVectorField&
        ) const;

        //- sample field on surface
        virtual tmp<sphericalTensorField> sample
        (
            const volSphericalTensorField&
        ) const;

        //- sample field on surface
        virtual tmp<symmTensorField> sample
        (
            const volSymmTensorField&
        ) const;

        //- sample field on surface
        virtual tmp<tensorField> sample
        (
            const volTensorField&
        ) const;


        //- interpolate field on surface
        virtual tmp<scalarField> interpolate
        (
            const interpolation<scalar>&
        ) const;

        //- interpolate field on surface
        virtual tmp<vectorField> interpolate
        (
            const interpolation<vector>&
        ) const;

        //- interpolate field on surface
        virtual tmp<sphericalTensorField> interpolate
        (
            const interpolation<sphericalTensor>&
        ) const;

        //- interpolate field on surface
        virtual tmp<symmTensorField> interpolate
        (
            const interpolation<symmTensor>&
        ) const;

        //- interpolate field on surface
        virtual tmp<tensorField> interpolate
        (
            const interpolation<tensor>&
        ) const;

        //- Write
        virtual void print(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "volFieldsFwd.hpp"
#include "pointFields.hpp"
#include "volPointInterpolation.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class Type>
CML::tmp<CML::Field<Type> >
CML::distanceSurface::sampleField
(
    const GeometricField<Type, fvPatchField, volMesh>& vField
) const
{
    if (cell_)
    {
        return tmp<Field<Type> >
        (
            new Field<Type>(vField, isoSurfCellPtr_().meshCells())
        );
    }
    else
    {
        return tmp<Field<Type> >
        (
            new Field<Type>(vField, isoSurfPtr_().meshCells())
        );
    }
}


template<class Type>
CML::tmp<CML::Field<Type> >
CML::distanceSurface::interpolateField
(
    const interpolation<Type>& interpolator
) const
{
    const fvMesh& fvm = static_cast<const fvMesh&>(mesh());

    // Get fields to sample. Assume volPointInterpolation!
    const GeometricField<Type, fvPatchField, volMesh>& volFld =
        interpolator.psi();

    tmp<GeometricField<Type, pointPatchField, pointMesh> > pointFld
    (
        volPointInterpolation::New(fvm).interpolate(volFld)
    );

    // Sample.
    if (cell_)
    {
        return isoSurfCellPtr_().interpolate
        (
            (
                average_
              ? pointAverage(pointFld())()
              : volFld
            ),
            pointFld()
        );
    }
    else
    {
        return isoSurfPtr_().interpolate
        (
            (
                average_
              ? pointAverage(pointFld())()
              : volFld
            ),
            pointFld()
        );
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
