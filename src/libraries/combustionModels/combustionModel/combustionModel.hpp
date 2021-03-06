/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2018 OpenFOAM Foundation
-------------------------------------------------------------------------------
License

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
    CML::combustionModel

Description
    Base class for combustion models

SourceFiles
    combustionModel.cpp

\*---------------------------------------------------------------------------*/

#ifndef combustionModel_H
#define combustionModel_H

#include "IOdictionary.hpp"
#include "compressibleTurbulenceModel.hpp"
#include "surfaceFields.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                     Class combustionModel Declaration
\*---------------------------------------------------------------------------*/

class combustionModel
:
    public IOdictionary
{

    //- Disallow copy construct
    combustionModel(const combustionModel&);

    //- Disallow default bitwise assignment
    void operator=(const combustionModel&);

    //- Construct the base IO object
    IOobject createIOobject
    (
        basicThermo& thermo,
        const word& combustionProperties
    ) const;


protected:

    //- Reference to the mesh database
    const fvMesh& mesh_;

    //- Reference to the turbulence model
    const compressible::turbulenceModel& turb_;

    //- Dictionary of the model
    dictionary coeffs_;

    //- Model type
    const word modelType_;


public:

    //- Runtime type information
    TypeName("combustionModel");

    //- Default combustionProperties dictionary name
    static const word combustionPropertiesName;

    //- Construct from components
    combustionModel
    (
        const word& modelType,
        basicThermo& thermo,
        const compressible::turbulenceModel& turb,
        const word& combustionProperties=combustionPropertiesName
    );


    // Selectors

    //- Generic New for each of the related chemistry model
    template<class CombustionModel>
    static autoPtr<CombustionModel> New
    (
        typename CombustionModel::reactionThermo& thermo,
        const compressible::turbulenceModel& turb,
        const word& combustionProperties
    );


    //- Destructor
    virtual ~combustionModel()
    {}


    // Member Functions

    //- Return const access to the mesh database
    inline const fvMesh& mesh() const
    {
        return mesh_;
    }

    //- Return access to turbulence
    inline const compressible::turbulenceModel& turbulence() const
    {
        return turb_;
    }

    //- Return const access to rho
    inline const volScalarField& rho() const
    {
        return turbulence().rho();
    }

    //- Return const access to phi
    inline tmp<surfaceScalarField> phi() const
    {
        return turbulence().phi();
    }

    //- Return const dictionary of the model
    inline const dictionary& coeffs() const
    {
        return coeffs_;
    }

    //- Correct combustion rate
    virtual void correct() = 0;

    //- Fuel consumption rate matrix, i.e. source term for fuel equation
    virtual tmp<fvScalarMatrix> R(volScalarField& Y) const = 0;

    //- Heat release rate [kg/m/s3]
    virtual tmp<volScalarField> Qdot() const = 0;

    //- Update properties from given dictionary
    virtual bool read();

};


} // End namespace CML


// * * * * * * * * * * * * * * * * Selectors * * * * * * * * * * * * * * * * //

template<class CombustionModel>
CML::autoPtr<CombustionModel> CML::combustionModel::New
(
    typename CombustionModel::reactionThermo& thermo,
    const compressible::turbulenceModel& turb,
    const word& combustionProperties
)
{
    IOobject combIO
    (
        IOobject
        (
            thermo.phasePropertyName(combustionProperties),
            thermo.db().time().constant(),
            thermo.db(),
            IOobject::MUST_READ,
            IOobject::NO_WRITE,
            false
        )
    );

    word combModelName("none");
    if (combIO.headerOk())
    {
        IOdictionary(combIO).lookup("combustionModel") >> combModelName;
    }
    else
    {
        Info<< "Combustion model not active: "
            << thermo.phasePropertyName(combustionProperties)
            << " not found" << endl;
    }

    Info<< "Selecting combustion model " << combModelName << endl;

    const wordList cmpts2(basicThermo::splitThermoName(combModelName, 2));
    const wordList cmpts3(basicThermo::splitThermoName(combModelName, 3));
    if (cmpts2.size() == 2 || cmpts3.size() == 3)
    {
        combModelName = cmpts2.size() ? cmpts2[0] : cmpts3[0];

        WarningInFunction
            << "Template parameters are no longer required when selecting a "
            << combustionModel::typeName << ". This information is now "
            << "obtained directly from the thermodynamics. Actually selecting "
            << "combustion model " << combModelName << "." << endl;
    }

    typedef typename CombustionModel::dictionaryConstructorTable cstrTableType;
    cstrTableType* cstrTable = CombustionModel::dictionaryConstructorTablePtr_;

    const word compCombModelName =
        combModelName + '<' + CombustionModel::reactionThermo::typeName + '>';

    const word thermoCombModelName =
        combModelName + '<' + CombustionModel::reactionThermo::typeName + ','
      + thermo.thermoName() + '>';

    typename cstrTableType::iterator compCstrIter =
        cstrTable->find(compCombModelName);

    typename cstrTableType::iterator thermoCstrIter =
        cstrTable->find(thermoCombModelName);

    if (compCstrIter == cstrTable->end() && thermoCstrIter == cstrTable->end())
    {
        FatalErrorInFunction
            << "Unknown " << combustionModel::typeName << " type "
            << combModelName << endl << endl;

        const wordList names(cstrTable->toc());

        wordList thisCmpts;
        thisCmpts.append(word::null);
        thisCmpts.append(CombustionModel::reactionThermo::typeName);
        thisCmpts.append(basicThermo::splitThermoName(thermo.thermoName(), 5));

        wordList validNames;
        forAll(names, i)
        {
            wordList cmpts(basicThermo::splitThermoName(names[i], 2));
            if (cmpts.size() != 2)
            {
                cmpts = basicThermo::splitThermoName(names[i], 7);
            }

            bool isValid = true;
            for (label i = 1; i < cmpts.size() && isValid; ++ i)
            {
                isValid = isValid && cmpts[i] == thisCmpts[i];
            }

            if (isValid)
            {
                validNames.append(cmpts[0]);
            }
        }

        FatalErrorInFunction
            << "Valid " << combustionModel::typeName << " types for this "
            << "thermodynamic model are:" << endl << validNames << endl;

        List<wordList> validCmpts2, validCmpts7;
        validCmpts2.append(wordList(2, word::null));
        validCmpts2[0][0] = combustionModel::typeName;
        validCmpts2[0][1] = "reactionThermo";
        validCmpts7.append(wordList(7, word::null));
        validCmpts7[0][0] = combustionModel::typeName;
        validCmpts7[0][1] = "reactionThermo";
        validCmpts7[0][2] = "transport";
        validCmpts7[0][3] = "thermo";
        validCmpts7[0][4] = "equationOfState";
        validCmpts7[0][5] = "specie";
        validCmpts7[0][6] = "energy";
        forAll(names, i)
        {
            const wordList cmpts2(basicThermo::splitThermoName(names[i], 2));
            const wordList cmpts7(basicThermo::splitThermoName(names[i], 7));
            if (cmpts2.size() == 2)
            {
                validCmpts2.append(cmpts2);
            }
            if (cmpts7.size() == 7)
            {
                validCmpts7.append(cmpts7);
            }
        }

        FatalErrorInFunction
            << "All " << validCmpts2[0][0] << '/' << validCmpts2[0][1]
            << " combinations are:" << endl << endl;
        printTable(validCmpts2, FatalErrorInFunction);

        FatalErrorInFunction << endl;

        FatalErrorInFunction
            << "All " << validCmpts7[0][0] << '/' << validCmpts7[0][1]
            << "/thermoPhysics combinations are:" << endl << endl;
        printTable(validCmpts7, FatalErrorInFunction);

        FatalErrorInFunction << exit(FatalError);
    }

    return autoPtr<CombustionModel>
    (
        thermoCstrIter != cstrTable->end()
      ? thermoCstrIter()(combModelName, thermo, turb, combustionProperties)
      : compCstrIter()(combModelName, thermo, turb, combustionProperties)
    );
}


#endif
