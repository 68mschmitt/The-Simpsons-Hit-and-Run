// Minimal Pure3D load manager shim for the Linux PoC.
#ifndef P3D_LOADMANAGER_HPP
#define P3D_LOADMANAGER_HPP

namespace p3d
{
    class LoadManager
    {
    public:
        void SwitchTask()
        {
        }
    };

    inline LoadManager gLinuxPocLoadManager;
    inline LoadManager* loadManager = &gLinuxPocLoadManager;
}

#endif // P3D_LOADMANAGER_HPP
