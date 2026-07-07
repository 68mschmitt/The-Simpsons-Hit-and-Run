// Minimal Pure3D context shim for the Linux PoC.
//
// BootupContext only touches p3d::device->NewShader("simple") to build its
// shared shader, so this stub declares just enough of the Pure3D context to
// hand back a ref-counted dummy pddiShader (see linux_poc_bootup_stubs.cpp).
#ifndef P3D_CONTEXT_HPP
#define P3D_CONTEXT_HPP

#include <pddi/pddi.hpp>

class tContext
{
public:
    pddiShader* NewShader(const char* name);
};

namespace p3d
{
    extern tContext* device;
}

#endif // P3D_CONTEXT_HPP
