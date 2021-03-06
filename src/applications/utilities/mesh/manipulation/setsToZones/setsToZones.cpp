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

Description
    Add pointZones/faceZones/cellZones to the mesh from similar named
    pointSets/faceSets/cellSets.

    There is one catch: for faceZones you also need to specify a flip
    condition which basically denotes the side of the face. In this app
    it reads a cellSet (xxxCells if 'xxx' is the name of the faceSet) which
    is the masterCells of the zone.
    There are lots of situations in which this will go wrong but it is the
    best I can think of for now.

    If one is not interested in sideNess specify the -noFlipMap
    command line option.

\*---------------------------------------------------------------------------*/

#include "argList.hpp"
#include "Time.hpp"
#include "polyMesh.hpp"
#include "IStringStream.hpp"
#include "cellSet.hpp"
#include "faceSet.hpp"
#include "pointSet.hpp"
#include "OFstream.hpp"
#include "IFstream.hpp"
#include "IOobjectList.hpp"
#include "SortableList.hpp"

using namespace CML;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Main program:

int main(int argc, char *argv[])
{
    argList::addNote
    (
        "add point/face/cell Zones from similar named point/face/cell Sets"
    );

    argList::addBoolOption
    (
        "noFlipMap",
        "ignore orientation of faceSet"
    );

    #include "addRegionOption.hpp"
    #include "addTimeOptions.hpp"
    #include "setRootCase.hpp"
    #include "createTime.hpp"

    const bool noFlipMap = args.optionFound("noFlipMap");

    // Get times list
    instantList Times = runTime.times();

    label startTime = Times.size()-1;
    label endTime = Times.size();

    // check -time and -latestTime options
    #include "checkTimeOption.hpp"

    runTime.setTime(Times[startTime], startTime);

    #include "createNamedPolyMesh.hpp"

    // Search for list of objects for the time of the mesh
    word setsInstance = runTime.findInstance
    (
        polyMesh::meshSubDir/"sets",
        word::null,
        IOobject::MUST_READ,
        mesh.facesInstance()
    );

    IOobjectList objects(mesh, setsInstance, polyMesh::meshSubDir/"sets");

    Info<< "Searched : " << setsInstance/polyMesh::meshSubDir/"sets"
        << nl
        << "Found    : " << objects.names() << nl
        << endl;


    IOobjectList pointObjects(objects.lookupClass(pointSet::typeName));

    //Pout<< "pointSets:" << pointObjects.names() << endl;

    forAllConstIter(IOobjectList, pointObjects, iter)
    {
        // Not in memory. Load it.
        pointSet set(*iter());
        SortableList<label> pointLabels(set.toc());

        label zoneID = mesh.pointZones().findZoneID(set.name());
        if (zoneID == -1)
        {
            Info<< "Adding set " << set.name() << " as a pointZone." << endl;
            label sz = mesh.pointZones().size();
            mesh.pointZones().setSize(sz+1);
            mesh.pointZones().set
            (
                sz,
                new pointZone
                (
                    set.name(),             //name
                    pointLabels,            //addressing
                    sz,                     //index
                    mesh.pointZones()       //pointZoneMesh
                )
            );
            mesh.pointZones().writeOpt() = IOobject::AUTO_WRITE;
            mesh.pointZones().instance() = mesh.facesInstance();
        }
        else
        {
            Info<< "Overwriting contents of existing pointZone " << zoneID
                << " with that of set " << set.name() << "." << endl;
            mesh.pointZones()[zoneID] = pointLabels;
            mesh.pointZones().writeOpt() = IOobject::AUTO_WRITE;
            mesh.pointZones().instance() = mesh.facesInstance();
        }
    }



    IOobjectList faceObjects(objects.lookupClass(faceSet::typeName));

    HashSet<word> slaveCellSets;

    //Pout<< "faceSets:" << faceObjects.names() << endl;

    forAllConstIter(IOobjectList, faceObjects, iter)
    {
        // Not in memory. Load it.
        faceSet set(*iter());
        SortableList<label> faceLabels(set.toc());

        DynamicList<label> addressing(set.size());
        DynamicList<bool> flipMap(set.size());

        if (noFlipMap)
        {
            // No flip map.
            forAll(faceLabels, i)
            {
                label faceI = faceLabels[i];
                addressing.append(faceI);
                flipMap.append(false);
            }
        }
        else
        {
            const word setName(set.name() + "SlaveCells");

            Info<< "Trying to load cellSet " << setName
                << " to find out the slave side of the zone." << nl
                << "If you do not care about the flipMap"
                << " (i.e. do not use the sideness)" << nl
                << "use the -noFlipMap command line option."
                << endl;

            // Load corresponding cells
            cellSet cells(mesh, setName);

            // Store setName to exclude from cellZones further on
            slaveCellSets.insert(setName);

            forAll(faceLabels, i)
            {
                label faceI = faceLabels[i];

                bool flip = false;

                if (mesh.isInternalFace(faceI))
                {
                    if
                    (
                        cells.found(mesh.faceOwner()[faceI])
                    && !cells.found(mesh.faceNeighbour()[faceI])
                    )
                    {
                        flip = false;
                    }
                    else if
                    (
                       !cells.found(mesh.faceOwner()[faceI])
                     && cells.found(mesh.faceNeighbour()[faceI])
                    )
                    {
                        flip = true;
                    }
                    else
                    {
                        FatalErrorInFunction
                            << "One of owner or neighbour of internal face "
                            << faceI << " should be in cellSet " << cells.name()
                            << " to be able to determine orientation." << endl
                            << "Face:" << faceI
                            << " own:" << mesh.faceOwner()[faceI]
                            << " OwnInCellSet:"
                            << cells.found(mesh.faceOwner()[faceI])
                            << " nei:" << mesh.faceNeighbour()[faceI]
                            << " NeiInCellSet:"
                            << cells.found(mesh.faceNeighbour()[faceI])
                            << abort(FatalError);
                    }
                }
                else
                {
                    if (cells.found(mesh.faceOwner()[faceI]))
                    {
                        flip = false;
                    }
                    else
                    {
                        flip = true;
                    }
                }

                addressing.append(faceI);
                flipMap.append(flip);
            }
        }

        label zoneID = mesh.faceZones().findZoneID(set.name());
        if (zoneID == -1)
        {
            Info<< "Adding set " << set.name() << " as a faceZone." << endl;
            label sz = mesh.faceZones().size();
            mesh.faceZones().setSize(sz+1);
            mesh.faceZones().set
            (
                sz,
                new faceZone
                (
                    set.name(),             //name
                    addressing.shrink(),    //addressing
                    flipMap.shrink(),       //flipmap
                    sz,                     //index
                    mesh.faceZones()        //pointZoneMesh
                )
            );
            mesh.faceZones().writeOpt() = IOobject::AUTO_WRITE;
            mesh.faceZones().instance() = mesh.facesInstance();
        }
        else
        {
            Info<< "Overwriting contents of existing faceZone " << zoneID
                << " with that of set " << set.name() << "." << endl;
            mesh.faceZones()[zoneID].resetAddressing
            (
                addressing.shrink(),
                flipMap.shrink()
            );
            mesh.faceZones().writeOpt() = IOobject::AUTO_WRITE;
            mesh.faceZones().instance() = mesh.facesInstance();
        }
    }



    IOobjectList cellObjects(objects.lookupClass(cellSet::typeName));

    //Pout<< "cellSets:" << cellObjects.names() << endl;

    forAllConstIter(IOobjectList, cellObjects, iter)
    {
        if (!slaveCellSets.found(iter.key()))
        {
            // Not in memory. Load it.
            cellSet set(*iter());
            SortableList<label> cellLabels(set.toc());

            label zoneID = mesh.cellZones().findZoneID(set.name());
            if (zoneID == -1)
            {
                Info<< "Adding set " << set.name() << " as a cellZone." << endl;
                label sz = mesh.cellZones().size();
                mesh.cellZones().setSize(sz+1);
                mesh.cellZones().set
                (
                    sz,
                    new cellZone
                    (
                        set.name(),             //name
                        cellLabels,             //addressing
                        sz,                     //index
                        mesh.cellZones()        //pointZoneMesh
                    )
                );
                mesh.cellZones().writeOpt() = IOobject::AUTO_WRITE;
                mesh.cellZones().instance() = mesh.facesInstance();
            }
            else
            {
                Info<< "Overwriting contents of existing cellZone " << zoneID
                    << " with that of set " << set.name() << "." << endl;
                mesh.cellZones()[zoneID] = cellLabels;
                mesh.cellZones().writeOpt() = IOobject::AUTO_WRITE;
                mesh.cellZones().instance() = mesh.facesInstance();
            }
        }
    }



    Info<< "Writing mesh." << endl;

    if (!mesh.write())
    {
        FatalErrorInFunction
            << "Failed writing polyMesh."
            << exit(FatalError);
    }

    Info<< "\nEnd\n" << endl;

    return 0;
}


// ************************************************************************* //
