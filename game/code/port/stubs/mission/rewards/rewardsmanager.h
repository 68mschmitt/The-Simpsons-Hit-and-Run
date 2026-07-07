// Minimal RewardsManager shim for the Linux PoC.
#ifndef REWARDSMANAGER_H
#define REWARDSMANAGER_H

class RewardsManager
{
public:
    static RewardsManager* GetInstance();
    static void DestroyInstance();

    void LoadScript();

private:
    RewardsManager();
    ~RewardsManager();

    static RewardsManager* spInstance;
};

inline RewardsManager* GetRewardsManager() { return RewardsManager::GetInstance(); }

#endif // REWARDSMANAGER_H
