//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


//=============================================================================
//
// File:        sdldrive.cpp
//
// Subsystem:   Radical Drive System
//
// Description:	This file contains the implementation of the radSdlDrive class.
//
// Revisions:
//
// Notes:       We keep a serial number when the first file is opened. Then if the
//              media is removed, we don't allow ops until the original serial number
//              is detected, or all files are closed.
//=============================================================================

//=============================================================================
// Include Files
//=============================================================================

#include "pch.hpp"
#include <algorithm>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdldrive.hpp"
#include <string>
#include <SDL.h>
#if defined(__linux__)
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#if SDL_MAJOR_VERSION < 3
#ifdef WIN32
#include <direct.h>
#elif !defined(__linux__)
#include <unistd.h>
#endif
#endif

namespace
{
#if defined(RAD_SDL) && defined(__linux__)
    const char* SDL_SAVE_DRIVE_NAME = "SAVE:";

    void Srr2SaveLog( const char* format, ... )
    {
        va_list args;
        va_start( args, format );
        fprintf( stderr, "SRR2 save: " );
        vfprintf( stderr, format, args );
        fprintf( stderr, "\n" );
        fflush( stderr );
        va_end( args );
    }

    bool IsAbsoluteLinuxPath( const char* path )
    {
        return path != NULL && path[ 0 ] == '/';
    }

    bool IsNoFreeSpaceError( int error )
    {
        if( error == ENOSPC )
        {
            return true;
        }

#ifdef EDQUOT
        if( error == EDQUOT )
        {
            return true;
        }
#endif

        return false;
    }

    unsigned int GetAvailableSpace( const struct statvfs& filesystemInfo )
    {
        const unsigned long blockSize = filesystemInfo.f_frsize != 0
            ? filesystemInfo.f_frsize
            : filesystemInfo.f_bsize;

        if( blockSize == 0 || filesystemInfo.f_bavail > UINT_MAX / blockSize )
        {
            return UINT_MAX;
        }

        return static_cast<unsigned int>( filesystemInfo.f_bavail * blockSize );
    }

    bool SafeCopyPath( char* dest, unsigned int destSize, const char* source, const char* sourceName )
    {
        int written = snprintf( dest, destSize, "%s", source );
        if( written < 0 || static_cast<unsigned int>( written ) >= destSize )
        {
            Srr2SaveLog( "%s path is too long: [%s]", sourceName, source );
            if( destSize > 0 )
            {
                dest[ 0 ] = '\0';
            }
            return false;
        }

        return true;
    }

    bool SafeJoinPath( char* dest, unsigned int destSize, const char* base, const char* leaf, const char* sourceName )
    {
        const char* separator = "";
        size_t baseLength = strlen( base );
        if( baseLength > 0 && base[ baseLength - 1 ] != '/' )
        {
            separator = "/";
        }

        int written = snprintf( dest, destSize, "%s%s%s", base, separator, leaf );
        if( written < 0 || static_cast<unsigned int>( written ) >= destSize )
        {
            Srr2SaveLog( "%s save path is too long: base [%s] leaf [%s]", sourceName, base, leaf );
            if( destSize > 0 )
            {
                dest[ 0 ] = '\0';
            }
            return false;
        }

        return true;
    }

    bool EnsureTrailingSlash( char* path, unsigned int pathSize, const char* sourceName )
    {
        size_t pathLength = strlen( path );
        if( pathLength == 0 )
        {
            return false;
        }

        if( path[ pathLength - 1 ] == '/' )
        {
            return true;
        }

        if( pathLength + 1 >= pathSize )
        {
            Srr2SaveLog( "%s save path is too long to append trailing slash: [%s]", sourceName, path );
            return false;
        }

        path[ pathLength ] = '/';
        path[ pathLength + 1 ] = '\0';
        return true;
    }

    bool CreateOneDirectory( const char* path, const char* sourceName )
    {
        struct stat info;
        if( stat( path, &info ) == 0 )
        {
            if( S_ISDIR( info.st_mode ) )
            {
                return true;
            }

            const int savedErrno = ENOTDIR;
            Srr2SaveLog( "create/access failed for source [%s] path [%s]: exists but is not a directory errno=%d (%s)",
                         sourceName,
                         path,
                         savedErrno,
                         strerror( savedErrno ) );
            return false;
        }

        const int statErrno = errno;
        if( statErrno != ENOENT )
        {
            Srr2SaveLog( "create/access failed for source [%s] path [%s]: stat errno=%d (%s)",
                         sourceName,
                         path,
                         statErrno,
                         strerror( statErrno ) );
            return false;
        }

        if( mkdir( path, 0700 ) == 0 )
        {
            return true;
        }

        const int mkdirErrno = errno;
        if( mkdirErrno == EEXIST )
        {
            if( stat( path, &info ) == 0 && S_ISDIR( info.st_mode ) )
            {
                return true;
            }
        }

        Srr2SaveLog( "create/access failed for source [%s] path [%s]: mkdir errno=%d (%s)",
                     sourceName,
                     path,
                     mkdirErrno,
                     strerror( mkdirErrno ) );
        return false;
    }

    bool CreateDirectoryRecursive( const char* path, const char* sourceName )
    {
        char partial[ radFileFilenameMax + 1 ];
        if( !SafeCopyPath( partial, sizeof( partial ), path, sourceName ) )
        {
            return false;
        }

        size_t pathLength = strlen( partial );
        while( pathLength > 1 && partial[ pathLength - 1 ] == '/' )
        {
            partial[ pathLength - 1 ] = '\0';
            pathLength--;
        }

        for( char* cursor = partial + 1; *cursor != '\0'; cursor++ )
        {
            if( *cursor == '/' )
            {
                *cursor = '\0';
                if( !CreateOneDirectory( partial, sourceName ) )
                {
                    *cursor = '/';
                    return false;
                }
                *cursor = '/';
            }
        }

        return CreateOneDirectory( partial, sourceName );
    }

    bool CheckDirectoryAccess( const char* path, const char* sourceName )
    {
        if( access( path, R_OK | W_OK | X_OK ) == 0 )
        {
            return true;
        }

        const int accessErrno = errno;
        Srr2SaveLog( "create/access failed for source [%s] path [%s]: access errno=%d (%s)",
                     sourceName,
                     path,
                     accessErrno,
                     strerror( accessErrno ) );
        return false;
    }

    bool PrepareSaveDirectory( const char* sourceName, const char* directory, char* drivePath, unsigned int drivePathSize )
    {
        if( !CreateDirectoryRecursive( directory, sourceName ) )
        {
            return false;
        }

        if( !CheckDirectoryAccess( directory, sourceName ) )
        {
            return false;
        }

        if( !SafeCopyPath( drivePath, drivePathSize, directory, sourceName ) )
        {
            return false;
        }

        if( !EnsureTrailingSlash( drivePath, drivePathSize, sourceName ) )
        {
            return false;
        }

        Srr2SaveLog( "chosen save drive [SAVE:] path [%s] source [%s]", drivePath, sourceName );
        return true;
    }

    bool CopyCurrentDirectoryForSaveFallback( char* cwd, unsigned int cwdSize )
    {
        const char* pwd = getenv( "PWD" );
        if( IsAbsoluteLinuxPath( pwd ) )
        {
            struct stat pwdInfo;
            struct stat dotInfo;
            if( stat( pwd, &pwdInfo ) == 0 && stat( ".", &dotInfo ) == 0 &&
                pwdInfo.st_dev == dotInfo.st_dev && pwdInfo.st_ino == dotInfo.st_ino )
            {
                return SafeCopyPath( cwd, cwdSize, pwd, "PWD" );
            }
        }

        char* result = getcwd( cwd, cwdSize );
        if( result == NULL )
        {
            const int getcwdErrno = errno;
            Srr2SaveLog( "fallback save path failed: getcwd errno=%d (%s)",
                         getcwdErrno,
                         strerror( getcwdErrno ) );
            if( cwdSize > 0 )
            {
                cwd[ 0 ] = '\0';
            }
            return false;
        }

        cwd[ cwdSize - 1 ] = '\0';
        return true;
    }

    bool ResolveLinuxSaveDrivePath( char* drivePath, unsigned int drivePathSize )
    {
        char directory[ radFileFilenameMax + 1 ];
        const char* xdgDataHome = getenv( "XDG_DATA_HOME" );
        if( xdgDataHome != NULL && xdgDataHome[ 0 ] != '\0' )
        {
            if( IsAbsoluteLinuxPath( xdgDataHome ) )
            {
                if( !SafeJoinPath( directory, sizeof( directory ), xdgDataHome, "srr2", "XDG_DATA_HOME" ) )
                {
                    return false;
                }

                return PrepareSaveDirectory( "XDG_DATA_HOME", directory, drivePath, drivePathSize );
            }

            Srr2SaveLog( "ignored relative XDG_DATA_HOME [%s]; trying HOME", xdgDataHome );
        }

        const char* home = getenv( "HOME" );
        if( home != NULL && home[ 0 ] != '\0' )
        {
            if( IsAbsoluteLinuxPath( home ) )
            {
                if( !SafeJoinPath( directory, sizeof( directory ), home, ".local/share/srr2", "HOME" ) )
                {
                    return false;
                }

                return PrepareSaveDirectory( "HOME", directory, drivePath, drivePathSize );
            }

            Srr2SaveLog( "ignored HOME [%s] because it is not an absolute path", home );
        }

        char cwd[ radFileFilenameMax + 1 ];
        if( !CopyCurrentDirectoryForSaveFallback( cwd, sizeof( cwd ) ) )
        {
            return false;
        }

        if( !SafeJoinPath( directory, sizeof( directory ), cwd, "save", "asset-root/cwd fallback" ) )
        {
            return false;
        }

        return PrepareSaveDirectory( "asset-root/cwd fallback", directory, drivePath, drivePathSize );
    }
#endif
}

//=============================================================================
// Public Functions 
//=============================================================================

//=============================================================================
// Function:    radSdlDriveFactory
//=============================================================================
// Description: This member is responsible for constructing a radSdlDriveObject.
//
// Parameters:  pointer to receive drive object
//              pointer to the drive name
//              allocator
//              
// Returns:     
//------------------------------------------------------------------------------

void radSdlDriveFactory
( 
    radDrive**         ppDrive, 
    const char*        pDriveName,
    radMemoryAllocator alloc
)
{
    //
    // Simply constuct the drive object.
    //
    *ppDrive = new( alloc ) radSdlDrive( pDriveName, alloc );
    rAssert( *ppDrive != NULL );
}


//=============================================================================
// Public Member Functions
//=============================================================================

//=============================================================================
// Function:    radSdlDrive::radSdlDrive
//=============================================================================

radSdlDrive::radSdlDrive( const char* pdrivespec, radMemoryAllocator alloc )
    : 
    radDrive( ),
    m_OpenFiles( 0 ),
    m_IsSaveDrive( false ),
    m_MediaAvailable( true ),
    m_pMutex( NULL )
{
    m_DriveName[ 0 ] = '\0';
    m_DrivePath[ 0 ] = '\0';

    //
    // Create a mutex for lock/unlock
    //
    radThreadCreateMutex( &m_pMutex, alloc );
    rAssert( m_pMutex != NULL );

    //
    // Create the drive thread.
    //
    m_pDriveThread = new( alloc ) radDriveThread( m_pMutex, alloc );
    rAssert( m_pDriveThread != NULL );

#if defined(RAD_SDL) && defined(__linux__)
    if( strcmp( pdrivespec, SDL_SAVE_DRIVE_NAME ) == 0 )
    {
        strncpy( m_DriveName, SDL_SAVE_DRIVE_NAME, radFileDrivenameMax );
        m_DriveName[ radFileDrivenameMax ] = '\0';
        m_IsSaveDrive = true;
        m_MediaAvailable = ResolveLinuxSaveDrivePath( m_DrivePath, radFileFilenameMax + 1 );
        if( !m_MediaAvailable )
        {
            m_DrivePath[ 0 ] = '\0';
            Srr2SaveLog( "save drive [SAVE:] unavailable; saves will fail without asset-root fallback" );
        }
    }
    else
#endif
    {
        //
        // Copy the drivename
        //
        radGetDefaultDrive( m_DriveName );
        if ( strcmp(m_DriveName, pdrivespec ) != 0 )
        {
            strncpy( m_DriveName, pdrivespec, radFileDrivenameMax );
            strncpy( m_DrivePath, pdrivespec, radFileFilenameMax );
            m_DriveName[radFileDrivenameMax] = '\0';
            m_DrivePath[radFileFilenameMax] = '\0';
            SDL_strupr( m_DriveName );
            // Do not lowercase m_DrivePath. Linux filesystems are case-sensitive,
            // so lowercasing the current working directory breaks installs whose
            // parent path contains uppercase characters.
        }

        if(!m_DrivePath[0])
        {
#if SDL_MAJOR_VERSION < 3
#ifdef WIN32
            _getcwd( m_DrivePath, radFileFilenameMax );
            strncat(m_DrivePath, "/", radFileFilenameMax);
#else
            getcwd( m_DrivePath, radFileFilenameMax );
            strncat(m_DrivePath, "/", radFileFilenameMax);
#endif
#else
            char* cwd = SDL_GetCurrentDirectory();
            strncpy(m_DrivePath, cwd, radFileFilenameMax);
            SDL_free(cwd);
#endif
            m_DrivePath[radFileFilenameMax] = '\0';
        }
    }

#if SDL_MAJOR_VERSION < 3
    m_Capabilities = ( radDriveWriteable | radDriveFile );
#else
    m_Capabilities = ( radDriveEnumerable | radDriveWriteable | radDriveDirectory | radDriveFile );
#endif
}

//=============================================================================
// Function:    radSdlDrive::~radSdlDrive
//=============================================================================

radSdlDrive::~radSdlDrive( void )
{
    m_pMutex->Release( );
    m_pDriveThread->Release( );
}

//=============================================================================
// Function:    radSdlDrive::Lock
//=============================================================================
// Description: Start a critical section
//
// Parameters:  
//
// Returns:     
//------------------------------------------------------------------------------

void radSdlDrive::Lock( void )
{
    m_pMutex->Lock( );
}

//=============================================================================
// Function:    radSdlDrive::Unlock
//=============================================================================
// Description: End a critical section
//
// Parameters:  
//
// Returns:     
//------------------------------------------------------------------------------

void radSdlDrive::Unlock( void )
{
    m_pMutex->Unlock( );
}

//=============================================================================
// Function:    radSdlDrive::GetCapabilities
//=============================================================================

unsigned int radSdlDrive::GetCapabilities( void )
{
    return m_Capabilities;
}

//=============================================================================
// Function:    radGcnDVDDrive::GetDriveName
//=============================================================================

const char* radSdlDrive::GetDriveName( void )
{
    return m_DriveName;
}

//=============================================================================
// Function:    radSdlDrive::Initialize
//=============================================================================

radDrive::CompletionStatus radSdlDrive::Initialize( void )
{
    SetMediaInfo();

    //
    // Success: SetMediaInfo leaves NoMedia in place for an unavailable SAVE: drive.
    //
    if( m_MediaAvailable )
    {
        m_LastError = Success;
    }
    return Complete;
}

//=============================================================================
// Function:    radSdlDrive::OpenFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::OpenFile
( 
    const char*         fileName, 
    radFileOpenFlags    flags, 
    bool                writeAccess, 
    radFileHandle*      pHandle, 
    unsigned int*       pSize 
)
{
    if( !m_MediaAvailable )
    {
#if defined(RAD_SDL) && defined(__linux__)
        if( m_IsSaveDrive )
        {
            Srr2SaveLog( "open refused: drive [%s] file [%s] because save media is unavailable", m_DriveName, fileName );
        }
#endif
        m_LastError = NoMedia;
        return Error;
    }

    //
    // Build the full filename
    //
    char fullName[ radFileFilenameMax + 1 ];
    BuildFileSpec( fileName, fullName, radFileFilenameMax + 1 );

    //
    // Translate flags to SDL
    //
    const char* createFlags;
    switch( flags )
    {
    case OpenExisting:
        createFlags = writeAccess ? "rb+" : "rb";
        break;
    case OpenAlways:
        createFlags = "ab+";
        break;
    case CreateAlways:
        createFlags = "wb+";
        break;
    default:
        rAssertMsg( false, "radFileSystem: sdldrive: attempting to open file with unknown flag" );
        return Error;
    }

    errno = 0;
#if SDL_MAJOR_VERSION < 3
    *pHandle = SDL_RWFromFile(fullName, createFlags);
#else
    *pHandle = SDL_IOFromFile( fullName, createFlags );
#endif

    if ( *pHandle )
    {
        m_OpenFiles++;
#if SDL_MAJOR_VERSION < 3
        *pSize = SDL_RWsize( (SDL_RWops*)*pHandle );
#else
        *pSize = SDL_GetIOSize( (SDL_IOStream*)*pHandle );
#endif
        m_LastError = Success;
        return Complete;
    }
    else
    {
        int openErrno = errno;
#if defined(RAD_SDL) && defined(__linux__)
        if( m_IsSaveDrive && openErrno == 0 && flags == OpenExisting && !writeAccess )
        {
            if( access( fullName, F_OK ) != 0 )
            {
                openErrno = errno;
            }
        }

        if( m_IsSaveDrive && ( writeAccess || openErrno != ENOENT ) )
        {
            Srr2SaveLog( "open failed: drive [%s] file [%s] path [%s] mode [%s] errno=%d (%s) sdl=[%s]",
                         m_DriveName,
                         fileName,
                         fullName,
                         createFlags,
                         openErrno,
                         strerror( openErrno ),
                         SDL_GetError() );
        }
#endif
#if defined(RAD_SDL) && defined(__linux__)
        if( m_IsSaveDrive && IsNoFreeSpaceError( openErrno ) )
        {
            m_LastError = NoFreeSpace;
        }
        else
#endif
        {
            m_LastError = ( openErrno == ENOENT ) ? FileNotFound : HardwareFailure;
        }
        return Error;
    }
}

//=============================================================================
// Function:    radSdlDrive::CloseFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::CloseFile( radFileHandle handle, const char* fileName )
{
#if SDL_MAJOR_VERSION < 3
    SDL_RWclose( (SDL_RWops*)handle );
#else
    SDL_CloseIO( (SDL_IOStream*)handle );
#endif
    m_OpenFiles--;
    return Complete;
}

//=============================================================================
// Function:    radSdlDrive::ReadFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::ReadFile
( 
    radFileHandle   handle, 
    const char*     fileName,
    IRadFile::BufferedReadState buffState,
    unsigned int    position, 
    void*           pData, 
    unsigned int    bytesToRead, 
    unsigned int*   bytesRead, 
    radMemorySpace  pDataSpace 
)
{
    rAssertMsg( pDataSpace == radMemorySpace_Local, 
                "radFileSystem: radSdlDrive: External memory not supported for reads." );

    //
    // set file pointer
    //
#if SDL_MAJOR_VERSION < 3
    if ( SDL_RWseek( (SDL_RWops*)handle, position, RW_SEEK_SET ) >= 0 )
    {
        if (SDL_RWread( (SDL_RWops*)handle, pData, 1, bytesToRead ) > 0 )
        {
#else
    if ( SDL_SeekIO( (SDL_IOStream*)handle, position, SDL_IO_SEEK_SET ) >= 0 )
    {
        if ( SDL_ReadIO( (SDL_IOStream*)handle, pData, bytesToRead ) > 0 )
        {
#endif
            //
            // Successful read!
            //
            
            //
            // Change this during buffered read!!
            //
            *bytesRead = bytesToRead;
            m_LastError = Success;
            return Complete;
        }
    }

    //
    // Failed!
    //
    m_LastError = FileNotFound;
    return Error;
}

//=============================================================================
// Function:    radSdlDrive::WriteFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::WriteFile
( 
    radFileHandle     handle,
    const char*       fileName,
    IRadFile::BufferedReadState buffState,
    unsigned int      position, 
    const void*       pData, 
    unsigned int      bytesToWrite, 
    unsigned int*     bytesWritten, 
    unsigned int*     pSize, 
    radMemorySpace    pDataSpace 
)
{
    if ( !( m_Capabilities & radDriveWriteable ) )
    {
        rWarningMsg( m_Capabilities & radDriveWriteable, "This drive does not support the WriteFile function." );
        return Error;
    }

    rAssertMsg( pDataSpace == radMemorySpace_Local, 
                "radFileSystem: radSdlDrive: External memory not supported for reads." );

    //
    // do the write
    //
    errno = 0;
#if SDL_MAJOR_VERSION < 3
    if ( SDL_RWseek( (SDL_RWops*)handle, position, RW_SEEK_SET ) >= 0 )
    {
        *bytesWritten = SDL_RWwrite( (SDL_RWops*)handle, pData, 1, bytesToWrite );
#else
    if ( SDL_SeekIO( (SDL_IOStream*)handle, position, SDL_IO_SEEK_SET ) >= 0 )
    {
        *bytesWritten = SDL_WriteIO( (SDL_IOStream*)handle, pData, bytesToWrite );
#endif
        if ( *bytesWritten == bytesToWrite )
        {
            //
            // Sucessful write
            //
#if SDL_MAJOR_VERSION < 3
            *pSize = SDL_RWsize( (SDL_RWops*)handle );
#else
            *pSize = SDL_GetIOSize( (SDL_IOStream*)handle );
#endif
            m_LastError = Success;
            return Complete;
        }
    }

    //
    // Failed!
    //
    const int writeErrno = errno;
#if defined(RAD_SDL) && defined(__linux__)
    if( m_IsSaveDrive )
    {
        char fullName[ radFileFilenameMax + 1 ];
        BuildFileSpec( fileName, fullName, radFileFilenameMax + 1 );
        Srr2SaveLog( "write failed: drive [%s] file [%s] path [%s] position=%u bytes=%u wrote=%u errno=%d (%s) sdl=[%s]",
                     m_DriveName,
                     fileName,
                     fullName,
                     position,
                     bytesToWrite,
                     *bytesWritten,
                     writeErrno,
                     strerror( writeErrno ),
                     SDL_GetError() );

        if( IsNoFreeSpaceError( writeErrno ) )
        {
            m_LastError = NoFreeSpace;
            return Error;
        }
    }
#endif
    m_LastError = HardwareFailure;
    return Error;
}

#if SDL_MAJOR_VERSION > 2
//=============================================================================
// Function:    radSdlDrive::FindFirst
//=============================================================================

radDrive::CompletionStatus radSdlDrive::FindFirst
( 
    const char*                 searchSpec, 
    IRadDrive::DirectoryInfo*   pDirectoryInfo, 
    radFileDirHandle*           pHandle,
    bool                        firstSearch
)
{
    //
    // Find first
    //
    const char* pattern = strrchr(searchSpec, '\\');
    std::string path;
    if (!pattern)
        pattern = strrchr(searchSpec, '/');
    if (pattern)
        path = std::string(searchSpec, pattern - searchSpec);
    else
        pattern = searchSpec;
    path = m_DrivePath + path;
    std::replace(path.begin(), path.end(), '\\', '/');

    char** handle = SDL_GlobDirectory( path.c_str(), pattern, SDL_GLOB_CASEINSENSITIVE, NULL );
    if ( handle )
    {
        SDL_PathInfo info;
        if ( SDL_GetPathInfo( handle[0], &info))
            m_LastError = TranslateDirInfo( pDirectoryInfo, &info, pHandle );
        else
            m_LastError = TranslateDirInfo( pDirectoryInfo, NULL, pHandle );
        // HACK: We don't need the first element anymore, so use it to store the iterator
        ((char***)handle)[0] = handle;
    }
    else
    {
        m_LastError = FileNotFound;
    }

    //
    // Fill in our directory info structure
    //
    if ( m_LastError == Success )
    {
        return Complete;
    }
    else
    {
        return Error;
    }
}

//=============================================================================
// Function:    radSdlDrive::FindNext
//=============================================================================

radDrive::CompletionStatus radSdlDrive::FindNext( radFileDirHandle* pHandle, IRadDrive::DirectoryInfo* pDirectoryInfo )
{
    //
    // If we don't have a handle, return file not found.
    //
    if ( *pHandle == NULL )
    {
        m_LastError = FileNotFound;
        return Error;
    }

    //
    // Find the next entry
    //
    char*** handle = (char***)*pHandle;
    *handle++;
    SDL_PathInfo info;
    if ( SDL_GetPathInfo(  **handle, &info ))
        m_LastError = TranslateDirInfo( pDirectoryInfo, &info, pHandle );
    else
        m_LastError = TranslateDirInfo( pDirectoryInfo, NULL, pHandle );
    
    if ( m_LastError == Success )
    {
        m_LastError = Success;
        return Complete;
    }
    else
    {
        m_LastError = FileNotFound;
        return Error;
    }
}

//=============================================================================
// Function:    radSdlDrive::FindClose
//=============================================================================

radDrive::CompletionStatus radSdlDrive::FindClose( radFileDirHandle* pHandle )
{
    SDL_free( *pHandle );
    *pHandle = NULL;

    return Complete;
}

//=============================================================================
// Function:    radSdlDrive::CreateDir
//=============================================================================

radDrive::CompletionStatus radSdlDrive::CreateDir( const char* pName )
{
    rWarningMsg( m_Capabilities & radDriveDirectory, 
        "This drive does not support the CreateDir function." );

    //
    // Build the full filename
    //
    char fullSpec[ radFileFilenameMax + 1 ];
    BuildFileSpec( pName, fullSpec, radFileFilenameMax + 1 );

    if ( SDL_CreateDirectory( fullSpec ) )
    {
        m_LastError = Success;
        return Complete;
    }
    else
    {
        m_LastError = FileNotFound;
        return Error;
    }
}

//=============================================================================
// Function:    radSdlDrive::DestroyDir
//=============================================================================

radDrive::CompletionStatus radSdlDrive::DestroyDir( const char* pName )
{
    rWarningMsg( m_Capabilities & radDriveDirectory,
        "This drive does not support the DestroyDir function." );

    //
    // Someday check if pName is a dir!
    //

    //
    // Build the full filename
    //
    char fullSpec[ radFileFilenameMax + 1 ];
    BuildFileSpec( pName, fullSpec, radFileFilenameMax + 1 );

    if ( SDL_RemovePath( fullSpec ) )
    {
        m_LastError = Success;
        return Complete;
    }
    else
    {
        m_LastError = FileNotFound;
        return Error;
    }
}
#endif

//=============================================================================
// Function:    radSdlDrive::DestroyFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::DestroyFile( const char* filename )
{
    rWarningMsg( m_Capabilities & radDriveWriteable, "This drive does not support the DestroyFile function." );

    if( !m_MediaAvailable )
    {
#if defined(RAD_SDL) && defined(__linux__)
        if( m_IsSaveDrive )
        {
            Srr2SaveLog( "delete refused: drive [%s] file [%s] because save media is unavailable", m_DriveName, filename );
        }
#endif
        m_LastError = NoMedia;
        return Error;
    }

    //
    // Someday check if the file is open!
    //

    //
    // Build the full filename
    //
    char fullSpec[ radFileFilenameMax + 1 ];
    BuildFileSpec( filename, fullSpec, radFileFilenameMax + 1 );

    errno = 0;
    if ( ::remove( fullSpec ) == 0 )
    {
        m_LastError = Success;
        return Complete;
    }
    else
    {
        const int removeErrno = errno;
#if defined(RAD_SDL) && defined(__linux__)
        if( m_IsSaveDrive && removeErrno != ENOENT )
        {
            Srr2SaveLog( "delete failed: drive [%s] file [%s] path [%s] errno=%d (%s)",
                         m_DriveName,
                         filename,
                         fullSpec,
                         removeErrno,
                         strerror( removeErrno ) );
        }
#endif
        m_LastError = ( removeErrno == ENOENT ) ? FileNotFound : HardwareFailure;
        return Error;
    }
}

//=============================================================================
// Private Member Functions
//=============================================================================

//=============================================================================
// Function:    radSdlDrive::SetMediaInfo
//=============================================================================

void radSdlDrive::SetMediaInfo( void )
{
    //
    // Get volume information.
    //
    const char* realDriveName = m_DriveName;

    //rAssert( strlen( realDriveName ) == 2 );
    strcpy(m_MediaInfo.m_VolumeName, realDriveName );
    //strcat(m_MediaInfo.m_VolumeName, "\\");

    m_MediaInfo.m_SectorSize = SDL_DEFAULT_SECTOR_SIZE;

    if( !m_MediaAvailable )
    {
        m_MediaInfo.m_MediaState = IRadDrive::MediaInfo::MediaNotPresent;
        m_MediaInfo.m_FreeSpace = 0;
        m_MediaInfo.m_FreeFiles = 0;
        m_LastError = NoMedia;
        return;
    }

#if defined(RAD_SDL) && defined(__linux__)
    if( m_IsSaveDrive )
    {
        struct statvfs filesystemInfo;
        if( statvfs( m_DrivePath, &filesystemInfo ) == 0 )
        {
            m_MediaInfo.m_MediaState = IRadDrive::MediaInfo::MediaPresent;
            m_MediaInfo.m_FreeSpace = GetAvailableSpace( filesystemInfo );
            m_MediaInfo.m_FreeFiles = m_MediaInfo.m_FreeSpace / m_MediaInfo.m_SectorSize;
            m_LastError = Success;
            return;
        }

        const int statvfsErrno = errno;
        Srr2SaveLog( "free-space query failed: drive [%s] path [%s] errno=%d (%s)",
                     m_DriveName,
                     m_DrivePath,
                     statvfsErrno,
                     strerror( statvfsErrno ) );

        // The save directory is still present, but without a reliable space
        // measurement we must not advertise any empty slots as writable.
        m_MediaInfo.m_MediaState = IRadDrive::MediaInfo::MediaPresent;
        m_MediaInfo.m_FreeSpace = 0;
        m_MediaInfo.m_FreeFiles = 0;
        m_LastError = Success;
        return;
    }
#endif

    //
    // Non-save SDL drives do not expose capacity information.
    //
    m_MediaInfo.m_MediaState = IRadDrive::MediaInfo::MediaPresent;
    m_MediaInfo.m_FreeSpace = UINT_MAX;
    m_MediaInfo.m_FreeFiles = m_MediaInfo.m_FreeSpace / m_MediaInfo.m_SectorSize;
    m_LastError = Success;
}

//=============================================================================
// Function:    radSdlDrive::BuildFileSpec
//=============================================================================

void radSdlDrive::BuildFileSpec( const char* fileName, char* fullName, unsigned int size )
{
    std::string path(m_DrivePath);
    path += fileName;
    std::replace(path.begin(), path.end(), '\\', '/');

    strncpy( fullName, path.c_str(), size - 1 );
    fullName[ size - 1 ] = '\0';
}

#if SDL_MAJOR_VERSION > 2
//=============================================================================
// Function:    radSdlDrive::TranslateDirInfo
//=============================================================================
// Description: Translate the directory info and return an error status. A handle
//              with value directory_iterator() means the find_first/next call
//              failed and needs to be checked if something went wrong or if the
//              search just ended.
//
// Parameters:  
//              
// Returns:     
//------------------------------------------------------------------------------

radFileError radSdlDrive::TranslateDirInfo
( 
    IRadDrive::DirectoryInfo*   pDirectoryInfo, 
    const SDL_PathInfo*         pPathInfo,
    const radFileDirHandle*     pHandle
)
{
    char*** handle = (char***)*pHandle;
    if ( !pPathInfo || !*handle )
    {
        //
        // Either we failed or we're out of games.
        //
        if ( !pPathInfo )
        {
            return FileNotFound;
        }
        else
        {
            pDirectoryInfo->m_Name[0] = '\0';
            pDirectoryInfo->m_Type = IRadDrive::DirectoryInfo::IsDone;
        }
    }
    else
    {
        strncpy( pDirectoryInfo->m_Name, **handle, radFileFilenameMax );
        pDirectoryInfo->m_Name[ radFileFilenameMax ] = '\0';

        if ( pPathInfo->type == SDL_PATHTYPE_DIRECTORY )
        {
            pDirectoryInfo->m_Type = IRadDrive::DirectoryInfo::IsDirectory;
        }
        else
        {
            pDirectoryInfo->m_Type = IRadDrive::DirectoryInfo::IsFile;
        }
    }
    return Success;
}
#endif
