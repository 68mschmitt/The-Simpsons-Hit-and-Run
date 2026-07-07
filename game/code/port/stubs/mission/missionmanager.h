// Minimal MissionManager shim for the Linux PoC.
#ifndef MISSIONMANAGER_H
#define MISSIONMANAGER_H

#include <mission/gameplaymanager.h>

class MissionManager : public GameplayManager
{
public:
    static MissionManager* GetInstance();
    static void DestroyInstance();

private:
    MissionManager();
    ~MissionManager();

    static MissionManager* spMissionInstance;
};

inline MissionManager* GetMissionManager() { return MissionManager::GetInstance(); }

#endif // MISSIONMANAGER_H
