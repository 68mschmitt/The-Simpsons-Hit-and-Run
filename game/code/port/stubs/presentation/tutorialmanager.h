// Minimal TutorialManager shim for the Linux PoC.
#ifndef TUTORIALMANAGER_H
#define TUTORIALMANAGER_H

class TutorialManager
{
public:
    static TutorialManager* GetInstance();
    static void DestroyInstance();

    void Initialize();

private:
    TutorialManager();
    ~TutorialManager();

    static TutorialManager* spInstance;
};

inline TutorialManager* GetTutorialManager() { return TutorialManager::GetInstance(); }

#endif // TUTORIALMANAGER_H
