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

Description
    Writes the mesh in fpma format readable by AVL's CfdWM

Author: Franjo Juretic (franjo.juretic@c-fields.com)

\*---------------------------------------------------------------------------*/

#include "argList.hpp"
#include "polyMeshGenModifier.hpp"
#include "writeMeshFPMA.hpp"

using namespace CML;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
#   include "setRootCase.hpp"
#   include "createTime.hpp"

    polyMeshGen pmg(runTime);
    pmg.read();
    
    if( Pstream::parRun() )
    {
        polyMeshGenModifier(pmg).addBufferCells();
        createFIRESelections(pmg);
    }
    
    writeMeshFPMA(pmg, "convertedMesh");
    
    Info << "End\n" << endl;
    return 0;
}

// ************************************************************************* //
