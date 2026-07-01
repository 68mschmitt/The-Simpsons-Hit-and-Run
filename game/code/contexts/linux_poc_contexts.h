//=============================================================================
// Lightweight contexts for the Linux-native PoC GameFlow slice.
//=============================================================================

#ifndef LINUX_POC_CONTEXTS_H
#define LINUX_POC_CONTEXTS_H

#include <contexts/contextenum.h>

class Context;

Context* CreateLinuxPocContext(ContextEnum context);
const char* LinuxPocContextName(ContextEnum context);

#endif // LINUX_POC_CONTEXTS_H
