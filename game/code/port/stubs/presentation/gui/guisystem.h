// Minimal GUI system shim for the Linux PoC.
//
// The real CGuiSystem drives the Scrooby UI stack.  For the PoC we only need to
// reproduce the bootup license-screen control flow that carries BootupContext
// from OnStart to CONTEXT_FRONTEND:
//
//   1. The real license screen intro calls GetBootupContext()->StartLoadingSound()
//      (guiscreenlicense.cpp:155) and, once running, ResetLicenseScreenDisplayTime()
//      (guiscreenlicense.cpp:178).  Init() mimics that here.
//   2. BootupContext::OnUpdate sends GUI_MSG_QUIT_BOOTUP once the license screen
//      minimum time has elapsed and both load flags are set.
//   3. The real CGuiManagerBootUp turns that into StartMovies()
//      (guimanagerbootup.cpp:314); HandleMessage() short-circuits to the same call.
#ifndef GUISYSTEM_H
#define GUISYSTEM_H

// Subset of eGuiMessage (presentation/gui/guientity.h) used by the bootup path.
enum eGuiMessage
{
    GUI_MSG_RELEASE_BOOTUP,
    GUI_MSG_QUIT_BOOTUP
};

class CGuiSystem
{
public:
    static CGuiSystem* CreateInstance();
    static CGuiSystem* GetInstance();
    static void DestroyInstance();

    void Init();
    void Update( unsigned int elapsedTime );

    void RegisterUserInputHandlers( int controllerIDs = ~0 );
    void UnregisterUserInputHandlers( int controllerIDs = ~0 );

    void HandleMessage( eGuiMessage message,
                        unsigned int param1 = 0,
                        unsigned int param2 = 0 );

private:
    CGuiSystem();
    ~CGuiSystem();

    static CGuiSystem* spInstance;

    bool mLicenseScreenStarted;
};

inline CGuiSystem* GetGuiSystem() { return CGuiSystem::GetInstance(); }

#endif // GUISYSTEM_H
