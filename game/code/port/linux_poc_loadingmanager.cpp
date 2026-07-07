//=============================================================================
// Linux PoC LoadingManager fake-complete implementation.
//=============================================================================

#include <loading/loadingmanager.h>

#include <port/linux_poc_filesystem.h>

#include <raddebug.hpp>

LoadingManager* LoadingManager::spInstance = nullptr;

LoadingManager* LoadingManager::CreateInstance()
{
    rAssertMsg(spInstance == nullptr, "Trying to create more than one LoadingManager instance");
    spInstance = new LoadingManager;
    return spInstance;
}

LoadingManager* LoadingManager::GetInstance()
{
    return spInstance;
}

void LoadingManager::DestroyInstance()
{
    delete spInstance;
    spInstance = nullptr;
}

CementFileHandle LoadingManager::RegisterCementLibrary(const char* filename)
{
    rReleasePrintf("LoadingManager ignoring cement library registration: %s\n",
                   filename != nullptr ? filename : "<null>");
    LinuxPocFileSystem::RecordRequest(filename, "cement", nullptr);
    return 0;
}

void LoadingManager::UnregisterCementLibrary(CementFileHandle handle)
{
    (void)handle;
}

void LoadingManager::AddCallback(ProcessRequestsCallback* pCallback, void* pUserData)
{
    rReleasePrintf("LoadingManager completing queued requests immediately (Linux PoC stub)\n");
    if(pCallback != nullptr)
    {
        pCallback->OnProcessRequestsComplete(pUserData);
    }
}

void LoadingManager::AddRequest(FileHandlerEnum handlerType,
                                const char* filename,
                                GameMemoryAllocator heap,
                                const char* sectionName,
                                const char* groupTag,
                                ProcessRequestsCallback* pCallback,
                                void* pUserData)
{
    (void)handlerType;
    (void)heap;
    (void)groupTag;
    rReleasePrintf("LoadingManager fake request: file=%s section=%s\n",
                   filename != nullptr ? filename : "<null>",
                   sectionName != nullptr ? sectionName : "<none>");
    LinuxPocFileSystem::RecordRequest(filename, "async", sectionName);
    if(pCallback != nullptr)
    {
        pCallback->OnProcessRequestsComplete(pUserData);
    }
}

void LoadingManager::LoadSync(FileHandlerEnum handlerType,
                              const char* filename,
                              GameMemoryAllocator heap,
                              const char* sectionName)
{
    (void)handlerType;
    (void)heap;
    rReleasePrintf("LoadingManager fake sync load: file=%s section=%s\n",
                   filename != nullptr ? filename : "<null>",
                   sectionName != nullptr ? sectionName : "<none>");
    LinuxPocFileSystem::RecordRequest(filename, "sync", sectionName);
}

bool LoadingManager::IsLoading()
{
    return false;
}

LoadingManager::LoadingManager()
{
    rReleasePrintf("LoadingManager initialized (Linux PoC stub)\n");
}

LoadingManager::~LoadingManager()
{
    rReleasePrintf("LoadingManager shut down (Linux PoC stub)\n");
}
