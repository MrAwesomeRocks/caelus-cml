/*---------------------------------------------------------------------------*\
Copyright (C) 2009-2016 Bernhard Gschaider
Copyright (C) 2016-2017 OpenCFD Ltd.
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

Class
    CML::profiling

Description
    Code profiling.

    This is typically activated from within the system/controlDict as follows
    (defaults shown):
    \code
        profiling
        {
            active      true;  // default: true
            sysInfo     true;  // default: true
        }
    \endcode
    or simply using all defaults:
    \code
        profiling
        {}
    \endcode

SourceFiles
    profiling.cpp

\*---------------------------------------------------------------------------*/

#ifndef profiling_HPP
#define profiling_HPP

#include "profilingTrigger.hpp"
#include "HashPtrTable.hpp"
#ifdef windows
#undef DebugInfo
#endif
#include "LIFOStack.hpp"
#include "Map.hpp"
#include "Time.hpp"
#include "clockTime.hpp"

namespace CML
{

// Forward declaration of classes
class Ostream;
class profilingSysInfo;

/*---------------------------------------------------------------------------*\
                          Class profiling Declaration
\*---------------------------------------------------------------------------*/

class profiling
:
    public regIOobject
{
public:

    // Public typedefs

        typedef profilingInformation Information;
        typedef profilingTrigger Trigger;

private:

    // Private typedefs

        typedef profilingSysInfo sysInfo;
        typedef HashPtrTable<Information, string> StorageContainer;
        typedef LIFOStack<Information*> StackContainer;


    // Private Static Data Members

        //- Only one global pool object is possible
        static profiling *pool_;


    // Private Data Members

        //- The owner of the profiling
        const Time& owner_;

        //- A global timer for the profiling
        clockTime clockTime_;

        //- Storage of profiling information
        StorageContainer hash_;

        //- Local stack of profiling information
        StackContainer stack_;

        //- Note the timers (by Id) for the correct stack-output
        Map<clockTime*> timers_;

        //- General system information (optional)
        sysInfo* sysInfo_;


    // Private Member Functions

        //- Disallow default bitwise copy construct
        profiling(const profiling&) = delete;

        //- Disallow default bitwise assignment
        void operator=(const profiling&) = delete;


protected:

    // Friendship

        friend class profilingTrigger;
        friend class Time;


    // Constructors

        //- Construct IO object, everything enabled
        profiling(const IOobject& io, const Time& owner);

        //- Construct IO object with finer control over behaviour
        profiling
        (
            const dictionary& dict,
            const IOobject& io,
            const Time& owner
        );


    //- Destructor
    ~profiling();


    // Protected Member Functions

        //- Find named profiling information element or null on failure
        profilingInformation* find(const string& name);

        //- Add to hashed storage,
        //  returns pointer to newly stored element for chaining
        profilingInformation* store(profilingInformation* info);

        //- Add to stack and set timer lookup (based on Id)
        void push(profilingInformation* info, clockTime& timer);

        //- Remove from stack and remove timer lookup (based on Id).
        //  Returns pointer to profiling information element
        profilingInformation* pop();


    // Static control elements

        //- Singleton to initialize profiling pool, everything enabled
        static void initialize
        (
            const IOobject& ioObj,
            const Time& owner
        );

        //- Singleton to initialize profiling pool with finer control
        static void initialize
        (
            const dictionary& dict,
            const IOobject& ioObj,
            const Time& owner
        );

        //- Stop profiling, cleanup pool if possible
        static void stop(const Time& owner);

        //- Existing or new element on pool, add to stack.
        //  Returns null if profiling has not been initialized
        static profilingInformation* New
        (
            const string& descr,
            clockTime& timer
        );

        //- Remove the information from the top of the stack
        static void unstack(const profilingInformation* info);

public:

    // Static Member Functions

        //- True if profiling is active
        static bool active();

        //- Print profiling information to specified output
        //  Forwards to writeData member of top-level object
        static bool print(Ostream& os);

        //- Write profiling information now
        static bool writeNow();


    // Member Functions

        //- The owner of the profiling
        const Time& owner() const;

        //- The size of the current stack
        label size() const;

        //- writeData member function required by regIOobject
        virtual bool writeData(Ostream& os) const;

        //- Write as uncompressed ASCII, using given format
        virtual bool writeObject
        (
            IOstream::streamFormat ignoreAlwaysASCII,
            IOstream::versionNumber ver,
            IOstream::compressionType ignoreAlwaysUncompressed
        ) const;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
