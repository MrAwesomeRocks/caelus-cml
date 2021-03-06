/*---------------------------------------------------------------------------*\
Copyright (C) 2011-2015 OpenFOAM Foundation
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

#include "layerParameters.hpp"
#include "polyBoundaryMesh.hpp"
#include "unitConversion.hpp"
#include "refinementSurfaces.hpp"
#include "searchableSurfaces.hpp"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

const CML::scalar CML::layerParameters::defaultConcaveAngle = 90;


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

CML::scalar CML::layerParameters::layerExpansionRatio
(
    const label n,
    const scalar totalOverFirst
) const
{
    if (n <= 1)
    {
        return 1.0;
    }

    //scalar totalOverFirst = totalThickness/firstLayerThickess;

    const label maxIters = 10;
    const scalar tol = 1e-8;


    if (mag(n-totalOverFirst) < tol)
    {
        return 1.0;
    }

    // Calculate the bounds of the solution
    scalar minR;
    scalar maxR;

    if (totalOverFirst < n)
    {
        minR = 0.0;
        maxR = pow(totalOverFirst/n, 1/(n-1));
    }
    else
    {
        minR = pow(totalOverFirst/n, 1/(n-1));
        maxR = totalOverFirst/(n - 1);
    }

    //Info<< "Solution bounds = (" << minR << ", " << maxR << ")" << nl << endl;

    // Starting guess
    scalar r = 0.5*(minR + maxR);

    for (label i = 0; i < maxIters; ++i)
    {
        const scalar prevr = r;

        const scalar fx = pow(r, n) - totalOverFirst*r - (1 - totalOverFirst);
        const scalar dfx = n*pow(r, n - 1) - totalOverFirst;

        r -= fx/dfx;

        const scalar error = mag(r - prevr);

        //Info<< i << " " << r << " Error = " << error << endl;

        if (error < tol)
        {
            break;
        }
    }
    return r;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from dictionary
CML::layerParameters::layerParameters
(
    const dictionary& dict,
    const polyBoundaryMesh& boundaryMesh
)
:
    numLayers_(boundaryMesh.size(), -1),
    relativeSizes_(dict.lookup("relativeSizes")),
    layerSpec_(ILLEGAL),
    firstLayerThickness_(boundaryMesh.size(), -123),
    finalLayerThickness_(boundaryMesh.size(), -123),
    thickness_(boundaryMesh.size(), -123),
    expansionRatio_(boundaryMesh.size(), -123),
    minThickness_
    (
        boundaryMesh.size(),
        readScalar(dict.lookup("minThickness"))
    ),
    featureAngle_(readScalar(dict.lookup("featureAngle"))),
    slipFeatureAngle_
    (
        dict.found("slipFeatureAngle")
      ? readScalar(dict.lookup("slipFeatureAngle"))
      : 0.5*featureAngle_
    ),
    concaveAngle_
    (
        dict.lookupOrDefault("concaveAngle", defaultConcaveAngle)
    ),
    nGrow_(readLabel(dict.lookup("nGrow"))),
    nSmoothSurfaceNormals_
    (
        readLabel(dict.lookup("nSmoothSurfaceNormals"))
    ),
    nSmoothNormals_(readLabel(dict.lookup("nSmoothNormals"))),
    nSmoothThickness_(readLabel(dict.lookup("nSmoothThickness"))),
    maxFaceThicknessRatio_
    (
        readScalar(dict.lookup("maxFaceThicknessRatio"))
    ),
    layerTerminationCos_
    (
        CML::cos(degToRad(0.5*featureAngle_))
    ),
    maxThicknessToMedialRatio_
    (
        readScalar(dict.lookup("maxThicknessToMedialRatio"))
    ),
    minMedianAxisAngleCos_
    (
        CML::cos(degToRad(readScalar(dict.lookup("minMedianAxisAngle"))))
    ),
    nBufferCellsNoExtrude_
    (
        readLabel(dict.lookup("nBufferCellsNoExtrude"))
    ),
    nSnap_(readLabel(dict.lookup("nRelaxIter"))),
    nLayerIter_(readLabel(dict.lookup("nLayerIter"))),
    nRelaxedIter_(labelMax),
    additionalReporting_(dict.lookupOrDefault("additionalReporting", false))
{
    // Detect layer specification mode

    label nSpec = 0;

    bool haveFirst = dict.found("firstLayerThickness");
    if (haveFirst)
    {
        firstLayerThickness_ = scalarField
        (
            boundaryMesh.size(),
            readScalar(dict.lookup("firstLayerThickness"))
        );
        nSpec++;
    }
    bool haveFinal = dict.found("finalLayerThickness");
    if (haveFinal)
    {
        finalLayerThickness_ = scalarField
        (
            boundaryMesh.size(),
            readScalar(dict.lookup("finalLayerThickness"))
        );
        nSpec++;
    }
    bool haveTotal = dict.found("thickness");
    if (haveTotal)
    {
        thickness_ = scalarField
        (
            boundaryMesh.size(),
            readScalar(dict.lookup("thickness"))
        );
        nSpec++;
    }
    bool haveExp = dict.found("expansionRatio");
    if (haveExp)
    {
        expansionRatio_ = scalarField
        (
            boundaryMesh.size(),
            readScalar(dict.lookup("expansionRatio"))
        );
        nSpec++;
    }


    if (haveFirst && haveTotal)
    {
        layerSpec_ = FIRST_AND_TOTAL;
        Info<< "Layer thickness specified as first layer and overall thickness."
            << endl;
    }
    else if (haveFirst && haveExp)
    {
        layerSpec_ = FIRST_AND_EXPANSION;
        Info<< "Layer thickness specified as first layer and expansion ratio."
            << endl;
    }
    else if (haveFinal && haveTotal)
    {
        layerSpec_ = FINAL_AND_TOTAL;
        Info<< "Layer thickness specified as final layer and overall thickness."
            << endl;
    }
    else if (haveFinal && haveExp)
    {
        layerSpec_ = FINAL_AND_EXPANSION;
        Info<< "Layer thickness specified as final layer and expansion ratio."
            << endl;
    }


    if (layerSpec_ == ILLEGAL || nSpec != 2)
    {
        FatalIOErrorIn
        (
            "layerParameters::layerParameters"
            "(const dictionary&, const polyBoundaryMesh&)",
            dict
        )   << "Over- or underspecified layer thickness."
            << " Please specify" << nl
            << "    first layer thickness ('firstLayerThickness')"
            << " and overall thickness ('thickness') or" << nl
            << "    first layer thickness ('firstLayerThickness')"
            << " and expansion ratio ('expansionRatio') or" << nl
            << "    final layer thickness ('finalLayerThickness')"
            << " and expansion ratio ('expansionRatio') or" << nl
            << "    final layer thickness ('finalLayerThickness')"
            << " and overall thickness ('thickness')"
            << exit(FatalIOError);
    }


    dict.readIfPresent("nRelaxedIter", nRelaxedIter_);

    if (nLayerIter_ < 0 || nRelaxedIter_ < 0)
    {
        FatalIOErrorIn("layerParameters::layerParameters(..)", dict)
            << "Layer iterations should be >= 0." << endl
            << "nLayerIter:" << nLayerIter_
            << " nRelaxedIter:" << nRelaxedIter_
            << exit(FatalIOError);
    }


    const dictionary& layersDict = dict.subDict("layers");

    forAllConstIter(dictionary, layersDict, iter)
    {
        if (iter().isDict())
        {
            const keyType& key = iter().keyword();
            const labelHashSet patchIDs
            (
                boundaryMesh.patchSet(List<wordRe>(1, wordRe(key)))
            );

            if (patchIDs.size() == 0)
            {
                IOWarningInFunction(layersDict)
                    << "Layer specification for " << key
                    << " does not match any patch." << endl
                    << "Valid patches are " << boundaryMesh.names() << endl;
            }
            else
            {
                const dictionary& layerDict = iter().dict();

                forAllConstIter(labelHashSet, patchIDs, patchIter)
                {
                    label patchI = patchIter.key();

                    numLayers_[patchI] =
                        readLabel(layerDict.lookup("nSurfaceLayers"));

                    switch (layerSpec_)
                    {
                        case FIRST_AND_TOTAL:
                            layerDict.readIfPresent
                            (
                                "firstLayerThickness",
                                firstLayerThickness_[patchI]
                            );
                            layerDict.readIfPresent
                            (
                                "thickness",
                                thickness_[patchI]
                            );
                        break;

                        case FIRST_AND_EXPANSION:
                            layerDict.readIfPresent
                            (
                                "firstLayerThickness",
                                firstLayerThickness_[patchI]
                            );
                            layerDict.readIfPresent
                            (
                                "expansionRatio",
                                expansionRatio_[patchI]
                            );
                        break;

                        case FINAL_AND_TOTAL:
                            layerDict.readIfPresent
                            (
                                "finalLayerThickness",
                                finalLayerThickness_[patchI]
                            );
                            layerDict.readIfPresent
                            (
                                "thickness",
                                thickness_[patchI]
                            );
                        break;

                        case FINAL_AND_EXPANSION:
                            layerDict.readIfPresent
                            (
                                "finalLayerThickness",
                                finalLayerThickness_[patchI]
                            );
                            layerDict.readIfPresent
                            (
                                "expansionRatio",
                                expansionRatio_[patchI]
                            );
                        break;

                        default:
                            FatalIOErrorIn
                            (
                                "layerParameters::layerParameters(..)",
                                dict
                            )   << "problem." << exit(FatalIOError);
                        break;
                    }

                    layerDict.readIfPresent
                    (
                        "minThickness",
                        minThickness_[patchI]
                    );
                }
            }
        }
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

CML::scalar CML::layerParameters::layerThickness
(
    const label nLayers,
    const scalar firstLayerThickess,
    const scalar finalLayerThickess,
    const scalar totalThickness,
    const scalar expansionRatio
) const
{
    switch (layerSpec_)
    {
        case FIRST_AND_TOTAL:
        case FINAL_AND_TOTAL:
        {
            return totalThickness;
        }
        break;

        case FIRST_AND_EXPANSION:
        {
            if (mag(expansionRatio-1) < SMALL)
            {
                return firstLayerThickess * nLayers;
            }
            else
            {
                return firstLayerThickess *
                    (1.0 - pow(expansionRatio, nLayers))
                  / (1.0 - expansionRatio);
            }
        }
        break;

        case FINAL_AND_EXPANSION:
        {
            if (mag(expansionRatio-1) < SMALL)
            {
                return finalLayerThickess * nLayers;
            }
            else
            {
                scalar invExpansion = 1.0 / expansionRatio;
                return finalLayerThickess *
                    (1.0 - pow(invExpansion, nLayers))
                  / (1.0 - invExpansion);
            }
        }
        break;

        default:
        {
            FatalErrorIn("layerParameters::layerThickness(..)")
                << "Illegal thickness specification " <<    layerSpec_
                << exit(FatalError);
            return -VGREAT;
        }
    }
}


CML::scalar CML::layerParameters::layerExpansionRatio
(
    const label nLayers,
    const scalar firstLayerThickess,
    const scalar finalLayerThickess,
    const scalar totalThickness,
    const scalar expansionRatio
) const
{
    switch (layerSpec_)
    {
        case FIRST_AND_EXPANSION:
        case FINAL_AND_EXPANSION:
        {
            return expansionRatio;
        }
        break;

        case FIRST_AND_TOTAL:
        {
            return layerExpansionRatio
            (
                nLayers,
                totalThickness/firstLayerThickess
            );
        }
        break;

        case FINAL_AND_TOTAL:
        {
            return
                1.0
              / layerExpansionRatio
                (
                    nLayers,
                    totalThickness/finalLayerThickess
                );
        }
        break;

        default:
        {
            FatalErrorIn("layerParameters::layerThickness(..)")
                << "Illegal thickness specification" << exit(FatalError);
            return -VGREAT;
        }
    }
}


CML::scalar CML::layerParameters::firstLayerThickness
(
    const label nLayers,
    const scalar firstLayerThickess,
    const scalar finalLayerThickess,
    const scalar totalThickness,
    const scalar expansionRatio
) const
{
    switch (layerSpec_)
    {
        case FIRST_AND_EXPANSION:
        case FIRST_AND_TOTAL:
        {
            return firstLayerThickess;
        }

        case FINAL_AND_EXPANSION:
        {
            return finalLayerThickess*pow(1.0/expansionRatio, nLayers-1);
        }
        break;

        case FINAL_AND_TOTAL:
        {
            scalar r = layerExpansionRatio
            (
                nLayers,
                firstLayerThickess,
                finalLayerThickess,
                totalThickness,
                expansionRatio
            );
            return finalLayerThickess/pow(r, nLayers-1);
        }
        break;

        default:
        {
            FatalErrorIn("layerParameters::layerThickness(..)")
                << "Illegal thickness specification" << exit(FatalError);
            return -VGREAT;
        }
    }
}


CML::scalar CML::layerParameters::finalLayerThicknessRatio
(
    const label nLayers,
    const scalar expansionRatio
) const
{
    if (nLayers > 0)
    {
        if (mag(expansionRatio-1) < SMALL)
        {
            return 1.0/nLayers;
        }
        else
        {
            return
                pow(expansionRatio, nLayers - 1)
              * (1.0 - expansionRatio)
              / (1.0 - pow(expansionRatio, nLayers));
        }
    }
    else
    {
        return 0.0;
    }
}


// ************************************************************************* //
