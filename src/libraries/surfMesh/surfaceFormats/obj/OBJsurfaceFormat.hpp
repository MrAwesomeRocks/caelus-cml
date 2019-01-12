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
    CML::fileFormats::OBJsurfaceFormat

Description
    Provide a means of reading/writing Alias/Wavefront OBJ format.

    Does not handle negative face indices.


\*---------------------------------------------------------------------------*/

#ifndef OBJsurfaceFormat_H
#define OBJsurfaceFormat_H

#include "MeshedSurface.hpp"
#include "MeshedSurfaceProxy.hpp"
#include "UnsortedMeshedSurface.hpp"
#include "clock.hpp"
#include "IFstream.hpp"
#include "IStringStream.hpp"
#include "Ostream.hpp"
#include "OFstream.hpp"
#include "ListOps.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
namespace fileFormats
{

/*---------------------------------------------------------------------------*\
                      Class OBJsurfaceFormat Declaration
\*---------------------------------------------------------------------------*/

template<class Face>
class OBJsurfaceFormat
:
    public MeshedSurface<Face>
{
    // Private Member Functions

        //- Disallow default bitwise copy construct
        OBJsurfaceFormat(const OBJsurfaceFormat<Face>&);

        //- Disallow default bitwise assignment
        void operator=(const OBJsurfaceFormat<Face>&);


public:

    // Constructors

        //- Construct from file name
        OBJsurfaceFormat(const fileName&);


    // Selectors

        //- Read file and return surface
        static autoPtr<MeshedSurface<Face> > New(const fileName& name)
        {
            return autoPtr<MeshedSurface<Face> >
            (
                new OBJsurfaceFormat<Face>(name)
            );
        }


    //- Destructor
    virtual ~OBJsurfaceFormat()
    {}


    // Member Functions

        //- Write surface mesh components by proxy
        static void write(const fileName&, const MeshedSurfaceProxy<Face>&);

        //- Read from file
        virtual bool read(const fileName&);

        //- Write object file
        virtual void write(const fileName& name) const
        {
            write(name, MeshedSurfaceProxy<Face>(*this));
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace fileFormats
} // End namespace CML


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Face>
CML::fileFormats::OBJsurfaceFormat<Face>::OBJsurfaceFormat
(
    const fileName& filename
)
{
    read(filename);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Face>
bool CML::fileFormats::OBJsurfaceFormat<Face>::read
(
    const fileName& filename
)
{
    const bool mustTriangulate = this->isTri();
    this->clear();

    IFstream is(filename);
    if (!is.good())
    {
        FatalErrorInFunction
            << "Cannot read file " << filename
            << exit(FatalError);
    }

    // assume that the groups are not intermixed
    bool sorted = true;

    DynamicList<point> dynPoints;
    DynamicList<Face>  dynFaces;
    DynamicList<label> dynZones;
    DynamicList<word>  dynNames;
    DynamicList<label> dynSizes;
    HashTable<label>   lookup;

    // place faces without a group in zone0
    label zoneI = 0;
    lookup.insert("zone0", zoneI);
    dynNames.append("zone0");
    dynSizes.append(0);

    while (is.good())
    {
        string line = this->getLineNoComment(is);

        // handle continuations
        if (line[line.size()-1] == '\\')
        {
            line.substr(0, line.size()-1);
            line += this->getLineNoComment(is);
        }

        // Read first word
        IStringStream lineStream(line);
        word cmd;
        lineStream >> cmd;

        if (cmd == "v")
        {
            scalar x, y, z;
            lineStream >> x >> y >> z;
            dynPoints.append(point(x, y, z));
        }
        else if (cmd == "g")
        {
            word name;
            lineStream >> name;

            HashTable<label>::const_iterator fnd = lookup.find(name);
            if (fnd != lookup.end())
            {
                if (zoneI != fnd())
                {
                    // group appeared out of order
                    sorted = false;
                }
                zoneI = fnd();
            }
            else
            {
                zoneI = dynSizes.size();
                lookup.insert(name, zoneI);
                dynNames.append(name);
                dynSizes.append(0);
            }
        }
        else if (cmd == "f")
        {
            DynamicList<label> dynVertices;

            // Assume 'f' is followed by space.
            string::size_type endNum = 1;

            while (true)
            {
                string::size_type startNum =
                    line.find_first_not_of(' ', endNum);

                if (startNum == string::npos)
                {
                    break;
                }

                endNum = line.find(' ', startNum);

                string vertexSpec;
                if (endNum != string::npos)
                {
                    vertexSpec = line.substr(startNum, endNum-startNum);
                }
                else
                {
                    vertexSpec = line.substr(startNum, line.size() - startNum);
                }

                string::size_type slashPos = vertexSpec.find('/');

                label vertI = 0;
                if (slashPos != string::npos)
                {
                    IStringStream intStream(vertexSpec.substr(0, slashPos));

                    intStream >> vertI;
                }
                else
                {
                    IStringStream intStream(vertexSpec);

                    intStream >> vertI;
                }
                dynVertices.append(vertI - 1);
            }
            dynVertices.shrink();

            labelUList& f = static_cast<labelUList&>(dynVertices);

            if (mustTriangulate && f.size() > 3)
            {
                // simple face triangulation about f[0]
                // points may be incomplete
                for (label fp1 = 1; fp1 < f.size() - 1; fp1++)
                {
                    label fp2 = f.fcIndex(fp1);

                    dynFaces.append(triFace(f[0], f[fp1], f[fp2]));
                    dynZones.append(zoneI);
                    dynSizes[zoneI]++;
                }
            }
            else
            {
                dynFaces.append(Face(f));
                dynZones.append(zoneI);
                dynSizes[zoneI]++;
            }
        }
    }


    // transfer to normal lists
    this->storedPoints().transfer(dynPoints);

    this->sortFacesAndStore(dynFaces.xfer(), dynZones.xfer(), sorted);

    // add zones, culling empty ones
    this->addZones(dynSizes, dynNames, true);
    return true;
}


template<class Face>
void CML::fileFormats::OBJsurfaceFormat<Face>::write
(
    const fileName& filename,
    const MeshedSurfaceProxy<Face>& surf
)
{
    const pointField& pointLst = surf.points();
    const List<Face>&  faceLst = surf.faces();
    const List<label>& faceMap = surf.faceMap();

    // for no zones, suppress the group name
    const List<surfZone>& zones =
    (
        surf.surfZones().empty()
      ? surfaceFormatsCore::oneZone(faceLst, "")
      : surf.surfZones()
    );

    const bool useFaceMap = (surf.useFaceMap() && zones.size() > 1);

    OFstream os(filename);
    if (!os.good())
    {
        FatalErrorInFunction
            << "Cannot open file for writing " << filename
            << exit(FatalError);
    }


    os  << "# Wavefront OBJ file written " << clock::dateTime().c_str() << nl
        << "o " << os.name().lessExt().name() << nl
        << nl
        << "# points : " << pointLst.size() << nl
        << "# faces  : " << faceLst.size() << nl
        << "# zones  : " << zones.size() << nl;

    // Print zone names as comment
    forAll(zones, zoneI)
    {
        os  << "#   " << zoneI << "  " << zones[zoneI].name()
            << "  (nFaces: " << zones[zoneI].size() << ")" << nl;
    }

    os  << nl
        << "# <points count=\"" << pointLst.size() << "\">" << nl;

    // Write vertex coords
    forAll(pointLst, ptI)
    {
        const point& pt = pointLst[ptI];

        os  << "v " << pt.x() << ' '  << pt.y() << ' '  << pt.z() << nl;
    }

    os  << "# </points>" << nl
        << nl
        << "# <faces count=\"" << faceLst.size() << "\">" << endl;


    label faceIndex = 0;
    forAll(zones, zoneI)
    {
        const surfZone& zone = zones[zoneI];

        if (zone.name().size())
        {
            os << "g " << zone.name() << endl;
        }

        if (useFaceMap)
        {
            forAll(zone, localFaceI)
            {
                const Face& f = faceLst[faceMap[faceIndex++]];

                os << 'f';
                forAll(f, fp)
                {
                    os << ' ' << f[fp] + 1;
                }
                os << endl;
            }
        }
        else
        {
            forAll(zone, localFaceI)
            {
                const Face& f = faceLst[faceIndex++];

                os << 'f';
                forAll(f, fp)
                {
                    os << ' ' << f[fp] + 1;
                }
                os << endl;
            }
        }
    }
    os << "# </faces>" << endl;
}



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
