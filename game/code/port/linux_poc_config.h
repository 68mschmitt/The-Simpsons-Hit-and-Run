//=============================================================================
// Linux-native PoC configuration.
//=============================================================================

#ifndef LINUX_POC_CONFIG_H
#define LINUX_POC_CONFIG_H

#include <string>

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

    // Optional root of an on-disk game data tree (a copy of game/cd). When
    // set, every asset request is probed against it. Empty means "record
    // requests only".
    inline std::string RuntimeDataRoot;

    // Optional output path for the asset request manifest written at
    // shutdown. Empty disables the manifest file (the summary still logs).
    inline std::string RuntimeAssetManifestPath;
}

#endif // LINUX_POC_CONFIG_H
