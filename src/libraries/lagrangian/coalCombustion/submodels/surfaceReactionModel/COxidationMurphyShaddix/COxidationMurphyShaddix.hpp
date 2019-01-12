/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2015 OpenFOAM Foundation
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
    CML::COxidationMurphyShaddix

Description
    Limited to C(s) + O2 -> CO2

    Loosely based on the reference:
        Murphy, J. J., Shaddix, C. R., Combustion kinetics of coal chars
        in oxygen-enriched environments, Combustion and Flame 144,
        pp710-729, 2006

\*---------------------------------------------------------------------------*/

#ifndef COxidationMurphyShaddix_H
#define COxidationMurphyShaddix_H

#include "SurfaceReactionModel.hpp"
#include "mathematicalConstants.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward class declarations
template<class CloudType>
class COxidationMurphyShaddix;

/*---------------------------------------------------------------------------*\
                  Class COxidationMurphyShaddix Declaration
\*---------------------------------------------------------------------------*/

template<class CloudType>
class COxidationMurphyShaddix
:
    public SurfaceReactionModel<CloudType>
{
    // Private data

        // Model constants

            //- Reference diffusion constant
            const scalar D0_;

            //- Reference density for reference diffusion constant
            const scalar rho0_;

            //- Reference temperature for reference diffusion constant
            const scalar T0_;

            //- Exponent for diffusion equation
            const scalar Dn_;

            //- Kinetic rate coefficient
            const scalar A_;

            //- Kinetic rate activation energy
            const scalar E_;

            //- Reaction order
            const scalar n_;

            //- Effective molecular weight of gaseous volatiles
            const scalar WVol_;


        // Static constants

            //- Maximum number of iterations
            static label maxIters_;

            //- Tolerance used in inner iterations
            static scalar tolerance_;


        // Addressing

            //- Cs positions in global/local lists
            label CsLocalId_;

            //- O2 position in global list
            label O2GlobalId_;

            //- CO2 positions in global list
            label CO2GlobalId_;


        // Local copies of thermo properties

            //- Molecular weight of C [kg/kmol]
            scalar WC_;

            //- Molecular weight of O2 [kg/kmol]
            scalar WO2_;

            //- Formation enthalpy for CO2 [J/kg]
            scalar HcCO2_;


public:

    //- Runtime type information
    TypeName("COxidationMurphyShaddix");


    // Constructors

        //- Construct from dictionary
        COxidationMurphyShaddix
        (
            const dictionary& dict,
            CloudType& owner
        );

        //- Construct copy
        COxidationMurphyShaddix
        (
            const COxidationMurphyShaddix<CloudType>& srm
        );

        //- Construct and return a clone
        virtual autoPtr<SurfaceReactionModel<CloudType> > clone() const
        {
            return autoPtr<SurfaceReactionModel<CloudType> >
            (
                new COxidationMurphyShaddix<CloudType>(*this)
            );
        }


    //- Destructor
    virtual ~COxidationMurphyShaddix();


    // Member Functions

        //- Update surface reactions
        virtual scalar calculate
        (
            const scalar dt,
            const label celli,
            const scalar d,
            const scalar T,
            const scalar Tc,
            const scalar pc,
            const scalar rhoc,
            const scalar mass,
            const scalarField& YGas,
            const scalarField& YLiquid,
            const scalarField& YSolid,
            const scalarField& YMixture,
            const scalar N,
            scalarField& dMassGas,
            scalarField& dMassLiquid,
            scalarField& dMassSolid,
            scalarField& dMassSRCarrier
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

template<class CloudType>
CML::label CML::COxidationMurphyShaddix<CloudType>::maxIters_ = 1000;

template<class CloudType>
CML::scalar CML::COxidationMurphyShaddix<CloudType>::tolerance_ = 1e-06;


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
CML::COxidationMurphyShaddix<CloudType>::COxidationMurphyShaddix
(
    const dictionary& dict,
    CloudType& owner
)
:
    SurfaceReactionModel<CloudType>(dict, owner, typeName),
    D0_(readScalar(this->coeffDict().lookup("D0"))),
    rho0_(readScalar(this->coeffDict().lookup("rho0"))),
    T0_(readScalar(this->coeffDict().lookup("T0"))),
    Dn_(readScalar(this->coeffDict().lookup("Dn"))),
    A_(readScalar(this->coeffDict().lookup("A"))),
    E_(readScalar(this->coeffDict().lookup("E"))),
    n_(readScalar(this->coeffDict().lookup("n"))),
    WVol_(readScalar(this->coeffDict().lookup("WVol"))),
    CsLocalId_(-1),
    O2GlobalId_(owner.composition().carrierId("O2")),
    CO2GlobalId_(owner.composition().carrierId("CO2")),
    WC_(0.0),
    WO2_(0.0),
    HcCO2_(0.0)
{
    // Determine Cs ids
    label idSolid = owner.composition().idSolid();
    CsLocalId_ = owner.composition().localId(idSolid, "C");

    // Set local copies of thermo properties
    WO2_ = owner.thermo().carrier().Wi(O2GlobalId_);
    const scalar WCO2 = owner.thermo().carrier().Wi(CO2GlobalId_);
    WC_ = WCO2 - WO2_;

    HcCO2_ = owner.thermo().carrier().Hc(CO2GlobalId_);

    const scalar YCloc = owner.composition().Y0(idSolid)[CsLocalId_];
    const scalar YSolidTot = owner.composition().YMixture0()[idSolid];
    Info<< "    C(s): particle mass fraction = " << YCloc*YSolidTot << endl;
}


template<class CloudType>
CML::COxidationMurphyShaddix<CloudType>::COxidationMurphyShaddix
(
    const COxidationMurphyShaddix<CloudType>& srm
)
:
    SurfaceReactionModel<CloudType>(srm),
    D0_(srm.D0_),
    rho0_(srm.rho0_),
    T0_(srm.T0_),
    Dn_(srm.Dn_),
    A_(srm.A_),
    E_(srm.E_),
    n_(srm.n_),
    WVol_(srm.WVol_),
    CsLocalId_(srm.CsLocalId_),
    O2GlobalId_(srm.O2GlobalId_),
    CO2GlobalId_(srm.CO2GlobalId_),
    WC_(srm.WC_),
    WO2_(srm.WO2_),
    HcCO2_(srm.HcCO2_)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class CloudType>
CML::COxidationMurphyShaddix<CloudType>::~COxidationMurphyShaddix()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
CML::scalar CML::COxidationMurphyShaddix<CloudType>::calculate
(
    const scalar dt,
    const label celli,
    const scalar d,
    const scalar T,
    const scalar Tc,
    const scalar pc,
    const scalar rhoc,
    const scalar mass,
    const scalarField& YGas,
    const scalarField& YLiquid,
    const scalarField& YSolid,
    const scalarField& YMixture,
    const scalar N,
    scalarField& dMassGas,
    scalarField& dMassLiquid,
    scalarField& dMassSolid,
    scalarField& dMassSRCarrier
) const
{
    // Fraction of remaining combustible material
    const label idSolid = CloudType::parcelType::SLD;
    const scalar fComb = YMixture[idSolid]*YSolid[CsLocalId_];

    // Surface combustion until combustible fraction is consumed
    if (fComb < SMALL)
    {
        return 0.0;
    }

    const SLGThermo& thermo = this->owner().thermo();

    // Cell carrier phase O2 species density [kg/m^3]
    const scalar rhoO2 = rhoc*thermo.carrier().Y(O2GlobalId_)[celli];

    if (rhoO2 < SMALL)
    {
        return 0.0;
    }

    // Particle surface area [m^2]
    const scalar Ap = constant::mathematical::pi*sqr(d);

    // Calculate diffision constant at continuous phase temperature
    // and density [m^2/s]
    const scalar D = D0_*(rho0_/rhoc)*pow(Tc/T0_, Dn_);

    // Far field partial pressure O2 [Pa]
    const scalar ppO2 = rhoO2/WO2_*RR*Tc;

    // Total molar concentration of the carrier phase [kmol/m^3]
    const scalar C = pc/(RR*Tc);

    if (debug)
    {
        Pout<< "mass  = " << mass << nl
            << "fComb = " << fComb << nl
            << "Ap    = " << Ap << nl
            << "dt    = " << dt << nl
            << "C     = " << C << nl
            << endl;
    }

    // Molar reaction rate per unit surface area [kmol/(m^2.s)]
    scalar qCsOld = 0;
    scalar qCs = 1;

    const scalar qCsLim = mass*fComb/(WC_*Ap*dt);

    if (debug)
    {
        Pout<< "qCsLim = " << qCsLim << endl;
    }

    label iter = 0;
    while ((mag(qCs - qCsOld)/qCs > tolerance_) && (iter <= maxIters_))
    {
        qCsOld = qCs;
        const scalar PO2Surface = ppO2*exp(-(qCs + N)*d/(2*C*D));
        qCs = A_*exp(-E_/(RR*T))*pow(PO2Surface, n_);
        qCs = (100.0*qCs + iter*qCsOld)/(100.0 + iter);
        qCs = min(qCs, qCsLim);

        if (debug)
        {
            Pout<< "iter = " << iter
                << ", qCsOld = " << qCsOld
                << ", qCs = " << qCs
                << nl << endl;
        }

        iter++;
    }

    if (iter > maxIters_)
    {
        WarningInFunction
            << "iter limit reached (" << maxIters_ << ")" << nl << endl;
    }

    // Calculate the number of molar units reacted
    scalar dOmega = qCs*Ap*dt;

    // Add to carrier phase mass transfer
    dMassSRCarrier[O2GlobalId_] += -dOmega*WO2_;
    dMassSRCarrier[CO2GlobalId_] += dOmega*(WC_ + WO2_);

    // Add to particle mass transfer
    dMassSolid[CsLocalId_] += dOmega*WC_;

    const scalar HsC = thermo.solids().properties()[CsLocalId_].Hs(T);

    // carrier sensible enthalpy exchange handled via change in mass

    // Heat of reaction [J]
    return dOmega*(WC_*HsC - (WC_ + WO2_)*HcCO2_);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
