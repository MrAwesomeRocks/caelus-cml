#include "createTimeControls.hpp"

bool correctPhi(pimple.dict().lookupOrDefault("correctPhi", mesh.dynamic()));

bool checkMeshCourantNo(pimple.dict().lookupOrDefault("checkMeshCourantNo", false));

bool moveMeshOuterCorrectors
(
    pimple.dict().lookupOrDefault("moveMeshOuterCorrectors", false)    
);
