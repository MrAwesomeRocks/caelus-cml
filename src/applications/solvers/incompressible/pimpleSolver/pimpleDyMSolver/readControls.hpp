    #include "readTimeControls.hpp"

    const bool correctPhi =
        pimple.dict().lookupOrDefault("correctPhi", false);

    const bool checkMeshCourantNo =
        pimple.dict().lookupOrDefault("checkMeshCourantNo", false);
