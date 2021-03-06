/*---------------------------------------------------------------------------*\
Copyright (C) Creative Fields, Ltd.
-------------------------------------------------------------------------------
License
    This file is part of cfMesh.

    cfMesh is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 3 of the License, or (at your
    option) any later version.

    cfMesh is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with cfMesh.  If not, see <http://www.gnu.org/licenses/>.

Class
    helperFunctionsPar

Description
    Functions used for exchanging data between processors

Author: Franjo Juretic (franjo.juretic@c-fields.com)

SourceFiles

\*---------------------------------------------------------------------------*/

#ifndef helperFunctionsPar_HPP
#define helperFunctionsPar_HPP

#include "labelList.hpp"
#include "LongList.hpp"

#include <map>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace CML
{
    
namespace help
{

//- exchanges the data between processors such that in the end thwy all have
//- the necessary information to proceed with the local work
template<class sendOp, class combineOp, class T, class ListType>
void combineData(const sendOp&, combineOp&);

//- each processor exchanges the data with all of its neighbours
//- processors with lower labels first send the data to processors with
//- higher labels first and and then to the processors with lower labels
template<class T, class ListType, class scatterOp, class gatherOp>
void whisperReduce(const ListType&, const scatterOp&, gatherOp&);

//- send the data stored in the map to other processors and collects the data
//- sent from other processors into the list
template<class T, class ListType>
void exchangeMap
(
    const std::map<label, ListType>&,
    LongList<T>&,
    const Pstream::commsTypes commsType = Pstream::blocking
);

//- sends the data stored in a map to other processors and receives the data
//- received from other processors into another map
template<class T, class ListType>
void exchangeMap(const std::map<label, ListType>&, std::map<label, List<T> >&);

//- calculates the reverse addressing of the graph by transposing the graph
template<class RowType, template<class ListTypeArg> class GraphType>
void reverseAddressingSMP(const GraphType<RowType>&, GraphType<RowType>&);

} // End namespace help

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace CML

#include "error.hpp"
#include "DynList.hpp"
#include "labelPair.hpp"
#include "HashSet.hpp"

# ifdef USE_OMP
#include <omp.h>
# endif

namespace CML
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

namespace help
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

template<class sendOp, class combineOp, class T, class ListType>
void combineData(const sendOp& sop, combineOp& cop)
{
    std::map<label, LongList<T> > sMap;
    sop(sMap);

    LongList<T> data;

    exchangeMap(sMap, data);

    cop(data);
}

template<class T, class ListType, class scatterOp, class gatherOp>
void whisperReduce(const ListType& neis, const scatterOp& sop, gatherOp& gop)
{
    DynList<label> below, above;
    forAll(neis, i)
    {
        if( neis[i] < Pstream::myProcNo() )
        {
            above.append(neis[i]);
        }
        else if( neis[i] > Pstream::myProcNo() )
        {
            below.append(neis[i]);
        }
    }

    //- receive the data from the processors above
    forAll(above, aboveI)
    {
        //- receive the data
        List<T> receivedData;
        IPstream fromOtherProc(Pstream::blocking, above[aboveI]);
        fromOtherProc >> receivedData;

        gop(receivedData);
    }

    //- send the data to the processors below
    forAll(below, belowI)
    {
        const label neiProc = below[belowI];

        LongList<T> dts;
        sop(dts);

        //- send the data
        OPstream toOtherProc(Pstream::blocking, neiProc, dts.byteSize());
        toOtherProc << dts;
    }

    //- gather the data from the processors below to the processors above
    //- receive the data from the processors below
    forAllReverse(below, belowI)
    {
        //- receive the data
        List<T> receivedData;
        IPstream fromOtherProc(Pstream::blocking, below[belowI]);
        fromOtherProc >> receivedData;

        gop(receivedData);
    }

    //- send the data to the processors below
    forAllReverse(above, aboveI)
    {
        const label neiProc = above[aboveI];

        LongList<T> dts;
        sop(dts);

        //- send the data
        OPstream toOtherProc(Pstream::blocking, neiProc, dts.byteSize());
        toOtherProc << dts;
    }
}

template<class T, class ListType>
void exchangeMap
(
    const std::map<label, ListType>& m,
    LongList<T>& data,
    const Pstream::commsTypes commsType
)
{
    data.clear();

    if( !contiguous<T>() )
        FatalError << "Data is not contiguous" << exit(FatalError);

    typename std::map<label, ListType>::const_iterator iter;

    //- check which processors shall exchange the data and which ones shall not
    labelHashSet receiveData;
    for(iter=m.begin();iter!=m.end();++iter)
    {
        OPstream toOtherProc(Pstream::blocking, iter->first, sizeof(label));

        toOtherProc << iter->second.size();
    }

    for(iter=m.begin();iter!=m.end();++iter)
    {
        IPstream fromOtherProc(Pstream::blocking, iter->first, sizeof(label));

        label s;
        fromOtherProc >> s;

        if( s != 0 )
            receiveData.insert(iter->first);
    }

    if( commsType == Pstream::blocking )
    {
        //- start with blocking type of send and received operation

        //- send data to other processors
        for(iter=m.begin();iter!=m.end();++iter)
        {
            const ListType& dts = iter->second;

            if( dts.size() == 0 )
                continue;

            OPstream toOtherProc
            (
                Pstream::blocking,
                iter->first,
                dts.byteSize()
            );
            toOtherProc << dts;
        }

        //- receive data from other processors
        for(iter=m.begin();iter!=m.end();++iter)
        {
            if( !receiveData.found(iter->first) )
                continue;

            IPstream fromOtherProc(Pstream::blocking, iter->first);
            data.appendFromStream(fromOtherProc);
        }
    }
    else if( commsType == Pstream::scheduled )
    {
        //- start with scheduled data transfer
        //- this type of transfer is intended for long messages because
        //- it does not require any buffer

        //- receive data from processors with lower ids
        for(iter=m.begin();iter!=m.end();++iter)
        {
            if( iter->first >= Pstream::myProcNo() )
                continue;
            if( !receiveData.found(iter->first) )
                continue;

            //List<T> receive;
            IPstream fromOtherProc(Pstream::scheduled, iter->first);
            //fromOtherProc >> receive;
            data.appendFromStream(fromOtherProc);

            //forAll(receive, i)
            //    data.append(receive[i]);
        }

        //- send data to processors with greater ids
        for(iter=m.begin();iter!=m.end();++iter)
        {
            if( iter->first <= Pstream::myProcNo() )
                continue;

            const ListType& dts = iter->second;

            if( dts.size() == 0 )
                continue;

            OPstream toOtherProc
            (
                Pstream::scheduled,
                iter->first,
                dts.byteSize()
            );

            toOtherProc << dts;
        }

        //- receive data from processors with greater ids
        typename std::map<label, ListType>::const_reverse_iterator riter;
        for(riter=m.rbegin();riter!=m.rend();++riter)
        {
            if( riter->first <= Pstream::myProcNo() )
                continue;
            if( !receiveData.found(riter->first) )
                continue;

            IPstream fromOtherProc(Pstream::scheduled, riter->first);
            //List<T> receive;
            //fromOtherProc >> receive;
            data.appendFromStream(fromOtherProc);

            //forAll(receive, i)
             //   data.append(receive[i]);
        }

        //- send data to processors with lower ids
        for(riter=m.rbegin();riter!=m.rend();++riter)
        {
            if( riter->first >= Pstream::myProcNo() )
                continue;

            const ListType& dts = riter->second;

            if( dts.size() == 0 )
                continue;

            OPstream toOtherProc
            (
                Pstream::scheduled,
                riter->first,
                dts.byteSize()
            );

            toOtherProc << dts;
        }
    }
    else
    {
        FatalErrorInFunction
          << "Unknown communication type" << exit(FatalError);
    }

    # ifdef DEBUGExchangeMap
    labelList nReceived(Pstream::nProcs(), 0);
    for(iter=m.begin();iter!=m.end();++iter)
        nReceived[iter->first] += iter->second.size();

    reduce(nReceived, sumOp<labelList>());

    if( nReceived[Pstream::myProcNo()] != data.size() )
        FatalError << "Invalid data read!" << abort(FatalError);
    # endif
}

template<class T, class ListType>
void exchangeMap
(
    const std::map<label, ListType>& m,
    std::map<label, List<T> >&mOut
)
{
    mOut.clear();

    if( !contiguous<T>() )
        FatalError << "Data is not contigous" << exit(FatalError);

    typename std::map<label, ListType>::const_iterator iter;

    //- send data to other processors
    for(iter=m.begin();iter!=m.end();++iter)
    {
        const ListType& dataToSend = iter->second;

        OPstream toOtherProc
        (
            Pstream::blocking,
            iter->first,
            dataToSend.byteSize()
        );
        toOtherProc << dataToSend;
    }

    //- receive data from other processors
    for(iter=m.begin();iter!=m.end();++iter)
    {
        mOut.insert(std::make_pair(iter->first, List<T>()));
        List<T>& dataToReceive = mOut[iter->first];

        IPstream fromOtherProc(Pstream::blocking, iter->first);
        fromOtherProc >> dataToReceive;
    }
}

template<class RowType, template<class ListTypeArg> class GraphType>
void reverseAddressingSMP
(
    const GraphType<RowType>& addr,
    GraphType<RowType>& reverseAddr
)
{
    reverseAddr.setSize(0);
    labelList nAppearances;

    label minRow(INT_MAX), maxRow(0);
    DynList<std::map<label, DynList<labelPair, 64> > > dataForOtherThreads;

    # ifdef USE_OMP
    # pragma omp parallel
    # endif
    {
        //- find min and max entry in the graph
        //- they are used for assigning ranges of values local for each process
        label localMinRow(minRow), localMaxRow(0);
        # ifdef USE_OMP
        # pragma omp for schedule(guided)
        # endif
        forAll(addr, rowI)
        {
            const RowType& row = addr[rowI];

            forAll(row, i)
            {
                localMaxRow = CML::max(localMaxRow, row[i]);
                localMinRow = CML::min(localMinRow, row[i]);
            }
        }

        ++localMaxRow;

        # ifdef USE_OMP
        # pragma omp critical
        # endif
        {
            minRow = CML::min(minRow, localMinRow);
            maxRow = CML::max(maxRow, localMaxRow);
        }

        # ifdef USE_OMP
        # pragma omp barrier

        //- allocate the rows of the transposed graph
        # pragma omp master
        # endif
        {
            # ifdef USE_OMP
            dataForOtherThreads.setSize(omp_get_num_threads());
            # else
            dataForOtherThreads.setSize(1);
            # endif
            nAppearances.setSize(maxRow);
            reverseAddr.setSize(maxRow);
        }

        # ifdef USE_OMP
        const label nProcs = omp_get_num_threads();
        const label procNo = omp_get_thread_num();
        # else
        const label nProcs = 1;
        const label procNo = 0;
        # endif

        # ifdef USE_OMP
        # pragma omp barrier

        //- initialise appearances
        # pragma omp for
        # endif
        for(label i=0;i<maxRow;++i)
            nAppearances[i] = 0;

        const label range = (maxRow - minRow) / nProcs + 1;
        const label localMin = minRow + procNo * range;
        const label localMax = CML::min(localMin + range, maxRow);

        //- find the number of appearances of each element in the original graph
        # ifdef USE_OMP
        # pragma omp for
        # endif
        forAll(addr, rowI)
        {
            const RowType& row = addr[rowI];

            forAll(row, j)
            {
                const label entryI = row[j];

                const label procI = (entryI - minRow) / range;
                if( procI != procNo )
                {
                    dataForOtherThreads[procNo][procI].append
                    (
                        labelPair(entryI, rowI)
                    );
                }
                else
                {
                    ++nAppearances[entryI];
                }
            }
        }

        # ifdef USE_OMP
        # pragma omp barrier
        # endif

        //- count the appearances which are not local to the processor
        for(label i=0;i<nProcs;++i)
        {
            const std::map<label, DynList<labelPair, 64> >& data =
                dataForOtherThreads[i];

            std::map<label, DynList<labelPair, 64> >::const_iterator it =
                data.find(procNo);

            if( it != data.end() )
            {
                const DynList<labelPair, 64>& entries = it->second;

                forAll(entries, j)
                    ++nAppearances[entries[j].first()];
            }
        }

        # ifdef USE_OMP
        # pragma omp barrier
        # endif

        //- allocate reverse matrix
        for(label i=localMin;i<localMax;++i)
        {
            reverseAddr[i].setSize(nAppearances[i]);
            nAppearances[i] = 0;
        }

        //- start filling reverse addressing graph
        //- update data from processors with smaller labels
        for(label i=0;i<procNo;++i)
        {
            const std::map<label, DynList<labelPair, 64> >& data =
                dataForOtherThreads[i];
            std::map<label, DynList<labelPair, 64> >::const_iterator it =
                data.find(procNo);

            if( it != data.end() )
            {
                const DynList<labelPair, 64>& entries = it->second;

                forAll(entries, j)
                {
                    const label entryI = entries[j].first();
                    reverseAddr[entryI][nAppearances[entryI]++] =
                        entries[j].second();
                }
            }
        }

        //- update data local to the processor
        # ifdef USE_OMP
        # pragma omp for
        # endif
        forAll(addr, rowI)
        {
            const RowType& row = addr[rowI];

            forAll(row, j)
            {
                const label entryI = row[j];

                if( (entryI >= localMin) && (entryI < localMax) )
                {
                    reverseAddr[entryI][nAppearances[entryI]++] = rowI;
                }
            }
        }

        //- update data from the processors with higher labels
        for(label i=procNo+1;i<nProcs;++i)
        {
            const std::map<label, DynList<labelPair, 64> >& data =
                dataForOtherThreads[i];
            std::map<label, DynList<labelPair, 64> >::const_iterator it =
                data.find(procNo);

            if( it != data.end() )
            {
                const DynList<labelPair, 64>& entries = it->second;

                forAll(entries, j)
                {
                    const label entryI = entries[j].first();
                    reverseAddr[entryI][nAppearances[entryI]++] =
                        entries[j].second();
                }
            }
        }
    }
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

} // End namespace help

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

} // End namespace CML

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
