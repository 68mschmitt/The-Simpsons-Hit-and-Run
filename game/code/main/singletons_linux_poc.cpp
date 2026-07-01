//=============================================================================
// Reduced singleton lifecycle for the Linux PoC.
//=============================================================================

#include <main/singletons_linux_poc.h>

#include <raddebug.hpp>

void CreateSingletonsLinuxPoc()
{
    rReleasePrintf("CreateSingletonsLinuxPoc\n");
    rReleasePrintf("  RenderFlow: stub\n");
    rReleasePrintf("  SoundManager: stub\n");
    rReleasePrintf("  LoadingManager: fake-complete stub\n");
}

void DestroySingletonsLinuxPoc()
{
    rReleasePrintf("DestroySingletonsLinuxPoc\n");
}
