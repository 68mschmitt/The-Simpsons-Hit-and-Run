// Minimal CharacterSheetManager shim for the Linux PoC.
#ifndef CHARACTERSHEETMANAGER_H
#define CHARACTERSHEETMANAGER_H

class CharacterSheetManager
{
public:
    static CharacterSheetManager* GetInstance();
    static void DestroyInstance();

    void InitCharacterSheet();

private:
    CharacterSheetManager();
    ~CharacterSheetManager();

    static CharacterSheetManager* spInstance;
};

inline CharacterSheetManager* GetCharacterSheetManager() { return CharacterSheetManager::GetInstance(); }

#endif // CHARACTERSHEETMANAGER_H
