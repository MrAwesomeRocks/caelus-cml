/*---------------------------------------------------------------------------*\
Copyright (C) 2014 Applied CCM
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
    CML::FacePostProcessing

Description
    Records particle face quantities on used-specified face zone

    Currently supports:
        accummulated mass
        average mass flux


\*---------------------------------------------------------------------------*/

#ifndef FacePostProcessing_H
#define FacePostProcessing_H

#include "CloudFunctionObject.hpp"
#include "faceZone.hpp"
#include "Pstream.hpp"
#include "ListListOps.hpp"
#include "surfaceWriter.hpp"
#include "globalIndex.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                     Class FacePostProcessing Declaration
\*---------------------------------------------------------------------------*/

template<class CloudType>
class FacePostProcessing
:
    public CloudFunctionObject<CloudType>
{
    // Private Data

        // Typedefs

            //- Convenience typedef for parcel type
            typedef typename CloudType::parcelType parcelType;


        //- Face zone IDs
        labelList faceZoneIDs_;

        //- Surface output format
        const word surfaceFormat_;

        //- Flag to indicate whether data should be reset/cleared on writing
        Switch resetOnWrite_;

        //- Total time
        scalar totalTime_;

        //- Mass storage
        List<scalarField> mass_;

        //- Mass total storage
        List<scalarField> massTotal_;

        //- Mass flow rate storage
        List<scalarField> massFlowRate_;

        //- Flag to indicate whether data should be written to file
        Switch log_;

        //- Output file pointer per zone
        PtrList<OFstream> outputFilePtr_;

        //- Last calculation time
        scalar timeOld_;


    // Private Member Functions

        //- Helper function to create log files
        void makeLogFile
        (
            const word& zoneName,
            const label zoneI,
            const label nFaces,
            const scalar totArea
        );


protected:

    // Protected Member Functions

        //- Write post-processing info
        void write();


public:

    //- Runtime type information
    TypeName("facePostProcessing");


    // Constructors

        //- Construct from dictionary
        FacePostProcessing
        (
            const dictionary& dict,
            CloudType& owner,
            const word& modelName
        );

        //- Construct copy
        FacePostProcessing(const FacePostProcessing<CloudType>& ppm);

        //- Construct and return a clone
        virtual autoPtr<CloudFunctionObject<CloudType> > clone() const
        {
            return autoPtr<CloudFunctionObject<CloudType> >
            (
                new FacePostProcessing<CloudType>(*this)
            );
        }


    //- Destructor
    virtual ~FacePostProcessing();


    // Member Functions

        // Access

            //- Return const access to the reset on write flag
            inline const Switch& resetOnWrite() const;


        // Evaluation

            //- Post-face hook
            virtual void postFace(const parcelType& p, bool& keepParticle);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class CloudType>
inline const CML::Switch&
CML::FacePostProcessing<CloudType>::resetOnWrite() const
{
    return resetOnWrite_;
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class CloudType>
void CML::FacePostProcessing<CloudType>::makeLogFile
(
    const word& zoneName,
    const label zoneI,
    const label nFaces,
    const scalar totArea
)
{
    // Create the output file if not already created
    if (log_)
    {
        if (debug)
        {
            Info<< "Creating output file." << endl;
        }

        if (Pstream::master())
        {
            // Create directory if does not exist
            mkDir(this->outputTimeDir());

            // Open new file at start up
            outputFilePtr_.set
            (
                zoneI,
                new OFstream
                (
                    this->outputTimeDir()/(type() + '_' + zoneName + ".dat")
                )
            );

            outputFilePtr_[zoneI]
                << "# Source    : " << type() << nl
                << "# Face zone : " << zoneName << nl
                << "# Faces     : " << nFaces << nl
                << "# Area      : " << totArea << nl
                << "# Time" << tab << "mass" << tab << "massFlowRate" << endl;
        }
    }
}


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

template<class CloudType>
void CML::FacePostProcessing<CloudType>::write()
{
    const fvMesh& mesh = this->owner().mesh();
    const Time& time = mesh.time();
    const faceZoneMesh& fzm = mesh.faceZones();
    scalar timeNew = time.value();
    scalar timeElapsed = timeNew - timeOld_;

    totalTime_ += timeElapsed;

    const scalar alpha = (totalTime_ - timeElapsed)/totalTime_;
    const scalar beta = timeElapsed/totalTime_;

    forAll(faceZoneIDs_, zoneI)
    {
        massFlowRate_[zoneI] =
            alpha*massFlowRate_[zoneI] + beta*mass_[zoneI]/timeElapsed;
        massTotal_[zoneI] += mass_[zoneI];
    }

    const label proci = Pstream::myProcNo();

    Info<< type() << " output:" << nl;

    List<scalarField> zoneMassTotal(mass_.size());
    List<scalarField> zoneMassFlowRate(massFlowRate_.size());
    forAll(faceZoneIDs_, zoneI)
    {
        const word& zoneName = fzm[faceZoneIDs_[zoneI]].name();

        scalarListList allProcMass(Pstream::nProcs());
        allProcMass[proci] = massTotal_[zoneI];
        Pstream::gatherList(allProcMass);
        zoneMassTotal[zoneI] =
            ListListOps::combine<scalarList>
            (
                allProcMass, accessOp<scalarList>()
            );
        const scalar sumMassTotal = sum(zoneMassTotal[zoneI]);

        scalarListList allProcMassFlowRate(Pstream::nProcs());
        allProcMassFlowRate[proci] = massFlowRate_[zoneI];
        Pstream::gatherList(allProcMassFlowRate);
        zoneMassFlowRate[zoneI] =
            ListListOps::combine<scalarList>
            (
                allProcMassFlowRate, accessOp<scalarList>()
            );
        const scalar sumMassFlowRate = sum(zoneMassFlowRate[zoneI]);

        Info<< "    " << zoneName
            << ": total mass = " << sumMassTotal
            << "; average mass flow rate = " << sumMassFlowRate
            << nl;

        if (outputFilePtr_.set(zoneI))
        {
            OFstream& os = outputFilePtr_[zoneI];
            os  << time.timeName() << token::TAB << sumMassTotal << token::TAB
                << sumMassFlowRate<< endl;
        }
    }

    Info<< endl;


    if (surfaceFormat_ != "none")
    {
        forAll(faceZoneIDs_, zoneI)
        {
            const faceZone& fZone = fzm[faceZoneIDs_[zoneI]];

            labelList pointToGlobal;
            labelList uniqueMeshPointLabels;
            autoPtr<globalIndex> globalPointsPtr =
                mesh.globalData().mergePoints
                (
                    fZone().meshPoints(),
                    fZone().meshPointMap(),
                    pointToGlobal,
                    uniqueMeshPointLabels
                );

            pointField uniquePoints(mesh.points(), uniqueMeshPointLabels);
            List<pointField> allProcPoints(Pstream::nProcs());
            allProcPoints[proci] = uniquePoints;
            Pstream::gatherList(allProcPoints);

            faceList faces(fZone().localFaces());
            forAll(faces, i)
            {
                inplaceRenumber(pointToGlobal, faces[i]);
            }
            List<faceList> allProcFaces(Pstream::nProcs());
            allProcFaces[proci] = faces;
            Pstream::gatherList(allProcFaces);

            if (Pstream::master())
            {
                pointField allPoints
                (
                    ListListOps::combine<pointField>
                    (
                        allProcPoints, accessOp<pointField>()
                    )
                );

                faceList allFaces
                (
                    ListListOps::combine<faceList>
                    (
                        allProcFaces, accessOp<faceList>()
                    )
                );

                autoPtr<surfaceWriter> writer
                (
                    surfaceWriter::New
                    (
                        surfaceFormat_,
                        this->coeffDict().subOrEmptyDict("formatOptions").
                            subOrEmptyDict(surfaceFormat_)
                    )
                );

                writer->write
                (
                    this->outputTimeDir(),
                    fZone.name(),
                    allPoints,
                    allFaces,
                    "massTotal",
                    zoneMassTotal[zoneI],
                    false
                );

                writer->write
                (
                    this->outputTimeDir(),
                    fZone.name(),
                    allPoints,
                    allFaces,
                    "massFlowRate",
                    zoneMassFlowRate[zoneI],
                    false
                );
            }
        }
    }


    if (resetOnWrite_)
    {
        forAll(faceZoneIDs_, zoneI)
        {
            massFlowRate_[zoneI] = 0.0;
        }
        timeOld_ = timeNew;
        totalTime_ = 0.0;
    }

    forAll(mass_, zoneI)
    {
        mass_[zoneI] = 0.0;
    }

    // writeProperties();
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
CML::FacePostProcessing<CloudType>::FacePostProcessing
(
    const dictionary& dict,
    CloudType& owner,
    const word& modelName
)
:
    CloudFunctionObject<CloudType>(dict, owner, modelName, typeName),
    faceZoneIDs_(),
    surfaceFormat_(this->coeffDict().lookup("surfaceFormat")),
    resetOnWrite_(this->coeffDict().lookup("resetOnWrite")),
    totalTime_(0.0),
    mass_(),
    massTotal_(),
    massFlowRate_(),
    log_(this->coeffDict().lookup("log")),
    outputFilePtr_(),
    timeOld_(owner.mesh().time().value())
{
    wordList faceZoneNames(this->coeffDict().lookup("faceZones"));
    mass_.setSize(faceZoneNames.size());
    massTotal_.setSize(faceZoneNames.size());
    massFlowRate_.setSize(faceZoneNames.size());

    outputFilePtr_.setSize(faceZoneNames.size());

    DynamicList<label> zoneIDs;
    const faceZoneMesh& fzm = owner.mesh().faceZones();
    const surfaceScalarField& magSf = owner.mesh().magSf();
    const polyBoundaryMesh& pbm = owner.mesh().boundaryMesh();
    forAll(faceZoneNames, i)
    {
        const word& zoneName = faceZoneNames[i];
        label zoneI = fzm.findZoneID(zoneName);
        if (zoneI != -1)
        {
            zoneIDs.append(zoneI);
            const faceZone& fz = fzm[zoneI];
            mass_[i].setSize(fz.size(), 0.0);
            massTotal_[i].setSize(fz.size(), 0.0);
            massFlowRate_[i].setSize(fz.size(), 0.0);

            label nFaces = returnReduce(fz.size(), sumOp<label>());
            Info<< "        " << zoneName << " faces: " << nFaces << nl;

            scalar totArea = 0.0;
            forAll(fz, j)
            {
                label facei = fz[j];
                if (facei < owner.mesh().nInternalFaces())
                {
                    totArea += magSf[fz[j]];
                }
                else
                {
                    label bFacei = facei - owner.mesh().nInternalFaces();
                    label patchi = pbm.patchID()[bFacei];
                    const polyPatch& pp = pbm[patchi];

                    if
                    (
                        !magSf.boundaryField()[patchi].coupled()
                     || refCast<const coupledPolyPatch>(pp).owner()
                    )
                    {
                        label localFacei = pp.whichFace(facei);
                        totArea += magSf.boundaryField()[patchi][localFacei];
                    }
                }
            }
            totArea = returnReduce(totArea, sumOp<scalar>());

            makeLogFile(zoneName, i, nFaces, totArea);
        }
    }

    faceZoneIDs_.transfer(zoneIDs);

    // readProperties(); AND initialise mass... fields
}


template<class CloudType>
CML::FacePostProcessing<CloudType>::FacePostProcessing
(
    const FacePostProcessing<CloudType>& pff
)
:
    CloudFunctionObject<CloudType>(pff),
    faceZoneIDs_(pff.faceZoneIDs_),
    surfaceFormat_(pff.surfaceFormat_),
    resetOnWrite_(pff.resetOnWrite_),
    totalTime_(pff.totalTime_),
    mass_(pff.mass_),
    massTotal_(pff.massTotal_),
    massFlowRate_(pff.massFlowRate_),
    log_(pff.log_),
    outputFilePtr_(),
    timeOld_(0.0)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class CloudType>
CML::FacePostProcessing<CloudType>::~FacePostProcessing()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
void CML::FacePostProcessing<CloudType>::postFace(const parcelType& p, bool&)
{
    if
    (
        this->owner().solution().output()
     || this->owner().solution().transient()
    )
    {
        const faceZoneMesh& fzm = this->owner().mesh().faceZones();

        forAll(faceZoneIDs_, i)
        {
            const faceZone& fz = fzm[faceZoneIDs_[i]];

            label faceId = -1;
            forAll(fz, j)
            {
                if (fz[j] == p.face())
                {
                    faceId = j;
                    break;
                }
            }

            if (faceId != -1)
            {
                mass_[i][faceId] += p.mass()*p.nParticle();
            }
        }
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
