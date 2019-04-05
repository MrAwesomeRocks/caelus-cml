/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
Copyright (C) 2011-2018 OpenFOAM Foundation
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
    CML::ThermoSurfaceFilm

Description
    Thermo parcel surface film model.

    Responsible for:
    - injecting parcelss from the film model into the cloud, e.g. for dripping
    - parcel interaction with the film, e.g absorb, bounce, splash

    Splash model references:

        Bai and Gosman, `Mathematical modelling of wall films formed by
        impinging sprays', SAE 960626, 1996

        Bai et al, `Modelling of gasoline spray impingement', Atom. Sprays,
        vol 12, pp 1-27, 2002


\*---------------------------------------------------------------------------*/

#ifndef ThermoSurfaceFilm_H
#define ThermoSurfaceFilm_H

#include "SurfaceFilmModel_.hpp"
#include "addToRunTimeSelectionTable.hpp"
#include "mathematicalConstants.hpp"
#include "Pstream.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                      Class ThermoSurfaceFilm Declaration
\*---------------------------------------------------------------------------*/

template<class CloudType>
class ThermoSurfaceFilm
:
    public SurfaceFilmModel<CloudType>
{
public:

    // Public data

        //  Interaction type enumerations
        enum interactionType
        {
            itAbsorb,
            itBounce,
            itSplashBai
        };

        //- Word descriptions of interaction type names
        static wordList interactionTypeNames_;


    // Public Member Functions

        // Return interaction type enum from word
        interactionType interactionTypeEnum(const word& it) const;

        // Return word from interaction type enum
        word interactionTypeStr(const interactionType& it) const;


protected:

    // Protected data

        //- Convenience typedef to the cloud's parcel type
        typedef typename CloudType::parcelType parcelType;

        //- Reference to the cloud random number generator
        Random& rndGen_;

        //- Reference to the cloud thermo package
        const SLGThermo& thermo_;


        // Cached injector fields per film patch

            //- Film temperature / patch face
            scalarList TFilmPatch_;

            //- Film specific heat capacity / patch face
            scalarList CpFilmPatch_;


        // Interaction model data

            //- Interaction type enumeration
            interactionType interactionType_;

            //- Film thickness beyond which patch is assumed to be wet
            scalar deltaWet_;

            //- Splash parcel type label - id assigned to identify parcel for
            //  post-processing. If not specified, defaults to originating cloud
            //  type
            label splashParcelType_;

            //- Number of new parcels resulting from splash event
            label parcelsPerSplash_;


            // Surface roughness coefficient typically in the range 1300 - 5200
            // and decreases with increasing surface roughness

                //- Dry surface roughness coefficient
                //  = 2630 for dry interaction (ref. Bai)
                scalar Adry_;

                //- Wet surface roughness coefficient
                //  = 1320 for wet interaction (ref. Bai)
                scalar Awet_;


            //- Skin friction typically in the range 0.6 < Cf < 0.8
            scalar Cf_;

            //- Counter for number of new splash parcels
            label nParcelsSplashed_;


    // Protected Member Functions

        //- Return a vector tangential to input vector, v
        vector tangentVector(const vector& v) const;

        //- Return splashed parcel direction
        vector splashDirection
        (
            const vector& tanVec1,
            const vector& tanVec2,
            const vector& nf
        ) const;


        // Interaction models

            //- Absorb parcel into film
            void absorbInteraction
            (
                regionModels::surfaceFilmModels::surfaceFilmRegionModel&,
                const parcelType& p,
                const polyPatch& pp,
                const label facei,
                const scalar mass,
                bool& keepParticle
            );

            //- Bounce parcel (flip parcel normal velocity)
            void bounceInteraction
            (
                parcelType& p,
                const polyPatch& pp,
                const label facei,
                bool& keepParticle
            ) const;

            //- Parcel interaction with dry surface
            void drySplashInteraction
            (
                regionModels::surfaceFilmModels::surfaceFilmRegionModel&,
                const parcelType& p,
                const polyPatch& pp,
                const label facei,
                bool& keepParticle
            );

            //- Parcel interaction with wetted surface
            void wetSplashInteraction
            (
                regionModels::surfaceFilmModels::surfaceFilmRegionModel&,
                parcelType& p,
                const polyPatch& pp,
                const label facei,
                bool& keepParticle
            );

            //- Bai parcel splash interaction model
            void splashInteraction
            (
                regionModels::surfaceFilmModels::surfaceFilmRegionModel&,
                const parcelType& p,
                const polyPatch& pp,
                const label facei,
                const scalar mRatio,
                const scalar We,
                const scalar Wec,
                const scalar sigma,
                bool& keepParticle
            );


        // Injection from sheet (ejection) helper functions

            //- Cache the film fields in preparation for injection
            virtual void cacheFilmFields
            (
                const label filmPatchi,
                const label primaryPatchi,
                const regionModels::surfaceFilmModels::surfaceFilmRegionModel&
                    filmModel
            );

            //- Set the individual parcel properties
            virtual void setParcelProperties
            (
                parcelType& p,
                const label filmFacei
            ) const;


public:

    //- Runtime type information
    TypeName("thermoSurfaceFilm");


    // Constructors

        //- Construct from components
        ThermoSurfaceFilm(const dictionary& dict, CloudType& owner);

        //- Construct copy
        ThermoSurfaceFilm(const ThermoSurfaceFilm<CloudType>& sfm);

        //- Construct and return a clone using supplied owner cloud
        virtual autoPtr<SurfaceFilmModel<CloudType> > clone() const
        {
            return autoPtr<SurfaceFilmModel<CloudType> >
            (
                new ThermoSurfaceFilm<CloudType>(*this)
            );
        }


    //- Destructor
    virtual ~ThermoSurfaceFilm();


    // Member Functions

        // Evaluation

            //- Transfer parcel from cloud to surface film
            //  Returns true if parcel is to be transferred
            virtual bool transferParcel
            (
                parcelType& p,
                const polyPatch& pp,
                bool& keepParticle
            );


        // I-O

            //- Write surface film info to stream
            virtual void info(Ostream& os);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML


using namespace CML::constant::mathematical;

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

template<class CloudType>
CML::wordList CML::ThermoSurfaceFilm<CloudType>::interactionTypeNames_
(
    IStringStream
    (
        "(absorb bounce splashBai)"
    )()
);


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

template<class CloudType>
typename CML::ThermoSurfaceFilm<CloudType>::interactionType
CML::ThermoSurfaceFilm<CloudType>::interactionTypeEnum(const word& it) const
{
    forAll(interactionTypeNames_, i)
    {
        if (interactionTypeNames_[i] == it)
        {
            return interactionType(i);
        }
    }

    FatalErrorInFunction
        << "Unknown interaction type " << it
        << ". Valid interaction types include: " << interactionTypeNames_
        << abort(FatalError);

    return interactionType(0);
}


template<class CloudType>
CML::word CML::ThermoSurfaceFilm<CloudType>::interactionTypeStr
(
    const interactionType& it
) const
{
    if (it >= interactionTypeNames_.size())
    {
        FatalErrorInFunction
            << "Unknown interaction type enumeration" << abort(FatalError);
    }

    return interactionTypeNames_[it];
}


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

template<class CloudType>
CML::vector CML::ThermoSurfaceFilm<CloudType>::tangentVector
(
    const vector& v
) const
{
    vector tangent = Zero;
    scalar magTangent = 0.0;

    while (magTangent < SMALL)
    {
        vector vTest = rndGen_.sample01<vector>();
        tangent = vTest - (vTest & v)*v;
        magTangent = mag(tangent);
    }

    return tangent/magTangent;
}


template<class CloudType>
CML::vector CML::ThermoSurfaceFilm<CloudType>::splashDirection
(
    const vector& tanVec1,
    const vector& tanVec2,
    const vector& nf
) const
{
    // Azimuthal angle [rad]
    const scalar phiSi = twoPi*rndGen_.sample01<scalar>();

    // Ejection angle [rad]
    const scalar thetaSi = pi/180.0*(rndGen_.sample01<scalar>()*(50 - 5) + 5);

    // Direction vector of new parcel
    const scalar alpha = sin(thetaSi);
    const scalar dcorr = cos(thetaSi);
    const vector normal = alpha*(tanVec1*cos(phiSi) + tanVec2*sin(phiSi));
    vector dirVec = dcorr*nf;
    dirVec += normal;

    return dirVec/mag(dirVec);
}


template<class CloudType>
void CML::ThermoSurfaceFilm<CloudType>::absorbInteraction
(
    regionModels::surfaceFilmModels::surfaceFilmRegionModel& filmModel,
    const parcelType& p,
    const polyPatch& pp,
    const label facei,
    const scalar mass,
    bool& keepParticle
)
{
    if (debug)
    {
        Info<< "Parcel " << p.origId() << " absorbInteraction" << endl;
    }

    // Patch face normal
    const vector& nf = pp.faceNormals()[facei];

    // Patch velocity
    const vector& Up = this->owner().U().boundaryField()[pp.index()][facei];

    // Relative parcel velocity
    const vector Urel = p.U() - Up;

    // Parcel normal velocity
    const vector Un = nf*(Urel & nf);

    // Parcel tangential velocity
    const vector Ut = Urel - Un;

    filmModel.addSources
    (
        pp.index(),
        facei,
        mass,                           // mass
        mass*Ut,                        // tangential momentum
        mass*mag(Un),                   // impingement pressure
        mass*p.hs()                     // energy
    );

    this->nParcelsTransferred()++;

    keepParticle = false;
}


template<class CloudType>
void CML::ThermoSurfaceFilm<CloudType>::bounceInteraction
(
    parcelType& p,
    const polyPatch& pp,
    const label facei,
    bool& keepParticle
) const
{
    if (debug)
    {
        Info<< "Parcel " << p.origId() << " bounceInteraction" << endl;
    }

    // Patch face normal
    const vector& nf = pp.faceNormals()[facei];

    // Patch velocity
    const vector& Up = this->owner().U().boundaryField()[pp.index()][facei];

    // Relative parcel velocity
    const vector Urel = p.U() - Up;

    // Flip parcel normal velocity component
    p.U() -= 2.0*nf*(Urel & nf);

    keepParticle = true;
}


template<class CloudType>
void CML::ThermoSurfaceFilm<CloudType>::drySplashInteraction
(
    regionModels::surfaceFilmModels::surfaceFilmRegionModel& filmModel,
    const parcelType& p,
    const polyPatch& pp,
    const label facei,
    bool& keepParticle
)
{
    if (debug)
    {
        Info<< "Parcel " << p.origId() << " drySplashInteraction" << endl;
    }

    const liquidProperties& liq = thermo_.liquids().properties()[0];

    // Patch face velocity and normal
    const vector& Up = this->owner().U().boundaryField()[pp.index()][facei];
    const vector& nf = pp.faceNormals()[facei];

    // Local pressure
    const scalar pc = thermo_.thermo().p()[p.cell()];

    // Retrieve parcel properties
    const scalar m = p.mass()*p.nParticle();
    const scalar rho = p.rho();
    const scalar d = p.d();
    const scalar sigma = liq.sigma(pc, p.T());
    const scalar mu = liq.mu(pc, p.T());
    const vector Urel = p.U() - Up;
    const vector Un = nf*(Urel & nf);

    // Laplace number
    const scalar La = rho*sigma*d/sqr(mu);

    // Weber number
    const scalar We = rho*magSqr(Un)*d/sigma;

    // Critical Weber number
    const scalar Wec = Adry_*pow(La, -0.183);

    if (We < Wec) // Adhesion - assume absorb
    {
        absorbInteraction(filmModel, p, pp, facei, m, keepParticle);
    }
    else // Splash
    {
        // Ratio of incident mass to splashing mass
        const scalar mRatio = 0.2 + 0.6*rndGen_.sample01<scalar>();
        splashInteraction
            (filmModel, p, pp, facei, mRatio, We, Wec, sigma, keepParticle);
    }
}


template<class CloudType>
void CML::ThermoSurfaceFilm<CloudType>::wetSplashInteraction
(
    regionModels::surfaceFilmModels::surfaceFilmRegionModel& filmModel,
    parcelType& p,
    const polyPatch& pp,
    const label facei,
    bool& keepParticle
)
{
    if (debug)
    {
        Info<< "Parcel " << p.origId() << " wetSplashInteraction" << endl;
    }

    const liquidProperties& liq = thermo_.liquids().properties()[0];

    // Patch face velocity and normal
    const vector& Up = this->owner().U().boundaryField()[pp.index()][facei];
    const vector& nf = pp.faceNormals()[facei];

    // Local pressure
    const scalar pc = thermo_.thermo().p()[p.cell()];

    // Retrieve parcel properties
    const scalar m = p.mass()*p.nParticle();
    const scalar rho = p.rho();
    const scalar d = p.d();
    vector& U = p.U();
    const scalar sigma = liq.sigma(pc, p.T());
    const scalar mu = liq.mu(pc, p.T());
    const vector Urel = p.U() - Up;
    const vector Un = nf*(Urel & nf);
    const vector Ut = Urel - Un;

    // Laplace number
    const scalar La = rho*sigma*d/sqr(mu);

    // Weber number
    const scalar We = rho*magSqr(Un)*d/sigma;

    // Critical Weber number
    const scalar Wec = Awet_*pow(La, -0.183);

    if (We < 2) // Adhesion - assume absorb
    {
        absorbInteraction(filmModel, p, pp, facei, m, keepParticle);
    }
    else if ((We >= 2) && (We < 20)) // Bounce
    {
        // Incident angle of impingement
        const scalar theta = pi/2 - acos(U/mag(U) & nf);

        // Restitution coefficient
        const scalar epsilon = 0.993 - theta*(1.76 - theta*(1.56 - theta*0.49));

        // Update parcel velocity
        U = -epsilon*(Un) + 5.0/7.0*(Ut);

        keepParticle = true;
        return;
    }
    else if ((We >= 20) && (We < Wec)) // Spread - assume absorb
    {
        absorbInteraction(filmModel, p, pp, facei, m, keepParticle);
    }
    else    // Splash
    {
        // Ratio of incident mass to splashing mass
        // splash mass can be > incident mass due to film entrainment
        const scalar mRatio = 0.2 + 0.9*rndGen_.sample01<scalar>();
        splashInteraction
            (filmModel, p, pp, facei, mRatio, We, Wec, sigma, keepParticle);
    }
}


template<class CloudType>
void CML::ThermoSurfaceFilm<CloudType>::splashInteraction
(
    regionModels::surfaceFilmModels::surfaceFilmRegionModel& filmModel,
    const parcelType& p,
    const polyPatch& pp,
    const label facei,
    const scalar mRatio,
    const scalar We,
    const scalar Wec,
    const scalar sigma,
    bool& keepParticle
)
{
    // Patch face velocity and normal
    const fvMesh& mesh = this->owner().mesh();
    const vector& Up = this->owner().U().boundaryField()[pp.index()][facei];
    const vector& nf = pp.faceNormals()[facei];

    // Determine direction vectors tangential to patch normal
    const vector tanVec1 = tangentVector(nf);
    const vector tanVec2 = nf^tanVec1;

    // Retrieve parcel properties
    const scalar np = p.nParticle();
    const scalar m = p.mass()*np;
    const scalar d = p.d();
    const vector Urel = p.U() - Up;
    const vector Un = nf*(Urel & nf);
    const vector Ut = Urel - Un;
    const vector& posC = mesh.C()[p.cell()];
    const vector& posCf = mesh.Cf().boundaryField()[pp.index()][facei];

    // Total mass of (all) splashed parcels
    const scalar mSplash = m*mRatio;

    // Number of splashed particles per incoming particle
    const scalar Ns = 5.0*(We/Wec - 1.0);

    // Average diameter of splashed particles
    const scalar dBarSplash = 1/cbrt(6.0)*cbrt(mRatio/Ns)*d + ROOTVSMALL;

    // Cumulative diameter splash distribution
    const scalar dMax = 0.9*cbrt(mRatio)*d;
    const scalar dMin = 0.1*dMax;
    const scalar K = exp(-dMin/dBarSplash) - exp(-dMax/dBarSplash);

    // Surface energy of secondary parcels [J]
    scalar ESigmaSec = 0;

    // Sample splash distribution to determine secondary parcel diameters
    scalarList dNew(parcelsPerSplash_);
    scalarList npNew(parcelsPerSplash_);
    forAll(dNew, i)
    {
        const scalar y = rndGen_.sample01<scalar>();
        dNew[i] = -dBarSplash*log(exp(-dMin/dBarSplash) - y*K);
        npNew[i] = mRatio*np*pow3(d)/pow3(dNew[i])/parcelsPerSplash_;
        ESigmaSec += npNew[i]*sigma*p.areaS(dNew[i]);
    }

    // Incident kinetic energy [J]
    const scalar EKIn = 0.5*m*magSqr(Un);

    // Incident surface energy [J]
    const scalar ESigmaIn = np*sigma*p.areaS(d);

    // Dissipative energy
    const scalar Ed = max(0.8*EKIn, np*Wec/12*pi*sigma*sqr(d));

    // Total energy [J]
    const scalar EKs = EKIn + ESigmaIn - ESigmaSec - Ed;

    // Switch to absorb if insufficient energy for splash
    if (EKs <= 0)
    {
        absorbInteraction(filmModel, p, pp, facei, m, keepParticle);
        return;
    }

    // Helper variables to calculate magUns0
    const scalar logD = log(d);
    const scalar coeff2 = log(dNew[0]) - logD + ROOTVSMALL;
    scalar coeff1 = 0.0;
    forAll(dNew, i)
    {
        coeff1 += sqr(log(dNew[i]) - logD);
    }

    // Magnitude of the normal velocity of the first splashed parcel
    const scalar magUns0 =
        sqrt(2.0*parcelsPerSplash_*EKs/mSplash/(1.0 + coeff1/sqr(coeff2)));

    // Set splashed parcel properties
    forAll(dNew, i)
    {
        const vector dirVec = splashDirection(tanVec1, tanVec2, -nf);

        // Create a new parcel by copying source parcel
        parcelType* pPtr = new parcelType(p);

        pPtr->origId() = pPtr->getNewParticleID();

        pPtr->origProc() = Pstream::myProcNo();

        if (splashParcelType_ >= 0)
        {
            pPtr->typeId() = splashParcelType_;
        }

        // Perturb new parcels towards the owner cell centre
        pPtr->track(0.5*rndGen_.sample01<scalar>()*(posC - posCf), 0);

        pPtr->nParticle() = npNew[i];

        pPtr->d() = dNew[i];

        pPtr->U() = dirVec*(mag(Cf_*Ut) + magUns0*(log(dNew[i]) - logD)/coeff2);

        // Apply correction to velocity for 2-D cases
        meshTools::constrainDirection(mesh, mesh.solutionD(), pPtr->U());

        // Add the new parcel
        this->owner().addParticle(pPtr);

        nParcelsSplashed_++;
    }

    // Transfer remaining part of parcel to film 0 - splashMass can be -ve
    // if entraining from the film
    const scalar mDash = m - mSplash;
    absorbInteraction(filmModel, p, pp, facei, mDash, keepParticle);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
CML::ThermoSurfaceFilm<CloudType>::ThermoSurfaceFilm
(
    const dictionary& dict,
    CloudType& owner
)
:
    SurfaceFilmModel<CloudType>(dict, owner, typeName),
    rndGen_(owner.rndGen()),
    thermo_
    (
        owner.db().objectRegistry::template lookupObject<SLGThermo>("SLGThermo")
    ),
    TFilmPatch_(0),
    CpFilmPatch_(0),
    interactionType_
    (
        interactionTypeEnum(this->coeffDict().lookup("interactionType"))
    ),
    deltaWet_(0.0),
    splashParcelType_(0),
    parcelsPerSplash_(0),
    Adry_(0.0),
    Awet_(0.0),
    Cf_(0.0),
    nParcelsSplashed_(0)
{
    Info<< "    Applying " << interactionTypeStr(interactionType_)
        << " interaction model" << endl;

    if (interactionType_ == itSplashBai)
    {
        this->coeffDict().lookup("deltaWet") >> deltaWet_;
        splashParcelType_ =
            this->coeffDict().lookupOrDefault("splashParcelType", -1);
        parcelsPerSplash_ =
            this->coeffDict().lookupOrDefault("parcelsPerSplash", 2);
        this->coeffDict().lookup("Adry") >> Adry_;
        this->coeffDict().lookup("Awet") >> Awet_;
        this->coeffDict().lookup("Cf") >> Cf_;
    }
}


template<class CloudType>
CML::ThermoSurfaceFilm<CloudType>::ThermoSurfaceFilm
(
    const ThermoSurfaceFilm<CloudType>& sfm
)
:
    SurfaceFilmModel<CloudType>(sfm),
    rndGen_(sfm.rndGen_),
    thermo_(sfm.thermo_),
    TFilmPatch_(sfm.TFilmPatch_),
    CpFilmPatch_(sfm.CpFilmPatch_),
    interactionType_(sfm.interactionType_),
    deltaWet_(sfm.deltaWet_),
    splashParcelType_(sfm.splashParcelType_),
    parcelsPerSplash_(sfm.parcelsPerSplash_),
    Adry_(sfm.Adry_),
    Awet_(sfm.Awet_),
    Cf_(sfm.Cf_),
    nParcelsSplashed_(sfm.nParcelsSplashed_)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class CloudType>
CML::ThermoSurfaceFilm<CloudType>::~ThermoSurfaceFilm()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
bool CML::ThermoSurfaceFilm<CloudType>::transferParcel
(
    parcelType& p,
    const polyPatch& pp,
    bool& keepParticle
)
{
    // Retrieve the film model from the owner database
    regionModels::surfaceFilmModels::surfaceFilmRegionModel& filmModel =
        const_cast<regionModels::surfaceFilmModels::surfaceFilmRegionModel&>
        (
            this->owner().db().time().objectRegistry::template
                lookupObject<regionModels::surfaceFilmModels::surfaceFilmRegionModel>
                (
                    "surfaceFilmProperties"
                )
        );

    const label patchi = pp.index();

    if (filmModel.isRegionPatch(patchi))
    {
        const label facei = pp.whichFace(p.face());

        switch (interactionType_)
        {
            case itBounce:
            {
                bounceInteraction(p, pp, facei, keepParticle);

                break;
            }
            case itAbsorb:
            {
                const scalar m = p.nParticle()*p.mass();
                absorbInteraction(filmModel, p, pp, facei, m, keepParticle);

                break;
            }
            case itSplashBai:
            {
                bool dry = this->deltaFilmPatch_[patchi][facei] < deltaWet_;

                if (dry)
                {
                    drySplashInteraction(filmModel, p, pp, facei, keepParticle);
                }
                else
                {
                    wetSplashInteraction(filmModel, p, pp, facei, keepParticle);
                }

                break;
            }
            default:
            {
                FatalErrorInFunction
                    << "Unknown interaction type enumeration"
                    << abort(FatalError);
            }
        }

        // Transfer parcel/parcel interactions complete
        return true;
    }

    // Parcel not interacting with film
    return false;
}


template<class CloudType>
void CML::ThermoSurfaceFilm<CloudType>::cacheFilmFields
(
    const label filmPatchi,
    const label primaryPatchi,
    const regionModels::surfaceFilmModels::surfaceFilmRegionModel& filmModel
)
{
    SurfaceFilmModel<CloudType>::cacheFilmFields
    (
        filmPatchi,
        primaryPatchi,
        filmModel
    );

    TFilmPatch_ = filmModel.Ts().boundaryField()[filmPatchi];
    filmModel.toPrimary(filmPatchi, TFilmPatch_);

    CpFilmPatch_ = filmModel.Cp().boundaryField()[filmPatchi];
    filmModel.toPrimary(filmPatchi, CpFilmPatch_);
}


template<class CloudType>
void CML::ThermoSurfaceFilm<CloudType>::setParcelProperties
(
    parcelType& p,
    const label filmFacei
) const
{
    SurfaceFilmModel<CloudType>::setParcelProperties(p, filmFacei);

    // Set parcel properties
    p.T() = TFilmPatch_[filmFacei];
    p.Cp() = CpFilmPatch_[filmFacei];
}


template<class CloudType>
void CML::ThermoSurfaceFilm<CloudType>::info(Ostream& os)
{
    SurfaceFilmModel<CloudType>::info(os);

    label nSplash0 = this->template getModelProperty<label>("nParcelsSplashed");
    label nSplashTotal =
        nSplash0 + returnReduce(nParcelsSplashed_, sumOp<label>());

    os  << "    New film splash parcels         = " << nSplashTotal << endl;

    if (this->outputTime())
    {
        this->setModelProperty("nParcelsSplashed", nSplashTotal);
        nParcelsSplashed_ = 0;
    }
}


#endif

// ************************************************************************* //
