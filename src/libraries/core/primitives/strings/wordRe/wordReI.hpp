/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2016 OpenFOAM Foundation
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

// * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * * //

inline bool CML::wordRe::meta(char c)
{
    return regExp::meta(c);
}


inline bool CML::wordRe::isPattern(const string& str)
{
    return string::meta<regExp>(str);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

inline CML::wordRe::wordRe()
:
    word(),
    re_()
{}


inline CML::wordRe::wordRe(const wordRe& str)
:
    word(str),
    re_()
{
    if (str.isPattern())
    {
        compile();
    }
}


inline CML::wordRe::wordRe(const word& str)
:
    word(str),
    re_()
{}


inline CML::wordRe::wordRe(const keyType& str)
:
    word(str, false),
    re_()
{
    if (str.isPattern())
    {
        compile();
    }
}


inline CML::wordRe::wordRe(const keyType& str, const compOption opt)
:
    word(str, false),
    re_()
{
    if (str.isPattern())
    {
        compile(opt);
    }
}


inline CML::wordRe::wordRe(const char* str, const compOption opt)
:
    word(str, false),
    re_()
{
    compile(opt);
}


inline CML::wordRe::wordRe(const string& str, const compOption opt)
:
    word(str, false),
    re_()
{
    compile(opt);
}


inline CML::wordRe::wordRe(const std::string& str, const compOption opt)
:
    word(str, false),
    re_()
{
    compile(opt);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

inline bool CML::wordRe::isPattern() const
{
    return re_.exists();
}


inline bool CML::wordRe::compile(const compOption opt) const
{
    bool doCompile = false;

    if (opt & wordRe::REGEXP)
    {
        doCompile = true;
    }
    else if (opt & wordRe::DETECT)
    {
        if (string::meta<regExp>(*this) || !string::valid<word>(*this))
        {
            doCompile = true;
        }
    }
    else if (opt & wordRe::NOCASE)
    {
        doCompile = true;
    }


    if (doCompile)
    {
        re_.set(*this, (opt & wordRe::NOCASE));
    }
    else
    {
        re_.clear();
    }

    return re_.exists();
}


inline bool CML::wordRe::compile() const
{
    re_ = *this;
    return re_.exists();
}


inline bool CML::wordRe::recompile() const
{
    if (re_.exists())
    {
        re_ = *this;
    }

    return re_.exists();
}


inline void CML::wordRe::uncompile(const bool doStripInvalid) const
{
    if (re_.clear())
    {
        // skip stripping unless debug is active to avoid costly operations
        if (word::debug && doStripInvalid)
        {
            string::stripInvalid<word>
            (
                const_cast<word&>(static_cast<const word&>(*this))
            );
        }
    }
}


inline void CML::wordRe::clear()
{
    word::clear();
    re_.clear();
}


inline bool CML::wordRe::match(const std::string& str, bool literalMatch) const
{
    if (literalMatch || !re_.exists())
    {
        // check as string
        return (str == *this);
    }
    else
    {
        // check as regex
        return re_.match(str);
    }
}


inline CML::string CML::wordRe::quotemeta() const
{
    return string::quotemeta<regExp>(*this);
}


inline void CML::wordRe::set(const std::string& str, const compOption opt)
{
    string::operator=(str);
    compile(opt);
}


inline void CML::wordRe::set(const char* str, const compOption opt)
{
    string::operator=(str);
    compile(opt);
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

inline void CML::wordRe::operator=(const wordRe& str)
{
    string::operator=(str);

    if (str.isPattern())
    {
        compile();
    }
    else
    {
        re_.clear();
    }
}


inline void CML::wordRe::operator=(const word& str)
{
    word::operator=(str);
    re_.clear();
}


inline void CML::wordRe::operator=(const keyType& str)
{
    string::operator=(str);
    if (str.isPattern())
    {
        compile();
    }
}


inline void CML::wordRe::operator=(const string& str)
{
    string::operator=(str);
    compile(DETECT);  // auto-detect regex
}


inline void CML::wordRe::operator=(const std::string& str)
{
    string::operator=(str);
    compile(DETECT);  // auto-detect regex
}


inline void CML::wordRe::operator=(const char* str)
{
    string::operator=(str);
    compile(DETECT);  // auto-detect regex
}


// ************************************************************************* //
