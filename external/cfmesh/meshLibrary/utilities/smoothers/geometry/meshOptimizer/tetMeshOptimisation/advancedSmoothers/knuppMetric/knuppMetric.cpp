/*---------------------------------------------------------------------------*\
Copyright (C) Creative Fields, Ltd.
-------------------------------------------------------------------------------
License
    This file is part of cfMesh.

    cfMesh is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 3 of the License, or (at your
    option) any later version.

    cfMesh is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with cfMesh.  If not, see <http://www.gnu.org/licenses/>.

Author: Franjo Juretic (franjo.juretic@c-fields.com)

\*---------------------------------------------------------------------------*/

#include "demandDrivenData.hpp"
#include "knuppMetric.hpp"
#include "partTetMeshSimplex.hpp"

//#define DEBUGSmooth

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
    
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
// Private member functions
    
scalar knuppMetric::evaluateMetric() const
{
    scalar val(0.0);
    
    forAll(normals_, nI)
    {
        const scalar fx = (normals_[nI] & (p_ - centres_[nI])) - beta_;
        val += CML::sqr(mag(fx) - fx);
    }
    
    return val;
}

scalar knuppMetric::evaluateMetricNoBeta() const
{
    scalar val(0.0);
    
    forAll(normals_, nI)
    {
        const scalar fx = (normals_[nI] & (p_ - centres_[nI]));
        val += CML::sqr(mag(fx) - fx);
    }
    
    return val;
}
    
void knuppMetric::evaluateGradients(vector& grad, tensor& gradGrad) const
{
    grad = vector::zero;
    gradGrad = tensor::zero;
    
    forAll(normals_, nI)
    {
        const scalar fx = (normals_[nI] & (p_ - centres_[nI])) - beta_;
        const vector gfx = (CML::sign(fx) - 1) * normals_[nI];
        
        grad += (CML::mag(fx) - fx) * gfx;
        gradGrad += gfx * gfx;
    }
}
    
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    
knuppMetric::knuppMetric(partTetMeshSimplex& simplex)
:
    simplexSmoother(simplex),
    p_(simplex.pts()[simplex.tets()[0][3]]),
    normals_(),
    centres_(),
    beta_()
{
    forAll(tets_, tetI)
    {
    const partTet& pt = tets_[tetI];
    const triangle<point, point> tri
    (
        points_[pt.a()],
        points_[pt.b()],
        points_[pt.c()]
    );
        
    const vector n = tri.normal();
        const scalar d = mag(n);
        
        if( d > VSMALL )
        {
            centres_.append(tri.centre());
            normals_.append(n/d);
        }
    }
    
    beta_ = 0.01 * bb_.mag();
}
            

knuppMetric::~knuppMetric()
{
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Implementation of knupp metric untangling
void knuppMetric::optimizeNodePosition(const scalar /*tolObsolete*/)
{
    if( !bb_.contains(p_) )
        p_ = 0.5 * (bb_.min() + bb_.max());

    const scalar tol = CML::sqr(2.0 * SMALL) * magSqr(bb_.min() - bb_.max());

    label iterI, outerIter(0);

    vector gradF, disp;
    tensor gradGradF;

    scalar func, lastFunc;
    
    # ifdef DEBUGSmooth
    forAll(normals_, nI)
    {
        const scalar fx = normals_[nI] & (p_ - centres_[nI]);
        Info << "Tet " << nI << " has distance " << fx << " func "
            << CML::sqr(mag(fx) - fx) << endl;
    }
    Info << "BoundBox size " << (bb_.max() - bb_.min()) << endl;
    Info << "Tolerance " << tol << endl;
    # endif
    
    bool finished;
    do
    {
        finished = true;
    
        lastFunc = evaluateMetric();
        
        iterI = 0;
        do
        {
            # ifdef DEBUGSmooth
            Info << "Iteration " << iterI << endl;
            Info << "Initial metric value " << lastFunc << endl;
            # endif
            
            //- store previous value
            const point pOrig = p_;
            
            //- evaluate gradients
            evaluateGradients(gradF, gradGradF);
            
            //- calculate displacement
            const scalar determinant = det(gradGradF);
            if( determinant > SMALL )
            {
                disp = (inv(gradGradF, determinant) & gradF);
                
                for(direction i=0;i<vector::nComponents;++i)
                {
                    const scalar& val = disp[i];
                    if( (val != val) || ((val - val) != (val - val)) )
                    {
                        disp = vector::zero;
                        break;
                    }
                }
                
                p_ -= disp;
                
                func = evaluateMetric();
            
                # ifdef DEBUGSmooth
                Info << "Second grad " << gradGradF << endl;
        Info << "inv(gradGradF, determinant) " << inv(gradGradF, determinant) << endl;
                Info << "Gradient " << gradF << endl;
                Info << "Determinant " << determinant << endl;
                Info << "Displacement " << disp << endl;
                Info << "New metric value " << func << endl;
                # endif
                

        scalar relax(0.8);
        label nLoops(0);
        while( func > lastFunc )
        {
            p_ = pOrig - relax * disp;
            relax *= 0.5;
            func = evaluateMetric();
          
            if( func < lastFunc )
            continue;
            
            //- it seems that this direction is wrong
            if( ++nLoops == 5 )
            {
            p_ = pOrig;
            disp = vector::zero;
            func = 0.0;
            }
        }
                
                lastFunc = func;
            }
            else
            {
                disp = vector::zero;
            }
        } while( (magSqr(disp) > tol) && (++iterI < 10) );
        
        if( (lastFunc < VSMALL) && (evaluateMetricNoBeta() > VSMALL) )
        {
            beta_ /= 2.0;
            finished = false;
        }
        
    } while( !finished && (++outerIter < 5) );
    
    # ifdef DEBUGSmooth
    Info << "Last value " << lastFunc << endl;
    Info << "Beta " << beta_ << endl;
    Info << "Metric with no beta " << evaluateMetricNoBeta() << endl;
    forAll(normals_, nI)
    {
        const scalar fx = normals_[nI] & (p_ - centres_[nI]);
        Info << "Tet " << nI << " has distance " << fx << " func "
            << CML::sqr(mag(fx) - fx) << endl;
    }
    //::exit(1);
    # endif
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //