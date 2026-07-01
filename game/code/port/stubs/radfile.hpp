// Minimal RAD file shim for the Linux PoC.
#ifndef RADFILE_HPP
#define RADFILE_HPP

struct IRadDrive
{
    virtual void Release() { delete this; }

protected:
    virtual ~IRadDrive() = default;
};

enum radFileError
{
    Success = 0,
    FileNotFound,
    DriveNotReady,
    UnknownError
};

struct IRadDriveErrorCallback
{
    virtual bool OnDriveError(radFileError error, const char* pDriveName, void* pUserData) = 0;

protected:
    virtual ~IRadDriveErrorCallback() = default;
};

inline void radFileService()
{
}

#endif // RADFILE_HPP
