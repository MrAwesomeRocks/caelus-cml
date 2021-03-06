/*---------------------------------------------------------------------------*\
Copyright  Niels Gjøl Jacobsen, Technical University of Denmark.
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

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

#include "bichromaticFirstProperties.hpp"
#include "addToRunTimeSelectionTable.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

defineTypeNameAndDebug(bichromaticFirstProperties, 0);
addToRunTimeSelectionTable
(
    setWaveProperties,
    bichromaticFirstProperties,
    setWaveProperties
);

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //


bichromaticFirstProperties::bichromaticFirstProperties
(
    const Time& rT,
    dictionary& dict,
    bool write
)
:
    setWaveProperties(rT, dict, write),
    sfp1_(rT, dict, write, "1"),
    sfp2_(rT, dict, write, "2")
{
    Info << "\nConstructing: " << this->type() << endl;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


void bichromaticFirstProperties::set(Ostream& os)
{
    scalar k1 = sfp1_.linearWaveNumber();
    scalar k2 = sfp2_.linearWaveNumber();

    // Write the beginning of the sub-dictionary
    writeBeginning(os);

    // Write the already given parameters
    writeGiven(os, "waveType");

    if (dict_.found("Tsoft"))
    {
        writeGiven(os, "Tsoft");
    }

    writeGiven(os, "depth");

    writeGiven(os, "period1");
    writeGiven(os, "period2");

    writeGiven(os, "direction1");
    writeGiven(os, "direction2");

    writeGiven(os, "height1");
    writeGiven(os, "height2");

    writeGiven(os, "phi1");
    writeGiven(os, "phi2");

    if (write_)
    {
        vector direction1(vector(dict_.lookup("direction1")));
        vector direction2(vector(dict_.lookup("direction2")));

        direction1 /= CML::mag(direction1);
        direction2 /= CML::mag(direction2);

        direction1 *= k1;
        direction2 *= k2;

        writeDerived(os, "waveNumber1", direction1);
        writeDerived(os, "waveNumber2", direction2);

        writeDerived(os, "omega1", sfp1_.omega());
        writeDerived(os, "omega2", sfp2_.omega());
    }

    // Write the relaxation zone
    writeRelaxationZone(os);

    // Write the closing bracket
    writeEnding(os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// ************************************************************************* //
