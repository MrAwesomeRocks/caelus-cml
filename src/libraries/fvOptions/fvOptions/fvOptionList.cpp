/*---------------------------------------------------------------------------*\
Copyright (C) 2011 OpenFOAM Foundation
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Caelus is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "fvOptionList.hpp"
#include "surfaceFields.hpp"

namespace CML
{
namespace fv
{
    defineTypeNameAndDebug(optionList, 0);
}
}


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

const CML::dictionary& CML::fv::optionList::optionsDict
(
    const dictionary& dict
) const
{
    if (dict.found("options"))
    {
        return dict.subDict("options");
    }
    else
    {
        return dict;
    }
}


bool CML::fv::optionList::readOptions(const dictionary& dict)
{
    checkTimeIndex_ = mesh_.time().timeIndex() + 2;

    bool allOk = true;
    forAll(*this, i)
    {
        option& bs = this->operator[](i);
        bool ok = bs.read(dict.subDict(bs.name()));
        allOk = (allOk && ok);
    }
    return allOk;
}


void CML::fv::optionList::checkApplied() const
{
    if (mesh_.time().timeIndex() == checkTimeIndex_)
    {
        forAll(*this, i)
        {
            const option& bs = this->operator[](i);
            bs.checkApplied();
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::fv::optionList::optionList(const fvMesh& mesh, const dictionary& dict)
:
    PtrList<option>(),
    mesh_(mesh),
    checkTimeIndex_(mesh_.time().startTimeIndex() + 2)
{
    reset(optionsDict(dict));
}


CML::fv::optionList::optionList(const fvMesh& mesh)
:
    PtrList<option>(),
    mesh_(mesh),
    checkTimeIndex_(mesh_.time().startTimeIndex() + 2)
{}


void CML::fv::optionList::reset(const dictionary& dict)
{
    label count = 0;
    forAllConstIter(dictionary, dict, iter)
    {
        // safety:
        if (iter().isDict())
        {
            count ++;
        }
    }

    this->setSize(count);
    label i = 0;
    forAllConstIter(dictionary, dict, iter)
    {
        if (iter().isDict())
        {
            const word& name = iter().keyword();
            const dictionary& sourceDict = iter().dict();

            this->set
            (
                i++,
                option::New(name, sourceDict, mesh_)
            );
        }
    }
}


void CML::fv::optionList::makeRelative(surfaceScalarField& phi) const
{
    forAll(*this, i)
    {
        this->operator[](i).makeRelative(phi);
    }
}


void CML::fv::optionList::makeRelative
(
    const surfaceScalarField& rho,
    surfaceScalarField& phi
) const
{
    forAll(*this, i)
    {
        this->operator[](i).makeRelative(rho, phi);
    }
}


CML::tmp<CML::FieldField<CML::fvsPatchField, CML::scalar> >
CML::fv::optionList::relative
(
    const tmp<FieldField<fvsPatchField, scalar> >& phi
) const
{
    tmp<FieldField<fvsPatchField, scalar> > rphi(phi.ptr());

    forAll(*this, i)
    {
        operator[](i).makeRelative(rphi());
    }

    return rphi;
}


void CML::fv::optionList::makeAbsolute(surfaceScalarField& phi) const
{
    forAll(*this, i)
    {
        this->operator[](i).makeAbsolute(phi);
    }
}


void CML::fv::optionList::makeAbsolute
(
    const surfaceScalarField& rho,
    surfaceScalarField& phi
) const
{
    forAll(*this, i)
    {
        this->operator[](i).makeAbsolute(rho, phi);
    }
}


bool CML::fv::optionList::read(const dictionary& dict)
{
    return readOptions(optionsDict(dict));
}


bool CML::fv::optionList::writeData(Ostream& os) const
{
    // Write list contents
    forAll(*this, i)
    {
        os  << nl;
        this->operator[](i).writeData(os);
    }

    // Check state of IOstream
    return os.good();
}


namespace CML
{
    Ostream& operator<<
    (
        Ostream& os,
        const fv::optionList& options
    )
    {
        options.writeData(os);
        return os;
    }
}



