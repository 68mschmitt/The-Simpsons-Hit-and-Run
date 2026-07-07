// Minimal ATCManager shim for the Linux PoC.
#ifndef ATCMANAGER_H
#define ATCMANAGER_H

class ATCManager
{
public:
    static ATCManager* GetInstance();
    static void DestroyInstance();

    void Init();

private:
    ATCManager();
    ~ATCManager();

    static ATCManager* spInstance;
};

inline ATCManager* GetATCManager() { return ATCManager::GetInstance(); }

#endif // ATCMANAGER_H
