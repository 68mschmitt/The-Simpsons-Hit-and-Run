// Minimal CharacterManager shim for the Linux PoC.
#ifndef CHARACTERMANAGER_H
#define CHARACTERMANAGER_H

class CharacterManager
{
public:
    static CharacterManager* GetInstance();
    static void DestroyInstance();

    void PreLoad();

private:
    CharacterManager();
    ~CharacterManager();

    static CharacterManager* spInstance;
};

inline CharacterManager* GetCharacterManager() { return CharacterManager::GetInstance(); }

#endif // CHARACTERMANAGER_H
