/*---------------------------------------------------------------------------*\
Copyright (C) 2004-6 H. Jasak
-------------------------------------------------------------------------------
License
    This file is part of Caelus.

    Caelus is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    Caelus is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Caelus.  If not, see <http://www.gnu.org/licenses/>.

Class
    BlockNoPrecon

Description
    No preconditioning

    This file was obtained from Hrvoje Jasak at the NUMAP-FOAM summer school
    in Zagreb, Croatia, 2010.

    Please report any omissions of authorship and/or copyright to
    info@appliedccm.com.au. Corrections will be made upon provision of proof.

Author
    Hrvoje Jasak, Wikki Ltd.  All rights reserved.


\*---------------------------------------------------------------------------*/

#ifndef BlockNoPrecon_HPP
#define BlockNoPrecon_HPP


namespace CML
{

template<class Type>
class BlockLduPrecon;

/*---------------------------------------------------------------------------*\
                       Class BlockNoPrecon Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class BlockNoPrecon
:
    public BlockLduPrecon<Type>
{

    //- Disallow default bitwise copy construct
    BlockNoPrecon(const BlockNoPrecon&);

    //- Disallow default bitwise assignment
    void operator=(const BlockNoPrecon&);


public:

    //- Runtime type information
    TypeName("none");

    //- Construct from components
    BlockNoPrecon
    (
        const BlockLduMatrix<Type>& matrix,
        const dictionary& dict
    )
    :
        BlockLduPrecon<Type>(matrix)
    {}

    virtual ~BlockNoPrecon()
    {}

    // Member Functions
    //- Execute preconditioning
    virtual void precondition
    (
        Field<Type>& x,
        const Field<Type>& b
    ) const
    {
        x = b;
    }

    //- Execute preconditioning
    virtual void preconditionT
    (
        Field<Type>& xT,
        const Field<Type>& bT
    ) const
    {
        precondition(xT, bT);
    }
};


} // End namespace CML
#endif
