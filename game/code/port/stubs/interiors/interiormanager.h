// Minimal InteriorManager shim for the Linux PoC.
#ifndef INTERIORMANAGER_H
#define INTERIORMANAGER_H

class InteriorManager
{
public:
    static InteriorManager* GetInstance();
    static void DestroyInstance();

    void OnBootupStart();

private:
    InteriorManager();
    ~InteriorManager();

    static InteriorManager* spInstance;
};

inline InteriorManager* GetInteriorManager() { return InteriorManager::GetInstance(); }

#endif // INTERIORMANAGER_H
