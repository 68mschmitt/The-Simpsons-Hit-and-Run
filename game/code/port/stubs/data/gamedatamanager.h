// Minimal GameDataManager shim for the Linux PoC.
#ifndef GAMEDATAMANAGER_H
#define GAMEDATAMANAGER_H

class GameDataManager
{
public:
    static GameDataManager* GetInstance();
    static void DestroyInstance();

    void Init();
    void Update( unsigned int elapsedTime );

private:
    GameDataManager();
    ~GameDataManager();

    static GameDataManager* spInstance;
};

inline GameDataManager* GetGameDataManager() { return GameDataManager::GetInstance(); }

#endif // GAMEDATAMANAGER_H
