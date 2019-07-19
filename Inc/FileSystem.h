#pragma once

#include <Redefs.h>
#include <BaseString.h>
#include <BaseStringPart.h>
#include <Array.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Global functions for managing the file system.
namespace FileSystem {

//////////////////////////////////////////////////////////////////////////

// Check if the given name is syntactically valid.
bool REAPI IsNameValid( CUnicodePart name );

// Name comparison. Different file separators are considered equal.
int REAPI CompareNames( CUnicodePart leftName, CUnicodePart rightName );
bool REAPI NamesEqual( CUnicodePart leftName, CUnicodePart rightName );

// Path types.
enum TPathType {
	PT_Relative,	// relative path ( folder\subfolder )
	PT_RelativeFromRoot,	// relative path starting from the root catalog ( \folder\subfolder )
	PT_RelativeWithDrive,	// relative path with the given drive name ( c:folder\subfolder )
	PT_Absolute,	// absolute path ( c:\folder\subfolder )
	PT_UNC	// UNC path ( \\serverName\folder\subfolder )
};
TPathType REAPI GetPathType( CUnicodePart path );

// Split the path into components.
void REAPI SplitName( CUnicodeView fullName, CUnicodeString& drive, CUnicodeString& dir, CUnicodeString& name, CUnicodeString& ext );
TPathType REAPI SplitName( CUnicodePart path, CArray<CUnicodePart>& components );
// Merge a full path from different components.
CUnicodeString REAPI MergeName( CUnicodePart driveDir, CUnicodePart nameExt );
CUnicodeString REAPI MergeName( CUnicodePart driveDir, CUnicodeView name, CUnicodeView ext );
CUnicodeString REAPI MergeName( CUnicodeView drive, CUnicodeView dir, CUnicodeView name, CUnicodeView ext );
// Combines a full path from the base dir and a relative path.
CUnicodeString REAPI MergePath( CUnicodePart dir, CUnicodePart relativePath );
// Add the extension if necessary. Extention is provided without the leading period.
void REAPI AddExtIfNone( CUnicodeString& name, CUnicodeView ext );
// Replace the extension.
void REAPI ReplaceExt( CUnicodeString& name, CUnicodeView ext );
// Create a full path assuming that the given relative path starts from the same catalog as the current process.
CUnicodeString REAPI CreateFullPath( CUnicodeView path );
// Merge dir and relativePath and create a full path from the result.
CUnicodeString REAPI CreateFullPath( CUnicodeView dir, CUnicodeView relativePath );
// Convert a shortened path to a long path.
CUnicodeString REAPI CreateLongPath( CUnicodeView path );

// Get the path components.
CUnicodeString REAPI GetRoot( CUnicodePart name );
CUnicodeString REAPI GetDrive( CUnicodePart name );
CUnicodeString REAPI GetPath( CUnicodePart name );
CUnicodeString REAPI GetDrivePath( CUnicodePart name );
CUnicodeString REAPI GetName( CUnicodePart name );
// Extension includes the dot symbol.
CUnicodeString REAPI GetExt( CUnicodePart name );
CUnicodeString REAPI GetNameExt( CUnicodePart name );

// Add the separator to the end of path if there is none.
void REAPI AddPathSeparator( CUnicodeString& path );
// Add a trailing separator if the path contains only the drive name or remove the trailing separator otherwise.
// All normalized paths look like "C:\" or "C:\directory\etc".
void REAPI NormalizePath( CUnicodeString& path );
// Change all the file separators to "\".
void REAPI ForceBackSlashes( CUnicodeString& path );

// Check directory/file existence.
bool REAPI DirAccessible( CUnicodeView dir );
bool REAPI FileExists( CUnicodeView fileName );
bool REAPI CanOpenFile( CUnicodeView fileName, TFileReadWriteMode readWriteMode, TFileCreationMode createMode, TFileShareMode shareMode );

// File attributes management.
DWORD REAPI GetAttributes( CUnicodeView fileName );
void REAPI SetAttributes( CUnicodeView fileName, DWORD attributes );

// File redaction.
void REAPI Rename( CUnicodeView fileName, CUnicodeView newFileName );
void REAPI Delete( CUnicodeView fileName );
void REAPI Copy( CUnicodeView src, CUnicodeView dest );
void REAPI Move( CUnicodeView src, CUnicodeView dest );

// Directory redaction.
void REAPI CreateDir( CUnicodeView dir );
void REAPI DeleteDir( CUnicodeView dir );
void REAPI DeleteTree( CUnicodeView dirTree );
void REAPI CopyTree( CUnicodePart src, CUnicodeView dest );
void REAPI MoveTree( CUnicodeView src, CUnicodeView dest );

bool REAPI IsDirEmpty( CUnicodePart dir );
// Return the number of non-directories at the given path.
int REAPI GetFileCount( CUnicodePart dir );

enum TFileIncludeFlags {
	FIF_Files = 1,	// include files in results
	FIF_Directories = 2,	// include directories in results
	FIF_Recursive = 4,	// include files from inner directories in results
	FIF_Hidden = 8	// include hidden files in results
};

// Fill the fileList with the files from dir. Mask can contain several masks separated by a semicolon ( L"*.pdf;*.txt ).
void REAPI GetFilesInDir( CUnicodePart dir, CArray<CFileStatus>& result, DWORD flags = FIF_Files, CUnicodePart mask = L"*" );

// Create a unique name in the given folder.
CUnicodeString REAPI CreateUniqueName( CUnicodePart dir, CUnicodePart prefix, CUnicodeView extension );

// Get the current executable full name.
CUnicodeString REAPI GetExecutableName();
// Get/Set the current process catalog.
CUnicodeString REAPI GetCurrentDir();
void REAPI SetCurrentDir( CUnicodeView newDir );
// Windows catalog.
CUnicodeString REAPI GetWindowsDir();
// Windows temp file catalog.
CUnicodeString REAPI GetWindowsTempDir();

// Get the specified environment variable value.
CUnicodeString REAPI GetEnvironmentVariable( CUnicodeView name );

// Path to the application data for different situations.
enum TAppDataPathType {
	ADPT_AllUsers,
	ADPT_SpecificUser
};

// Get/Set the path for the application to store modifiable data.
CUnicodeView REAPI GetAppDataPath( TAppDataPathType type );
void REAPI SetAppDataPath( TAppDataPathType type, CUnicodePart path );
// Add the given relative path to the current application data folder.
void REAPI SetAppDataRelativePath( CUnicodeView path );

// Get the free space on the disk for the given path. Disk space is stored in KB.
int REAPI GetDiskFreeSpace( CUnicodeView path );
// Get the free space for the current process directory.
int REAPI GetCurrentDiskFreeSpace();

//////////////////////////////////////////////////////////////////////////

}	// namespace FileSystem.

}	// namespace Relib.
