//=============================================================================
// Lightweight contexts for the Linux-native PoC GameFlow slice.
//=============================================================================

#include <contexts/linux_poc_contexts.h>

#include <contexts/context.h>
#include <gameflow/gameflow.h>
#include <loading/loadingmanager.h>
#include <memory/srrmemory.h>
#include <raddebug.hpp>
#include <sound/soundmanager.h>

namespace
{
    class LinuxPocContext : public Context,
                            public LoadingManager::ProcessRequestsCallback
    {
    public:
        explicit LinuxPocContext(ContextEnum context)
            : mContext(context),
              mUpdateCount(0),
              mBootupLoadComplete(false),
              mSoundLoadComplete(false)
        {
        }

        void OnProcessRequestsComplete(void* pUserData) override
        {
            if(pUserData == GetSoundManager())
            {
                mSoundLoadComplete = true;
                rReleasePrintf("%s fake sound loading complete\n", LinuxPocContextName(mContext));
            }
            else
            {
                mBootupLoadComplete = true;
                rReleasePrintf("%s fake asset loading complete\n", LinuxPocContextName(mContext));
            }
        }

    protected:
        void OnStart(ContextEnum previousContext) override
        {
            m_state = S_ACTIVE;
            mUpdateCount = 0;
            rReleasePrintf("%s::OnStart(previous=%s)\n",
                           LinuxPocContextName(mContext),
                           LinuxPocContextName(previousContext));

            if(mContext == CONTEXT_BOOTUP)
            {
                mBootupLoadComplete = false;
                mSoundLoadComplete = false;

                GetSoundManager()->OnBootupStart();
                GetLoadingManager()->AddRequest(FILEHANDLER_PURE3D,
                                                "linux-poc/fake-bootup.p3d",
                                                GMA_DEFAULT,
                                                "LinuxPocBootup",
                                                nullptr,
                                                this,
                                                nullptr);
                GetLoadingManager()->AddCallback(this, GetSoundManager());
            }
        }

        void OnStop(ContextEnum nextContext) override
        {
            rReleasePrintf("%s::OnStop(next=%s)\n",
                           LinuxPocContextName(mContext),
                           LinuxPocContextName(nextContext));
            m_state = S_EXIT;
        }

        void OnUpdate(unsigned int elapsedTime) override
        {
            ++mUpdateCount;

            if(mContext == CONTEXT_BOOTUP)
            {
                if(mBootupLoadComplete && mSoundLoadComplete && mUpdateCount == 1)
                {
                    GetSoundManager()->OnBootupComplete();
                    rReleasePrintf("CONTEXT_BOOTUP ready; holding in boot context for fixed-frame PoC\n");
                }
            }

            rReleasePrintf("%s::OnUpdate(frame-in-context=%u elapsed=%u ms)\n",
                           LinuxPocContextName(mContext),
                           mUpdateCount,
                           elapsedTime);
        }

        void OnSuspend() override
        {
            rReleasePrintf("%s::OnSuspend\n", LinuxPocContextName(mContext));
        }

        void OnResume() override
        {
            rReleasePrintf("%s::OnResume\n", LinuxPocContextName(mContext));
        }

        void OnHandleEvent(EventEnum id, void* pEventData) override
        {
            (void)id;
            (void)pEventData;
        }

    private:
        ContextEnum mContext;
        unsigned int mUpdateCount;
        bool mBootupLoadComplete;
        bool mSoundLoadComplete;
    };
}

Context* CreateLinuxPocContext(ContextEnum context)
{
    return new(GMA_PERSISTENT) LinuxPocContext(context);
}

const char* LinuxPocContextName(ContextEnum context)
{
    switch(context)
    {
        case CONTEXT_ENTRY: return "CONTEXT_ENTRY";
        case CONTEXT_BOOTUP: return "CONTEXT_BOOTUP";
        case CONTEXT_FRONTEND: return "CONTEXT_FRONTEND";
        case CONTEXT_LOADING_DEMO: return "CONTEXT_LOADING_DEMO";
        case CONTEXT_DEMO: return "CONTEXT_DEMO";
        case CONTEXT_SUPERSPRINT_FE: return "CONTEXT_SUPERSPRINT_FE";
        case CONTEXT_LOADING_SUPERSPRINT: return "CONTEXT_LOADING_SUPERSPRINT";
        case CONTEXT_SUPERSPRINT: return "CONTEXT_SUPERSPRINT";
        case CONTEXT_LOADING_GAMEPLAY: return "CONTEXT_LOADING_GAMEPLAY";
        case CONTEXT_GAMEPLAY: return "CONTEXT_GAMEPLAY";
        case CONTEXT_PAUSE: return "CONTEXT_PAUSE";
        case CONTEXT_EXIT: return "CONTEXT_EXIT";
        case NUM_CONTEXTS: return "NUM_CONTEXTS";
    }

    return "CONTEXT_UNKNOWN";
}
