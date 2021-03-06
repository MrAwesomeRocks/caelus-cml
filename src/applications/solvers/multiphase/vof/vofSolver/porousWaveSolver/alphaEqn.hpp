{
	if (MULESCorr)
	{
		FatalErrorInFunction
			<< "Implicit solution to the alpha field is chosen for porousWaveSolver.\n"
			<< "No implementation is currently available that corrects for the\n"
			<< "porosity in the implicit advection of alpha1.\n"
			<< "\nSwitch to the explicit advection routine."
			<< endl << endl << exit(FatalError);
	}

    word alphaScheme("div(phi,alpha)");
    word alpharScheme("div(phirb,alpha)");

    tmp<fv::ddtScheme<scalar> > ddtAlpha
    (
        fv::ddtScheme<scalar>::New
        (
            mesh,
            mesh.ddtScheme("ddt(alpha)")
        )
    );

    // Set the off-centering coefficient according to ddt scheme
    scalar ocCoeff = 0;
    if
    (
        isType<fv::EulerDdtScheme<scalar> >(ddtAlpha())
     || isType<fv::localEulerDdtScheme<scalar> >(ddtAlpha())
    )
    {
        ocCoeff = 0;
    }
    else if (isType<fv::CrankNicolsonDdtScheme<scalar> >(ddtAlpha()))
    {
        if (nAlphaSubCycles > 1)
        {
            FatalErrorInFunction
                << "Sub-cycling is not supported "
                   "with the CrankNicolson ddt scheme"
                << exit(FatalError);
        }

        ocCoeff =
            refCast<const fv::CrankNicolsonDdtScheme<scalar> >(ddtAlpha())
           .ocCoeff();
    }
    else
    {
        FatalErrorInFunction
            << "Only Euler and CrankNicolson ddt schemes are supported"
            << exit(FatalError);
    }

    scalar cnCoeff = 1.0/(1.0 + ocCoeff);

    tmp<surfaceScalarField> phiCN(phi);

    // Calculate the Crank-Nicolson off-centred volumetric flux
    if (ocCoeff > 0)
    {
        phiCN = cnCoeff*phi + (1.0 - cnCoeff)*phi.oldTime();
    }

    if (MULESCorr)
    {
        fvScalarMatrix alpha1Eqn
        (
            (
                LTS
              ? fv::localEulerDdtScheme<scalar>(mesh).fvmDdt(alpha1)
              : fv::EulerDdtScheme<scalar>(mesh).fvmDdt(alpha1)
            )
          + 1.0/porosity*fv::gaussConvectionScheme<scalar>
            (
                mesh,
                phiCN,
                upwind<scalar>(mesh, phiCN)
            ).fvmDiv(phiCN, alpha1)
        );

        alpha1Eqn.solve();

        Info<< "Phase-1 volume fraction = "
            << alpha1.weightedAverage(mesh.Vsc()).value()
            << "  Min(" << alpha1.name() << ") = " << min(alpha1).value()
            << "  Max(" << alpha1.name() << ") = " << max(alpha1).value()
            << endl;

        tmp<surfaceScalarField> talphaPhiUD(alpha1Eqn.flux());
        alphaPhi = talphaPhiUD();

        if (alphaApplyPrevCorr && talphaPhiCorr0.valid())
        {
            Info<< "Applying the previous iteration compression flux" << endl;
            MULES::correct(alpha1, alphaPhi, talphaPhiCorr0(), 1, 0);

            alphaPhi += talphaPhiCorr0();
        }

        // Cache the upwind-flux
        talphaPhiCorr0 = talphaPhiUD;

        mixture.correct();
        interface.correct();
    }


    for (int aCorr=0; aCorr<nAlphaCorr; aCorr++)
    {
        surfaceScalarField phir(interface.phir());

        tmp<surfaceScalarField> talphaPhiUn
        (
            fvc::flux
            (
                phi,
                alpha1,
                alphaScheme
            )
          + fvc::flux
            (
               -fvc::flux(-phir, scalar(1) - alpha1, alpharScheme),
                alpha1,
                alpharScheme
            )
        );

        // Calculate the Crank-Nicolson off-centred alpha flux
        if (ocCoeff > 0)
        {
            talphaPhiUn =
                cnCoeff*talphaPhiUn + (1.0 - cnCoeff)*alphaPhi.oldTime();
        }

        if (MULESCorr)
        {
            tmp<surfaceScalarField> talphaPhiCorr(talphaPhiUn() - alphaPhi);
            volScalarField alpha10("alpha10", alpha1);

            MULES::correct
            (
                porosity,
                alpha1,
                talphaPhiUn(),
                talphaPhiCorr(),
                zeroField(),
                zeroField(),
                1,
                0
            );

            // Under-relax the correction for all but the 1st corrector
            if (aCorr == 0)
            {
                alphaPhi += talphaPhiCorr();
            }
            else
            {
                alpha1 = 0.5*alpha1 + 0.5*alpha10;
                alphaPhi += 0.5*talphaPhiCorr();
            }
        }
        else
        {
            alphaPhi = talphaPhiUn;

            MULES::explicitSolve
            (
                porosity,
                alpha1,
                phiCN,
                alphaPhi,
                zeroField(),
                zeroField(),
                1,
                0
            );
        }

        mixture.correct();
        interface.correct();
    }

    if (alphaApplyPrevCorr && MULESCorr)
    {
        talphaPhiCorr0 = alphaPhi - talphaPhiCorr0;
    }

    if
    (
        word(mesh.ddtScheme("ddt(rho,U)"))
     == fv::EulerDdtScheme<vector>::typeName
    )
    {
        rhoPhi = alphaPhi*(rho1 - rho2) + phiCN*rho2;
    }
    else
    {
        if (ocCoeff > 0)
        {
            // Calculate the end-of-time-step alpha flux
            alphaPhi = (alphaPhi - (1.0 - cnCoeff)*alphaPhi.oldTime())/cnCoeff;
        }

        // Calculate the end-of-time-step mass flux
        rhoPhi = alphaPhi*(rho1 - rho2) + phi*rho2;
    }

    scalar porousVolume = CML::sum
        (
            alpha1.internalField()*mesh.V()*porosity.internalField()
        );
    porousVolume /= CML::sum(mesh.V()).value();

    Info<< "Phase-1 volume fraction = "
        << alpha1.weightedAverage(mesh.Vsc()).value()
        << "  Min(" << alpha1.name() << ") = " << min(alpha1).value()
        << "  Max(" << alpha1.name() << ") = " << max(alpha1).value()
        << " Porosity weighted volume: " << porousVolume
        << endl;
}
