// Minimal LoadingManager shim for the Linux PoC.
#ifndef LOADINGMANAGER_H
#define LOADINGMANAGER_H

#include <memory/srrmemory.h>

enum FileHandlerEnum
{
    FILEHANDLER_PURE3D = 0,
    FILEHANDLER_UNKNOWN
};

using CementFileHandle = int;

class LoadingManager
{
public:
    struct ProcessRequestsCallback
    {
        virtual void OnProcessRequestsComplete(void* pUserData) = 0;

    protected:
        virtual ~ProcessRequestsCallback() = default;
    };

    static LoadingManager* CreateInstance();
    static LoadingManager* GetInstance();
    static void DestroyInstance();

    CementFileHandle RegisterCementLibrary(const char* filename);
    void UnregisterCementLibrary(CementFileHandle handle);

    void AddCallback(ProcessRequestsCallback* pCallback = nullptr, void* pUserData = nullptr);
    void AddRequest(FileHandlerEnum handlerType,
                    const char* filename,
                    GameMemoryAllocator heap,
                    const char* sectionName = nullptr,
                    const char* groupTag = nullptr,
                    ProcessRequestsCallback* pCallback = nullptr,
                    void* pUserData = nullptr);

    void LoadSync(FileHandlerEnum handlerType,
                  const char* filename,
                  GameMemoryAllocator heap = GMA_DEFAULT,
                  const char* sectionName = nullptr);

    bool IsLoading();
    int GetRequestHead() const { return 0; }
    int GetRequestTail() const { return 0; }
    int GetNumCurrentRequests() const { return 0; }
    void CancelPendingRequests() {}

private:
    LoadingManager();
    ~LoadingManager();

    static LoadingManager* spInstance;
};

inline LoadingManager* GetLoadingManager() { return LoadingManager::GetInstance(); }

#endif // LOADINGMANAGER_H
