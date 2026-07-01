//=============================================================================
// Reduced singleton lifecycle for the Linux PoC.
//=============================================================================

#include <main/singletons_linux_poc.h>

#include <input/inputmanager.h>
#include <loading/loadingmanager.h>
#include <main/commandlineoptions.h>
#include <raddebug.hpp>
#include <render/RenderFlow/RenderFlow.h>
#include <sound/soundmanager.h>

void CreateSingletonsLinuxPoc()
{
    rReleasePrintf("CreateSingletonsLinuxPoc\n");

    InputManager::CreateInstance()->Init();
    RenderFlow::CreateInstance();
    LoadingManager::CreateInstance();
    SoundManager::CreateInstance(CommandLineOptions::Get(CLO_MUTE),
                                 CommandLineOptions::Get(CLO_NO_MUSIC),
                                 CommandLineOptions::Get(CLO_NO_EFFECTS),
                                 CommandLineOptions::Get(CLO_NO_DIALOG));
}

void DestroySingletonsLinuxPoc()
{
    rReleasePrintf("DestroySingletonsLinuxPoc\n");

    SoundManager::DestroyInstance();
    LoadingManager::DestroyInstance();
    RenderFlow::DestroyInstance();
    InputManager::DestroyInstance();
}
