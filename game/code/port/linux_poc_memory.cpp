//=============================================================================
// Linux PoC host-backed memory implementation.
//=============================================================================

#include <cstdlib>
#include <new>

#include <memory/srrmemory.h>

bool gMemorySystemInitialized = true;

namespace
{
    HeapManager* sHeapManager = nullptr;

    void* Allocate(std::size_t size)
    {
        if(size == 0)
        {
            size = 1;
        }

        if(void* memory = std::malloc(size))
        {
            return memory;
        }

        throw std::bad_alloc();
    }
}

void* operator new(std::size_t size)
{
    return Allocate(size);
}

void operator delete(void* pMemory) noexcept
{
    std::free(pMemory);
}

void operator delete(void* pMemory, std::size_t size) noexcept
{
    (void)size;
    std::free(pMemory);
}

void* operator new[](std::size_t size)
{
    return Allocate(size);
}

void operator delete[](void* pMemory) noexcept
{
    std::free(pMemory);
}

void operator delete[](void* pMemory, std::size_t size) noexcept
{
    (void)size;
    std::free(pMemory);
}

void* operator new(std::size_t size, GameMemoryAllocator allocator)
{
    (void)allocator;
    return ::operator new(size);
}

void operator delete(void* pMemory, GameMemoryAllocator allocator) noexcept
{
    (void)allocator;
    ::operator delete(pMemory);
}

void* operator new[](std::size_t size, GameMemoryAllocator allocator)
{
    (void)allocator;
    return ::operator new[](size);
}

void operator delete[](void* pMemory, GameMemoryAllocator allocator) noexcept
{
    (void)allocator;
    ::operator delete[](pMemory);
}

HeapManager* HeapManager::GetInstance()
{
    if(sHeapManager == nullptr)
    {
        sHeapManager = new HeapManager;
    }

    return sHeapManager;
}

bool HeapManager::IsCreated()
{
    return sHeapManager != nullptr;
}

void HeapManager::DestroyInstance()
{
    delete sHeapManager;
    sHeapManager = nullptr;
}

HeapManager* HeapMgr()
{
    return HeapManager::GetInstance();
}
