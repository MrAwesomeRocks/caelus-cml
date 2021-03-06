/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright (C) 1991-2005 OpenCFD Ltd.
-------------------------------------------------------------------------------
License
    This file is based on CAELUS.

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

Application
    funkyDoCalc

Description

Contributors/Copyright:
    2011-2014 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

#include "fvCFD.hpp"

#include "CommonValueExpressionDriver.hpp"

#include "timeSelector.hpp"

#include "IFstream.hpp"

#include "printSwakVersion.hpp"

#include "AccumulationCalculation.hpp"

#include "RepositoryBase.hpp"

template <typename Type>
Ostream& writeValue(
    Ostream &o,
    Type value
) {
    for(direction i=0;i<pTraits<Type>::nComponents;i++) {
        o << ","
            << component(value,i);
    }
    return o;
}

// Yeah. Global. Bad. But convenient
fileName dataDir="";

typedef HashPtrTable<OFstream> CsvFiles;
CsvFiles csvFiles;

template <class T>
void writeData(
    CommonValueExpressionDriver &driver,
    const List<NumericAccumulationNamedEnum::accuSpecification> &accumulations,
    const Time &time,
    const word &name,
    const bool writeCsv,
    const bool writeDistributions
)
{
    if(
        ( writeCsv || writeDistributions)
        &&
        !exists(dataDir)
    ) {
        //        Info << "Creating data directory " << dataDir << endl;
        mkDir(dataDir);
    }

    bool firstTime=false;
    OStringStream header;
    OStringStream data;

    if(writeCsv) {
        if(!csvFiles.found(name)) {
            firstTime=true;
            csvFiles.insert(
                name,
                new OFstream(dataDir / name+".csv")
            );
        }
    }

    header << "Time";
    data << time.timeName();

    bool isPoint=driver.result().isPoint();

    Field<T> result(driver.getResult<T>(isPoint));

    AccumulationCalculation<T> calculator(
        result,
        isPoint,
        driver
    );
    forAll(accumulations,i) {
        const NumericAccumulationNamedEnum::accuSpecification accu=
            accumulations[i];

        T val=calculator(accu);

        if(pTraits<T>::nComponents==1) {
            header << "," << accumulations[i];
        } else {
            for(label j=0;j<pTraits<T>::nComponents;j++) {
                header << "," << accumulations[i] << "_"
                    << pTraits<T>::componentNames[j];
            }
        }
        writeValue(data,val);

        Info << " " << accu << "=" << val;
    }

    if(writeDistributions) {
        Info << " - write distribution";

        fileName toDir=dataDir
            /
            time.timeName();

        mkDir(toDir);

        calculator.distribution().write(
            toDir
            /
            name
        );
        calculator.weightedDistribution().write(
            toDir
            /
            name+"_weighted"
        );
    }
    if(writeCsv) {
        if(firstTime) {
            *csvFiles[name] << header.str().c_str() << endl;
        }
        *csvFiles[name] << data.str().c_str() << endl;
    }
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
// Main program:

int main(int argc, char *argv[])
{

    CML::timeSelector::addOptions(false);
    CML::argList::validArgs.append("expressionDict");
#   include "addRegionOption.hpp"
    argList::validOptions.insert("noDimensionChecking","");
    argList::validOptions.insert("foreignMeshesThatFollowTime",
                                  "<list of mesh names>");
    argList::validOptions.insert("writeCsv","");
    argList::validOptions.insert("writeDistributions","");
    argList::validOptions.insert("noDimensionChecking","");

#   include "setRootCase.hpp"

    printSwakVersion();

    IFstream theFile(args.args()[1]);
    dictionary theExpressions(theFile);
    wordList foreignMeshesThatFollowTime(0);

    bool globalWriteCsv=args.options().found("writeCsv");
    bool globalWriteDistributions=args.options().found("writeDistributions");

    dataDir=args.path()/fileName(args.args()[1]).name()+"_data";

    if (!args.options().found("time") && !args.options().found("latestTime")) {
        FatalErrorInFunction
            << args.executable()
            << ": time/latestTime option is required" << endl
            << exit(FatalError);
    }

    if(args.options().found("noDimensionChecking")) {
        dimensionSet::debug=0;
    }
    if(args.options().found("foreignMeshesThatFollowTime")) {
        string followMeshes(
            args.options()["foreignMeshesThatFollowTime"]
        );
        IStringStream followStream("("+followMeshes+")");
        foreignMeshesThatFollowTime=wordList(followStream);
    }

#   include "createTime.hpp"
    CML::instantList timeDirs = CML::timeSelector::select0(runTime, args);

#   include "createNamedMesh.hpp"

    forAllConstIter(IDLList<entry>, theExpressions, iter)
    {
        const dictionary &dict=iter().dict();

        CommonValueExpressionDriver::readForeignMeshInfo(dict);
    }

    forAll(timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI], timeI);

        CML::Info << "\nTime = " << runTime.timeName() << CML::endl;

        RepositoryBase::updateRepos();

        mesh.readUpdate();

        forAll(foreignMeshesThatFollowTime,i) {
            const word &name=foreignMeshesThatFollowTime[i];
            if(MeshesRepository::getRepository().hasMesh(name)) {
                Info << "Setting mesh " << name << " to current Time"
                    << endl;

                MeshesRepository::getRepository().setTime(
                    name,
                    runTime.value()
                );
            } else {
                FatalErrorInFunction
                    << "No mesh with name " << name << " declared. " << nl
                    << "Can't follow current time"
                    << endl
                    << exit(FatalError);

            }
        }

        forAllConstIter(IDLList<entry>, theExpressions, iter)
        {
            const word name=iter().keyword();

           Info << name << " : " << flush;

            const dictionary &dict=iter().dict();

            bool writeCsv=
                globalWriteCsv
                ||
                dict.lookupOrDefault<bool>("writeCsv",false);
            bool writeDistributions=
                globalWriteDistributions
                ||
                dict.lookupOrDefault<bool>("writeDistributions",false);

            autoPtr<CommonValueExpressionDriver> driver=
                CommonValueExpressionDriver::New(dict,mesh);
            wordList accumulationNames(dict.lookup("accumulations"));
            List<NumericAccumulationNamedEnum::accuSpecification> accumulations(
                NumericAccumulationNamedEnum::readAccumulations(
                    accumulationNames,
                    dict.name()
                )
            );

            driver->setSearchBehaviour(
                true,
                true,
                true         // search on disc
            );

            driver->clearVariables();
            driver->parse(
                exprString(
                    dict.lookup("expression"),
                    dict)
            );
            word rType=driver->CommonValueExpressionDriver::getResultType();

            if(rType==pTraits<scalar>::typeName) {
                writeData<scalar>(driver(),accumulations,runTime,
                                  name,writeCsv,writeDistributions);
            } else if(rType==pTraits<vector>::typeName) {
                writeData<vector>(driver(),accumulations,runTime,
                                  name,writeCsv,writeDistributions);
            } else if(rType==pTraits<tensor>::typeName) {
                writeData<tensor>(driver(),accumulations,runTime,
                                  name,writeCsv,writeDistributions);
            } else if(rType==pTraits<symmTensor>::typeName) {
                writeData<symmTensor>(driver(),accumulations,runTime,
                                  name,writeCsv,writeDistributions);
            } else if(rType==pTraits<sphericalTensor>::typeName) {
                writeData<sphericalTensor>(driver(),accumulations,runTime,
                                  name,writeCsv,writeDistributions);
            } else {
                WarningInFunction
                    << "Don't know how to handle type " << rType
                    << endl;
            }

            Info << endl;
        }
        Info << endl;
    }

    if(csvFiles.size()>0) {
        Info << csvFiles.size() << " CSV files written" << endl;
        csvFiles.clear();
    }

    Info << "End\n" << endl;

    return 0;
}


// ************************************************************************* //
