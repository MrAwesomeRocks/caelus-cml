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
    CML::ODESolidChemistryModel

Description
    Extends base chemistry model by adding a thermo package, and ODE functions.
    Introduces chemistry equation system and evaluation of chemical source
    terms.

SourceFiles
    ODESolidChemistryModelI.hpp
    ODESolidChemistryModel.cpp

\*---------------------------------------------------------------------------*/

#ifndef ODESolidChemistryModel_H
#define ODESolidChemistryModel_H

#include "solidReaction.hpp"
#include "ODE.hpp"
#include "volFieldsFwd.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// Forward declaration of classes
class fvMesh;

/*---------------------------------------------------------------------------*\
                   Class ODESolidChemistryModel Declaration
\*---------------------------------------------------------------------------*/

template<class CompType, class SolidThermo, class GasThermo>
class ODESolidChemistryModel
:
    public CompType,
    public ODE
{
    // Private Member Functions

        //- Disallow default bitwise assignment
        void operator=(const ODESolidChemistryModel&);


protected:

        //- Reference to solid mass fractions
        PtrList<volScalarField>& Ys_;

        //- List of gas species present in reaction system
        speciesTable pyrolisisGases_;

        //- Reactions
        const PtrList<solidReaction>& reactions_;

        //- Thermodynamic data of solids
        const PtrList<SolidThermo>& solidThermo_;

        //- Thermodynamic data of gases
        PtrList<GasThermo> gasThermo_;

         //- Number of gas species
        label nGases_;

        //- Number of components being solved by ODE
        label nSpecie_;

        //- Number of solid components
        label nSolids_;

        //- Number of solid reactions
        label nReaction_;

        //- List of reaction rate per solid [kg/m3/s]
        PtrList<scalarField> RRs_;

        //- List of reaction rate per gas [kg/m3/s]
        PtrList<scalarField> RRg_;


    // Protected Member Functions

        //- Write access to source terms for solids
        inline PtrList<scalarField>& RRs();

        //- Write access to source terms for gases
        inline PtrList<scalarField>& RRg();


private:

        //- List of accumulative solid concentrations
        mutable PtrList<volScalarField> Ys0_;

        //- Cell counter
        label cellCounter_;

        //- List of active reacting cells
        List<bool> reactingCells_;


        // Private members

            //- Set reacting status of cell, cellI
            void setCellReacting(const label cellI, const bool active);


public:

    //- Runtime type information
    TypeName("ODESolidChemistryModel");


    // Constructors

        //- Construct from components
        ODESolidChemistryModel
        (
            const fvMesh& mesh,
            const word& compTypeName,
            const word& SolidThermoName
        );


    //- Destructor
    virtual ~ODESolidChemistryModel();


    // Member Functions

        //- The reactions
        inline const PtrList<solidReaction>& reactions() const;

        //- Thermodynamic data of gases
        inline const PtrList<GasThermo>& gasThermo() const;

        //- Gases table
        inline const speciesTable& gasTable() const;

        //- The number of solids
        inline label nSpecie() const;

        //- The number of solids
        inline label nGases() const;

        //- The number of reactions
        inline label nReaction() const;


        //- dc/dt = omega, rate of change in concentration, for each species
        virtual scalarField omega
        (
            const scalarField& c,
            const scalar T,
            const scalar p,
            const bool updateC0 = false
        ) const;

        //- Return the reaction rate for reaction r and the reference
        //  species and charateristic times
        virtual scalar omega
        (
            const solidReaction& r,
            const scalarField& c,
            const scalar T,
            const scalar p,
            scalar& pf,
            scalar& cf,
            label& lRef,
            scalar& pr,
            scalar& cr,
            label& rRef
        ) const;

        //- Calculates the reaction rates
        virtual void calculate();


        // Chemistry model functions

            //- Return const access to the chemical source terms for solids
            inline tmp<volScalarField> RRs(const label i) const;

            //- Return const access to the chemical source terms for gases
            inline tmp<volScalarField> RRg(const label i) const;

            //- Return total gas source term
            inline tmp<volScalarField> RRg() const;

            //- Return total solid source term
            inline tmp<volScalarField> RRs() const;

            //- Return const access to the total source terms
            inline tmp<volScalarField> RR(const label i) const;

            //- Return sensible enthalpy for gas i [J/Kg]
            virtual tmp<volScalarField> gasHs
            (
                const volScalarField& T,
                const label i
            ) const;

            //- Solve the reaction system for the given start time and time
            //  step and return the characteristic time
            virtual scalar solve(const scalar t0, const scalar deltaT);

            //- Return the chemical time scale
            virtual tmp<volScalarField> tc() const;

            //- Return source for enthalpy equation [kg/m/s3]
            virtual tmp<volScalarField> Sh() const;

            //- Return the heat release, i.e. enthalpy/sec [m2/s3]
            virtual tmp<volScalarField> dQ() const;


        // ODE functions (overriding abstract functions in ODE.H)

            //- Number of ODE's to solve
            virtual label nEqns() const;

            virtual void derivatives
            (
                const scalar t,
                const scalarField& c,
                scalarField& dcdt
            ) const;

            virtual void jacobian
            (
                const scalar t,
                const scalarField& c,
                scalarField& dcdt,
                scalarSquareMatrix& dfdc
            ) const;

            virtual scalar solve
            (
                scalarField &c,
                const scalar T,
                const scalar p,
                const scalar t0,
                const scalar dt
            ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

#include "volFields.hpp"
#include "zeroGradientFvPatchFields.hpp"

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CompType, class SolidThermo, class GasThermo>
inline CML::PtrList<CML::scalarField>&
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::RRs()
{
    return RRs_;
}


template<class CompType, class SolidThermo, class GasThermo>
inline CML::PtrList<CML::scalarField>&
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::RRg()
{
    return RRg_;
}


template<class CompType, class SolidThermo, class GasThermo>
inline const CML::PtrList<CML::solidReaction>&
CML::ODESolidChemistryModel<CompType, SolidThermo,GasThermo>::reactions() const
{
    return reactions_;
}


template<class CompType, class SolidThermo, class GasThermo>
inline const CML::PtrList<GasThermo>&
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::
gasThermo() const
{
    return gasThermo_;
}


template<class CompType, class SolidThermo, class GasThermo>
inline const CML::speciesTable&
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::gasTable() const
{
    return pyrolisisGases_;
}


template<class CompType, class SolidThermo, class GasThermo>
inline CML::label
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::nSpecie() const
{
    return nSpecie_;
}


template<class CompType, class SolidThermo, class GasThermo>
inline CML::label
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::
nReaction() const
{
    return nReaction_;
}


template<class CompType, class SolidThermo, class GasThermo>
inline CML::tmp<CML::volScalarField>
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::RRs
(
    const label i
) const
{
    tmp<volScalarField> tRRs
    (
        new volScalarField
        (
            IOobject
            (
                "RRs(" + Ys_[i].name() + ')',
                this->time().timeName(),
                this->mesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            this->mesh(),
            dimensionedScalar("zero", dimMass/dimVolume/dimTime, 0.0),
            zeroGradientFvPatchScalarField::typeName
        )
    );

    if (this->chemistry_)
    {
        tRRs().internalField() = RRs_[i];
        tRRs().correctBoundaryConditions();
    }
    return tRRs;
}


template<class CompType, class SolidThermo, class GasThermo>
inline CML::tmp<CML::volScalarField>
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::RRg
(
    const label i
) const
{
    tmp<volScalarField> tRRg
    (
        new volScalarField
        (
            IOobject
            (
                "RRg(" + this->pyrolisisGases_[i] + ')',
                this->time().timeName(),
                this->mesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            this->mesh(),
            dimensionedScalar("zero", dimMass/dimVolume/dimTime, 0.0),
            zeroGradientFvPatchScalarField::typeName
        )
    );

    if (this->chemistry_)
    {
        tRRg().internalField() = RRg_[i];
        tRRg().correctBoundaryConditions();
    }
    return tRRg;
}


template<class CompType, class SolidThermo, class GasThermo>
inline CML::tmp<CML::volScalarField>
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::RRg() const
{
    tmp<volScalarField> tRRg
    (
        new volScalarField
        (
            IOobject
            (
                "RRg",
                this->time().timeName(),
                this->mesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            this->mesh(),
            dimensionedScalar("zero", dimMass/dimVolume/dimTime, 0.0),
            zeroGradientFvPatchScalarField::typeName
        )
    );

    if (this->chemistry_)
    {
        for (label i=0; i < nGases_; i++)
        {
            tRRg().internalField() += RRg_[i];
        }
        tRRg().correctBoundaryConditions();
    }
    return tRRg;
}


template<class CompType, class SolidThermo, class GasThermo>
inline CML::tmp<CML::volScalarField>
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::RRs() const
{
    tmp<volScalarField> tRRs
    (
        new volScalarField
        (
            IOobject
            (
                "RRs",
                this->time().timeName(),
                this->mesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            this->mesh(),
            dimensionedScalar("zero", dimMass/dimVolume/dimTime, 0.0),
            zeroGradientFvPatchScalarField::typeName
        )
    );

    if (this->chemistry_)
    {
        for (label i=0; i < nSolids_; i++)
        {
            tRRs().internalField() += RRs_[i];
        }
        tRRs().correctBoundaryConditions();
    }
    return tRRs;
}


template<class CompType, class SolidThermo, class GasThermo>
inline CML::tmp<CML::volScalarField>
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::RR
(
    const label i
) const
{
    notImplemented("ODESolidChemistryModel::RR(const label)");
    return (CML::volScalarField::null());
}



#include "reactingSolidMixture.hpp"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CompType, class SolidThermo, class GasThermo>
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::
ODESolidChemistryModel
(
    const fvMesh& mesh,
    const word& compTypeName,
    const word& solidThermoName
)
:
    CompType(mesh, solidThermoName),
    ODE(),
    Ys_(this->solidThermo().composition().Y()),
    pyrolisisGases_
    (
        mesh.lookupObject<dictionary>
            ("chemistryProperties").lookup("species")
    ),
    reactions_
    (
        static_cast<const reactingSolidMixture<SolidThermo>& >
            (this->solidThermo().composition())
    ),
    solidThermo_
    (
        static_cast<const reactingSolidMixture<SolidThermo>& >
            (this->solidThermo().composition()).solidData()
    ),
    gasThermo_(pyrolisisGases_.size()),
    nGases_(pyrolisisGases_.size()),
    nSpecie_(Ys_.size() + nGases_),
    nSolids_(Ys_.size()),
    nReaction_(reactions_.size()),
    RRs_(nSolids_),
    RRg_(nGases_),
    Ys0_(nSolids_),
    cellCounter_(0),
    reactingCells_(mesh.nCells(), true)
{
    // create the fields for the chemistry sources
    forAll(RRs_, fieldI)
    {
        RRs_.set
        (
            fieldI,
            new scalarField(mesh.nCells(), 0.0)
        );


        IOobject header
        (
            Ys_[fieldI].name() + "0",
            mesh.time().timeName(),
            mesh,
            IOobject::NO_READ
        );

        // check if field exists and can be read
        if (header.headerOk())
        {
            Ys0_.set
            (
                fieldI,
                new volScalarField
                (
                    IOobject
                    (
                        Ys_[fieldI].name() + "0",
                        mesh.time().timeName(),
                        mesh,
                        IOobject::MUST_READ,
                        IOobject::AUTO_WRITE
                    ),
                    mesh
                )
            );
        }
        else
        {
            volScalarField Y0Default
            (
                IOobject
                (
                    "Y0Default",
                    mesh.time().timeName(),
                    mesh,
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                ),
                mesh
            );

            Ys0_.set
            (
                fieldI,
                new volScalarField
                (
                    IOobject
                    (
                        Ys_[fieldI].name() + "0",
                        mesh.time().timeName(),
                        mesh,
                        IOobject::NO_READ,
                        IOobject::AUTO_WRITE
                    ),
                    Y0Default
                )
            );
        // Calculate initial values of Ysi0 = rho*delta*Yi
            Ys0_[fieldI].internalField() =
                this->solidThermo().rho()
               *max(Ys_[fieldI], scalar(0.001))*mesh.V();
        }
   }

    forAll(RRg_, fieldI)
    {
        RRg_.set(fieldI, new scalarField(mesh.nCells(), 0.0));
    }

    forAll(gasThermo_, gasI)
    {
        dictionary thermoDict =
            mesh.lookupObject<dictionary>
            (
                "chemistryProperties"
            ).subDict(pyrolisisGases_[gasI]);

        gasThermo_.set
        (
            gasI,
            new GasThermo(thermoDict)
        );
    }

    Info<< "ODESolidChemistryModel: Number of solids = " << nSolids_
        << " and reactions = " << nReaction_ << endl;

    Info<< "Number of gases from pyrolysis = " << nGases_ << endl;

    forAll(reactions_, i)
    {
        Info<< indent << "Reaction " << i << nl << reactions_[i] << nl;
    }

}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class CompType, class SolidThermo, class GasThermo>
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::
~ODESolidChemistryModel()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CompType, class SolidThermo, class GasThermo>
CML::scalarField CML::
ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::omega
(
    const scalarField& c,
    const scalar T,
    const scalar p,
    const bool updateC0
) const
{
    scalar pf, cf, pr, cr;
    label lRef, rRef;

    const label cellI = cellCounter_;

    scalarField om(nEqns(), 0.0);

    forAll(reactions_, i)
    {
        const solidReaction& R = reactions_[i];

        scalar omegai = omega
        (
            R, c, T, 0.0, pf, cf, lRef, pr, cr, rRef
        );
        scalar rhoL = 0.0;
        forAll(R.slhs(), s)
        {
            label si = R.slhs()[s];
            om[si] -= omegai;
            rhoL = solidThermo_[si].rho(T);
        }
        scalar sr = 0.0;
        forAll(R.srhs(), s)
        {
            label si = R.srhs()[s];
            scalar rhoR = solidThermo_[si].rho(T);
            sr = rhoR/rhoL;
            om[si] += sr*omegai;

            if (updateC0)
            {
                Ys0_[si][cellI] += sr*omegai;
            }
        }
        forAll(R.grhs(), g)
        {
            label gi = R.grhs()[g];
            om[gi + nSolids_] += (1.0 - sr)*omegai;
        }
    }

    return om;
}


template<class CompType, class SolidThermo, class GasThermo>
CML::scalar
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::omega
(
    const solidReaction& R,
    const scalarField& c,
    const scalar T,
    const scalar p,
    scalar& pf,
    scalar& cf,
    label& lRef,
    scalar& pr,
    scalar& cr,
    label& rRef
) const
{
    scalarField c1(nSpecie_, 0.0);

    label cellI = cellCounter_;

    for (label i=0; i<nSpecie_; i++)
    {
        c1[i] = max(0.0, c[i]);
    }

    scalar kf = R.kf(T, 0.0, c1);

    scalar exponent = R.nReact();

    const label Nl = R.slhs().size();

    for (label s=0; s<Nl; s++)
    {
        label si = R.slhs()[s];

        kf *=
            pow(c1[si]/Ys0_[si][cellI], exponent)
           *(Ys0_[si][cellI]);
    }

    return kf;
}


template<class CompType, class SolidThermo, class GasThermo>
void CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::derivatives
(
    const scalar time,
    const scalarField &c,
    scalarField& dcdt
) const
{
    scalar T = c[nSpecie_];

    dcdt = 0.0;

    dcdt = omega(c, T, 0);

    //Total mass concentration
    scalar cTot = 0.0;
    for (label i=0; i<nSolids_; i++)
    {
        cTot += c[i];
    }

    scalar newCp = 0.0;
    scalar newhi = 0.0;
    for (label i=0; i<nSolids_; i++)
    {
        scalar dYidt = dcdt[i]/cTot;
        scalar Yi = c[i]/cTot;
        newCp += Yi*solidThermo_[i].Cp(T);
        newhi -= dYidt*solidThermo_[i].hf();
    }

    scalar dTdt = newhi/newCp;
    scalar dtMag = min(500.0, mag(dTdt));
    dcdt[nSpecie_] = dTdt*dtMag/(mag(dTdt) + 1.0e-10);

    // dp/dt = ...
    dcdt[nSpecie_ + 1] = 0.0;
}


template<class CompType, class SolidThermo, class GasThermo>
void CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::jacobian
(
    const scalar t,
    const scalarField& c,
    scalarField& dcdt,
    scalarSquareMatrix& dfdc
) const
{
    scalar T = c[nSpecie_];

    scalarField c2(nSpecie_, 0.0);

    for (label i=0; i<nSolids_; i++)
    {
        c2[i] = max(c[i], 0.0);
    }

    for (label i=0; i<nEqns(); i++)
    {
        for (label j=0; j<nEqns(); j++)
        {
            dfdc[i][j] = 0.0;
        }
    }

    // length of the first argument must be nSolids
    dcdt = omega(c2, T, 0.0);

    for (label ri=0; ri<reactions_.size(); ri++)
    {
        const solidReaction& R = reactions_[ri];

        scalar kf0 = R.kf(T, 0.0, c2);

        forAll(R.slhs(), j)
        {
            label sj = R.slhs()[j];
            scalar kf = kf0;
            forAll(R.slhs(), i)
            {
                label si = R.slhs()[i];
                scalar exp = R.nReact();
                if (i == j)
                {
                    if (exp < 1.0)
                    {
                        if (c2[si]>SMALL)
                        {
                            kf *= exp*pow(c2[si] + VSMALL, exp - 1.0);
                        }
                        else
                        {
                            kf = 0.0;
                        }
                    }
                    else
                    {
                        kf *= exp*pow(c2[si], exp - 1.0);
                    }
                }
                else
                {
                    Info<< "Solid reactions have only elements on slhs"
                        << endl;
                    kf = 0.0;
                }
            }

            forAll(R.slhs(), i)
            {
                label si = R.slhs()[i];
                dfdc[si][sj] -= kf;
            }
            forAll(R.srhs(), i)
            {
                label si = R.srhs()[i];
                dfdc[si][sj] += kf;
            }
        }
    }

    // calculate the dcdT elements numerically
    scalar delta = 1.0e-8;
    scalarField dcdT0 = omega(c2, T - delta, 0);
    scalarField dcdT1 = omega(c2, T + delta, 0);

    for (label i=0; i<nEqns(); i++)
    {
        dfdc[i][nSpecie_] = 0.5*(dcdT1[i] - dcdT0[i])/delta;
    }

}


template<class CompType, class SolidThermo, class GasThermo>
CML::tmp<CML::volScalarField>
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::tc() const
{
    notImplemented
    (
        "ODESolidChemistryModel::tc()"
    );

    return volScalarField::null();
}


template<class CompType, class SolidThermo, class GasThermo>
CML::tmp<CML::volScalarField>
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::Sh() const
{
    tmp<volScalarField> tSh
    (
        new volScalarField
        (
            IOobject
            (
                "Sh",
                this->mesh_.time().timeName(),
                this->mesh_,
                IOobject::NO_READ,
                IOobject::AUTO_WRITE,
                false
            ),
            this->mesh_,
            dimensionedScalar("zero", dimEnergy/dimTime/dimVolume, 0.0),
            zeroGradientFvPatchScalarField::typeName
        )
    );

    if (this->chemistry_)
    {
        scalarField& Sh = tSh();

        forAll(Ys_, i)
        {
            forAll(Sh, cellI)
            {
                scalar hf = solidThermo_[i].hf();
                Sh[cellI] -= hf*RRs_[i][cellI];
            }
        }
    }

    return tSh;
}


template<class CompType, class SolidThermo, class GasThermo>
CML::tmp<CML::volScalarField>
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::dQ() const
{
    tmp<volScalarField> tdQ
    (
        new volScalarField
        (
            IOobject
            (
                "dQ",
                this->mesh_.time().timeName(),
                this->mesh_,
                IOobject::NO_READ,
                IOobject::NO_WRITE,
                false
            ),
            this->mesh_,
            dimensionedScalar("dQ", dimEnergy/dimTime, 0.0),
            zeroGradientFvPatchScalarField::typeName
        )
    );

    if (this->chemistry_)
    {
        volScalarField& dQ = tdQ();
        dQ.dimensionedInternalField() = this->mesh_.V()*Sh()();
    }

    return tdQ;
}


template<class CompType, class SolidThermo, class GasThermo>
CML::label CML::
ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::nEqns() const
{
    // nEqns = number of solids + gases + temperature + pressure
    return (nSpecie_ + 2);
}


template<class CompType, class SolidThermo, class GasThermo>
void CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::
calculate()
{

    const volScalarField rho
    (
        IOobject
        (
            "rho",
            this->time().timeName(),
            this->mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            false
        ),
        this->solidThermo().rho()
    );

    if (this->mesh().changing())
    {
        forAll(RRs_, i)
        {
            RRs_[i].setSize(rho.size());
        }
        forAll(RRg_, i)
        {
            RRg_[i].setSize(rho.size());
        }
    }

    forAll(RRs_, i)
    {
        RRs_[i] = 0.0;
    }
    forAll(RRg_, i)
    {
        RRg_[i] = 0.0;
    }

    if (this->chemistry_)
    {
        forAll(rho, celli)
        {
            cellCounter_ = celli;

            const scalar delta = this->mesh().V()[celli];

            if (reactingCells_[celli])
            {
                scalar rhoi = rho[celli];
                scalar Ti = this->solidThermo().T()[celli];

                scalarField c(nSpecie_, 0.0);
                for (label i=0; i<nSolids_; i++)
                {
                    c[i] = rhoi*Ys_[i][celli]*delta;
                }

                const scalarField dcdt = omega(c, Ti, 0.0, true);

                forAll(RRs_, i)
                {
                    RRs_[i][celli] = dcdt[i]/delta;
                }

                forAll(RRg_, i)
                {
                    RRg_[i][celli] = dcdt[nSolids_ + i]/delta;
                }
            }
        }
    }
}


template<class CompType, class SolidThermo, class GasThermo>
CML::scalar
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::solve
(
    const scalar t0,
    const scalar deltaT
)
{
    const volScalarField rho
    (
        IOobject
        (
            "rho",
            this->time().timeName(),
            this->mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            false
        ),
        this->solidThermo().rho()
    );

    if (this->mesh().changing())
    {
        forAll(RRs_, i)
        {
            RRs_[i].setSize(rho.size());
        }
        forAll(RRg_, i)
        {
            RRg_[i].setSize(rho.size());
        }
    }

    forAll(RRs_, i)
    {
        RRs_[i] = 0.0;
    }
    forAll(RRg_, i)
    {
        RRg_[i] = 0.0;
    }

    if (!this->chemistry_)
    {
        return GREAT;
    }

    scalar deltaTMin = GREAT;

    forAll(rho, celli)
    {
        if (reactingCells_[celli])
        {
            cellCounter_ = celli;

            scalar rhoi = rho[celli];
            scalar Ti = this->solidThermo().T()[celli];

            scalarField c(nSpecie_, 0.0);
            scalarField c0(nSpecie_, 0.0);
            scalarField dc(nSpecie_, 0.0);

            scalar delta = this->mesh().V()[celli];

            for (label i=0; i<nSolids_; i++)
            {
                c[i] = rhoi*Ys_[i][celli]*delta;
            }

            c0 = c;

            scalar t = t0;
            scalar tauC = this->deltaTChem_[celli];
            scalar dt = min(deltaT, tauC);
            scalar timeLeft = deltaT;

            // calculate the chemical source terms
            while (timeLeft > SMALL)
            {
                tauC = this->solve(c, Ti, 0.0, t, dt);
                t += dt;

                // update the temperature
                scalar cTot = 0.0;

                //Total mass concentration
                for (label i=0; i<nSolids_; i++)
                {
                    cTot += c[i];
                }

                scalar newCp = 0.0;
                scalar newhi = 0.0;
                scalar invRho = 0.0;
                scalarList dcdt = (c - c0)/dt;

                for (label i=0; i<nSolids_; i++)
                {
                    scalar dYi = dcdt[i]/cTot;
                    scalar Yi = c[i]/cTot;
                    newCp += Yi*solidThermo_[i].Cp(Ti);
                    newhi -= dYi*solidThermo_[i].hf();
                    invRho += Yi/solidThermo_[i].rho(Ti);
                }

                scalar dTi = (newhi/newCp)*dt;

                Ti += dTi;

                timeLeft -= dt;
                this->deltaTChem_[celli] = tauC;
                dt = min(timeLeft, tauC);
                dt = max(dt, SMALL);
            }

            deltaTMin = min(tauC, deltaTMin);
            dc = c - c0;

            forAll(RRs_, i)
            {
                RRs_[i][celli] = dc[i]/(deltaT*delta);
            }

            forAll(RRg_, i)
            {
                RRg_[i][celli] = dc[nSolids_ + i]/(deltaT*delta);
            }

            // Update Ys0_
            dc = omega(c0, Ti, 0.0, true);
        }
    }

    // Don't allow the time-step to change more than a factor of 2
    deltaTMin = min(deltaTMin, 2*deltaT);

    return deltaTMin;
}


template<class CompType, class SolidThermo,class GasThermo>
CML::tmp<CML::volScalarField>
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::gasHs
(
    const volScalarField& T,
    const label index
) const
{

    tmp<volScalarField> tHs
    (
        new volScalarField
        (
            IOobject
            (
                "Hs_" + pyrolisisGases_[index],
                this->mesh_.time().timeName(),
                this->mesh_,
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            this->mesh_,
            dimensionedScalar("zero", dimEnergy/dimMass, 0.0),
            zeroGradientFvPatchScalarField::typeName
        )
    );

    volScalarField::InternalField& gasHs = tHs().internalField();

    const GasThermo& mixture = gasThermo_[index];

    forAll(gasHs, cellI)
    {
        gasHs[cellI] = mixture.Hs(T[cellI]);
    }

    return tHs;
}


template<class CompType, class SolidThermo,class GasThermo>
CML::scalar
CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::solve
(
    scalarField &c,
    const scalar T,
    const scalar p,
    const scalar t0,
    const scalar dt
) const
{
    notImplemented
    (
        "ODESolidChemistryModel::solve"
        "("
            "scalarField&, "
            "const scalar, "
            "const scalar, "
            "const scalar, "
            "const scalar"
        ")"
    );
    return (0);
}


template<class CompType, class SolidThermo,class GasThermo>
void CML::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::
setCellReacting(const label cellI, const bool active)
{
    reactingCells_[cellI] = active;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
