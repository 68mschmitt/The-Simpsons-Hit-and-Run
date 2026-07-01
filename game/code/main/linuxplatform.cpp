//=============================================================================
// Linux Platform Stub for the native PoC.
//=============================================================================

#include <main/linuxplatform.h>

#include <raddebug.hpp>

LinuxPlatform* LinuxPlatform::spInstance = NULL;

void LinuxPlatform::InitializeFoundation()
{
    rReleasePrintf("LinuxPlatform foundation initialized (stub)\n");
}

LinuxPlatform* LinuxPlatform::CreateInstance()
{
    rAssertMsg(spInstance == NULL, "Trying to create more than one LinuxPlatform instance");
    spInstance = new LinuxPlatform;
    return spInstance;
}

LinuxPlatform* LinuxPlatform::GetInstance()
{
    return spInstance;
}

void LinuxPlatform::DestroyInstance()
{
    delete spInstance;
    spInstance = NULL;
}

void LinuxPlatform::InitializePlatform()
{
    if(mInitialized)
    {
        return;
    }

    InitializeFoundationDrive();
    InitializePure3D();

    mInitialized = true;
    rReleasePrintf("LinuxPlatform initialized\n");
}

void LinuxPlatform::ShutdownPlatform()
{
    if(!mInitialized)
    {
        return;
    }

    ShutdownPure3D();
    ShutdownFoundation();

    mInitialized = false;
    rReleasePrintf("LinuxPlatform shut down\n");
}

void LinuxPlatform::LaunchDashboard()
{
    rReleasePrintf("LinuxPlatform::LaunchDashboard requested; marking PoC for quit\n");
    mQuitRequested = true;
}

void LinuxPlatform::ResetMachine()
{
    rReleasePrintf("LinuxPlatform::ResetMachine requested; marking PoC for quit\n");
    mQuitRequested = true;
}

void LinuxPlatform::DisplaySplashScreen(SplashScreen screenID,
                                        const char* overlayText,
                                        float fontScale,
                                        float textPosX,
                                        float textPosY,
                                        tColour textColour,
                                        int fadeFrames)
{
    (void)fontScale;
    (void)textPosX;
    (void)textPosY;
    (void)textColour;
    (void)fadeFrames;
    rReleasePrintf("LinuxPlatform::DisplaySplashScreen(%d, %s) ignored by PoC\n",
                   static_cast<int>(screenID),
                   overlayText != NULL ? overlayText : "no overlay");
}

void LinuxPlatform::DisplaySplashScreen(const char* textureName,
                                        const char* overlayText,
                                        float fontScale,
                                        float textPosX,
                                        float textPosY,
                                        tColour textColour,
                                        int fadeFrames)
{
    (void)fontScale;
    (void)textPosX;
    (void)textPosY;
    (void)textColour;
    (void)fadeFrames;
    rReleasePrintf("LinuxPlatform::DisplaySplashScreen(%s, %s) ignored by PoC\n",
                   textureName != NULL ? textureName : "no texture",
                   overlayText != NULL ? overlayText : "no overlay");
}

void LinuxPlatform::OnControllerError(const char* msg)
{
    mErrorState = CTL_ERROR;
    mPauseForError = true;
    rReleasePrintf("LinuxPlatform controller error: %s\n", msg != NULL ? msg : "unknown");
}

bool LinuxPlatform::OnDriveError(radFileError error, const char* pDriveName, void* pUserData)
{
    (void)pUserData;
    mErrorState = (error == Success) ? NONE : P_ERROR;
    mPauseForError = (error != Success);
    rReleasePrintf("LinuxPlatform drive error callback: error=%d drive=%s\n",
                   static_cast<int>(error),
                   pDriveName != NULL ? pDriveName : "none");
    return false;
}

LinuxPlatform::LinuxPlatform()
    : mInitialized(false),
      mQuitRequested(false)
{
    mErrorState = NONE;
}

LinuxPlatform::~LinuxPlatform()
{
    if(mInitialized)
    {
        ShutdownPlatform();
    }
}

void LinuxPlatform::InitializeFoundationDrive()
{
    mpIRadDrive = NULL;
    rReleasePrintf("LinuxPlatform foundation drive initialized (stub)\n");
}

void LinuxPlatform::ShutdownFoundation()
{
    mpIRadDrive = NULL;
    rReleasePrintf("LinuxPlatform foundation shut down (stub)\n");
}

void LinuxPlatform::InitializePure3D()
{
    rReleasePrintf("LinuxPlatform Pure3D initialized (stub)\n");
}

void LinuxPlatform::ShutdownPure3D()
{
    rReleasePrintf("LinuxPlatform Pure3D shut down (stub)\n");
}
