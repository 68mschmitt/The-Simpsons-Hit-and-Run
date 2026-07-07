//=============================================================================
// Linux PoC no-op manager singletons that back the real BootupContext.
//
// The real BootupContext (contexts/bootupcontext.cpp) touches roughly a dozen
// gameplay/presentation managers plus the Pure3D device.  None of those systems
// are portable yet, so this file provides minimal singletons whose Get*()
// accessors log once on creation (matching the existing PoC shim style) and
// whose methods are no-ops.
//
// The GUI system stub reproduces the minimal bootup control flow that carries
// BootupContext from OnStart through to CONTEXT_FRONTEND.  See guisystem.h for
// the mapping onto the real Scrooby license-screen / CGuiManagerBootUp path.
//=============================================================================

#include <atc/atcmanager.h>
#include <cards/cardgallery.h>
#include <cheats/cheatinputsystem.h>
#include <contexts/bootupcontext.h>
#include <data/gamedatamanager.h>
#include <interiors/interiormanager.h>
#include <mission/gameplaymanager.h>
#include <mission/missionmanager.h>
#include <mission/charactersheet/charactersheetmanager.h>
#include <mission/rewards/rewardsmanager.h>
#include <p3d/context.hpp>
#include <pddi/pddi.hpp>
#include <presentation/presentation.h>
#include <presentation/gui/guisystem.h>
#include <presentation/tutorialmanager.h>
#include <render/rendermanager/rendermanager.h>
#include <render/rendermanager/renderlayer.h>
#include <supersprint/supersprintmanager.h>
#include <worldsim/worldphysicsmanager.h>
#include <worldsim/character/charactermanager.h>

#include <raddebug.hpp>

//=============================================================================
// Generic singleton boilerplate.
//
// Each manager below is a trivial log-once singleton.  A macro keeps the
// GetInstance/DestroyInstance/ctor/dtor bodies uniform and matches the
// "(Linux PoC stub)" logging convention used by the other port shims.
//=============================================================================
#define LINUX_POC_SINGLETON( ClassName )                                       \
    ClassName* ClassName::spInstance = nullptr;                                \
    ClassName* ClassName::GetInstance()                                        \
    {                                                                          \
        if( spInstance == nullptr )                                            \
        {                                                                      \
            spInstance = new ClassName;                                        \
        }                                                                      \
        return spInstance;                                                     \
    }                                                                          \
    void ClassName::DestroyInstance()                                          \
    {                                                                          \
        delete spInstance;                                                     \
        spInstance = nullptr;                                                  \
    }                                                                          \
    ClassName::ClassName()                                                     \
    {                                                                          \
        rReleasePrintf( #ClassName " created (Linux PoC stub)\n" );            \
    }                                                                          \
    ClassName::~ClassName()                                                    \
    {                                                                          \
    }

//=============================================================================
// ATCManager
//=============================================================================
LINUX_POC_SINGLETON( ATCManager )
void ATCManager::Init() {}

//=============================================================================
// CardGallery
//=============================================================================
LINUX_POC_SINGLETON( CardGallery )
void CardGallery::Init() {}

//=============================================================================
// CheatInputSystem
//=============================================================================
LINUX_POC_SINGLETON( CheatInputSystem )
void CheatInputSystem::Init() {}
void CheatInputSystem::SetCheatEnabled( eCheatID cheatID, bool enable )
{
    (void)cheatID;
    (void)enable;
}
bool CheatInputSystem::IsCheatEnabled( eCheatID cheatID ) const
{
    (void)cheatID;
    return false;
}

//=============================================================================
// GameDataManager
//=============================================================================
LINUX_POC_SINGLETON( GameDataManager )
void GameDataManager::Init() {}
void GameDataManager::Update( unsigned int elapsedTime ) { (void)elapsedTime; }

//=============================================================================
// InteriorManager
//=============================================================================
LINUX_POC_SINGLETON( InteriorManager )
void InteriorManager::OnBootupStart() {}

//=============================================================================
// RewardsManager
//=============================================================================
LINUX_POC_SINGLETON( RewardsManager )
void RewardsManager::LoadScript() {}

//=============================================================================
// TutorialManager
//=============================================================================
LINUX_POC_SINGLETON( TutorialManager )
void TutorialManager::Initialize() {}

//=============================================================================
// WorldPhysicsManager
//=============================================================================
LINUX_POC_SINGLETON( WorldPhysicsManager )
void WorldPhysicsManager::Init() {}

//=============================================================================
// CharacterManager
//=============================================================================
LINUX_POC_SINGLETON( CharacterManager )
void CharacterManager::PreLoad() {}

//=============================================================================
// CharacterSheetManager
//=============================================================================
LINUX_POC_SINGLETON( CharacterSheetManager )
void CharacterSheetManager::InitCharacterSheet() {}

//=============================================================================
// PresentationManager
//=============================================================================
LINUX_POC_SINGLETON( PresentationManager )
void PresentationManager::InitializePlayerDrawable() {}
void PresentationManager::Update( unsigned int elapsedTime ) { (void)elapsedTime; }
// The PoC never queues FMVs, so the presentation queue is always empty.
// Returning false keeps OnPresentationEventEnd (never called here) from firing
// a second, redundant transition to CONTEXT_FRONTEND.
bool PresentationManager::IsQueueEmpty() const { return false; }

//=============================================================================
// RenderManager / RenderLayer
//=============================================================================
LINUX_POC_SINGLETON( RenderManager )
RenderLayer* RenderManager::mpLayer( RenderEnums::LayerEnum layer )
{
    (void)layer;
    return &mLayer;
}
void RenderLayer::Warm() {}
void RenderLayer::Chill() {}

//=============================================================================
// GameplayManager and its bootup-time subclasses.
//=============================================================================
GameplayManager* GameplayManager::spInstance = nullptr;

GameplayManager* GameplayManager::GetInstance()
{
    return spInstance;
}

void GameplayManager::SetInstance( GameplayManager* pInstance )
{
    spInstance = pInstance;
}

void GameplayManager::SetLevelIndex( RenderEnums::LevelEnum level ) { (void)level; }
void GameplayManager::SetMissionIndex( RenderEnums::MissionEnum mission ) { (void)mission; }

GameplayManager::GameplayManager() {}
GameplayManager::~GameplayManager() {}

MissionManager* MissionManager::spMissionInstance = nullptr;

MissionManager* MissionManager::GetInstance()
{
    if( spMissionInstance == nullptr )
    {
        spMissionInstance = new MissionManager;
    }
    return spMissionInstance;
}

void MissionManager::DestroyInstance()
{
    delete spMissionInstance;
    spMissionInstance = nullptr;
}

MissionManager::MissionManager()
{
    rReleasePrintf( "MissionManager created (Linux PoC stub)\n" );
}

MissionManager::~MissionManager() {}

SuperSprintManager* SuperSprintManager::spSuperSprintInstance = nullptr;

SuperSprintManager* SuperSprintManager::GetInstance()
{
    if( spSuperSprintInstance == nullptr )
    {
        spSuperSprintInstance = new SuperSprintManager;
    }
    return spSuperSprintInstance;
}

void SuperSprintManager::DestroyInstance()
{
    delete spSuperSprintInstance;
    spSuperSprintInstance = nullptr;
}

SuperSprintManager::SuperSprintManager()
{
    rReleasePrintf( "SuperSprintManager created (Linux PoC stub)\n" );
}

SuperSprintManager::~SuperSprintManager() {}

//=============================================================================
// Pure3D device stand-in.
//
// BootupContext's constructor calls p3d::device->NewShader("simple") and holds
// a ref-counted shader for its lifetime.  Provide a single static device and a
// heap-allocated pddiShader (ref count starts at 0; BootupContext AddRef's it
// and Release's it in its destructor, which deletes it).
//=============================================================================
pddiShader* tContext::NewShader( const char* name )
{
    rReleasePrintf( "p3d::device->NewShader(\"%s\") returning dummy shader (Linux PoC stub)\n",
                    name != nullptr ? name : "<null>" );
    return new pddiShader;
}

namespace p3d
{
    static tContext sLinuxPocDevice;
    tContext* device = &sLinuxPocDevice;
}

//=============================================================================
// GUI system stand-in.
//
// This is where the bootup->frontend transition is driven.  See guisystem.h.
//=============================================================================
CGuiSystem* CGuiSystem::spInstance = nullptr;

CGuiSystem* CGuiSystem::CreateInstance()
{
    if( spInstance == nullptr )
    {
        spInstance = new CGuiSystem;
    }
    return spInstance;
}

CGuiSystem* CGuiSystem::GetInstance()
{
    if( spInstance == nullptr )
    {
        spInstance = new CGuiSystem;
    }
    return spInstance;
}

void CGuiSystem::DestroyInstance()
{
    delete spInstance;
    spInstance = nullptr;
}

void CGuiSystem::Init()
{
    // Stands in for the real Scrooby bootup screen chain reaching the license
    // screen.  The real CGuiScreenLicense::InitIntro calls StartLoadingSound()
    // (guiscreenlicense.cpp:155) and CGuiScreenLicense::InitRunning calls
    // ResetLicenseScreenDisplayTime() (guiscreenlicense.cpp:178); we reproduce
    // both here so BootupContext's m_soundLoadCompleted flag gets set (via the
    // LoadingManager AddCallback path in StartLoadingSound) and m_elapsedTime
    // leaves its initial -1 sentinel so OnUpdate begins counting the license
    // screen minimum-display timer.
    GetBootupContext()->StartLoadingSound();
    GetBootupContext()->ResetLicenseScreenDisplayTime();
    mLicenseScreenStarted = true;
}

void CGuiSystem::Update( unsigned int elapsedTime )
{
    (void)elapsedTime;
    // BootupContext::OnUpdate is the real driver of GUI_MSG_QUIT_BOOTUP once the
    // license-screen minimum time has elapsed and both load flags are set, so
    // there is nothing to poll here.
}

void CGuiSystem::RegisterUserInputHandlers( int controllerIDs )
{
    (void)controllerIDs;
}

void CGuiSystem::UnregisterUserInputHandlers( int controllerIDs )
{
    (void)controllerIDs;
}

void CGuiSystem::HandleMessage( eGuiMessage message,
                                unsigned int param1,
                                unsigned int param2 )
{
    (void)param1;
    (void)param2;

    switch( message )
    {
        case GUI_MSG_QUIT_BOOTUP:
        {
            // In the real game GUI_MSG_QUIT_BOOTUP shuts the current screen down
            // and the resulting GUI_MSG_WINDOW_FINISHED calls StartMovies()
            // (guimanagerbootup.cpp:314).  Collapse that to the single effect the
            // PoC needs.
            GetBootupContext()->StartMovies();
            break;
        }
        case GUI_MSG_RELEASE_BOOTUP:
        {
            break;
        }
    }
}

CGuiSystem::CGuiSystem()
    : mLicenseScreenStarted( false )
{
    rReleasePrintf( "CGuiSystem created (Linux PoC stub)\n" );
}

CGuiSystem::~CGuiSystem() {}
