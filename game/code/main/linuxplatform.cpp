//=============================================================================
// Linux Platform shell for the native PoC.
//=============================================================================

#include <main/linuxplatform.h>

#include <cstdlib>

#include <input/inputmanager.h>
#include <port/linux_poc_config.h>
#include <raddebug.hpp>

#ifdef LINUX_POC_WITH_SDL
#include <SDL.h>
#endif

LinuxPlatform* LinuxPlatform::spInstance = NULL;

namespace
{
#ifdef LINUX_POC_WITH_SDL
    constexpr int MaxSdlControllers = 4;

    bool HasDisplayEnvironment()
    {
        return std::getenv("SDL_VIDEODRIVER") != NULL ||
               std::getenv("DISPLAY") != NULL ||
               std::getenv("WAYLAND_DISPLAY") != NULL;
    }
#endif
}

void LinuxPlatform::InitializeFoundation()
{
#ifdef LINUX_POC_WITH_SDL
    rReleasePrintf("LinuxPlatform foundation initialized (SDL2-capable stub)\n");
#else
    rReleasePrintf("LinuxPlatform foundation initialized (stub)\n");
#endif
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
    rReleasePrintf("LinuxPlatform initialized%s\n", mHostWindowActive ? " with SDL2 window" : " in headless mode");
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

void LinuxPlatform::ServiceHostEvents()
{
#ifdef LINUX_POC_WITH_SDL
    if(!mSdlInitialized)
    {
        return;
    }

    SDL_Event event;
    while(SDL_PollEvent(&event) != 0)
    {
        switch(event.type)
        {
            case SDL_QUIT:
                rReleasePrintf("LinuxPlatform SDL quit event received\n");
                RequestQuit();
                break;

            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    rReleasePrintf("LinuxPlatform SDL window close event received\n");
                    RequestQuit();
                }
                else if(event.window.event == SDL_WINDOWEVENT_RESIZED ||
                        event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    rReleasePrintf("LinuxPlatform SDL window resized to %dx%d\n",
                                   event.window.data1,
                                   event.window.data2);
                }
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                if(event.key.repeat != 0)
                {
                    break;
                }

                const bool pressed = (event.type == SDL_KEYDOWN);
                const unsigned int scancode = static_cast<unsigned int>(event.key.keysym.scancode);
                const char* keyName = SDL_GetScancodeName(event.key.keysym.scancode);
                HandleSdlKeyboardEvent(scancode, keyName, pressed);

                if(pressed && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                {
                    rReleasePrintf("LinuxPlatform Escape pressed; requesting quit\n");
                    RequestQuit();
                }
                break;
            }

            case SDL_CONTROLLERDEVICEADDED:
                OpenSdlController(event.cdevice.which);
                break;

            case SDL_CONTROLLERDEVICEREMOVED:
                CloseSdlController(event.cdevice.which);
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            {
                const bool pressed = (event.type == SDL_CONTROLLERBUTTONDOWN);
                const SDL_GameControllerButton button = static_cast<SDL_GameControllerButton>(event.cbutton.button);
                const char* buttonName = SDL_GameControllerGetStringForButton(button);
                rReleasePrintf("LinuxPlatform SDL controller button %s: instance_id=%d button=%s(%d)\n",
                               pressed ? "down" : "up",
                               event.cbutton.which,
                               buttonName != NULL ? buttonName : "unknown",
                               static_cast<int>(event.cbutton.button));
                HandleSdlControllerButtonEvent(event.cbutton.which,
                                               static_cast<unsigned int>(event.cbutton.button),
                                               buttonName,
                                               pressed);
                break;
            }

            default:
                break;
        }
    }
#endif
}

void LinuxPlatform::BeginHostFrame(unsigned int elapsedMilliseconds)
{
    (void)elapsedMilliseconds;

#ifdef LINUX_POC_WITH_SDL
    if(mpSdlRenderer == NULL)
    {
        return;
    }

    const unsigned char blue = static_cast<unsigned char>(32 + ((mPresentedFrameCount * 3) % 96));
    SDL_SetRenderDrawColor(mpSdlRenderer, 8, 16, blue, 255);
    SDL_RenderClear(mpSdlRenderer);
#endif
}

void LinuxPlatform::EndHostFrame()
{
#ifdef LINUX_POC_WITH_SDL
    if(mpSdlRenderer == NULL)
    {
        return;
    }

    SDL_RenderPresent(mpSdlRenderer);
    ++mPresentedFrameCount;
#endif
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
      mQuitRequested(false),
      mHostWindowActive(false),
      mPresentedFrameCount(0)
#ifdef LINUX_POC_WITH_SDL
      , mSdlInitialized(false),
      mpSdlWindow(NULL),
      mpSdlRenderer(NULL),
      mpSdlControllers{NULL, NULL, NULL, NULL},
      mSdlControllerInstanceIds{-1, -1, -1, -1}
#endif
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
#ifdef LINUX_POC_WITH_SDL
    if(ShouldAttemptSdlWindow())
    {
        InitializeSdlShell();
    }
    else if(LinuxPocConfig::RuntimeHostWindowMode == LinuxPocConfig::HostWindowMode::Disabled)
    {
        rReleasePrintf("LinuxPlatform SDL2 window disabled by command line\n");
    }
    else
    {
        rReleasePrintf("LinuxPlatform no DISPLAY/WAYLAND_DISPLAY detected; SDL2 window not created\n");
    }
#else
    if(LinuxPocConfig::RuntimeHostWindowMode != LinuxPocConfig::HostWindowMode::Disabled)
    {
        rReleasePrintf("LinuxPlatform SDL2 support not compiled; running without a host window\n");
    }
#endif

    rReleasePrintf("LinuxPlatform Pure3D initialized (stub)\n");
}

void LinuxPlatform::ShutdownPure3D()
{
#ifdef LINUX_POC_WITH_SDL
    ShutdownSdlShell();
#endif
    rReleasePrintf("LinuxPlatform Pure3D shut down (stub)\n");
}

#ifdef LINUX_POC_WITH_SDL
bool LinuxPlatform::ShouldAttemptSdlWindow() const
{
    if(LinuxPocConfig::RuntimeHostWindowMode == LinuxPocConfig::HostWindowMode::Disabled)
    {
        return false;
    }

    if(LinuxPocConfig::RuntimeHostWindowMode == LinuxPocConfig::HostWindowMode::Required)
    {
        return true;
    }

    return HasDisplayEnvironment();
}

void LinuxPlatform::InitializeSdlShell()
{
    if(mSdlInitialized)
    {
        return;
    }

    const unsigned int initFlags = SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER;
    if(SDL_InitSubSystem(initFlags) != 0)
    {
        rReleasePrintf("LinuxPlatform SDL_InitSubSystem failed: %s\n", SDL_GetError());
        return;
    }

    mSdlInitialized = true;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    mpSdlWindow = SDL_CreateWindow("SRR2 Linux PoC",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   static_cast<int>(LinuxPocConfig::RuntimeWindowWidth),
                                   static_cast<int>(LinuxPocConfig::RuntimeWindowHeight),
                                   SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if(mpSdlWindow == NULL)
    {
        rReleasePrintf("LinuxPlatform SDL_CreateWindow failed: %s\n", SDL_GetError());
        ShutdownSdlShell();
        return;
    }

    mpSdlRenderer = SDL_CreateRenderer(mpSdlWindow,
                                       -1,
                                       SDL_RENDERER_ACCELERATED);
    if(mpSdlRenderer == NULL)
    {
        rReleasePrintf("LinuxPlatform accelerated SDL renderer failed: %s; trying software renderer\n",
                       SDL_GetError());
        mpSdlRenderer = SDL_CreateRenderer(mpSdlWindow, -1, SDL_RENDERER_SOFTWARE);
    }

    if(mpSdlRenderer == NULL)
    {
        rReleasePrintf("LinuxPlatform SDL_CreateRenderer failed: %s\n", SDL_GetError());
        ShutdownSdlShell();
        return;
    }

    SDL_GameControllerEventState(SDL_ENABLE);
    const int joystickCount = SDL_NumJoysticks();
    for(int deviceIndex = 0; deviceIndex < joystickCount; ++deviceIndex)
    {
        if(SDL_IsGameController(deviceIndex) == SDL_TRUE)
        {
            OpenSdlController(deviceIndex);
        }
    }

    mHostWindowActive = true;
    rReleasePrintf("LinuxPlatform SDL2 window initialized (%ux%u)\n",
                   LinuxPocConfig::RuntimeWindowWidth,
                   LinuxPocConfig::RuntimeWindowHeight);
}

void LinuxPlatform::ShutdownSdlShell()
{
    CloseSdlController(-1);

    if(mpSdlRenderer != NULL)
    {
        SDL_DestroyRenderer(mpSdlRenderer);
        mpSdlRenderer = NULL;
    }

    if(mpSdlWindow != NULL)
    {
        SDL_DestroyWindow(mpSdlWindow);
        mpSdlWindow = NULL;
    }

    if(mSdlInitialized)
    {
        SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER);
        mSdlInitialized = false;
        rReleasePrintf("LinuxPlatform SDL2 shell shut down\n");
    }

    mHostWindowActive = false;
}

void LinuxPlatform::OpenSdlController(int deviceIndex)
{
    if(deviceIndex < 0)
    {
        return;
    }

    if(SDL_IsGameController(deviceIndex) != SDL_TRUE)
    {
        rReleasePrintf("LinuxPlatform SDL joystick is not a game controller: device_index=%d\n", deviceIndex);
        return;
    }

    const SDL_JoystickID deviceInstanceId = SDL_JoystickGetDeviceInstanceID(deviceIndex);
    if(deviceInstanceId >= 0 && FindSdlControllerSlot(static_cast<int>(deviceInstanceId)) >= 0)
    {
        rReleasePrintf("LinuxPlatform SDL controller already open: device_index=%d instance_id=%d\n",
                       deviceIndex,
                       static_cast<int>(deviceInstanceId));
        return;
    }

    int freeSlot = -1;
    for(int slot = 0; slot < MaxSdlControllers; ++slot)
    {
        if(mpSdlControllers[slot] == NULL)
        {
            freeSlot = slot;
            break;
        }
    }

    if(freeSlot < 0)
    {
        rReleasePrintf("LinuxPlatform SDL controller ignored; all %d PoC controller slots are full\n",
                       MaxSdlControllers);
        return;
    }

    SDL_GameController* controller = SDL_GameControllerOpen(deviceIndex);
    if(controller == NULL)
    {
        rReleasePrintf("LinuxPlatform SDL_GameControllerOpen failed for device_index=%d: %s\n",
                       deviceIndex,
                       SDL_GetError());
        return;
    }

    SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
    const SDL_JoystickID instanceId = joystick != NULL ? SDL_JoystickInstanceID(joystick) : -1;

    mpSdlControllers[freeSlot] = controller;
    mSdlControllerInstanceIds[freeSlot] = static_cast<int>(instanceId);

    rReleasePrintf("LinuxPlatform SDL controller opened: slot=%d device_index=%d instance_id=%d name=%s\n",
                   freeSlot,
                   deviceIndex,
                   static_cast<int>(instanceId),
                   SDL_GameControllerName(controller) != NULL ? SDL_GameControllerName(controller) : "unknown");
}

void LinuxPlatform::CloseSdlController(int instanceId)
{
    for(int slot = 0; slot < MaxSdlControllers; ++slot)
    {
        if(mpSdlControllers[slot] == NULL)
        {
            continue;
        }

        if(instanceId >= 0 && mSdlControllerInstanceIds[slot] != instanceId)
        {
            continue;
        }

        SDL_GameController* controller = static_cast<SDL_GameController*>(mpSdlControllers[slot]);
        rReleasePrintf("LinuxPlatform SDL controller closed: slot=%d instance_id=%d name=%s\n",
                       slot,
                       mSdlControllerInstanceIds[slot],
                       SDL_GameControllerName(controller) != NULL ? SDL_GameControllerName(controller) : "unknown");
        SDL_GameControllerClose(controller);
        mpSdlControllers[slot] = NULL;
        mSdlControllerInstanceIds[slot] = -1;
    }
}

int LinuxPlatform::FindSdlControllerSlot(int instanceId) const
{
    for(int slot = 0; slot < MaxSdlControllers; ++slot)
    {
        if(mpSdlControllers[slot] != NULL && mSdlControllerInstanceIds[slot] == instanceId)
        {
            return slot;
        }
    }

    return -1;
}

void LinuxPlatform::HandleSdlKeyboardEvent(unsigned int scancode, const char* keyName, bool pressed)
{
    InputManager* inputManager = InputManager::GetInstance();
    if(inputManager != NULL)
    {
        inputManager->OnHostKeyboardEvent(scancode, keyName, pressed);
    }
}

void LinuxPlatform::HandleSdlControllerButtonEvent(int instanceId,
                                                   unsigned int button,
                                                   const char* buttonName,
                                                   bool pressed)
{
    const int slot = FindSdlControllerSlot(instanceId);
    if(slot < 0)
    {
        return;
    }

    InputManager* inputManager = InputManager::GetInstance();
    if(inputManager != NULL)
    {
        inputManager->OnHostControllerButtonEvent(static_cast<unsigned int>(slot),
                                                  button,
                                                  buttonName,
                                                  pressed);
    }
}
#endif
