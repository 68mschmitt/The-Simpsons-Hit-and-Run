// STL allocator compatibility shim for the Linux PoC.
#ifndef STLALLOCATORS_H
#define STLALLOCATORS_H

#include <memory>

template <class T>
using s2alloc = std::allocator<T>;

#endif // STLALLOCATORS_H
