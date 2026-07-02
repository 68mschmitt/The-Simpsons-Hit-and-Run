//=============================================================================
// Linux-native PoC entry point.
//=============================================================================

#include <cstddef>
#include <cstdlib>
#include <cstring>

#include <raddebug.hpp>
#include <radtime.hpp>

#include <main/commandlineoptions.h>
#include <main/game.h>
#include <main/linuxplatform.h>
#include <main/singletons_linux_poc.h>
#include <port/linux_poc_config.h>

// CommandLineOptions references these legacy globals for some debug switches.
bool g_AllowDebugOutput = true;
bool gFruitless = true;
bool gTuneSound = false;

namespace
{
    struct LinuxPocOptions
    {
        unsigned int frameCount = LinuxPocConfig::DefaultFixedFrameCount;
        LinuxPocConfig::HostWindowMode hostWindowMode = LinuxPocConfig::HostWindowMode::Auto;
        unsigned int windowWidth = LinuxPocConfig::DefaultWindowWidth;
        unsigned int windowHeight = LinuxPocConfig::DefaultWindowHeight;
        bool showHelp = false;
    };

    const char* StripOptionPrefix(const char* argument)
    {
        while(argument != NULL && argument[0] == '-')
        {
            ++argument;
        }
        return argument;
    }

    bool ParseUnsigned(const char* text, unsigned int* value)
    {
        if(text == NULL || text[0] == '\0' || value == NULL)
        {
            return false;
        }

        char* end = NULL;
        const unsigned long parsed = std::strtoul(text, &end, 10);
        if(end == text || *end != '\0')
        {
            return false;
        }

        *value = static_cast<unsigned int>(parsed);
        return true;
    }

    bool ParseWindowSize(const char* text, unsigned int* width, unsigned int* height)
    {
        if(text == NULL || text[0] == '\0' || width == NULL || height == NULL)
        {
            return false;
        }

        char* widthEnd = NULL;
        const unsigned long parsedWidth = std::strtoul(text, &widthEnd, 10);
        if(widthEnd == text || (*widthEnd != 'x' && *widthEnd != 'X'))
        {
            return false;
        }

        char* heightEnd = NULL;
        const unsigned long parsedHeight = std::strtoul(widthEnd + 1, &heightEnd, 10);
        if(heightEnd == widthEnd + 1 || *heightEnd != '\0' || parsedWidth == 0 || parsedHeight == 0)
        {
            return false;
        }

        *width = static_cast<unsigned int>(parsedWidth);
        *height = static_cast<unsigned int>(parsedHeight);
        return true;
    }

    void PrintUsage(const char* executableName)
    {
        rReleasePrintf("Usage: %s [--frames N|--until-quit] [--headless|--window] [legacy-command-line-options...]\n",
                       executableName != NULL ? executableName : "srr2-linux-poc");
        rReleasePrintf("\n");
        rReleasePrintf("Linux PoC options:\n");
        rReleasePrintf("  --frames N          Run N fixed frames before shutdown; N=0 means until quit. Default: %u\n",
                       LinuxPocConfig::DefaultFixedFrameCount);
        rReleasePrintf("  --until-quit        Run until the SDL window is closed, Escape is pressed, or another quit path fires.\n");
        rReleasePrintf("  --headless          Disable the SDL window even when SDL2 support is compiled in.\n");
        rReleasePrintf("  --window            Try to create an SDL window even if no DISPLAY/WAYLAND_DISPLAY is detected.\n");
        rReleasePrintf("  --window-size WxH   Set SDL window size. Default: %ux%u\n",
                       LinuxPocConfig::DefaultWindowWidth,
                       LinuxPocConfig::DefaultWindowHeight);
        rReleasePrintf("  --help              Show this help and exit.\n");
        rReleasePrintf("\n");
        rReleasePrintf("Other options are passed to CommandLineOptions::HandleOption after leading '-' characters are stripped.\n");
    }

    LinuxPocOptions ProcessCommandLineArguments(int argc, char** argv)
    {
        LinuxPocOptions options;

        CommandLineOptions::InitDefaults();

        rDebugPrintf("*************************************************************************\n");
        rDebugPrintf("Command Line Args:\n");

        for(int argIndex = 1; argIndex < argc; ++argIndex)
        {
            rDebugPrintf("arg%d: %s\n", argIndex - 1, argv[argIndex] != NULL ? argv[argIndex] : "");
        }

        for(int i = 1; i < argc; ++i)
        {
            const char* argument = argv[i];

            if(argument == NULL)
            {
                continue;
            }

            if(std::strcmp(argument, "--help") == 0 ||
               std::strcmp(argument, "-h") == 0 ||
               std::strcmp(argument, "help") == 0)
            {
                options.showHelp = true;
                continue;
            }

            if(std::strcmp(argument, "--frames") == 0 || std::strcmp(argument, "-frames") == 0)
            {
                if(i + 1 < argc && ParseUnsigned(argv[i + 1], &options.frameCount))
                {
                    ++i;
                }
                else
                {
                    rReleasePrintf("Invalid or missing value for --frames; using default %u\n",
                                   options.frameCount);
                }
                continue;
            }

            const char framePrefix[] = "--frames=";
            const std::size_t framePrefixLength = sizeof(framePrefix) - 1;
            if(std::strncmp(argument, framePrefix, framePrefixLength) == 0)
            {
                if(!ParseUnsigned(argument + framePrefixLength, &options.frameCount))
                {
                    rReleasePrintf("Invalid value for --frames; using default %u\n", options.frameCount);
                }
                continue;
            }

            if(std::strcmp(argument, "--until-quit") == 0 || std::strcmp(argument, "-until-quit") == 0)
            {
                options.frameCount = 0;
                continue;
            }

            if(std::strcmp(argument, "--headless") == 0 ||
               std::strcmp(argument, "--no-window") == 0 ||
               std::strcmp(argument, "-headless") == 0)
            {
                options.hostWindowMode = LinuxPocConfig::HostWindowMode::Disabled;
                continue;
            }

            if(std::strcmp(argument, "--window") == 0 ||
               std::strcmp(argument, "--sdl") == 0 ||
               std::strcmp(argument, "-window") == 0)
            {
                options.hostWindowMode = LinuxPocConfig::HostWindowMode::Required;
                continue;
            }

            if(std::strcmp(argument, "--window-size") == 0 || std::strcmp(argument, "-window-size") == 0)
            {
                if(i + 1 < argc && ParseWindowSize(argv[i + 1], &options.windowWidth, &options.windowHeight))
                {
                    ++i;
                }
                else
                {
                    rReleasePrintf("Invalid or missing value for --window-size; using default %ux%u\n",
                                   options.windowWidth,
                                   options.windowHeight);
                }
                continue;
            }

            const char windowSizePrefix[] = "--window-size=";
            const std::size_t windowSizePrefixLength = sizeof(windowSizePrefix) - 1;
            if(std::strncmp(argument, windowSizePrefix, windowSizePrefixLength) == 0)
            {
                if(!ParseWindowSize(argument + windowSizePrefixLength, &options.windowWidth, &options.windowHeight))
                {
                    rReleasePrintf("Invalid value for --window-size; using default %ux%u\n",
                                   options.windowWidth,
                                   options.windowHeight);
                }
                continue;
            }

            const char* legacyOption = StripOptionPrefix(argument);
            if(legacyOption != NULL && legacyOption[0] != '\0')
            {
                CommandLineOptions::HandleOption(legacyOption);
            }
        }

        rDebugPrintf("*************************************************************************\n");
        return options;
    }

}

int main(int argc, char** argv)
{
    rReleasePrintf("SRR2 Linux native PoC starting\n");

    const LinuxPocOptions options = ProcessCommandLineArguments(argc, argv);
    if(options.showHelp)
    {
        PrintUsage(argc > 0 ? argv[0] : "srr2-linux-poc");
        return 0;
    }

    LinuxPocConfig::RuntimeFixedFrameCount = options.frameCount;
    LinuxPocConfig::RuntimeHostWindowMode = options.hostWindowMode;
    LinuxPocConfig::RuntimeWindowWidth = options.windowWidth;
    LinuxPocConfig::RuntimeWindowHeight = options.windowHeight;

    LinuxPlatform::InitializeFoundation();
    CreateSingletonsLinuxPoc();

    LinuxPlatform* platform = LinuxPlatform::CreateInstance();
    rAssert(platform != NULL);

    Game* game = Game::CreateInstance(platform);
    game->Initialize();
    game->Run();
    game->Terminate();
    Game::DestroyInstance();

    DestroySingletonsLinuxPoc();

    platform->ShutdownPlatform();
    LinuxPlatform::DestroyInstance();

    rReleasePrintf("Shutdown complete\n");
    return 0;
}
