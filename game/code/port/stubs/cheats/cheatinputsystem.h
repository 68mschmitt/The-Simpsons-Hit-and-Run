// Minimal CheatInputSystem shim for the Linux PoC.
#ifndef CHEATINPUTSYSTEM_H
#define CHEATINPUTSYSTEM_H

// The real cheat id enum is portable, so reuse it directly.
#include <cheats/cheats.h>

class CheatInputSystem
{
public:
    static CheatInputSystem* GetInstance();
    static void DestroyInstance();

    void Init();
    void SetCheatEnabled( eCheatID cheatID, bool enable );
    bool IsCheatEnabled( eCheatID cheatID ) const;

private:
    CheatInputSystem();
    ~CheatInputSystem();

    static CheatInputSystem* spInstance;
};

inline CheatInputSystem* GetCheatInputSystem() { return CheatInputSystem::GetInstance(); }

#endif // CHEATINPUTSYSTEM_H
