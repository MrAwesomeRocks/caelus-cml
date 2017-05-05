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

Class
    CML::CSV

Description
    Templated CSV container data entry.  Reference column is always a scalar,
    e.g. time

    \verbatim
        <entryName>   csvFile;
        csvFileCoeffs
        {
            hasHeaderLine   true;
            refColumn       0;          // reference column index
            componentColumns (1 2 3);   // component column indices
            separator       ",";        // optional (defaults to ",")
            fileName        "fileXYZ";  // name of csv data file
            outOfBounds     clamp;      // optional out-of-bounds handling
        }
    \endverbatim

SourceFiles
    CSV.cpp

\*---------------------------------------------------------------------------*/

#ifndef CSV_H
#define CSV_H

#include "DataEntry.hpp"
#include "TableBase.hpp"
#include "Tuple2.hpp"
#include "labelList.hpp"
#include "ISstream.hpp"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

template<class Type>
class CSV;

template<class Type>
Ostream& operator<<
(
    Ostream&,
    const CSV<Type>&
);

/*---------------------------------------------------------------------------*\
                           Class CSV Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class CSV
:
    public DataEntry<Type>,
    public TableBase<Type>
{
    // Private data

        //- Coefficients dictionary (for convenience on reading)
        dictionary coeffs_;

        //- Does the file have a header line?
        bool headerLine_;

        //- Column of the time
        label refColumn_;

        //- Labels of the components
        labelList componentColumns_;

        //- Separator character
        char separator_;

        //- File name for csv table (optional)
        fileName fName_;


    // Private Member Functions

        //- Read csv data table
        void read();

        //- Read the next value from the splitted string
        Type readValue(const List<string>&);

        //- Disallow default bitwise assignment
        void operator=(const CSV<Type>&);


public:

    //- Runtime type information
    TypeName("csvFile");


    // Constructors

        //- Construct from entry name and Istream
        CSV(const word& entryName, const dictionary& dict);

        //- Copy constructor
        CSV(const CSV<Type>& tbl);

        //- Construct and return a clone
        virtual tmp<DataEntry<Type> > clone() const
        {
            return tmp<DataEntry<Type> >(new CSV<Type>(*this));
        }


    //- Destructor
    virtual ~CSV();


    // Member Functions

        // Manipulation

            //- Convert time
            virtual void convertTimeBase(const Time& t)
            {
                TableBase<Type>::convertTimeBase(t);
            }


        // Evaluation

            //- Return Table value
            virtual Type value(const scalar x) const
            {
                return TableBase<Type>::value(x);
            }

            //- Integrate between two (scalar) values
            virtual Type integrate(const scalar x1, const scalar x2) const
            {
                return TableBase<Type>::integrate(x1, x2);
            }


    // I/O

        //- Ostream Operator
        friend Ostream& operator<< <Type>
        (
            Ostream& os,
            const CSV<Type>& cnst
        );

        //- Write in dictionary format
        virtual void writeData(Ostream& os) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "DynamicList.hpp"
#include "IFstream.hpp"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

namespace CML
{
    // doesn't recognize specialization otherwise
    template<>
    scalar CSV<scalar>::readValue(const List<string>& splitted)
    {
        if (componentColumns_[0] >= splitted.size())
        {
            FatalErrorIn("CSV<scalar>::readValue(const List<string>&)")
                << "No column " << componentColumns_[0] << " in "
                << splitted << endl
                << exit(FatalError);
        }

        return readScalar(IStringStream(splitted[componentColumns_[0]])());
    }


    template<class Type>
    Type CSV<Type>::readValue(const List<string>& splitted)
    {
        Type result;

        for (label i = 0; i < pTraits<Type>::nComponents; i++)
        {
            if (componentColumns_[i] >= splitted.size())
            {
                FatalErrorIn("CSV<Type>::readValue(const List<string>&)")
                    << "No column " << componentColumns_[i] << " in "
                    << splitted << endl
                    << exit(FatalError);
            }

            result[i] =
                readScalar(IStringStream(splitted[componentColumns_[i]])());
        }

        return result;
    }
}


template<class Type>
void CML::CSV<Type>::read()
{
    fileName expandedFile(fName_);
    IFstream is(expandedFile.expand());

    if (!is.good())
    {
        FatalIOErrorIn("CSV<Type>::read()", is)
            << "Cannot open CSV file for reading."
            << exit(FatalIOError);
    }

    DynamicList<Tuple2<scalar, Type> > values;

    // skip header
    if (headerLine_)
    {
        string line;
        is.getLine(line);
    }

    // read data
    while (is.good())
    {
        string line;
        is.getLine(line);

        DynamicList<string> splitted;

        std::size_t pos = 0;
        while (pos != std::string::npos)
        {
            std::size_t nPos = line.find(separator_, pos);

            if (nPos == std::string::npos)
            {
                splitted.append(line.substr(pos));
                pos = nPos;
            }
            else
            {
                splitted.append(line.substr(pos, nPos - pos));
                pos = nPos + 1;
            }
        }

        if (splitted.size() <= 1)
        {
            break;
        }

        scalar x = readScalar(IStringStream(splitted[refColumn_])());
        Type value = readValue(splitted);

        values.append(Tuple2<scalar,Type>(x, value));
    }

    this->table_.transfer(values);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
CML::CSV<Type>::CSV(const word& entryName, const dictionary& dict)
:
    DataEntry<Type>(entryName),
    TableBase<Type>(entryName, dict.subDict(type() + "Coeffs")),
    coeffs_(dict.subDict(type() + "Coeffs")),
    headerLine_(readBool(coeffs_.lookup("hasHeaderLine"))),
    refColumn_(readLabel(coeffs_.lookup("refColumn"))),
    componentColumns_(coeffs_.lookup("componentColumns")),
    separator_(coeffs_.lookupOrDefault<string>("separator", string(","))[0]),
    fName_(coeffs_.lookup("fileName"))
{
    if (componentColumns_.size() != pTraits<Type>::nComponents)
    {
        FatalErrorIn("CML::CSV<Type>::CSV(const word&, Istream&)")
            << componentColumns_ << " does not have the expected length of "
            << pTraits<Type>::nComponents << endl
            << exit(FatalError);
    }

    read();

    TableBase<Type>::check();
}


template<class Type>
CML::CSV<Type>::CSV(const CSV<Type>& tbl)
:
    DataEntry<Type>(tbl),
    TableBase<Type>(tbl),
    headerLine_(tbl.headerLine_),
    refColumn_(tbl.refColumn_),
    componentColumns_(tbl.componentColumns_),
    separator_(tbl.separator_),
    fName_(tbl.fName_)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class Type>
CML::CSV<Type>::~CSV()
{}


// * * * * * * * * * * * * * *  IOStream operators * * * * * * * * * * * * * //

#include "DataEntry.hpp"

// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

template<class Type>
CML::Ostream& CML::operator<<
(
    Ostream& os,
    const CSV<Type>& tbl
)
{
    if (os.format() == IOstream::ASCII)
    {
        os  << static_cast<const DataEntry<Type>& >(tbl)
            << token::SPACE << tbl.headerLine_
            << token::SPACE << tbl.timeColumn_
            << token::SPACE << tbl.componentColumns_
            << token::SPACE << tbl.separator_
            << token::SPACE << tbl.fileName_;
    }
    else
    {
        os  << static_cast<const DataEntry<Type>& >(tbl);
    }

    // Check state of Ostream
    os.check
    (
        "Ostream& operator<<(Ostream&, const CSV<Type>&)"
    );

    return os;
}


template<class Type>
void CML::CSV<Type>::writeData(Ostream& os) const
{
    DataEntry<Type>::writeData(os);
    os  << token::END_STATEMENT << nl;
    os  << indent << word(type() + "Coeffs") << nl;
    os  << indent << token::BEGIN_BLOCK << incrIndent << nl;

    // Note: for TableBase write the dictionary entries it needs but not
    // the values themselves
    TableBase<Type>::writeEntries(os);

    os.writeKeyword("hasHeaderLine") << headerLine_ << token::END_STATEMENT
        << nl;
    os.writeKeyword("refColumn") << refColumn_ << token::END_STATEMENT << nl;

    // Force writing labelList in ascii
    os.writeKeyword("componentColumns");
    if (os.format() == IOstream::BINARY)
    {
        os.format(IOstream::ASCII);
        os  << componentColumns_;
        os.format(IOstream::BINARY);
    }
    else
    {
        os  << componentColumns_;
    }
    os  << token::END_STATEMENT << nl;

    os.writeKeyword("separator") << string(separator_)
        << token::END_STATEMENT << nl;
    os.writeKeyword("fileName") << fName_ << token::END_STATEMENT << nl;
    os  << decrIndent << indent << token::END_BLOCK << endl;
}



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
