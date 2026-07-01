// Minimal RAD debug shim for the Linux PoC.
#ifndef RADDEBUG_HPP
#define RADDEBUG_HPP

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace radpoc
{
    inline void VPrintf(const char* prefix, const char* format, va_list args)
    {
        if(prefix != nullptr && prefix[0] != '\0')
        {
            std::fputs(prefix, stdout);
        }
        std::vfprintf(stdout, format, args);
        std::fflush(stdout);
    }

    inline void Printf(const char* prefix, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        VPrintf(prefix, format, args);
        va_end(args);
    }

    [[noreturn]] inline void AssertFailedFmt(const char* expression,
                                             const char* file,
                                             int line,
                                             const char* format,
                                             ...)
    {
        std::fprintf(stderr, "Assertion failed: %s (%s:%d)\n", expression, file, line);
        if(format != nullptr)
        {
            va_list args;
            va_start(args, format);
            std::vfprintf(stderr, format, args);
            va_end(args);
            std::fputc('\n', stderr);
        }
        std::abort();
    }
}

inline void rDebugPrintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    radpoc::VPrintf("", format, args);
    va_end(args);
}

inline void rReleasePrintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    radpoc::VPrintf("", format, args);
    va_end(args);
}

inline void rTunePrintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    radpoc::VPrintf("", format, args);
    va_end(args);
}

#define rAssert(expression) \
    do \
    { \
        if(!(expression)) \
        { \
            radpoc::AssertFailedFmt(#expression, __FILE__, __LINE__, nullptr); \
        } \
    } while(false)

#define rAssertMsg(expression, ...) \
    do \
    { \
        if(!(expression)) \
        { \
            radpoc::AssertFailedFmt(#expression, __FILE__, __LINE__, __VA_ARGS__); \
        } \
    } while(false)

#define rReleaseAssert(expression) rAssert(expression)
#define rReleaseAssertMsg(expression, ...) rAssertMsg(expression, __VA_ARGS__)

#endif // RADDEBUG_HPP
