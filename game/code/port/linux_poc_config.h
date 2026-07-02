//=============================================================================
// Linux-native PoC configuration.
//=============================================================================

#ifndef LINUX_POC_CONFIG_H
#define LINUX_POC_CONFIG_H

namespace LinuxPocConfig
{
    constexpr unsigned int DefaultFixedFrameCount = 300;
    constexpr unsigned int FixedElapsedMilliseconds = 16;
    constexpr unsigned int DefaultWindowWidth = 960;
    constexpr unsigned int DefaultWindowHeight = 540;

    enum class HostWindowMode
    {
        Auto,
        Disabled,
        Required
    };

    // A frame count of 0 means "run until the platform requests quit".
    inline unsigned int RuntimeFixedFrameCount = DefaultFixedFrameCount;
    inline HostWindowMode RuntimeHostWindowMode = HostWindowMode::Auto;
    inline unsigned int RuntimeWindowWidth = DefaultWindowWidth;
    inline unsigned int RuntimeWindowHeight = DefaultWindowHeight;
}

#endif // LINUX_POC_CONFIG_H
