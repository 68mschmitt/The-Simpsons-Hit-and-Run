// Minimal SuperSprintManager shim for the Linux PoC.
#ifndef SUPERSPRINTMANAGER_H
#define SUPERSPRINTMANAGER_H

#include <mission/gameplaymanager.h>

class SuperSprintManager : public GameplayManager
{
public:
    static SuperSprintManager* GetInstance();
    static void DestroyInstance();

private:
    SuperSprintManager();
    ~SuperSprintManager();

    static SuperSprintManager* spSuperSprintInstance;
};

inline SuperSprintManager* GetSSM() { return SuperSprintManager::GetInstance(); }

#endif // SUPERSPRINTMANAGER_H
