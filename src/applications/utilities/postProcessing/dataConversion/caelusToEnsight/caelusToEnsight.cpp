/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2012 OpenFOAM Foundation
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

Description
    Translates CAELUS data to EnSight format.

    An Ensight part is created for the internalMesh and for each patch.

Usage
    - caelusToEnsight [OPTION] \n
    Translates CAELUS data to Ensight format

    \param -ascii \n
    Write Ensight data in ASCII format instead of "C Binary"

    \param -patches patchList \n
    Specify particular patches to write.
    Specifying an empty list suppresses writing the internalMesh.

    \param -noPatches \n
    Suppress writing any patches.

    \param -faceZones zoneList \n
    Specify faceZones to write, with wildcards

    \param -cellZone zoneName \n
    Specify single cellZone to write (not lagrangian)

Note
    Parallel support for cloud data is not supported
    - writes to \a EnSight directory to avoid collisions with caelusToEnsightParts

\*---------------------------------------------------------------------------*/

#include "argList.hpp"
#include "timeSelector.hpp"
#include "IOobjectList.hpp"
#include "IOmanip.hpp"
#include "OFstream.hpp"

#include "volFields.hpp"

#include "labelIOField.hpp"
#include "scalarIOField.hpp"
#include "tensorIOField.hpp"

#include "ensightMesh.hpp"
#include "ensightField.hpp"

#include "ensightParticlePositions.hpp"
#include "ensightCloudField.hpp"

#include "fvc.hpp"

#include "cellSet.hpp"
#include "fvMeshSubset.hpp"

using namespace CML;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

bool inFileNameList
(
    const fileNameList& nameList,
    const word& name
)
{
    forAll(nameList, i)
    {
        if (nameList[i] == name)
        {
            return true;
        }
    }

    return false;
}



int main(int argc, char *argv[])
{
    timeSelector::addOptions();
#   include "addRegionOption.hpp"

    argList::addBoolOption
    (
        "ascii",
        "write in ASCII format instead of 'C Binary'"
    );
    argList::addBoolOption
    (
        "nodeValues",
        "write values in nodes"
    );
    argList::addBoolOption
    (
        "noPatches",
        "suppress writing any patches"
    );
    argList::addOption
    (
        "patches",
        "wordReList",
        "specify particular patches to write - eg '(outlet \"inlet.*\")'. "
        "An empty list suppresses writing the internalMesh."
    );
    argList::addOption
    (
        "faceZones",
        "wordReList",
        "specify faceZones to write - eg '( slice \"mfp-.*\" )'."
    );
    argList::addOption
    (
        "fields",
        "wordReList",
        "specify fields to export (all by default) - eg '( \"U.*\" )'."
    );
    argList::addOption
    (
        "cellZone",
        "word",
        "specify cellZone to write"
    );

#   include "setRootCase.hpp"

    // Check options
    const bool binary = !args.optionFound("ascii");
    const bool nodeValues = args.optionFound("nodeValues");

#   include "createTime.hpp"

    instantList Times = timeSelector::select0(runTime, args);

#   include "createNamedMesh.hpp"

    // Mesh instance (region0 gets filtered out)
    fileName regionPrefix = "";

    if (regionName != polyMesh::defaultRegion)
    {
        regionPrefix = regionName;
    }

    const label nVolFieldTypes = 5;
    const word volFieldTypes[] =
    {
        volScalarField::typeName,
        volVectorField::typeName,
        volSphericalTensorField::typeName,
        volSymmTensorField::typeName,
        volTensorField::typeName
    };

    // Path to EnSight folder at case level only
    // - For parallel cases, data only written from master
    fileName ensightDir = args.rootPath()/args.globalCaseName()/"EnSight";

    if (Pstream::master())
    {
        if (isDir(ensightDir))
        {
            rmDir(ensightDir);
        }

        mkDir(ensightDir);
    }

    // Start of case file header output
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    const word prepend = args.globalCaseName() + '.';

    OFstream *ensightCaseFilePtr = nullptr;
    if (Pstream::master())
    {
        fileName caseFileName = prepend + "case";
        Info<< nl << "write case: " << caseFileName.c_str() << endl;

        // the case file is always ASCII
        ensightCaseFilePtr = new OFstream
        (
            ensightDir/caseFileName,
            IOstream::ASCII
        );

        *ensightCaseFilePtr
            << "FORMAT" << nl
            << "type: ensight gold" << nl << nl;
    }

    OFstream& ensightCaseFile = *ensightCaseFilePtr;

    // Construct the EnSight mesh
    const bool selectedPatches = args.optionFound("patches");
    wordReList patchPatterns;
    if (selectedPatches)
    {
        patchPatterns = wordReList(args.optionLookup("patches")());
    }

    const bool selectedZones = args.optionFound("faceZones");
    wordReList zonePatterns;
    if (selectedZones)
    {
        zonePatterns = wordReList(args.optionLookup("faceZones")());
    }

    const bool selectedFields = args.optionFound("fields");
    wordReList fieldPatterns;
    if (selectedFields)
    {
        fieldPatterns = wordReList(args.optionLookup("fields")());
    }

    word cellZoneName;
    const bool doCellZone = args.optionReadIfPresent("cellZone", cellZoneName);

    fvMeshSubset meshSubsetter(mesh);
    if (doCellZone)
    {
        Info<< "Converting cellZone " << cellZoneName
            << " only (puts outside faces into patch "
            << mesh.boundaryMesh()[0].name()
            << ")" << endl;
        const cellZone& cz = mesh.cellZones()[cellZoneName];
        cellSet c0(mesh, "c0", labelHashSet(cz));
        meshSubsetter.setLargeCellSubset(c0, 0);
    }

    ensightMesh eMesh
    (
        (
            meshSubsetter.hasSubMesh()
          ? meshSubsetter.subMesh()
          : meshSubsetter.baseMesh()
        ),
        args.optionFound("noPatches"),
        selectedPatches,
        patchPatterns,
        selectedZones,
        zonePatterns,
        binary
    );

    // Set Time to the last time before looking for the lagrangian objects
    runTime.setTime(Times.last(), Times.size()-1);

    IOobjectList objects(mesh, runTime.timeName());

#   include "checkMeshMoving.hpp"

    if (meshMoving)
    {
        Info<< "Detected a moving mesh (multiple polyMesh/points files)."
            << " Writing meshes for every timestep." << endl;
    }


    wordHashSet allCloudNames;
    if (Pstream::master())
    {
        word geomFileName = prepend + "0000";

        // test pre check variable if there is a moving mesh
        if (meshMoving)
        {
            geomFileName = prepend + "****";
        }

        ensightCaseFile
            << "GEOMETRY" << nl
            << "model:        1     "
            << (geomFileName + ".mesh").c_str() << nl;
    }

    // Identify if lagrangian data exists at each time, and add clouds
    // to the 'allCloudNames' hash set
    forAll(Times, timeI)
    {
        runTime.setTime(Times[timeI], timeI);

        fileNameList cloudDirs = readDir
        (
            runTime.timePath()/regionPrefix/cloud::prefix,
            fileName::DIRECTORY
        );

        forAll(cloudDirs, cloudI)
        {
            IOobjectList cloudObjs
            (
                mesh,
                runTime.timeName(),
                cloud::prefix/cloudDirs[cloudI]
            );

            IOobject* positionsPtr = cloudObjs.lookup(word("positions"));

            if (positionsPtr)
            {
                allCloudNames.insert(cloudDirs[cloudI]);
            }
        }
    }

    HashTable<HashTable<word> > allCloudFields;
    forAllConstIter(wordHashSet, allCloudNames, cloudIter)
    {
        // Add the name of the cloud(s) to the case file header
        if (Pstream::master())
        {
            ensightCaseFile
            <<  (
                    "measured:     1     "
                  + prepend
                  + "***."
                  + cloudIter.key()
                ).c_str()
            << nl;
        }

        // Create a new hash table for each cloud
        allCloudFields.insert(cloudIter.key(), HashTable<word>());

        // Identify the new cloud in the hash table
        HashTable<HashTable<word> >::iterator newCloudIter =
            allCloudFields.find(cloudIter.key());

        // Loop over all times to build list of fields and field types
        // for each cloud
        forAll(Times, timeI)
        {
            runTime.setTime(Times[timeI], timeI);

            IOobjectList cloudObjs
            (
                mesh,
                runTime.timeName(),
                cloud::prefix/cloudIter.key()
            );

            forAllConstIter(IOobjectList, cloudObjs, fieldIter)
            {
                const IOobject obj = *fieldIter();

                if (obj.name() != "positions")
                {
                    // Add field and field type
                    newCloudIter().insert
                    (
                        obj.name(),
                        obj.headerClassName()
                    );
                }
            }
        }
    }

    label nTimeSteps = 0;
    forAll(Times, timeIndex)
    {
        nTimeSteps++;
        runTime.setTime(Times[timeIndex], timeIndex);

        word timeName = itoa(timeIndex);
        word timeFile = prepend + timeName;

        Info<< "Translating time = " << runTime.timeName() << nl;

        polyMesh::readUpdateState meshState = mesh.readUpdate();
        if (timeIndex != 0 && meshSubsetter.hasSubMesh())
        {
            Info<< "Converting cellZone " << cellZoneName
                << " only (puts outside faces into patch "
                << mesh.boundaryMesh()[0].name()
                << ")" << endl;
            const cellZone& cz = mesh.cellZones()[cellZoneName];
            cellSet c0(mesh, "c0", labelHashSet(cz));
            meshSubsetter.setLargeCellSubset(c0, 0);
        }


        if (meshState != polyMesh::UNCHANGED)
        {
            eMesh.correct();
        }

        if (timeIndex == 0 || meshMoving)
        {
            eMesh.write
            (
                ensightDir,
                prepend,
                timeIndex,
                meshMoving,
                ensightCaseFile
            );
        }


        // Start of field data output
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~

        if (timeIndex == 0 && Pstream::master())
        {
            ensightCaseFile<< nl << "VARIABLE" << nl;
        }


        // Cell field data output
        // ~~~~~~~~~~~~~~~~~~~~~~

        for (label i=0; i<nVolFieldTypes; i++)
        {
            wordList fieldNames = objects.names(volFieldTypes[i]);

            forAll(fieldNames, j)
            {
                const word& fieldName = fieldNames[j];

                // Check if the field has to be exported
                if (selectedFields)
                {
                    if (!findStrings(fieldPatterns, fieldName))
                    {
                        continue;
                    }
                }

#               include "checkData.hpp"

                if (!variableGood)
                {
                    continue;
                }

                IOobject fieldObject
                (
                    fieldName,
                    mesh.time().timeName(),
                    mesh,
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                );

                if (volFieldTypes[i] == volScalarField::typeName)
                {
                    volScalarField vf(fieldObject, mesh);
                    ensightField<scalar>
                    (
                        volField(meshSubsetter, vf),
                        eMesh,
                        ensightDir,
                        prepend,
                        timeIndex,
                        binary,
                        nodeValues,
                        ensightCaseFile
                    );
                }
                else if (volFieldTypes[i] == volVectorField::typeName)
                {
                    volVectorField vf(fieldObject, mesh);
                    ensightField<vector>
                    (
                        volField(meshSubsetter, vf),
                        eMesh,
                        ensightDir,
                        prepend,
                        timeIndex,
                        binary,
                        nodeValues,
                        ensightCaseFile
                    );
                }
                else if (volFieldTypes[i] == volSphericalTensorField::typeName)
                {
                    volSphericalTensorField vf(fieldObject, mesh);
                    ensightField<sphericalTensor>
                    (
                        volField(meshSubsetter, vf),
                        eMesh,
                        ensightDir,
                        prepend,
                        timeIndex,
                        binary,
                        nodeValues,
                        ensightCaseFile
                    );
                }
                else if (volFieldTypes[i] == volSymmTensorField::typeName)
                {
                    volSymmTensorField vf(fieldObject, mesh);
                    ensightField<symmTensor>
                    (
                        volField(meshSubsetter, vf),
                        eMesh,
                        ensightDir,
                        prepend,
                        timeIndex,
                        binary,
                        nodeValues,
                        ensightCaseFile
                    );
                }
                else if (volFieldTypes[i] == volTensorField::typeName)
                {
                    volTensorField vf(fieldObject, mesh);
                    ensightField<tensor>
                    (
                        volField(meshSubsetter, vf),
                        eMesh,
                        ensightDir,
                        prepend,
                        timeIndex,
                        binary,
                        nodeValues,
                        ensightCaseFile
                    );
                }
            }
        }


        // Cloud field data output
        // ~~~~~~~~~~~~~~~~~~~~~~~

        forAllConstIter(HashTable<HashTable<word> >, allCloudFields, cloudIter)
        {
            const word& cloudName = cloudIter.key();

            fileNameList currentCloudDirs = readDir
            (
                runTime.timePath()/regionPrefix/cloud::prefix,
                fileName::DIRECTORY
            );

            bool cloudExists = inFileNameList(currentCloudDirs, cloudName);
            ensightParticlePositions
            (
                mesh,
                ensightDir,
                timeFile,
                cloudName,
                cloudExists
            );

            forAllConstIter(HashTable<word>, cloudIter(), fieldIter)
            {
                const word& fieldName = fieldIter.key();
                const word& fieldType = fieldIter();

                IOobject fieldObject
                (
                    fieldName,
                    mesh.time().timeName(),
                    cloud::prefix/cloudName,
                    mesh,
                    IOobject::MUST_READ
                );

                bool fieldExists = fieldObject.headerOk();
                if (fieldType == scalarIOField::typeName)
                {
                    ensightCloudField<scalar>
                    (
                        fieldObject,
                        ensightDir,
                        prepend,
                        timeIndex,
                        cloudName,
                        ensightCaseFile,
                        fieldExists
                    );
                }
                else if (fieldType == vectorIOField::typeName)
                {
                    ensightCloudField<vector>
                    (
                        fieldObject,
                        ensightDir,
                        prepend,
                        timeIndex,
                        cloudName,
                        ensightCaseFile,
                        fieldExists
                    );
                }
                else
                {
                    Info<< "Unable to convert field type " << fieldType
                        << " for field " << fieldName << endl;
                }
            }
        }
    }

#   include "ensightCaseTail.hpp"

    if (Pstream::master())
    {
        delete ensightCaseFilePtr;
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
