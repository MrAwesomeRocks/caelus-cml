/*---------------------------------------------------------------------------*\
Copyright: ICE Stroemungsfoschungs GmbH
Copyright (C) 1991-2008 OpenCFD Ltd.
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

Contributors/Copyright:
    2011-2013 Bernhard F.W. Gschaider <bgschaid@ice-sf.at>

\*---------------------------------------------------------------------------*/

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "surfaceMesh.hpp"
#include "fvsPatchField.hpp"
#include "pointPatchField.hpp"
//#include "primitivePatchInterpolation.hpp"
#include "interpolation.hpp"

#include <cassert>

namespace CML {

    label SampledSetValueExpressionDriver::pointSize() const
    {
        FatalErrorInFunction
        << "sampledSets are a collection of points. So the 'cell' functionality should be used"
            << endl
            << exit(FatalError);

        return 0;
    }

    const fvMesh &SampledSetValueExpressionDriver::mesh() const
    {
        //        return dynamicCast<const fvMesh&>(faceZone_.zoneMesh().mesh()); // doesn't work with gcc 4.2
        return dynamic_cast<const fvMesh&>(theSet_.mesh());
    }

    template<class Type,class volField,class surfField>
    tmp<Field<Type> >
    SampledSetValueExpressionDriver::sampleOrInterpolateInternal(
        const word &name,
        bool oldTime
    )
    {
        if(debug) {
            Pout << "Sampling set: " << name << " on set "
                << this->theSet_.name() << " with " << this->size()
                << " points" << endl;
            //            this->theSet_.write(Pout);
            Pout << "Search memory: " << this->searchInMemory()
                << " Field there: " << this->mesh().foundObject<volField>(name)
                << " Is variable: " << this->hasVariable(name)
                << endl;
        }
        if(
            this->hasVariable(name)
            &&
            this->variable(name).valueType()==pTraits<Type>::typeName
        ) {
            return this->variable(name).getResult<Type>(true);
        }

        autoPtr<Field<Type> > result;
        const sampledSet& samples = this->theSet_;

        if(
            this->searchInMemory()
            &&
            this->mesh().foundObject<volField>(name)
        ) {
            if(interpolate_) {
                autoPtr<interpolation<Type> > inter(
                    interpolation<Type>::New(
                        interpolationType_,
                        (
                            !oldTime ?
                            this->mesh().lookupObject<volField>(name) :
                            this->mesh().lookupObject<volField>(name).oldTime()
                        )
                    )
                );

                result.set(
                    new Field<Type>(this->theSet_.size())
                );
                Field<Type> &values=result();

                forAll(samples, samplei)
                {
                    const point& samplePt = samples[samplei];
                    label celli = samples.cells()[samplei];
                    label facei = samples.faces()[samplei];

                    values[samplei] = inter().interpolate
                        (
                            samplePt,
                            celli,
                            facei
                        );
                }
                if(debug) {
                    Pout << "Interpolated values: " << values << endl;
                }
            } else {
                result.set(
                    new Field<Type>(this->theSet_.size())
                );
                Field<Type> &values=result();
                const volField &field=this->mesh().lookupObject<volField>(name);

                forAll(samples, samplei)
                {
                    if(!oldTime) {
                        values[samplei] = field[samples.cells()[samplei]];
                    } else {
                        values[samplei] = field.oldTime()[samples.cells()[samplei]];
                    }
                }
                if(debug) {
                    Pout << "Values: " << values << endl;
                }
            }
        } else {
            autoPtr<volField> f=this->getOrReadField<volField>(
                name,
                true, // fail if not found
                oldTime
            );

            if(interpolate_) {
                autoPtr<interpolation<Type> > inter(
                    interpolation<Type>::New(
                        interpolationType_,
                        (
                            !oldTime ?
                            f() :
                            f->oldTime()
                        )
                    )
                );

                result.set(
                    new Field<Type>(this->theSet_.size())
                );
                Field<Type> &values=result();

                forAll(samples, samplei)
                {
                    const point& samplePt = samples[samplei];
                    label celli = samples.cells()[samplei];
                    label facei = samples.faces()[samplei];

                    values[samplei] = inter().interpolate
                        (
                            samplePt,
                            celli,
                            facei
                        );
                }
            } else {
                result.set(
                    new Field<Type>(this->theSet_.size())
                );
                Field<Type> &values=result();
                const volField &field=f();

                forAll(samples, samplei)
                {
                    if(!oldTime) {
                        values[samplei] = field[samples.cells()[samplei]];
                    } else {
                        values[samplei] =
                            field.oldTime()[samples.cells()[samplei]];
                    }
                }
            }
        }

        if(!result.valid()) {
            FatalErrorInFunction
                << "No valid result"
                    << endl
                    << exit(FatalError);
        }

        return tmp<Field<Type> >(
            result.ptr()
        );
    }

}


// ************************************************************************* //
