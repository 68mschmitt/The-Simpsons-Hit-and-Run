// Minimal SoundManager shim for the Linux PoC.
#ifndef _SOUNDMANAGER_H
#define _SOUNDMANAGER_H

#include <contexts/contextenum.h>
#include <events/eventenum.h>

class SoundManager
{
public:
    static SoundManager* CreateInstance(bool muteSound, bool noMusic, bool noEffects, bool noDialogue);
    static SoundManager* GetInstance();
    static void DestroyInstance();

    void HandleEvent(EventEnum id, void* pEventData);

    void Update();
    void UpdateOncePerFrame(unsigned int elapsedTime,
                            ContextEnum context,
                            bool useContext = true,
                            bool isPausedForErrors = false);

    void OnBootupStart();
    void OnBootupComplete();

private:
    SoundManager(bool muteSound, bool noMusic, bool noEffects, bool noDialogue);
    ~SoundManager();

    static SoundManager* spInstance;

    bool mMuted;
    bool mNoMusic;
    bool mNoEffects;
    bool mNoDialogue;
};

inline SoundManager* GetSoundManager() { return SoundManager::GetInstance(); }

#endif // _SOUNDMANAGER_H
