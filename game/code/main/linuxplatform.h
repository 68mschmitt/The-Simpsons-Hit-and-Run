//=============================================================================
// Linux Platform Stub for the native PoC.
//=============================================================================

#ifndef LINUXPLATFORM_H
#define LINUXPLATFORM_H

#include <cstddef>

#include <main/platform.h>

#ifdef LINUX_POC_WITH_SDL
struct SDL_Renderer;
struct SDL_Window;
#endif

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

    void ServiceHostEvents();
    void BeginHostFrame(unsigned int elapsedMilliseconds);
    void EndHostFrame();

    bool HasHostWindow() const { return mHostWindowActive; }
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

#ifdef LINUX_POC_WITH_SDL
    bool ShouldAttemptSdlWindow() const;
    void InitializeSdlShell();
    void ShutdownSdlShell();
    void OpenSdlController(int deviceIndex);
    void CloseSdlController(int instanceId);
    int FindSdlControllerSlot(int instanceId) const;
    void HandleSdlKeyboardEvent(unsigned int scancode, const char* keyName, bool pressed);
    void HandleSdlControllerButtonEvent(int instanceId, unsigned int button, const char* buttonName, bool pressed);
#endif

    static LinuxPlatform* spInstance;

    bool mInitialized;
    bool mQuitRequested;
    bool mHostWindowActive;
    unsigned int mPresentedFrameCount;

#ifdef LINUX_POC_WITH_SDL
    bool mSdlInitialized;
    SDL_Window* mpSdlWindow;
    SDL_Renderer* mpSdlRenderer;
    void* mpSdlControllers[4];
    int mSdlControllerInstanceIds[4];
#endif
};

#endif // LINUXPLATFORM_H
