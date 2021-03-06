/*---------------------------------------------------------------------------*\
Copyright (C) 2011 OpenFOAM Foundation
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

InClass
    CML::dummyTransform

Description
    Dummy transform to be used with syncTools.

\*---------------------------------------------------------------------------*/

#ifndef dummyTransform_H
#define dummyTransform_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{

/*---------------------------------------------------------------------------*\
                           Class dummyTransform Declaration
\*---------------------------------------------------------------------------*/

class dummyTransform
{
public:
    template<class T>
    void operator()
    (
        const vectorTensorTransform& vt,
        const bool forward,
        List<T>& fld
    ) const
    {};
    template<class T>
    void operator()(const coupledPolyPatch& cpp, Field<T>& fld) const
    {}
    template<class T, template<class> class Container>
    void operator()(const coupledPolyPatch& cpp, Container<T>& map) const
    {}
};


template<class T>
class pTraits<List<T> >
:
    public List<T>
{
public:
    typedef label cmptType;

    pTraits(Istream& is)
    :
        List<T>(is)
    {}
};

template<class T>
class pTraits<UList<T> >
:
    public UList<T>
{
public:
    typedef label cmptType;

    pTraits(Istream& is)
    :
        UList<T>(is)
    {}
};

template<class T>
class pTraits<Field<T> >
:
    public Field<T>
{
public:
    typedef label cmptType;

    pTraits(Istream& is)
    :
        Field<T>(is)
    {}
};

template<>
class pTraits<face>
:
    public face
{
public:
    typedef label cmptType;

    pTraits(Istream& is)
    :
        face(is)
    {}
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
