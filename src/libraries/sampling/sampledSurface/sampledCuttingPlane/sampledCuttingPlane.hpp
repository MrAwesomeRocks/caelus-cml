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
    CML::sampledCuttingPlane

Description
    A sampledSurface defined by a plane

SourceFiles
    sampledCuttingPlane.cpp

\*---------------------------------------------------------------------------*/

#ifndef sampledCuttingPlane_H
#define sampledCuttingPlane_H

#include "sampledSurface.hpp"
#include "isoSurface.hpp"
//#include "isoSurfaceCell.hpp"
#include "plane.hpp"
#include "ZoneIDs.hpp"
#include "fvMeshSubset.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                       Class sampledCuttingPlane Declaration
\*---------------------------------------------------------------------------*/

class sampledCuttingPlane
:
    public sampledSurface
{
    // Private data

        //- Plane
        const plane plane_;

        //- Merge tolerance
        const scalar mergeTol_;

        //- Whether to coarsen
        const Switch regularise_;

        //- Whether to recalculate cell values as average of point values
        const Switch average_;

        //- zone name/index (if restricted to zones)
        mutable cellZoneID zoneID_;

        //- for zones: patch to put exposed faces into
        mutable word exposedPatchName_;

        //- Track if the surface needs an update
        mutable bool needsUpdate_;


        //- Optional subsetted mesh
        autoPtr<fvMeshSubset> subMeshPtr_;

        //- Distance to cell centres
        autoPtr<volScalarField> cellDistancePtr_;

        //- Distance to points
        scalarField pointDistance_;

        //- Constructed iso surface
        //autoPtr<isoSurfaceCell> isoSurfPtr_;
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
    TypeName("sampledCuttingPlane");


    // Constructors

        //- Construct from dictionary
        sampledCuttingPlane
        (
            const word& name,
            const polyMesh& mesh,
            const dictionary& dict
        );


    //- Destructor
    virtual ~sampledCuttingPlane();


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


        //const isoSurfaceCell& surface() const
        const isoSurface& surface() const
        {
            return isoSurfPtr_();
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

#include "volPointInterpolation.hpp"
#include "sampledCuttingPlane.hpp"
#include "volFieldsFwd.hpp"
#include "pointFields.hpp"
#include "volPointInterpolation.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class Type>
CML::tmp<CML::Field<Type> >
CML::sampledCuttingPlane::sampleField
(
    const GeometricField<Type, fvPatchField, volMesh>& vField
) const
{
    return tmp<Field<Type> >(new Field<Type>(vField, surface().meshCells()));
}


template<class Type>
CML::tmp<CML::Field<Type> >
CML::sampledCuttingPlane::interpolateField
(
    const interpolation<Type>& interpolator
) const
{
    // Get fields to sample. Assume volPointInterpolation!
    const GeometricField<Type, fvPatchField, volMesh>& volFld =
        interpolator.psi();

    if (subMeshPtr_.valid())
    {
        tmp<GeometricField<Type, fvPatchField, volMesh> > tvolSubFld =
            subMeshPtr_().interpolate(volFld);

        const GeometricField<Type, fvPatchField, volMesh>& volSubFld =
            tvolSubFld();

        tmp<GeometricField<Type, pointPatchField, pointMesh> > tpointSubFld =
            volPointInterpolation::New(volSubFld.mesh()).interpolate(volSubFld);

        // Sample.
        return surface().interpolate
        (
            (
                average_
              ? pointAverage(tpointSubFld())()
              : volSubFld
            ),
            tpointSubFld()
        );
    }
    else
    {
        tmp<GeometricField<Type, pointPatchField, pointMesh> > tpointFld
        (
            volPointInterpolation::New(volFld.mesh()).interpolate(volFld)
        );

        // Sample.
        return surface().interpolate
        (
            (
                average_
              ? pointAverage(tpointFld())()
              : volFld
            ),
            tpointFld()
        );
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
