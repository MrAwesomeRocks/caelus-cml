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

\*---------------------------------------------------------------------------*/

#include "probes.hpp"
#include "volFields.hpp"
#include "dictionary.hpp"
#include "Time.hpp"
#include "IOmanip.hpp"
#include "mapPolyMesh.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(probes, 0);
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void CML::probes::findElements(const fvMesh& mesh)
{
    if (debug)
    {
        Info<< "probes: resetting sample locations" << endl;
    }

    elementList_.clear();
    elementList_.setSize(size());

    faceList_.clear();
    faceList_.setSize(size());

    forAll(*this, probeI)
    {
        const vector& location = operator[](probeI);

        const label cellI = mesh.findCell(location);

        elementList_[probeI] = cellI;

        if (cellI != -1)
        {
            const labelList& cellFaces = mesh.cells()[cellI];
            const vector& cellCentre = mesh.cellCentres()[cellI];
            scalar minDistance = GREAT;
            label minFaceID = -1;
            forAll (cellFaces, i)
            {
                label faceI = cellFaces[i];
                vector dist = mesh.faceCentres()[faceI] - cellCentre;
                if (mag(dist) < minDistance)
                {
                    minDistance = mag(dist);
                    minFaceID = faceI;
                }
            }
            faceList_[probeI] = minFaceID;
        }
        else
        {
            faceList_[probeI] = -1;
        }

        if (debug && (elementList_[probeI] != -1 || faceList_[probeI] != -1))
        {
            Pout<< "probes : found point " << location
                << " in cell " << elementList_[probeI]
                << " and face " << faceList_[probeI] << endl;
        }
    }


    // Check if all probes have been found.
    forAll(elementList_, probeI)
    {
        const vector& location = operator[](probeI);
        label cellI = elementList_[probeI];
        label faceI = faceList_[probeI];

        // Check at least one processor with cell.
        reduce(cellI, maxOp<label>());
        reduce(faceI, maxOp<label>());

        if (cellI == -1)
        {
            if (Pstream::master())
            {
                WarningInFunction
                    << "Did not find location " << location
                    << " in any cell. Skipping location." << endl;
            }
        }
        else if (faceI == -1)
        {
            if (Pstream::master())
            {
                WarningInFunction
                    << "Did not find location " << location
                    << " in any face. Skipping location." << endl;
            }
        }
        else
        {
            // Make sure location not on two domains.
            if (elementList_[probeI] != -1 && elementList_[probeI] != cellI)
            {
                WarningInFunction
                    << "Location " << location
                    << " seems to be on multiple domains:"
                    << " cell " << elementList_[probeI]
                    << " on my domain " << Pstream::myProcNo()
                        << " and cell " << cellI << " on some other domain."
                    << endl
                    << "This might happen if the probe location is on"
                    << " a processor patch. Change the location slightly"
                    << " to prevent this." << endl;
            }

            if (faceList_[probeI] != -1 && faceList_[probeI] != faceI)
            {
                WarningInFunction
                    << "Location " << location
                    << " seems to be on multiple domains:"
                    << " cell " << faceList_[probeI]
                    << " on my domain " << Pstream::myProcNo()
                        << " and face " << faceI << " on some other domain."
                    << endl
                    << "This might happen if the probe location is on"
                    << " a processor patch. Change the location slightly"
                    << " to prevent this." << endl;
            }
        }
    }
}


CML::label CML::probes::prepare()
{
    const label nFields = classifyFields();

    // adjust file streams
    if (Pstream::master())
    {
        wordHashSet currentFields;

        currentFields.insert(scalarFields_);
        currentFields.insert(vectorFields_);
        currentFields.insert(sphericalTensorFields_);
        currentFields.insert(symmTensorFields_);
        currentFields.insert(tensorFields_);

        currentFields.insert(surfaceScalarFields_);
        currentFields.insert(surfaceVectorFields_);
        currentFields.insert(surfaceSphericalTensorFields_);
        currentFields.insert(surfaceSymmTensorFields_);
        currentFields.insert(surfaceTensorFields_);

        if (debug)
        {
            Info<< "Probing fields: " << currentFields << nl
                << "Probing locations: " << *this << nl
                << endl;
        }


        fileName probeDir;
        fileName probeSubDir = name_;

        if (mesh_.name() != polyMesh::defaultRegion)
        {
            probeSubDir = probeSubDir/mesh_.name();
        }
        probeSubDir = "postProcessing"/probeSubDir/mesh_.time().timeName();

        if (Pstream::parRun())
        {
            // Put in undecomposed case
            // (Note: gives problems for distributed data running)
            probeDir = mesh_.time().path()/".."/probeSubDir;
        }
        else
        {
            probeDir = mesh_.time().path()/probeSubDir;
        }

        // ignore known fields, close streams for fields that no longer exist
        forAllIter(HashPtrTable<OFstream>, probeFilePtrs_, iter)
        {
            if (!currentFields.erase(iter.key()))
            {
                if (debug)
                {
                    Info<< "close probe stream: " << iter()->name() << endl;
                }

                delete probeFilePtrs_.remove(iter);
            }
        }

        // currentFields now just has the new fields - open streams for them
        forAllConstIter(wordHashSet, currentFields, iter)
        {
            const word& fieldName = iter.key();

            // Create directory if does not exist.
            mkDir(probeDir);

            OFstream* fPtr = new OFstream(probeDir/fieldName);

            OFstream& fout = *fPtr;

            if (debug)
            {
                Info<< "open probe stream: " << fout.name() << endl;
            }

            probeFilePtrs_.insert(fieldName, fPtr);

            unsigned int w = IOstream::defaultPrecision() + 7;

            forAll(*this, probeI)
            {
                fout<< "# Probe " << probeI << ' ' << operator[](probeI)
                    << endl;
            }

            fout<< '#' << setw(IOstream::defaultPrecision() + 6)
                << "Probe";

            forAll(*this, probeI)
            {
                fout<< ' ' << setw(w) << probeI;
            }
            fout<< endl;

            fout<< '#' << setw(IOstream::defaultPrecision() + 6)
                << "Time" << endl;
        }
    }

    return nFields;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::probes::probes
(
    const word& name,
    const objectRegistry& obr,
    const dictionary& dict,
    const bool loadFromFiles
)
:
    pointField(0),
    name_(name),
    mesh_(refCast<const fvMesh>(obr)),
    loadFromFiles_(loadFromFiles),
    fieldSelection_(),
    fixedLocations_(true),
    interpolationScheme_("cell")
{
    read(dict);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::probes::~probes()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void CML::probes::execute()
{
    // Do nothing - only valid on write
}


void CML::probes::end()
{
    // Do nothing - only valid on write
}


void CML::probes::timeSet()
{
    // Do nothing - only valid on write
}


void CML::probes::write()
{
    if (size() && prepare())
    {
        sampleAndWrite(scalarFields_);
        sampleAndWrite(vectorFields_);
        sampleAndWrite(sphericalTensorFields_);
        sampleAndWrite(symmTensorFields_);
        sampleAndWrite(tensorFields_);

        sampleAndWriteSurfaceFields(surfaceScalarFields_);
        sampleAndWriteSurfaceFields(surfaceVectorFields_);
        sampleAndWriteSurfaceFields(surfaceSphericalTensorFields_);
        sampleAndWriteSurfaceFields(surfaceSymmTensorFields_);
        sampleAndWriteSurfaceFields(surfaceTensorFields_);
    }
}


void CML::probes::read(const dictionary& dict)
{
    dict.lookup("probeLocations") >> *this;
    dict.lookup("fields") >> fieldSelection_;

    dict.readIfPresent("fixedLocations", fixedLocations_);
    if (dict.readIfPresent("interpolationScheme", interpolationScheme_))
    {
        if (!fixedLocations_ && interpolationScheme_ != "cell")
        {
            WarningInFunction
                << "Only cell interpolation can be applied when "
                << "not using fixedLocations.  InterpolationScheme "
                << "entry will be ignored";
        }
    }

    // Initialise cells to sample from supplied locations
    findElements(mesh_);

    prepare();
}


void CML::probes::updateMesh(const mapPolyMesh& mpm)
{
    if (debug)
    {
        Info<< "probes: updateMesh" << endl;
    }

    if (fixedLocations_)
    {
        findElements(mesh_);
    }
    else
    {
        if (debug)
        {
            Info<< "probes: remapping sample locations" << endl;
        }

        // 1. Update cells
        {
            DynamicList<label> elems(elementList_.size());

            const labelList& reverseMap = mpm.reverseCellMap();
            forAll(elementList_, i)
            {
                label cellI = elementList_[i];
                label newCellI = reverseMap[cellI];
                if (newCellI == -1)
                {
                    // cell removed
                }
                else if (newCellI < -1)
                {
                    // cell merged
                    elems.append(-newCellI - 2);
                }
                else
                {
                    // valid new cell
                    elems.append(newCellI);
                }
            }

            elementList_.transfer(elems);
        }

        // 2. Update faces
        {
            DynamicList<label> elems(faceList_.size());

            const labelList& reverseMap = mpm.reverseFaceMap();
            forAll(faceList_, i)
            {
                label faceI = faceList_[i];
                label newFaceI = reverseMap[faceI];
                if (newFaceI == -1)
                {
                    // face removed
                }
                else if (newFaceI < -1)
                {
                    // face merged
                    elems.append(-newFaceI - 2);
                }
                else
                {
                    // valid new face
                    elems.append(newFaceI);
                }
            }

            faceList_.transfer(elems);
        }
    }
}

void CML::probes::movePoints(const pointField&)
{
    if (debug)
    {
        Info<< "probes: movePoints" << endl;
    }

    if (fixedLocations_)
    {
        findElements(mesh_);
    }
}


// ************************************************************************* //
