U = rAU*UEqn().H();

if (pimple.nCorrPISO() <= 1)
{
    UEqn.clear();
}

phi = (fvc::interpolate(U) & mesh.Sf())
    + fvc::ddtPhiCorr(rAU, U, phi);
fvOptions.makeRelative(phi);
adjustPhi(phi, U, p);

// Non-orthogonal pressure corrector loop
while (pimple.correctNonOrthogonal())
{
    // Pressure corrector
    fvScalarMatrix pEqn
    (
        fvm::laplacian(rAU, p) == fvc::div(phi)
    );

    pEqn.setReference(pRefCell, pRefValue);

    pEqn.solve(mesh.solver(p.select(pimple.finalInnerIter())));

    if (pimple.finalNonOrthogonalIter())
    {
        phi -= pEqn.flux();
    }
}

#include "continuityErrs.hpp"

// Explicitly relax pressure for momentum corrector
p.relax();

U -= rAU*fvc::grad(p);
U.correctBoundaryConditions();
fvOptions.correct(U);
