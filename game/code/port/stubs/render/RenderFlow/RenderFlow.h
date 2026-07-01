// Minimal RenderFlow shim for the Linux PoC.
#ifndef RENDERFLOW_H
#define RENDERFLOW_H

#include <radtime.hpp>

class RenderFlow : public IRadTimerCallback
{
public:
    static RenderFlow* CreateInstance();
    static RenderFlow* GetInstance();
    static void DestroyInstance();

    void DoAllRegistration();
    void OnTimerDone(unsigned int iElapsedTime, void* pUserData) override;

private:
    RenderFlow();
    ~RenderFlow();

    static RenderFlow* spInstance;
    bool mRegistered;
};

inline RenderFlow* GetRenderFlow() { return RenderFlow::GetInstance(); }

#endif // RENDERFLOW_H
