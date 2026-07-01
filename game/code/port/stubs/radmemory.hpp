// Minimal RAD memory shim for the Linux PoC.
#ifndef RADMEMORY_HPP
#define RADMEMORY_HPP

#include <cstdlib>

using radMemoryAllocator = int;

constexpr radMemoryAllocator RADMEMORY_ALLOC_DEFAULT = 0;
constexpr radMemoryAllocator RADMEMORY_ALLOC_TEMP = 1;
constexpr radMemoryAllocator RADMEMORY_ALLOC_VMM = 2;
constexpr radMemoryAllocator ALLOCATOR_SEARCH = -1;

struct IRadMemoryHeap
{
    virtual ~IRadMemoryHeap() = default;
};

struct IRadMemoryAllocator
{
    virtual ~IRadMemoryAllocator() = default;
};

struct IRadMemoryActivityCallback
{
    virtual void MemoryAllocated(radMemoryAllocator allocator, void* address, unsigned int size) = 0;
    virtual void MemoryFreed(radMemoryAllocator allocator, void* address) = 0;

protected:
    virtual ~IRadMemoryActivityCallback() = default;
};

struct IRadMemorySetAllocatorCallback
{
    virtual radMemoryAllocator GetCurrentAllocator() = 0;
    virtual radMemoryAllocator SetCurrentAllocator(radMemoryAllocator allocator) = 0;

protected:
    virtual ~IRadMemorySetAllocatorCallback() = default;
};

inline void radMemoryMonitorService()
{
}

#endif // RADMEMORY_HPP
