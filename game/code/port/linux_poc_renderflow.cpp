//=============================================================================
// Linux PoC RenderFlow no-op implementation.
//=============================================================================

#include <render/RenderFlow/RenderFlow.h>

#include <raddebug.hpp>

RenderFlow* RenderFlow::spInstance = nullptr;

RenderFlow* RenderFlow::CreateInstance()
{
    rAssertMsg(spInstance == nullptr, "Trying to create more than one RenderFlow instance");
    spInstance = new RenderFlow;
    return spInstance;
}

RenderFlow* RenderFlow::GetInstance()
{
    return spInstance;
}

void RenderFlow::DestroyInstance()
{
    delete spInstance;
    spInstance = nullptr;
}

void RenderFlow::DoAllRegistration()
{
    if(!mRegistered)
    {
        rReleasePrintf("RenderFlow registered (Linux PoC stub)\n");
        mRegistered = true;
    }
}

void RenderFlow::OnTimerDone(unsigned int iElapsedTime, void* pUserData)
{
    (void)iElapsedTime;
    (void)pUserData;
}

RenderFlow::RenderFlow()
    : mRegistered(false)
{
    rReleasePrintf("RenderFlow initialized (Linux PoC stub)\n");
}

RenderFlow::~RenderFlow()
{
    rReleasePrintf("RenderFlow shut down (Linux PoC stub)\n");
}
