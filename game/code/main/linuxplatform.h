//=============================================================================
// Linux Platform Stub for the native PoC.
//=============================================================================

#ifndef LINUXPLATFORM_H
#define LINUXPLATFORM_H

#include <cstddef>

#include <main/platform.h>

class LinuxPlatform : public Platform
{
public:
    static void InitializeFoundation();

    static LinuxPlatform* CreateInstance();
    static LinuxPlatform* GetInstance();
    static void DestroyInstance();

    virtual void InitializePlatform();
    virtual void ShutdownPlatform();

    virtual void LaunchDashboard();
    virtual void ResetMachine();

    virtual void DisplaySplashScreen(SplashScreen screenID,
                                     const char* overlayText = NULL,
                                     float fontScale = 1.0f,
                                     float textPosX = 0.0f,
                                     float textPosY = 0.0f,
                                     tColour textColour = tColour(255, 255, 255),
                                     int fadeFrames = 3);

    virtual void DisplaySplashScreen(const char* textureName,
                                     const char* overlayText = NULL,
                                     float fontScale = 1.0f,
                                     float textPosX = 0.0f,
                                     float textPosY = 0.0f,
                                     tColour textColour = tColour(255, 255, 255),
                                     int fadeFrames = 3);

    virtual void OnControllerError(const char* msg);
    virtual bool OnDriveError(radFileError error, const char* pDriveName, void* pUserData);

    bool QuitRequested() const { return mQuitRequested; }
    void RequestQuit() { mQuitRequested = true; }

private:
    LinuxPlatform();
    virtual ~LinuxPlatform();

    LinuxPlatform(const LinuxPlatform&);
    LinuxPlatform& operator=(const LinuxPlatform&);

    virtual void InitializeFoundationDrive();
    virtual void ShutdownFoundation();

    virtual void InitializePure3D();
    virtual void ShutdownPure3D();

    static LinuxPlatform* spInstance;

    bool mInitialized;
    bool mQuitRequested;
};

#endif // LINUXPLATFORM_H
