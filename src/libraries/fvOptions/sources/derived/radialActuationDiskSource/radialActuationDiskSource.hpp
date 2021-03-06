/*---------------------------------------------------------------------------*\
Copyright (C) 2011 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Caelus is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

Class
    CML::fv::radialActuationDiskSource

Description
    Actuation disk source including radial thrust

    Constant values for momentum source for actuation disk
    \f[
        T = 2 \rho A U_{o}^2 a (1-a)
    \f]
    and
    \f[
        U_1 = (1 - a)U_{o}
    \f]

    where:
    \vartable
        A   | disk area
        U_o | upstream velocity
        a   | 1 - Cp/Ct
        U_1 | velocity at the disk
    \endvartable


    The thrust is distributed by a radial function:
    \f[
        thrust(r) = T (C_0 + C_1 r^2 + C_2 r^4)
    \f]

    \heading Source usage

    Example usage:
    \verbatim
    actuationDiskSourceCoeffs
    {
        fieldName       U;          // name of field to apply source
        diskDir         (-1 0 0);   // disk direction
        Cp              0.1;        // power coefficient
        Ct              0.5;        // thrust coefficient
        diskArea        5.0;        // disk area
        coeffs          (0.1 0.5 0.01); // radial distribution coefficients
        upstreamPoint   (0 0 0);    // upstream point
    }
    \endverbatim


SourceFiles
    radialActuationDiskSource.cpp

\*---------------------------------------------------------------------------*/

#ifndef radialActuationDiskSource_H
#define radialActuationDiskSource_H

#include "actuationDiskSource.hpp"
#include "FixedList.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace fv
{

/*---------------------------------------------------------------------------*\
                  Class radialActuationDiskSource Declaration
\*---------------------------------------------------------------------------*/

class radialActuationDiskSource
:
    public actuationDiskSource
{
    // Private data

        //- Coeffcients for the radial distribution
        FixedList<scalar, 3> radialCoeffs_;


    // Private Member Functions

        //- Add resistance to the UEqn
        template<class RhoFieldType>
        void addRadialActuationDiskAxialInertialResistance
        (
            vectorField& Usource,
            const labelList& cells,
            const scalarField& V,
            const RhoFieldType& rho,
            const vectorField& U
        ) const;

        //- Disallow default bitwise copy construct
        radialActuationDiskSource(const radialActuationDiskSource&);

        //- Disallow default bitwise assignment
        void operator=(const radialActuationDiskSource&);


public:

    //- Runtime type information
    TypeName("radialActuationDiskSource");


    // Constructors

        //- Construct from components
        radialActuationDiskSource
        (
            const word& name,
            const word& modelType,
            const dictionary& dict,
            const fvMesh& mesh
        );


    //- Destructor
    virtual ~radialActuationDiskSource()
    {}


    // Member Functions

        //- Source term to momentum equation
        virtual void addSup
        (
            fvMatrix<vector>& eqn,
            const label fieldI
        );

        //- Source term to compressible momentum equation
        virtual void addSup
        (
            const volScalarField& rho,
            fvMatrix<vector>& eqn,
            const label fieldI
        );


    // I-O

        //- Write data
        virtual void writeData(Ostream&) const;

        //- Read dictionary
        virtual bool read(const dictionary& dict);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace fv
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "volFields.hpp"
#include "fvMatrix.hpp"
#include "fvm.hpp"
#include "mathematicalConstants.hpp"

// * * * * * * * * * * * * * * *  Member Functions * * * * * * * * * * * * * //

template<class RhoFieldType>
void CML::fv::radialActuationDiskSource::
addRadialActuationDiskAxialInertialResistance
(
    vectorField& Usource,
    const labelList& cells,
    const scalarField& Vcells,
    const RhoFieldType& rho,
    const vectorField& U
) const
{
    scalar a = 1.0 - Cp_/Ct_;
    scalarField Tr(cells.size());
    const vector uniDiskDir = diskDir_/mag(diskDir_);

    tensor E(tensor::zero);
    E.xx() = uniDiskDir.x();
    E.yy() = uniDiskDir.y();
    E.zz() = uniDiskDir.z();

    const Field<vector> zoneCellCentres(mesh().cellCentres(), cells);
    const Field<scalar> zoneCellVolumes(mesh().cellVolumes(), cells);

    const vector avgCentre = gSum(zoneCellVolumes*zoneCellCentres)/V();
    const scalar maxR = gMax(mag(zoneCellCentres - avgCentre));

    scalar intCoeffs =
        radialCoeffs_[0]
      + radialCoeffs_[1]*sqr(maxR)/2.0
      + radialCoeffs_[2]*pow4(maxR)/3.0;

    vector upU = vector(VGREAT, VGREAT, VGREAT);
    scalar upRho = VGREAT;
    if (upstreamCellId_ != -1)
    {
        upU =  U[upstreamCellId_];
        upRho = rho[upstreamCellId_];
    }
    reduce(upU, minOp<vector>());
    reduce(upRho, minOp<scalar>());

    scalar T = 2.0*upRho*diskArea_*mag(upU)*a*(1.0 - a);
    forAll(cells, i)
    {
        scalar r2 = magSqr(mesh().cellCentres()[cells[i]] - avgCentre);

        Tr[i] =
            T
           *(radialCoeffs_[0] + radialCoeffs_[1]*r2 + radialCoeffs_[2]*sqr(r2))
           /intCoeffs;

        Usource[cells[i]] += ((Vcells[cells[i]]/V_)*Tr[i]*E) & upU;
    }

    if (debug)
    {
        Info<< "Source name: " << name() << nl
            << "Average centre: " << avgCentre << nl
            << "Maximum radius: " << maxR << endl;
    }
}



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
