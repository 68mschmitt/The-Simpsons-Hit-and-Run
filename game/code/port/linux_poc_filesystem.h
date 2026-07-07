//=============================================================================
// Linux PoC filesystem shim.
//
// Records every asset path the game requests, normalizes legacy Windows-style
// paths, and optionally probes an on-disk data root (e.g. a copy of the
// original game/cd tree) so missing files and case-sensitivity problems are
// diagnosed before any real asset loading is attempted.
//=============================================================================

#ifndef LINUX_POC_FILESYSTEM_H
#define LINUX_POC_FILESYSTEM_H

#include <string>

namespace LinuxPocFileSystem
{
    enum class ProbeStatus
    {
        NoDataRoot,      // no --data-root configured; request recorded only
        Found,           // exact path exists under the data root
        FoundCaseFixed,  // exists only via case-insensitive component matching
        Missing          // no match under the data root
    };

    struct ProbeResult
    {
        ProbeStatus status = ProbeStatus::NoDataRoot;
        std::string normalizedPath;  // forward slashes, no drive letter/leading ./
        std::string resolvedPath;    // actual on-disk path when found
    };

    // Convert a legacy game path ("ART\\FRONTEND\\dialog.p3d", "\\\\cd\\...")
    // to a normalized relative POSIX-style path.
    std::string NormalizePath(const char* rawPath);

    // Record a request coming from the loading shims and probe the data root.
    // 'source' is a short tag such as "async", "sync", or "cement".
    // 'section' may be null.
    ProbeResult RecordRequest(const char* rawPath, const char* source, const char* section);

    // Write the accumulated manifest to the configured manifest path (if any)
    // and log a one-line summary. Safe to call when nothing was recorded.
    void WriteManifest();
}

#endif // LINUX_POC_FILESYSTEM_H
