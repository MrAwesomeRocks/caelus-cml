/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2018 OpenFOAM Foundation
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

#include "dictionary.hpp"
#include "primitiveEntry.hpp"
#include "dictionaryEntry.hpp"
#include "regExp.hpp"
#include "OSHA1stream.hpp"

/* * * * * * * * * * * * * * * Static Member Data  * * * * * * * * * * * * * */


namespace CML
{
    defineTypeNameAndDebug(dictionary, 0);

    const dictionary dictionary::null;
}

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

bool CML::dictionary::findInPatterns
(
    const bool patternMatch,
    const word& Keyword,
    DLList<entry*>::const_iterator& wcLink,
    DLList<autoPtr<regExp> >::const_iterator& reLink
) const
{
    if (patternEntries_.size())
    {
        while (wcLink != patternEntries_.end())
        {
            if
            (
                patternMatch
              ? reLink()->match(Keyword)
              : wcLink()->keyword() == Keyword
            )
            {
                return true;
            }

            ++reLink;
            ++wcLink;
        }
    }

    return false;
}


bool CML::dictionary::findInPatterns
(
    const bool patternMatch,
    const word& Keyword,
    DLList<entry*>::iterator& wcLink,
    DLList<autoPtr<regExp> >::iterator& reLink
)
{
    if (patternEntries_.size())
    {
        while (wcLink != patternEntries_.end())
        {
            if
            (
                patternMatch
              ? reLink()->match(Keyword)
              : wcLink()->keyword() == Keyword
            )
            {
                return true;
            }

            ++reLink;
            ++wcLink;
        }
    }

    return false;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

CML::dictionary::dictionary()
:
    parent_(dictionary::null)
{}


CML::dictionary::dictionary(const fileName& name)
:
    dictionaryName(name),
    parent_(dictionary::null)
{}


CML::dictionary::dictionary
(
    const dictionary& parentDict,
    const dictionary& dict
)
:
    dictionaryName(dict.name()),
    IDLList<entry>(dict, *this),
    parent_(parentDict)
{
    forAllIter(IDLList<entry>, *this, iter)
    {
        hashedEntries_.insert(iter().keyword(), &iter());

        if (iter().keyword().isPattern())
        {
            patternEntries_.insert(&iter());
            patternRegexps_.insert
            (
                autoPtr<regExp>(new regExp(iter().keyword()))
            );
        }
    }
}


CML::dictionary::dictionary
(
    const dictionary& dict
)
:
    dictionaryName(dict.name()),
    IDLList<entry>(dict, *this),
    parent_(dictionary::null)
{
    forAllIter(IDLList<entry>, *this, iter)
    {
        hashedEntries_.insert(iter().keyword(), &iter());

        if (iter().keyword().isPattern())
        {
            patternEntries_.insert(&iter());
            patternRegexps_.insert
            (
                autoPtr<regExp>(new regExp(iter().keyword()))
            );
        }
    }
}


CML::dictionary::dictionary
(
    const dictionary* dictPtr
)
:
    parent_(dictionary::null)
{
    if (dictPtr)
    {
        operator=(*dictPtr);
    }
}


CML::dictionary::dictionary
(
    const dictionary& parentDict,
    const Xfer<dictionary>& dict
)
:
    parent_(parentDict)
{
    transfer(dict());
    name() = parentDict.name() + "::" + name();
}


CML::dictionary::dictionary
(
    const Xfer<dictionary>& dict
)
:
    parent_(dictionary::null)
{
    transfer(dict());
}


CML::autoPtr<CML::dictionary> CML::dictionary::clone() const
{
    return autoPtr<dictionary>(new dictionary(*this));
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

CML::dictionary::~dictionary()
{
    // cerr<< "~dictionary() " << name() << " " << long(this) << std::endl;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const CML::dictionary& CML::dictionary::topDict() const
{
    const dictionary& p = parent();

    if (&p != this && !p.name().empty())
    {
        return p.topDict();
    }
    else
    {
        return *this;
    }
}


CML::label CML::dictionary::startLineNumber() const
{
    if (size())
    {
        return first()->startLineNumber();
    }
    else
    {
        return -1;
    }
}


CML::label CML::dictionary::endLineNumber() const
{
    if (size())
    {
        return last()->endLineNumber();
    }
    else
    {
        return -1;
    }
}


CML::SHA1Digest CML::dictionary::digest() const
{
    OSHA1stream os;

    // process entries
    forAllConstIter(IDLList<entry>, *this, iter)
    {
        os << *iter;
    }

    return os.digest();
}


bool CML::dictionary::found
(
    const word& keyword,
    bool recursive,
    bool patternMatch
) const
{
    if (hashedEntries_.found(keyword))
    {
        return true;
    }
    else
    {
        if (patternMatch && patternEntries_.size())
        {
            DLList<entry*>::const_iterator wcLink =
                patternEntries_.begin();
            DLList<autoPtr<regExp> >::const_iterator reLink =
                patternRegexps_.begin();

            // Find in patterns using regular expressions only
            if (findInPatterns(patternMatch, keyword, wcLink, reLink))
            {
                return true;
            }
        }

        if (recursive && &parent_ != &dictionary::null)
        {
            return parent_.found(keyword, recursive, patternMatch);
        }
        else
        {
            return false;
        }
    }
}


const CML::entry* CML::dictionary::lookupEntryPtr
(
    const word& keyword,
    bool recursive,
    bool patternMatch
) const
{
    HashTable<entry*>::const_iterator iter = hashedEntries_.find(keyword);

    if (iter == hashedEntries_.end())
    {
        if (patternMatch && patternEntries_.size())
        {
            DLList<entry*>::const_iterator wcLink =
                patternEntries_.begin();
            DLList<autoPtr<regExp> >::const_iterator reLink =
                patternRegexps_.begin();

            // Find in patterns using regular expressions only
            if (findInPatterns(patternMatch, keyword, wcLink, reLink))
            {
                return wcLink();
            }
        }

        if (recursive && &parent_ != &dictionary::null)
        {
            return parent_.lookupEntryPtr(keyword, recursive, patternMatch);
        }
        else
        {
            return nullptr;
        }
    }

    return iter();
}


CML::entry* CML::dictionary::lookupEntryPtr
(
    const word& keyword,
    bool recursive,
    bool patternMatch
)
{
    HashTable<entry*>::iterator iter = hashedEntries_.find(keyword);

    if (iter == hashedEntries_.end())
    {
        if (patternMatch && patternEntries_.size())
        {
            DLList<entry*>::iterator wcLink =
                patternEntries_.begin();
            DLList<autoPtr<regExp> >::iterator reLink =
                patternRegexps_.begin();

            // Find in patterns using regular expressions only
            if (findInPatterns(patternMatch, keyword, wcLink, reLink))
            {
                return wcLink();
            }
        }

        if (recursive && &parent_ != &dictionary::null)
        {
            return const_cast<dictionary&>(parent_).lookupEntryPtr
            (
                keyword,
                recursive,
                patternMatch
            );
        }
        else
        {
            return nullptr;
        }
    }

    return iter();
}


const CML::entry& CML::dictionary::lookupEntry
(
    const word& keyword,
    bool recursive,
    bool patternMatch
) const
{
    const entry* entryPtr = lookupEntryPtr(keyword, recursive, patternMatch);

    if (entryPtr == nullptr)
    {
        FatalIOErrorInFunction(*this)
            << "keyword " << keyword << " is undefined in dictionary "
            << name()
            << exit(FatalIOError);
    }

    return *entryPtr;
}


CML::ITstream& CML::dictionary::lookup
(
    const word& keyword,
    bool recursive,
    bool patternMatch
) const
{
    return lookupEntry(keyword, recursive, patternMatch).stream();
}


const CML::entry* CML::dictionary::lookupScopedEntryPtr
(
    const word& keyword,
    bool recursive,
    bool patternMatch
) const
{
    if (keyword[0] == ':')
    {
        // Go up to top level
        const dictionary* dictPtr = this;
        while (&dictPtr->parent_ != &dictionary::null)
        {
            dictPtr = &dictPtr->parent_;
        }

        // At top. Recurse to find entries
        return dictPtr->lookupScopedEntryPtr
        (
            keyword.substr(1, keyword.size()-1),
            false,
            patternMatch
        );
    }
    else
    {
        string::size_type dotPos = keyword.find('.');

        if (dotPos == string::npos)
        {
            // Non-scoped lookup
            return lookupEntryPtr(keyword, recursive, patternMatch);
        }
        else
        {
            if (dotPos == 0)
            {
                // Starting with a '.'. Go up for every 2nd '.' found

                const dictionary* dictPtr = this;

                string::size_type begVar = dotPos + 1;
                string::const_iterator iter = keyword.begin() + begVar;
                string::size_type endVar = begVar;
                while
                (
                    iter != keyword.end()
                 && *iter == '.'
                )
                {
                    ++iter;
                    ++endVar;

                    // Go to parent
                    if (&dictPtr->parent_ == &dictionary::null)
                    {
                        FatalIOErrorInFunction(*this)
                            << "No parent of current dictionary"
                            << " when searching for "
                            << keyword.substr(begVar, keyword.size()-begVar)
                            << exit(FatalIOError);
                    }
                    dictPtr = &dictPtr->parent_;
                }

                return dictPtr->lookupScopedEntryPtr
                (
                    keyword.substr(endVar),
                    false,
                    patternMatch
                );
            }
            else
            {
                // Extract the first word
                word firstWord = keyword.substr(0, dotPos);

                const entry* entPtr = lookupScopedEntryPtr
                (
                    firstWord,
                    false,          //recursive
                    patternMatch
                );

                if (!entPtr)
                {
                    FatalIOErrorInFunction(*this)
                        << "keyword " << firstWord
                        << " is undefined in dictionary "
                        << name() << endl
                        << "Valid keywords are " << keys()
                        << exit(FatalIOError);
                }

                if (entPtr->isDict())
                {
                    return entPtr->dict().lookupScopedEntryPtr
                    (
                        keyword.substr(dotPos, keyword.size()-dotPos),
                        false,
                        patternMatch
                    );
                }
                else
                {
                    return nullptr;
                }
            }
        }
    }
}


bool CML::dictionary::substituteScopedKeyword(const word& keyword)
{
    word varName = keyword(1, keyword.size()-1);

    // Lookup the variable name in the given dictionary
    const entry* ePtr = lookupScopedEntryPtr(varName, true, true);

    // If defined insert its entries into this dictionary
    if (ePtr != nullptr)
    {
        const dictionary& addDict = ePtr->dict();

        forAllConstIter(IDLList<entry>, addDict, iter)
        {
            add(iter());
        }

        return true;
    }

    return false;
}


bool CML::dictionary::isDict(const word& keyword) const
{
    // Find non-recursive with patterns
    const entry* entryPtr = lookupEntryPtr(keyword, false, true);

    if (entryPtr)
    {
        return entryPtr->isDict();
    }
    else
    {
        return false;
    }
}


const CML::dictionary* CML::dictionary::subDictPtr(const word& keyword) const
{
    const entry* entryPtr = lookupEntryPtr(keyword, false, true);

    if (entryPtr)
    {
        return &entryPtr->dict();
    }
    else
    {
        return nullptr;
    }
}


const CML::dictionary& CML::dictionary::subDict(const word& keyword) const
{
    const entry* entryPtr = lookupEntryPtr(keyword, false, true);

    if (entryPtr == nullptr)
    {
        FatalIOErrorInFunction(*this)
            << "keyword " << keyword << " is undefined in dictionary "
            << name()
            << exit(FatalIOError);
    }
    return entryPtr->dict();
}


CML::dictionary& CML::dictionary::subDict(const word& keyword)
{
    entry* entryPtr = lookupEntryPtr(keyword, false, true);

    if (entryPtr == nullptr)
    {
        FatalIOErrorInFunction(*this)
            << "keyword " << keyword << " is undefined in dictionary "
            << name()
            << exit(FatalIOError);
    }
    return entryPtr->dict();
}


CML::dictionary CML::dictionary::subOrEmptyDict
(
    const word& keyword,
    const bool mustRead
) const
{
    const entry* entryPtr = lookupEntryPtr(keyword, false, true);

    if (entryPtr == nullptr)
    {
        if (mustRead)
        {
            FatalIOErrorInFunction(*this)
                << "keyword " << keyword << " is undefined in dictionary "
                << name()
                << exit(FatalIOError);
            return entryPtr->dict();
        }
        else
        {
            return dictionary(*this, dictionary(name() + "::" + keyword));
        }
    }
    else
    {
        return entryPtr->dict();
    }
}


const CML::dictionary& CML::dictionary::optionalSubDict
(
    const word& keyword
) const
{
    const entry* entryPtr = lookupEntryPtr(keyword, false, true);

    if (entryPtr)
    {
        return entryPtr->dict();
    }
    else
    {
        return *this;
    }
}


CML::wordList CML::dictionary::toc() const
{
    wordList keys(size());

    label nKeys = 0;
    forAllConstIter(IDLList<entry>, *this, iter)
    {
        keys[nKeys++] = iter().keyword();
    }

    return keys;
}


CML::wordList CML::dictionary::sortedToc() const
{
    return hashedEntries_.sortedToc();
}


CML::List<CML::keyType> CML::dictionary::keys(bool patterns) const
{
    List<keyType> keys(size());

    label nKeys = 0;
    forAllConstIter(IDLList<entry>, *this, iter)
    {
        if (iter().keyword().isPattern() ? patterns : !patterns)
        {
            keys[nKeys++] = iter().keyword();
        }
    }
    keys.setSize(nKeys);

    return keys;
}


bool CML::dictionary::add(entry* entryPtr, bool mergeEntry)
{
    HashTable<entry*>::iterator iter = hashedEntries_.find
    (
        entryPtr->keyword()
    );

    if (mergeEntry && iter != hashedEntries_.end())
    {
        // merge dictionary with dictionary
        if (iter()->isDict() && entryPtr->isDict())
        {
            iter()->dict().merge(entryPtr->dict());
            delete entryPtr;

            return true;
        }
        else
        {
            // replace existing dictionary with entry or vice versa
            IDLList<entry>::replace(iter(), entryPtr);
            delete iter();
            hashedEntries_.erase(iter);

            if (hashedEntries_.insert(entryPtr->keyword(), entryPtr))
            {
                entryPtr->name() = name() + "::" + entryPtr->keyword();

                if (entryPtr->keyword().isPattern())
                {
                    patternEntries_.insert(entryPtr);
                    patternRegexps_.insert
                    (
                        autoPtr<regExp>(new regExp(entryPtr->keyword()))
                    );
                }

                return true;
            }
            else
            {
                IOWarningInFunction((*this))
                    << "problem replacing entry "<< entryPtr->keyword()
                    << " in dictionary " << name() << endl;

                IDLList<entry>::remove(entryPtr);
                delete entryPtr;
                return false;
            }
        }
    }

    if (hashedEntries_.insert(entryPtr->keyword(), entryPtr))
    {
        entryPtr->name() = name() + "::" + entryPtr->keyword();
        IDLList<entry>::append(entryPtr);

        if (entryPtr->keyword().isPattern())
        {
            patternEntries_.insert(entryPtr);
            patternRegexps_.insert
            (
                autoPtr<regExp>(new regExp(entryPtr->keyword()))
            );
        }

        return true;
    }
    else
    {
        IOWarningInFunction((*this))
            << "attempt to add entry "<< entryPtr->keyword()
            << " which already exists in dictionary " << name()
            << endl;

        delete entryPtr;
        return false;
    }
}


void CML::dictionary::add(const entry& e, bool mergeEntry)
{
    add(e.clone(*this).ptr(), mergeEntry);
}


void CML::dictionary::add(const keyType& k, const word& w, bool overwrite)
{
    add(new primitiveEntry(k, token(w)), overwrite);
}


void CML::dictionary::add
(
    const keyType& k,
    const CML::string& s,
    bool overwrite
)
{
    add(new primitiveEntry(k, token(s)), overwrite);
}


void CML::dictionary::add(const keyType& k, const label l, bool overwrite)
{
    add(new primitiveEntry(k, token(l)), overwrite);
}


void CML::dictionary::add(const keyType& k, const scalar s, bool overwrite)
{
    add(new primitiveEntry(k, token(s)), overwrite);
}


void CML::dictionary::add
(
    const keyType& k,
    const dictionary& d,
    bool mergeEntry
)
{
    add(new dictionaryEntry(k, *this, d), mergeEntry);
}


void CML::dictionary::set(entry* entryPtr)
{
    entry* existingPtr = lookupEntryPtr(entryPtr->keyword(), false, true);

    // Clear dictionary so merge acts like overwrite
    if (existingPtr && existingPtr->isDict())
    {
        existingPtr->dict().clear();
    }
    add(entryPtr, true);
}


void CML::dictionary::set(const entry& e)
{
    set(e.clone(*this).ptr());
}


void CML::dictionary::set(const keyType& k, const dictionary& d)
{
    set(new dictionaryEntry(k, *this, d));
}


bool CML::dictionary::remove(const word& Keyword)
{
    HashTable<entry*>::iterator iter = hashedEntries_.find(Keyword);

    if (iter != hashedEntries_.end())
    {
        // Delete from patterns first
        DLList<entry*>::iterator wcLink =
            patternEntries_.begin();
        DLList<autoPtr<regExp> >::iterator reLink =
            patternRegexps_.begin();

        // Find in pattern using exact match only
        if (findInPatterns(false, Keyword, wcLink, reLink))
        {
            patternEntries_.remove(wcLink);
            patternRegexps_.remove(reLink);
        }

        IDLList<entry>::remove(iter());
        delete iter();
        hashedEntries_.erase(iter);

        return true;
    }
    else
    {
        return false;
    }
}


bool CML::dictionary::changeKeyword
(
    const keyType& oldKeyword,
    const keyType& newKeyword,
    bool forceOverwrite
)
{
    // No change
    if (oldKeyword == newKeyword)
    {
        return false;
    }

    HashTable<entry*>::iterator iter = hashedEntries_.find(oldKeyword);

    // oldKeyword not found - do nothing
    if (iter == hashedEntries_.end())
    {
        return false;
    }

    if (iter()->keyword().isPattern())
    {
        FatalIOErrorInFunction(*this)
            << "Old keyword "<< oldKeyword
            << " is a pattern."
            << "Pattern replacement not yet implemented."
            << exit(FatalIOError);
    }


    HashTable<entry*>::iterator iter2 = hashedEntries_.find(newKeyword);

    // newKeyword already exists
    if (iter2 != hashedEntries_.end())
    {
        if (forceOverwrite)
        {
            if (iter2()->keyword().isPattern())
            {
                // Delete from patterns first
                DLList<entry*>::iterator wcLink =
                    patternEntries_.begin();
                DLList<autoPtr<regExp> >::iterator reLink =
                    patternRegexps_.begin();

                // Find in patterns using exact match only
                if (findInPatterns(false, iter2()->keyword(), wcLink, reLink))
                {
                    patternEntries_.remove(wcLink);
                    patternRegexps_.remove(reLink);
                }
            }

            IDLList<entry>::replace(iter2(), iter());
            delete iter2();
            hashedEntries_.erase(iter2);

        }
        else
        {
            IOWarningInFunction(*this)
                << "cannot rename keyword "<< oldKeyword
                << " to existing keyword " << newKeyword
                << " in dictionary " << name() << endl;
            return false;
        }
    }

    // Change name and HashTable, but leave DL-List untouched
    iter()->keyword() = newKeyword;
    iter()->name() = name() + "::" + newKeyword;
    hashedEntries_.erase(oldKeyword);
    hashedEntries_.insert(newKeyword, iter());

    if (newKeyword.isPattern())
    {
        patternEntries_.insert(iter());
        patternRegexps_.insert
        (
            autoPtr<regExp>(new regExp(newKeyword))
        );
    }

    return true;
}


bool CML::dictionary::merge(const dictionary& dict)
{
    // Check for assignment to self
    if (this == &dict)
    {
        FatalIOErrorInFunction(*this)
            << "attempted merge to self for dictionary " << name()
            << abort(FatalIOError);
    }

    bool changed = false;

    forAllConstIter(IDLList<entry>, dict, iter)
    {
        HashTable<entry*>::iterator fnd = hashedEntries_.find(iter().keyword());

        if (fnd != hashedEntries_.end())
        {
            // Recursively merge sub-dictionaries
            // TODO: merge without copying
            if (fnd()->isDict() && iter().isDict())
            {
                if (fnd()->dict().merge(iter().dict()))
                {
                    changed = true;
                }
            }
            else
            {
                add(iter().clone(*this).ptr(), true);
                changed = true;
            }
        }
        else
        {
            // Not found - just add
            add(iter().clone(*this).ptr());
            changed = true;
        }
    }

    return changed;
}


void CML::dictionary::clear()
{
    IDLList<entry>::clear();
    hashedEntries_.clear();
    patternEntries_.clear();
    patternRegexps_.clear();
}


void CML::dictionary::transfer(dictionary& dict)
{
    // Changing parents probably doesn't make much sense,
    // but what about the names?
    name() = dict.name();

    IDLList<entry>::transfer(dict);
    hashedEntries_.transfer(dict.hashedEntries_);
    patternEntries_.transfer(dict.patternEntries_);
    patternRegexps_.transfer(dict.patternRegexps_);
}


CML::Xfer<CML::dictionary> CML::dictionary::xfer()
{
    return xferMove(*this);
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

CML::ITstream& CML::dictionary::operator[](const word& keyword) const
{
    return lookup(keyword);
}


void CML::dictionary::operator=(const dictionary& rhs)
{
    // Check for assignment to self
    if (this == &rhs)
    {
        FatalIOErrorInFunction(*this)
            << "attempted assignment to self for dictionary " << name()
            << abort(FatalIOError);
    }

    name() = rhs.name();
    clear();

    // Create clones of the entries in the given dictionary
    // resetting the parentDict to this dictionary

    forAllConstIter(IDLList<entry>, rhs, iter)
    {
        add(iter().clone(*this).ptr());
    }
}


void CML::dictionary::operator+=(const dictionary& rhs)
{
    // Check for assignment to self
    if (this == &rhs)
    {
        FatalIOErrorInFunction(*this)
            << "attempted addition assignment to self for dictionary " << name()
            << abort(FatalIOError);
    }

    forAllConstIter(IDLList<entry>, rhs, iter)
    {
        add(iter().clone(*this).ptr());
    }
}


void CML::dictionary::operator|=(const dictionary& rhs)
{
    // Check for assignment to self
    if (this == &rhs)
    {
        FatalIOErrorInFunction(*this)
            << "attempted assignment to self for dictionary " << name()
            << abort(FatalIOError);
    }

    forAllConstIter(IDLList<entry>, rhs, iter)
    {
        if (!found(iter().keyword()))
        {
            add(iter().clone(*this).ptr());
        }
    }
}


void CML::dictionary::operator<<=(const dictionary& rhs)
{
    // Check for assignment to self
    if (this == &rhs)
    {
        FatalIOErrorInFunction(*this)
            << "attempted assignment to self for dictionary " << name()
            << abort(FatalIOError);
    }

    forAllConstIter(IDLList<entry>, rhs, iter)
    {
        set(iter().clone(*this).ptr());
    }
}


/* * * * * * * * * * * * * * * * Global operators  * * * * * * * * * * * * * */

CML::dictionary CML::operator+
(
    const dictionary& dict1,
    const dictionary& dict2
)
{
    dictionary sum(dict1);
    sum += dict2;
    return sum;
}


CML::dictionary CML::operator|
(
    const dictionary& dict1,
    const dictionary& dict2
)
{
    dictionary sum(dict1);
    sum |= dict2;
    return sum;
}


// ************************************************************************* //
