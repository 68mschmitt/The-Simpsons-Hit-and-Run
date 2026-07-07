// Minimal PresentationManager shim for the Linux PoC.
#ifndef PRESENTATION_H
#define PRESENTATION_H

class PresentationManager
{
public:
    static PresentationManager* GetInstance();
    static void DestroyInstance();

    void InitializePlayerDrawable();
    void Update( unsigned int elapsedTime );
    bool IsQueueEmpty() const;

private:
    PresentationManager();
    ~PresentationManager();

    static PresentationManager* spInstance;
};

inline PresentationManager* GetPresentationManager() { return PresentationManager::GetInstance(); }

#endif // PRESENTATION_H
