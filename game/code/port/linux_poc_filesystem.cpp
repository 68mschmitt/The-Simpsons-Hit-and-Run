//=============================================================================
// Linux PoC filesystem shim implementation.
//=============================================================================

#include <port/linux_poc_filesystem.h>

#include <port/linux_poc_config.h>

#include <raddebug.hpp>

#include <cctype>
#include <cstdio>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace
{
    struct RequestRecord
    {
        std::string firstSource;
        std::string firstSection;
        LinuxPocFileSystem::ProbeStatus status = LinuxPocFileSystem::ProbeStatus::NoDataRoot;
        std::string resolvedPath;
        unsigned int requestCount = 0;
    };

    // Keyed by normalized path; ordered so the manifest is stable and readable.
    std::map<std::string, RequestRecord> sRequests;

    // Directory listing cache for case-insensitive probing.
    std::map<std::string, std::vector<std::string>> sDirectoryCache;

    std::string ToLower(const std::string& text)
    {
        std::string result = text;
        for(char& c : result)
        {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return result;
    }

    const std::vector<std::string>& ListDirectory(const std::string& directory)
    {
        auto cached = sDirectoryCache.find(directory);
        if(cached != sDirectoryCache.end())
        {
            return cached->second;
        }

        std::vector<std::string>& entries = sDirectoryCache[directory];
        std::error_code errorCode;
        for(const fs::directory_entry& entry : fs::directory_iterator(directory, errorCode))
        {
            entries.push_back(entry.path().filename().string());
        }
        return entries;
    }

    // Walk 'relativePath' below 'root' one component at a time. Exact-name
    // matches are checked against the directory listing rather than
    // fs::exists so case mismatches are detected even on case-insensitive
    // filesystems (macOS APFS) where they would silently "exist". Returns
    // true and fills 'resolved' when every component matched; 'caseFixed'
    // reports whether any component needed a case-insensitive match.
    bool ResolveCaseInsensitive(const std::string& root,
                                const std::string& relativePath,
                                std::string* resolved,
                                bool* caseFixed)
    {
        std::string current = root;
        *caseFixed = false;

        std::size_t start = 0;
        while(start < relativePath.size())
        {
            std::size_t end = relativePath.find('/', start);
            if(end == std::string::npos)
            {
                end = relativePath.size();
            }
            const std::string component = relativePath.substr(start, end - start);
            start = end + 1;

            if(component.empty())
            {
                continue;
            }

            const std::vector<std::string>& entries = ListDirectory(current);

            bool matched = false;
            for(const std::string& entry : entries)
            {
                if(entry == component)
                {
                    current += "/" + entry;
                    matched = true;
                    break;
                }
            }

            if(!matched)
            {
                const std::string wanted = ToLower(component);
                for(const std::string& entry : entries)
                {
                    if(ToLower(entry) == wanted)
                    {
                        current += "/" + entry;
                        *caseFixed = true;
                        matched = true;
                        break;
                    }
                }
            }

            if(!matched)
            {
                return false;
            }
        }

        *resolved = current;
        return true;
    }

    const char* StatusName(LinuxPocFileSystem::ProbeStatus status)
    {
        switch(status)
        {
            case LinuxPocFileSystem::ProbeStatus::NoDataRoot:     return "recorded";
            case LinuxPocFileSystem::ProbeStatus::Found:          return "found";
            case LinuxPocFileSystem::ProbeStatus::FoundCaseFixed: return "case-mismatch";
            case LinuxPocFileSystem::ProbeStatus::Missing:        return "missing";
        }
        return "unknown";
    }
}

namespace LinuxPocFileSystem
{
    std::string NormalizePath(const char* rawPath)
    {
        std::string path = rawPath != nullptr ? rawPath : "";

        for(char& c : path)
        {
            if(c == '\\')
            {
                c = '/';
            }
        }

        // Drop a drive letter prefix such as "C:".
        if(path.size() >= 2 && std::isalpha(static_cast<unsigned char>(path[0])) && path[1] == ':')
        {
            path.erase(0, 2);
        }

        // Collapse duplicate separators and strip leading "/" and "./".
        std::string normalized;
        normalized.reserve(path.size());
        for(char c : path)
        {
            if(c == '/' && (normalized.empty() || normalized.back() == '/'))
            {
                continue;
            }
            normalized += c;
        }
        while(normalized.rfind("./", 0) == 0)
        {
            normalized.erase(0, 2);
        }

        return normalized;
    }

    ProbeResult RecordRequest(const char* rawPath, const char* source, const char* section)
    {
        ProbeResult result;
        result.normalizedPath = NormalizePath(rawPath);

        RequestRecord& record = sRequests[result.normalizedPath];
        ++record.requestCount;

        const bool firstRequest = record.requestCount == 1;
        if(firstRequest)
        {
            record.firstSource = source != nullptr ? source : "";
            record.firstSection = section != nullptr ? section : "";

            const std::string& dataRoot = LinuxPocConfig::RuntimeDataRoot;
            if(dataRoot.empty())
            {
                record.status = ProbeStatus::NoDataRoot;
            }
            else
            {
                bool caseFixed = false;
                std::string resolved;

                if(ResolveCaseInsensitive(dataRoot, result.normalizedPath, &resolved, &caseFixed))
                {
                    record.status = caseFixed ? ProbeStatus::FoundCaseFixed : ProbeStatus::Found;
                    record.resolvedPath = resolved;
                }
                else
                {
                    record.status = ProbeStatus::Missing;
                }
            }
        }

        result.status = record.status;
        result.resolvedPath = record.resolvedPath;

        if(firstRequest)
        {
            if(record.status == ProbeStatus::FoundCaseFixed)
            {
                rReleasePrintf("LinuxPocFileSystem %s: %s (on disk: %s)\n",
                               StatusName(record.status),
                               result.normalizedPath.c_str(),
                               record.resolvedPath.c_str());
            }
            else if(record.status == ProbeStatus::Missing)
            {
                rReleasePrintf("LinuxPocFileSystem missing under data root: %s\n",
                               result.normalizedPath.c_str());
            }
        }

        return result;
    }

    void WriteManifest()
    {
        if(sRequests.empty())
        {
            return;
        }

        unsigned int found = 0;
        unsigned int caseFixed = 0;
        unsigned int missing = 0;
        for(const auto& pair : sRequests)
        {
            switch(pair.second.status)
            {
                case ProbeStatus::Found:          ++found; break;
                case ProbeStatus::FoundCaseFixed: ++caseFixed; break;
                case ProbeStatus::Missing:        ++missing; break;
                case ProbeStatus::NoDataRoot:     break;
            }
        }

        rReleasePrintf("LinuxPocFileSystem manifest: %u unique path(s), %u found, %u case-mismatched, %u missing\n",
                       static_cast<unsigned int>(sRequests.size()),
                       found,
                       caseFixed,
                       missing);

        const std::string& manifestPath = LinuxPocConfig::RuntimeAssetManifestPath;
        if(manifestPath.empty())
        {
            return;
        }

        std::FILE* file = std::fopen(manifestPath.c_str(), "w");
        if(file == nullptr)
        {
            rReleasePrintf("LinuxPocFileSystem could not write manifest to %s\n", manifestPath.c_str());
            return;
        }

        std::fprintf(file, "# SRR2 Linux PoC asset request manifest\n");
        if(!LinuxPocConfig::RuntimeDataRoot.empty())
        {
            std::fprintf(file, "# data root: %s\n", LinuxPocConfig::RuntimeDataRoot.c_str());
        }
        std::fprintf(file, "# status\trequests\tpath\tsource\tsection\tresolved\n");

        for(const auto& pair : sRequests)
        {
            const RequestRecord& record = pair.second;
            std::fprintf(file,
                         "%s\t%u\t%s\t%s\t%s\t%s\n",
                         StatusName(record.status),
                         record.requestCount,
                         pair.first.c_str(),
                         record.firstSource.c_str(),
                         record.firstSection.empty() ? "-" : record.firstSection.c_str(),
                         record.resolvedPath.empty() ? "-" : record.resolvedPath.c_str());
        }

        std::fclose(file);
        rReleasePrintf("LinuxPocFileSystem manifest written to %s\n", manifestPath.c_str());
    }
}
