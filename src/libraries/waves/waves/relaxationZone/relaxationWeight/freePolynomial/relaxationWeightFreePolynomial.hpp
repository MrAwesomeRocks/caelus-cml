/*---------------------------------------------------------------------------*\
Copyright Niels Gjøl Jacobsen, Technical University of Denmark.
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

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
    CML::relaxationWeights::relaxationWeightFreePolynomial

Description
    An object used to create a weighting function in the relaxation zone, which
    has a polynomial behaviour with a user defined exponent. See eq. 6.17 in

    A. P. Engsig-Karup
    Unstructured Nodal DG-FEM Solution of High-Order Boussinesq-Type Equations
    PhD Thesis
    Technical University of Denmark

SourceFiles
    relaxationWeightFreePolynomial.cpp

Author
    Niels Gjøl Jacobsen, Technical University of Denmark.

\*---------------------------------------------------------------------------*/

#ifndef relaxationWeightFreePolynomial_HPP
#define relaxationWeightFreePolynomial_HPP

#include "relaxationWeight.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace relaxationWeights
{

/*---------------------------------------------------------------------------*\
                           Class relaxationShapeRectangular Declaration
\*---------------------------------------------------------------------------*/

class relaxationWeightFreePolynomial
:
    public relaxationWeight
{

private:

    // Private data
        scalar exponent_;

protected:

    // Protected Member functions
        virtual void computeWeights
        (
            const labelList&,
            const scalarField&,
            scalarField&
        );

public:

    //- Runtime type information
    TypeName("relaxationWeightFreePolynomial");

    // Constructors

        //- from components
        relaxationWeightFreePolynomial
        (
            const word& ,
            const fvMesh& mesh_
        );


    // Destructor

        ~relaxationWeightFreePolynomial()
        {}

    // Member Functions

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace relaxationWeights
} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
