//=============================================================================
// Linux PoC SoundManager no-op implementation.
//=============================================================================

#include <sound/soundmanager.h>

#include <raddebug.hpp>

SoundManager* SoundManager::spInstance = nullptr;

SoundManager* SoundManager::CreateInstance(bool muteSound,
                                           bool noMusic,
                                           bool noEffects,
                                           bool noDialogue)
{
    rAssertMsg(spInstance == nullptr, "Trying to create more than one SoundManager instance");
    spInstance = new SoundManager(muteSound, noMusic, noEffects, noDialogue);
    return spInstance;
}

SoundManager* SoundManager::GetInstance()
{
    return spInstance;
}

void SoundManager::DestroyInstance()
{
    delete spInstance;
    spInstance = nullptr;
}

void SoundManager::HandleEvent(EventEnum id, void* pEventData)
{
    (void)id;
    (void)pEventData;
}

void SoundManager::Update()
{
}

void SoundManager::UpdateOncePerFrame(unsigned int elapsedTime,
                                      ContextEnum context,
                                      bool useContext,
                                      bool isPausedForErrors)
{
    (void)elapsedTime;
    (void)context;
    (void)useContext;
    (void)isPausedForErrors;
}

void SoundManager::OnBootupStart()
{
    rReleasePrintf("SoundManager bootup start (Linux PoC stub)\n");
}

void SoundManager::OnBootupComplete()
{
    rReleasePrintf("SoundManager bootup complete (Linux PoC stub)\n");
}

SoundManager::SoundManager(bool muteSound, bool noMusic, bool noEffects, bool noDialogue)
    : mMuted(muteSound),
      mNoMusic(noMusic),
      mNoEffects(noEffects),
      mNoDialogue(noDialogue)
{
    rReleasePrintf("SoundManager initialized (Linux PoC stub, mute=%s music=%s effects=%s dialogue=%s)\n",
                   mMuted ? "true" : "false",
                   mNoMusic ? "off" : "on",
                   mNoEffects ? "off" : "on",
                   mNoDialogue ? "off" : "on");
}

SoundManager::~SoundManager()
{
    rReleasePrintf("SoundManager shut down (Linux PoC stub)\n");
}
