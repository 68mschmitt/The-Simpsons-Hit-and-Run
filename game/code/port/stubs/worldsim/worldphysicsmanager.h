// Minimal WorldPhysicsManager shim for the Linux PoC.
#ifndef WORLDPHYSICSMANAGER_H
#define WORLDPHYSICSMANAGER_H

class WorldPhysicsManager
{
public:
    static WorldPhysicsManager* GetInstance();
    static void DestroyInstance();

    void Init();

private:
    WorldPhysicsManager();
    ~WorldPhysicsManager();

    static WorldPhysicsManager* spInstance;
};

inline WorldPhysicsManager* GetWorldPhysicsManager() { return WorldPhysicsManager::GetInstance(); }

#endif // WORLDPHYSICSMANAGER_H
