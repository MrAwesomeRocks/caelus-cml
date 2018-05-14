/*---------------------------------------------------------------------------*\
Copyright (C) 2016 OpenFOAM Foundation
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

\*---------------------------------------------------------------------------*/

#include "rigidBodyModel.hpp"
#include "rigidBodyModelState.hpp"
#include "rigidBodyRestraint.hpp"

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void CML::RBD::rigidBodyModel::applyRestraints
(
    scalarField& tau,
    Field<spatialVector>& fx
) const
{
    if (restraints_.empty())
    {
        return;
    }

    forAll(restraints_, ri)
    {
        DebugInfo << "Restraint " << restraints_[ri].name()<<endl;

        // Accumulate the restraint forces
        restraints_[ri].restrain(tau, fx);
    }
}


void CML::RBD::rigidBodyModel::forwardDynamics
(
    rigidBodyModelState& state,
    const scalarField& tau,
    const Field<spatialVector>& fx
) const
{
    const scalarField& q = state.q();
    const scalarField& qDot = state.qDot();
    scalarField& qDdot = state.qDdot();

    DebugIn
    (
        "RBD::rigidBodyModel::forwardDynamics"
        "(rigidBodyModelState&, const scalarField&,"
        " const Field<spatialVector>&) const"
    )
        << "q = " << q << nl
        << "qDot = " << qDot << nl
        << "tau = " << tau << endl;

    // Joint state returned by jcalc
    joint::XSvc J;

    v_[0] = Zero;

    for (label i=1; i<nBodies(); i++)
    {
        const joint& jnt = joints()[i];
        jnt.jcalc(J, q, qDot);

        S_[i] = J.S;
        S1_[i] = J.S1;

        Xlambda_[i] = J.X & XT_[i];

        const label lambdai = lambda_[i];

        if (lambdai != 0)
        {
            X0_[i] = Xlambda_[i] & X0_[lambdai];
        }
        else
        {
            X0_[i] = Xlambda_[i];
        }

        v_[i] = (Xlambda_[i] & v_[lambdai]) + J.v;
        c_[i] = J.c + (v_[i] ^ J.v);
        IA_[i] = I(i);
        pA_[i] = v_[i] ^* (I(i) & v_[i]);

        if (fx.size())
        {
            pA_[i] -= *X0_[i] & fx[i];
        }
    }

    for (label i=nBodies()-1; i>0; i--)
    {
        const joint& jnt = joints()[i];
        const label qi = jnt.qIndex();

        if (jnt.nDoF() == 1)
        {
            U1_[i] = IA_[i] & S1_[i];
            Dinv_[i].xx() = 1/(S1_[i] && U1_[i]);
            u_[i].x() = tau[qi] - (S1_[i] && pA_[i]);

            const label lambdai = lambda_[i];

            if (lambdai != 0)
            {
                const spatialTensor Ia
                (
                    IA_[i] - (U1_[i]*(Dinv_[i].xx()*U1_[i]))
                );

                const spatialVector pa
                (
                    pA_[i] + (Ia & c_[i]) + U1_[i]*(Dinv_[i].xx()*u_[i].x())
                );

                IA_[lambdai] +=
                    spatialTensor(Xlambda_[i].T())
                  & Ia
                  & spatialTensor(Xlambda_[i]);

                pA_[lambdai] += Xlambda_[i].T() & pa;
            }
        }
        else
        {
            U_[i] = IA_[i] & S_[i];
            Dinv_[i] = (S_[i].T() & U_[i]).inv();

            u_[i] = tau.block<vector>(qi) - (S_[i].T() & pA_[i]);

            const label lambdai = lambda_[i];

            if (lambdai != 0)
            {
                spatialTensor Ia
                (
                    IA_[i]
                  - (U_[i] & Dinv_[i] & U_[i].T())
                );

                spatialVector pa
                (
                    pA_[i]
                  + (Ia & c_[i])
                  + (U_[i] & Dinv_[i] & u_[i])
                );

                IA_[lambdai] +=
                    spatialTensor(Xlambda_[i].T())
                  & Ia
                  & spatialTensor(Xlambda_[i]);

                pA_[lambdai] += Xlambda_[i].T() & pa;
            }
        }
    }

    a_[0] = spatialVector(Zero, -g_);

    for (label i=1; i<nBodies(); i++)
    {
        const joint& jnt = joints()[i];
        const label qi = jnt.qIndex();

        a_[i] = (Xlambda_[i] & a_[lambda_[i]]) + c_[i];

        if (jnt.nDoF() == 1)
        {
            qDdot[qi] = Dinv_[i].xx()*(u_[i].x() - (U1_[i] && a_[i]));
            a_[i] += S1_[i]*qDdot[qi];
        }
        else
        {
            vector qDdoti(Dinv_[i] & (u_[i] - (U_[i].T() & a_[i])));

            // Need to add mutable "block<vector>" to Field
            qDdot[qi] = qDdoti.x();
            qDdot[qi+1] = qDdoti.y();
            qDdot[qi+2] = qDdoti.z();

            a_[i] += (S_[i] & qDdoti);
        }
    }

    DebugInfo
        << "qDdot = " << qDdot << nl
        << "a = " << a_ << endl;
}


void CML::RBD::rigidBodyModel::forwardDynamicsCorrection
(
    const rigidBodyModelState& state
) const
{
    DebugIn("RBD::rigidBodyModel::forwardDynamicsCorrection(const rigidBodyModelState&) const") << endl;

    const scalarField& q = state.q();
    const scalarField& qDot = state.qDot();
    const scalarField& qDdot = state.qDdot();

    // Joint state returned by jcalc
    joint::XSvc J;

    v_[0] = Zero;
    a_[0] = spatialVector(Zero, -g_);

    for (label i=1; i<nBodies(); i++)
    {
        const joint& jnt = joints()[i];
        const label qi = jnt.qIndex();

        jnt.jcalc(J, q, qDot);

        S_[i] = J.S;
        S1_[i] = J.S1;

        Xlambda_[i] = J.X & XT_[i];

        const label lambdai = lambda_[i];

        if (lambdai != 0)
        {
            X0_[i] = Xlambda_[i] & X0_[lambdai];
        }
        else
        {
            X0_[i] = Xlambda_[i];
        }

        v_[i] = (Xlambda_[i] & v_[lambdai]) + J.v;
        c_[i] = J.c + (v_[i] ^ J.v);
        a_[i] = (Xlambda_[i] & a_[lambdai]) + c_[i];

        if (jnt.nDoF() == 1)
        {
            a_[i] += S1_[i]*qDdot[qi];
        }
        else
        {
            a_[i] += S_[i] & qDdot.block<vector>(qi);
        }
    }

    DebugInfo<< "a = " << a_ << endl;
}


// ************************************************************************* //
