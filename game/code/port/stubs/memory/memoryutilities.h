// Minimal memory diagnostics shim for the Linux PoC.
#ifndef MEMORYUTILITIES_H
#define MEMORYUTILITIES_H

#include <cstddef>

struct IRadMemoryAllocator;

namespace Memory
{
    inline void InitializeMemoryUtilities() {}
    inline float GetFreeMemoryEntropy(IRadMemoryAllocator*) { return 0.0f; }
    inline std::size_t GetFreeMemoryProfile() { return 0; }
    inline std::size_t GetLargestFreeBlock() { return 0; }
    inline std::size_t GetLargestFreeBlock(IRadMemoryAllocator*) { return 0; }
    inline void GetLargestNFreeBlocks(IRadMemoryAllocator*, const int, std::size_t[]) {}
    inline std::size_t GetMaxFreeMemory() { return 0; }
    inline std::size_t GetTotalMemoryFree() { return 0; }
    inline std::size_t GetTotalMemoryFreeLowWaterMark() { return 0; }
    inline std::size_t GetTotalMemoryUnavailable() { return 0; }
    inline std::size_t GetTotalMemoryUsed() { return 0; }
    inline void PrintMemoryStatsToTty() {}
}

#endif // MEMORYUTILITIES_H
