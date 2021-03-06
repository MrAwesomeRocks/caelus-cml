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

#include "polyTopoChanger.hpp"
#include "polyMesh.hpp"
#include "polyTopoChange.hpp"
#include "Time.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace CML
{
    defineTypeNameAndDebug(polyTopoChanger, 0);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

void CML::polyTopoChanger::readModifiers()
{
    if
    (
        readOpt() == IOobject::MUST_READ
     || readOpt() == IOobject::MUST_READ_IF_MODIFIED
     || (readOpt() == IOobject::READ_IF_PRESENT && headerOk())
    )
    {
        if (readOpt() == IOobject::MUST_READ_IF_MODIFIED)
        {
            WarningInFunction
                << "Specified IOobject::MUST_READ_IF_MODIFIED but class"
                << " does not support automatic rereading."
                << endl;
        }


        PtrList<polyMeshModifier>& modifiers = *this;

        // Read modifiers
        Istream& is = readStream(typeName);

        PtrList<entry> patchEntries(is);
        modifiers.setSize(patchEntries.size());

        forAll(modifiers, modifierI)
        {
            modifiers.set
            (
                modifierI,
                polyMeshModifier::New
                (
                    patchEntries[modifierI].keyword(),
                    patchEntries[modifierI].dict(),
                    modifierI,
                    *this
                )
            );
        }

        // Check state of IOstream
        is.check
        (
            "polyTopoChanger::polyTopoChanger"
            "(const IOobject&, const polyMesh&)"
        );

        close();
    }
}


// Read constructor given IOobject and a polyMesh reference
CML::polyTopoChanger::polyTopoChanger
(
    const IOobject& io,
    polyMesh& mesh
)
:
    PtrList<polyMeshModifier>(),
    regIOobject(io),
    mesh_(mesh)
{
    readModifiers();
}


// Read constructor given IOobject and a polyMesh reference
CML::polyTopoChanger::polyTopoChanger(polyMesh& mesh)
:
    PtrList<polyMeshModifier>(),
    regIOobject
    (
        IOobject
        (
            "meshModifiers",
            mesh.time().findInstance
            (
                mesh.meshDir(),
                "meshModifiers",
                IOobject::READ_IF_PRESENT
            ),
            mesh.meshSubDir,
            mesh,
            IOobject::READ_IF_PRESENT,
            IOobject::NO_WRITE
        )
    ),
    mesh_(mesh)
{
    readModifiers();
}


// Return a list of modifier types
CML::wordList CML::polyTopoChanger::types() const
{
    const PtrList<polyMeshModifier>& modifiers = *this;

    wordList t(modifiers.size());

    forAll(modifiers, modifierI)
    {
        t[modifierI] = modifiers[modifierI].type();
    }

    return t;
}


// Return a list of modifier names
CML::wordList CML::polyTopoChanger::names() const
{
    const PtrList<polyMeshModifier>& modifiers = *this;

    wordList t(modifiers.size());

    forAll(modifiers, modifierI)
    {
        t[modifierI] = modifiers[modifierI].name();
    }

    return t;
}


// Is topology change required
bool CML::polyTopoChanger::changeTopology() const
{
    // Go through all mesh modifiers and accumulate the morphing information
    const PtrList<polyMeshModifier>& topoChanges = *this;

    bool triggerChange = false;

    forAll(topoChanges, morphI)
    {
        if (topoChanges[morphI].active())
        {
            bool curTriggerChange = topoChanges[morphI].changeTopology();

            if (debug)
            {
                Info<< "Modifier " << morphI << " named "
                    << topoChanges[morphI].name();

                if (curTriggerChange)
                {
                    Info<< " morphing" << endl;
                }
                else
                {
                    Info<< " unchanged" << endl;
                }
            }

            triggerChange = triggerChange || curTriggerChange;
        }
        else
        {
            if (debug)
            {
                Info<< "Modifier " << morphI  << " named "
                    << topoChanges[morphI].name() << " inactive" << endl;
            }
        }

    }

    return triggerChange;
}


// Return topology change request
CML::autoPtr<CML::polyTopoChange>
CML::polyTopoChanger::topoChangeRequest() const
{
    // Collect changes from all modifiers
    const PtrList<polyMeshModifier>& topoChanges = *this;

    polyTopoChange* refPtr(new polyTopoChange(mesh()));
    polyTopoChange& ref = *refPtr;

    forAll(topoChanges, morphI)
    {
        if (topoChanges[morphI].active())
        {
            topoChanges[morphI].setRefinement(ref);
        }
    }

    return autoPtr<polyTopoChange>(refPtr);
}


// Correct polyTopoChanger after moving points
void CML::polyTopoChanger::modifyMotionPoints(pointField& p) const
{
    const PtrList<polyMeshModifier>& topoChanges = *this;

    forAll(topoChanges, morphI)
    {
        if (topoChanges[morphI].active())
        {
            topoChanges[morphI].modifyMotionPoints(p);
        }
    }
}


// Force recalculation of locally stored data on topological change
void CML::polyTopoChanger::update(const mapPolyMesh& m)
{
    // Go through all mesh modifiers and accumulate the morphing information
    PtrList<polyMeshModifier>& topoChanges = *this;

    forAll(topoChanges, morphI)
    {
        topoChanges[morphI].updateMesh(m);
    }

    // Force the mesh modifiers to auto-write.  This allows us to
    // preserve the current state of modifiers corresponding with
    // the mesh.
    writeOpt() = IOobject::AUTO_WRITE;
    instance() = mesh_.time().timeName();
}


CML::autoPtr<CML::mapPolyMesh> CML::polyTopoChanger::changeMesh
(
    const bool inflate,
    const bool syncParallel,
    const bool orderCells,
    const bool orderPoints
)
{
    if (changeTopology())
    {
        autoPtr<polyTopoChange> ref = topoChangeRequest();

        autoPtr<mapPolyMesh> topoChangeMap = ref().changeMesh
        (
            mesh_,
            inflate,
            syncParallel,
            orderCells,
            orderPoints
        );

        update(topoChangeMap());
        mesh_.updateMesh(topoChangeMap());
        return topoChangeMap;
    }
    else
    {
        return autoPtr<mapPolyMesh>(nullptr);
    }
}


// Add mesh modifiers to the morph engine
void CML::polyTopoChanger::addTopologyModifiers
(
    const List<polyMeshModifier*>& tm
)
{
    setSize(tm.size());

    // Copy the patch pointers
    forAll(tm, tmI)
    {
        if (tm[tmI]->topoChanger() != *this)
        {
            FatalErrorInFunction
                << "Mesh modifier created with different mesh reference."
                << abort(FatalError);
        }
        set(tmI, tm[tmI]);
    }

    writeOpt() = IOobject::AUTO_WRITE;
}


CML::label CML::polyTopoChanger::findModifierID
(
    const word& modName
) const
{
    const PtrList<polyMeshModifier>& topoChanges = *this;

    forAll(topoChanges, morphI)
    {
        if (topoChanges[morphI].name() == modName)
        {
            return morphI;
        }
    }

    // Modifier not found
    if (debug)
    {
        Info<< "label polyTopoChanger::::findModifierID(const word& "
            << "modName) const"
            << "Modifier named " << modName << " not found.  "
            << "List of available modifier names: " << names() << endl;
    }

    // Not found, return -1
    return -1;
}


// writeData member function required by regIOobject
bool CML::polyTopoChanger::writeData(Ostream& os) const
{
    os << *this;
    return os.good();
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

bool CML::polyTopoChanger::operator!=(const polyTopoChanger& me) const
{
    return &me != this;
}


bool CML::polyTopoChanger::operator==(const polyTopoChanger& me) const
{
    return &me == this;
}


// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

CML::Ostream& CML::operator<<(Ostream& os, const polyTopoChanger& mme)
{
    os  << mme.size() << nl << token::BEGIN_LIST;

    forAll(mme, mmeI)
    {
        mme[mmeI].writeDict(os);
    }

    os  << token::END_LIST;

    return os;
}


// ************************************************************************* //
