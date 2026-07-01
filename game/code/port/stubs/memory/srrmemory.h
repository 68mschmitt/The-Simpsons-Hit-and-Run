// Minimal SRR memory shim for the Linux PoC.
#ifndef SRRMEMORY_H
#define SRRMEMORY_H

#include <cstddef>
#include <new>
#include <radmemory.hpp>

extern bool gMemorySystemInitialized;

enum GameMemoryAllocator
{
    GMA_DEFAULT = RADMEMORY_ALLOC_DEFAULT,
    GMA_TEMP = RADMEMORY_ALLOC_TEMP,
    GMA_PERSISTENT = 3,
    GMA_LEVEL,
    GMA_LEVEL_MOVIE,
    GMA_LEVEL_FE,
    GMA_LEVEL_ZONE,
    GMA_LEVEL_OTHER,
    GMA_LEVEL_HUD,
    GMA_LEVEL_MISSION,
    GMA_LEVEL_AUDIO,
    GMA_DEBUG,
    GMA_SPECIAL,
    GMA_MUSIC,
    GMA_AUDIO_PERSISTENT,
    GMA_SMALL_ALLOC,
    GMA_CHARS_AND_GAGS = GMA_LEVEL_OTHER,
    GMA_ANYWHERE_IN_LEVEL = 25,
    GMA_ANYWHERE_IN_FE,
    GMA_EITHER_OTHER_OR_ZONE,
    GMA_ALLOCATOR_SEARCH = ALLOCATOR_SEARCH,
    NUM_GAME_MEMORY_ALLOCATORS
};

void* operator new(std::size_t size);
void operator delete(void* pMemory) noexcept;
void operator delete(void* pMemory, std::size_t size) noexcept;

void* operator new[](std::size_t size);
void operator delete[](void* pMemory) noexcept;
void operator delete[](void* pMemory, std::size_t size) noexcept;

void* operator new(std::size_t size, GameMemoryAllocator allocator);
void operator delete(void* pMemory, GameMemoryAllocator allocator) noexcept;

void* operator new[](std::size_t size, GameMemoryAllocator allocator);
void operator delete[](void* pMemory, GameMemoryAllocator allocator) noexcept;

class HeapManager
{
public:
    static HeapManager* GetInstance();
    static bool IsCreated();
    static void DestroyInstance();

    void PrepareHeapsStartup() {}
    void PrepareHeapsFeCleanup() {}
    void PrepareHeapsFeSetup() {}
    void PrepareHeapsInGame() {}
    void PrepareHeapsSuperSprint() {}

    int GetLoadedUsageFE() { return 0; }
    int GetLoadedUsageInGame() { return 0; }
    int GetLoadedUsageSuperSprint() { return 0; }
    int GetLoadedUsage(GameMemoryAllocator) { return 0; }

    void PushHeap(GameMemoryAllocator alloc) { mCurrentHeap = alloc; }
    void PopHeap(GameMemoryAllocator) { mCurrentHeap = GMA_DEFAULT; }
    GameMemoryAllocator GetCurrentHeap() const { return mCurrentHeap; }

    radMemoryAllocator GetCurrentAllocator() { return static_cast<radMemoryAllocator>(mCurrentHeap); }
    radMemoryAllocator SetCurrentAllocator(radMemoryAllocator allocator)
    {
        const radMemoryAllocator previous = GetCurrentAllocator();
        mCurrentHeap = static_cast<GameMemoryAllocator>(allocator);
        return previous;
    }

    void DumpHeapStats(bool text = false) { (void)text; }
    void DumpArtStats() {}
    void ResetArtStats() {}

private:
    HeapManager() : mCurrentHeap(GMA_DEFAULT) {}
    GameMemoryAllocator mCurrentHeap;
};

HeapManager* HeapMgr();

inline void* FindFreeMemory(GameMemoryAllocator, std::size_t) { return nullptr; }
inline void LockPersistentHeap() {}
inline void PrintOutOfMemoryMessage(void*, radMemoryAllocator, const unsigned int) {}
inline void SetupAllocatorSearch(GameMemoryAllocator) {}
inline void SetMemoryIdentification(const char*) {}

#define MEMTRACK_PUSH_GROUP(string)
#define MEMTRACK_POP_GROUP(string)
#define MEMTRACK_PUSH_FLAG(string)
#define MEMTRACK_POP_FLAG(string)

#endif // SRRMEMORY_H
