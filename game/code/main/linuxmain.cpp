//=============================================================================
// Linux-native PoC entry point.
//=============================================================================

#include <cstddef>
#include <cstdlib>
#include <cstring>

#include <raddebug.hpp>
#include <radtime.hpp>

#include <main/commandlineoptions.h>
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

    void PrintUsage(const char* executableName)
    {
        rReleasePrintf("Usage: %s [--frames N] [legacy-command-line-options...]\n",
                       executableName != NULL ? executableName : "srr2-linux-poc");
        rReleasePrintf("\n");
        rReleasePrintf("Linux PoC options:\n");
        rReleasePrintf("  --frames N      Run exactly N fixed frames before shutdown. Default: %u\n",
                       LinuxPocConfig::DefaultFixedFrameCount);
        rReleasePrintf("  --help          Show this help and exit.\n");
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

            const char* legacyOption = StripOptionPrefix(argument);
            if(legacyOption != NULL && legacyOption[0] != '\0')
            {
                CommandLineOptions::HandleOption(legacyOption);
            }
        }

        rDebugPrintf("*************************************************************************\n");
        return options;
    }

    class LinuxPocGame
    {
    public:
        explicit LinuxPocGame(LinuxPlatform* platform)
            : mPlatform(platform),
              mFrameCount(0),
              mInitialized(false)
        {
        }

        void Initialize()
        {
            rAssert(mPlatform != NULL);
            mPlatform->InitializePlatform();
            mInitialized = true;

            rReleasePrintf("Game initialized\n");
            rReleasePrintf("GameFlow context: ENTRY -> BOOTUP\n");
        }

        void Run(unsigned int frameLimit)
        {
            rAssert(mInitialized);

            for(unsigned int frame = 1; frame <= frameLimit; ++frame)
            {
                if(mPlatform->QuitRequested())
                {
                    rReleasePrintf("Quit requested before frame %u\n", frame);
                    break;
                }

                mFrameCount = frame;
                rReleasePrintf("Frame %u elapsed=%u ms\n",
                               mFrameCount,
                               LinuxPocConfig::FixedElapsedMilliseconds);
            }

            rReleasePrintf("GameFlow context: BOOTUP -> EXIT\n");
        }

        void Terminate()
        {
            if(!mInitialized)
            {
                return;
            }

            rReleasePrintf("Game terminated after %u frame(s)\n", mFrameCount);
            mInitialized = false;
        }

    private:
        LinuxPlatform* mPlatform;
        unsigned int mFrameCount;
        bool mInitialized;
    };
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

    LinuxPlatform::InitializeFoundation();
    CreateSingletonsLinuxPoc();

    LinuxPlatform* platform = LinuxPlatform::CreateInstance();
    rAssert(platform != NULL);

    LinuxPocGame game(platform);
    game.Initialize();
    game.Run(options.frameCount);
    game.Terminate();

    DestroySingletonsLinuxPoc();

    platform->ShutdownPlatform();
    LinuxPlatform::DestroyInstance();

    rReleasePrintf("Shutdown complete\n");
    return 0;
}
