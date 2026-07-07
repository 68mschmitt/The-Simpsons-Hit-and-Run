// Minimal GameplayManager shim for the Linux PoC.
//
// Only the surface BootupContext::StartMovies (skipfe path) needs is declared:
// the singleton accessors and SetLevelIndex/SetMissionIndex.  MissionManager and
// SuperSprintManager derive from this, matching the real hierarchy so the
// SetGameplayManager() calls type-check.
#ifndef GAMEPLAYMANAGER_H
#define GAMEPLAYMANAGER_H

#include <render/enums/renderenums.h>

class GameplayManager
{
public:
    static GameplayManager* GetInstance();
    static void SetInstance( GameplayManager* pInstance );

    virtual void SetLevelIndex( RenderEnums::LevelEnum level );
    virtual void SetMissionIndex( RenderEnums::MissionEnum mission );

protected:
    GameplayManager();
    virtual ~GameplayManager();

    static GameplayManager* spInstance;
};

inline GameplayManager* GetGameplayManager() { return GameplayManager::GetInstance(); }
inline void SetGameplayManager( GameplayManager* pInstance ) { GameplayManager::SetInstance( pInstance ); }

#endif // GAMEPLAYMANAGER_H
