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
bool REAPI IsNameValid( CStringPart name );

// Name comparison. Different file separators are considered equal.
int REAPI CompareNames( CStringPart leftName, CStringPart rightName );
bool REAPI NamesEqual( CStringPart leftName, CStringPart rightName );

// Path types.
enum TPathType {
	PT_Relative,	// relative path ( folder\subfolder )
	PT_RelativeFromRoot,	// relative path starting from the root catalog ( \folder\subfolder )
	PT_RelativeWithDrive,	// relative path with the given drive name ( c:folder\subfolder )
	PT_Absolute,	// absolute path ( c:\folder\subfolder )
	PT_UNC	// UNC path ( \\serverName\folder\subfolder )
};
TPathType REAPI GetPathType( CStringPart path );

// Split the path into components.
void REAPI SplitName( CStringView fullName, CString& drive, CString& dir, CString& name, CString& ext );
TPathType REAPI SplitName( CStringPart path, CArray<CStringPart>& components );
// Merge a full path from different components.
CString REAPI MergeName( CStringPart driveDir, CStringPart nameExt );
CString REAPI MergeName( CStringPart driveDir, CStringView name, CStringView ext );
CString REAPI MergeName( CStringView drive, CStringView dir, CStringView name, CStringView ext );
// Combines a full path from the base dir and a relative path.
CString REAPI MergePath( CStringPart dir, CStringPart relativePath );
// Add the extension if necessary. Extention is provided without the leading period.
void REAPI AddExtIfNone( CString& name, CStringView extNoPeriod );
// Replace the extension.
void REAPI ReplaceExt( CString& name, CStringView extNoPeriod );
// Create a full path assuming that the given relative path starts from the same catalog as the current process.
CString REAPI CreateFullPath( CStringPart path );
CUnicodeString REAPI CreateFullUnicodePath( CUnicodeView path );
// Merge dir and relativePath and create a full path from the result.
CString REAPI CreateFullPath( CStringPart dir, CStringPart relativePath );
// Convert a shortened path to a long path.
CString REAPI CreateLongPath( CStringPart path );

// Get the path components.
CString REAPI GetRoot( CStringPart name );
CString REAPI GetDrive( CStringPart name );
CString REAPI GetPath( CStringPart name );
CString REAPI GetDrivePath( CStringPart name );
CString REAPI GetName( CStringPart name );
// Extension includes the dot symbol.
CString REAPI GetExt( CStringPart name );
CString REAPI GetNameExt( CStringPart name );

// Add the separator to the end of path if there is none.
void REAPI AddPathSeparator( CString& path );
// Add a trailing separator if the path contains only the drive name or remove the trailing separator otherwise.
// All normalized paths look like "C:\" or "C:\directory\etc".
void REAPI NormalizePath( CString& path );
// Change all the file separators to "\".
void REAPI ForceBackSlashes( CString& path );

// Check directory/file existence.
bool REAPI DirAccessible( CStringPart dir );
bool REAPI FileExists( CStringPart fileName );
bool REAPI CanOpenFile( CStringPart fileName, TFileReadWriteMode readWriteMode, TFileCreationMode createMode, TFileShareMode shareMode );

// File attributes management.
DWORD REAPI GetAttributes( CStringPart fileName );
void REAPI SetAttributes( CStringPart fileName, DWORD attributes );

// File redaction.
void REAPI Rename( CStringPart fileName, CStringPart newFileName );
void REAPI Delete( CStringPart fileName );
void REAPI Copy( CStringPart src, CStringPart dest );
void REAPI Move( CStringPart src, CStringPart dest );

// Directory redaction.
void REAPI CreateDir( CStringPart dir );
void REAPI DeleteDir( CStringPart dir );
void REAPI DeleteTree( CStringPart dirTree );
void REAPI CopyTree( CStringPart src, CStringView dest );
void REAPI MoveTree( CStringView src, CStringView dest );

bool REAPI IsDirEmpty( CStringPart dir );
// Return the number of non-directories at the given path.
int REAPI GetFileCount( CStringPart dir );

enum TFileIncludeFlags {
	FIF_Files = 1,	// include files in results
	FIF_Directories = 2,	// include directories in results
	FIF_Recursive = 4,	// include files from inner directories in results
	FIF_Hidden = 8	// include hidden files in results
};

// Fill the fileList with the files from dir. Mask can contain several masks separated by a semicolon ( L"*.pdf;*.txt ).
void REAPI GetFilesInDir( CStringPart dir, CArray<CFileStatus>& result, DWORD flags = FIF_Files, CStringPart mask = "*" );

// Create a unique name in the given folder.
CString REAPI CreateUniqueName( CStringPart dir, CStringPart prefix, CStringView extension );

// Get the current executable full name.
CString REAPI GetExecutableName();
// Get/Set the current process catalog.
CString REAPI GetCurrentDir();
void REAPI SetCurrentDir( CStringPart newDir );
// Windows catalog.
CString REAPI GetWindowsDir();
// Windows temp file catalog.
CString REAPI GetWindowsTempDir();

// Get the specified environment variable value.
CString REAPI GetEnvironmentVariable( CStringPart name );

// Path to the application data for different situations.
enum TAppDataPathType {
	ADPT_AllUsers,
	ADPT_SpecificUser
};

// Get/Set the path for the application to store modifiable data.
CStringView REAPI GetAppDataPath( TAppDataPathType type );
void REAPI SetAppDataPath( TAppDataPathType type, CStringPart path );
// Add the given relative path to the current application data folder.
void REAPI SetAppDataRelativePath( CStringView path );

// Get the free space on the disk for the given path. Disk space is stored in KB.
int REAPI GetDiskFreeSpace( CStringPart path );
// Get the free space for the current process directory.
int REAPI GetCurrentDiskFreeSpace();

//////////////////////////////////////////////////////////////////////////

}	// namespace FileSystem.

}	// namespace Relib.
