#include <FileSystem.h>

#include <UnicodeSet.h>

#include <CriticalSection.h>
#include <Comparators.h>
#include <BaseStringView.h>
#include <StrConversions.h>
#include <Errors.h>
#include <DynamicFile.h>

#pragma warning( push )
#pragma warning( disable:4091 )	// 'typedef ': ignored on left of 'tagGPFIDL_FLAGS' when no variable is declared
#include <ShlObj.h>
#pragma warning( pop )

namespace Relib {

extern const CUnicodeSet InvalidFileNameSymbols;

namespace FileSystem {

//////////////////////////////////////////////////////////////////////////
	
static bool isLetterLatin( wchar_t letter )
{
	return ( letter >= L'A' && letter <= L'Z' ) || ( letter >= L'a' && letter <= L'z' );
}

static bool checkDrive( CUnicodePart drive )
{
	return drive.Length() == 2 && isLetterLatin( drive[0] ) && drive[1] == L':';
}

static bool isValidSymbol( wchar_t symbol )
{
	return !InvalidFileNameSymbols.Has( symbol );
}

static bool checkComponent( CUnicodePart str )
{
	if( str.IsEmpty() ) {
		return false;
	}
	bool nonSpaceSymbolFound = false;
	for( wchar_t c : str ) {
		if( !isValidSymbol( c ) ) {
			return false;
		}
		if( !CUnicodeString::IsCharWhiteSpace( c ) ) {
			nonSpaceSymbolFound = true;
		}
	}
	// Valid path cannot only consist of spaces.
	return nonSpaceSymbolFound;
}

bool IsNameValid( CUnicodePart name )
{
	CArray<CUnicodePart> components;
	const TPathType type = SplitName( name, components );
	if( components.IsEmpty() ) {
		return false;
	}
	int firstComponent = 0;
	if( type == PT_UNC && components.Size() < 2 ) {
		return false;
	} else if( type == PT_Absolute || type == PT_RelativeWithDrive ) {
		if( !checkDrive( components[0] ) ) {
			return false;
		}
		firstComponent = 1;
	}
	for( int i = firstComponent; i < components.Size(); i++ ) {
		if( !checkComponent( components[i] ) ) {
			return false;
		}
	}
	return true;
}

static wchar_t getNameSeparator()
{
	return L'\\';
}

static bool isNameSeparator( wchar_t symbol )
{
	return symbol == L'\\' || symbol == L'/';
}

int CompareNames( CUnicodePart leftName, CUnicodePart rightName )
{
	const int leftLength = leftName.Length();
	const int rightLength = rightName.Length();
	int leftPos = 0;
	int rightPos = 0;

	while( leftPos < leftLength && rightPos < rightLength ) {
		const wchar_t leftCh = isNameSeparator( leftName[leftPos] ) ? getNameSeparator() : ::towupper( leftName[leftPos] );
		const wchar_t rightCh = isNameSeparator( rightName[rightPos] ) ? getNameSeparator() : ::towupper( rightName[rightPos] );

		const int cmpResult = leftCh - rightCh;
		if( cmpResult != 0 ) {
			return cmpResult;
		}

		leftPos++;
		rightPos++;
	}

	// At this moment one of the strings has reached its end. We need to skip all the separators at the end.
	while( leftPos < leftLength && isNameSeparator( leftName[leftPos] ) ) {
		leftPos++;
	}
	while( rightPos < rightLength && isNameSeparator( rightName[rightPos] ) ) {
		rightPos++;
	}

	if( leftPos == leftLength ) {
		return rightPos == rightLength ? 0 : -1;
	} else {
		return 1;
	}
}

bool NamesEqual( CUnicodePart leftName, CUnicodePart rightName )
{
	return CompareNames( leftName, rightName ) == 0;
}

TPathType GetPathType( CUnicodePart path )
{
	const int length = path.Length();
	if( length >= 2 && isNameSeparator( path[0] ) && path[0] == path[1] ) {
		return PT_UNC;
	} else if( length >= 2 && path[1] == L':' ) {
		return length >= 3 && isNameSeparator( path[2] ) ? PT_Absolute : PT_RelativeWithDrive;
	} else if( length > 0 && isNameSeparator( path[0] ) ) {
		return PT_RelativeFromRoot;
	}
	return PT_Relative;
}

static int findLastSeparator( CUnicodePart path )
{
	for( int i = path.Length() - 1; i >= 0; i-- ) {
		if( isNameSeparator( path[i] ) ) {
			return i;
		}
	}
	return NotFound;
}

void SplitName( CUnicodeView fullName, CUnicodeString& drive, CUnicodeString& dir, CUnicodeString& name, CUnicodeString& ext )
{
	const int length = fullName.Length();
	_wsplitpath_s( fullName.Ptr(), drive.CreateRawBuffer( _MAX_DRIVE ), _MAX_DRIVE + 1, dir.CreateRawBuffer( length ), length + 1, 
		name.CreateRawBuffer( length ), length + 1, ext.CreateRawBuffer( length ), length + 1 );
}

TPathType SplitName( CUnicodePart path, CArray<CUnicodePart>& components )
{
	const TPathType pathType = GetPathType( path );
	components.Empty();
	
	while( !path.IsEmpty() ) {
		const int pos = findLastSeparator( path );
		CUnicodePart newComponent;
		if( pos == NotFound ) {
			newComponent = path;
			path = CUnicodePart();
		} else {
			newComponent = path.Mid( pos + 1 );
			path = path.Left( pos );
		}
		if( !newComponent.IsEmpty() ) {
			components.InsertAt( 0, move( newComponent ) );
		}
	}
	// Sometimes the first component has a drive name in it, we need to check for this case and split the component if necessary.
	if( pathType == PT_RelativeWithDrive ) {
		assert( components[0][1] == L':' );
		if( components[0].Length() > 2 ) {
			const auto folderName = components[0].Mid( 2 );
			components[0] = components[0].Left( 2 );
			components.InsertAt( 1, folderName );
		}
	}
	return pathType;
}

CUnicodeString MergeName( CUnicodePart driveDir, CUnicodePart nameExt )
{
	return MergeName( driveDir, GetName( nameExt ), GetExt( nameExt ) );
}

CUnicodeString MergeName( CUnicodePart driveDir, CUnicodeView name, CUnicodeView ext )
{
	CUnicodeString rawDriveDir = UnicodeStr( driveDir );
	if( !rawDriveDir.IsEmpty() ) {
		AddPathSeparator( rawDriveDir );
	}
	return MergeName( GetDrive( rawDriveDir ), GetPath( rawDriveDir ), name, ext );
}

CUnicodeString MergeName( CUnicodeView drive, CUnicodeView dir, CUnicodeView name, CUnicodeView ext )
{
	CUnicodeString result;
	const int bufferSize = max( MAX_PATH, drive.Length() + dir.Length() + name.Length() + ext.Length() + 3 );
	_wmakepath_s( result.CreateRawBuffer( bufferSize ), bufferSize + 1, drive.Ptr(), dir.Ptr(), name.Ptr(), ext.Ptr() );
	return result;
}

static CUnicodeString mergePath( CUnicodePart dir, CUnicodePart relativePath )
{
	if( dir.IsEmpty() ) {
		return UnicodeStr( relativePath );
	}
	int separatorCount = 0;
	if( isNameSeparator( dir.Last() ) ) {
		separatorCount++;
	}
	if( !relativePath.IsEmpty() && isNameSeparator( relativePath.First() ) ) {
		separatorCount++;
	}
	switch( separatorCount ) {
		case 0: {
			CUnicodeString dirCopy = UnicodeStr( dir );
			AddPathSeparator( dirCopy );
			return move( dirCopy ) + relativePath;
		}
		case 1:
			return dir + relativePath;
		case 2:
			return dir.Left( dir.Length() - 1 ) + relativePath;
		default:
			assert( false );
			return CUnicodeString();
	}
}

CUnicodeString MergePath( CUnicodePart dir, CUnicodePart relativePath )
{
	switch( GetPathType( relativePath ) ) {
		case PT_UNC:
		case PT_Absolute:
			// RelativePath is already absolute.
			return UnicodeStr( relativePath );
		case PT_Relative:
			return mergePath( dir, relativePath );
		case PT_RelativeFromRoot:
			return mergePath( GetDrive( dir ), relativePath );
		case PT_RelativeWithDrive:
			// Extremely rare case, not worth it to demand null termination from relative path, just make a copy here.
			return CreateFullPath( UnicodeStr( relativePath ) );
		default:
			assert( false );
			return CUnicodeString();
	}
}

void AddExtIfNone( CUnicodeString& fullName, CUnicodeView extNoPeriod )
{
	CUnicodeString drive;
	CUnicodeString dir;
	CUnicodeString name;
	CUnicodeString oldExt;
	SplitName( fullName, drive, dir, name, oldExt );
	if( oldExt.IsEmpty() ) {
		fullName = MergeName( drive, dir, name, extNoPeriod );
	}
}

void ReplaceExt( CUnicodeString& fullName, CUnicodeView extNoPeriod )
{
	CUnicodeString drive;
	CUnicodeString dir;
	CUnicodeString name;
	CUnicodeString oldExt;
	SplitName( fullName, drive, dir, name, oldExt );
	fullName = MergeName( drive, dir, name, extNoPeriod );
}

CUnicodeString CreateFullPath( CUnicodeView path )
{
	CUnicodeString result;
	int bufferLength = MAX_PATH + 1;
	auto resPtr = result.CreateRawBuffer( bufferLength - 1 );
	int length = ::GetFullPathNameW( path.Ptr(), bufferLength, resPtr, nullptr );
	if( length > bufferLength ) {
		bufferLength = length;
		resPtr.Release( 0 );
		resPtr = result.CreateRawBuffer( bufferLength - 1 );
		length = ::GetFullPathNameW( path.Ptr(), bufferLength, resPtr, nullptr );
	}
	assert( length < bufferLength );
	resPtr.Release( length );
	return result;
}

CUnicodeString CreateFullPath( CUnicodeView dir, CUnicodeView relativePath )
{
	return CreateFullPath( MergePath( dir, relativePath ) );
}

// Generate an exception from the fileName if condition is false.
static void checkLastFileError( bool condition, CUnicodePart fileName )
{
	if( !condition ) {
		ThrowFileException( GetLastError(), fileName );
	}
}

CUnicodeString CreateLongPath( CUnicodeView path )
{
	CUnicodeString result;
	int bufferLength = MAX_PATH + 1;
	auto resPtr = result.CreateRawBuffer( bufferLength - 1 );
	int length = ::GetLongPathNameW( path.Ptr(), resPtr, bufferLength );
	if( length > bufferLength ) {
		bufferLength = length;
		resPtr.Release( 0 );
		resPtr = result.CreateRawBuffer( bufferLength - 1 );
		length = ::GetLongPathNameW( path.Ptr(), resPtr, bufferLength );
	}
	assert( length < bufferLength );
	resPtr.Release( length );
	return result;
}

static const wchar_t* unicodePathSeparators = L"\\/";
CUnicodeString GetRoot( CUnicodePart path )
{
	CUnicodeString rawPath = UnicodeStr( path );
	if( GetPathType( rawPath ) == PT_UNC ) {
		int pos = rawPath.FindOneOf( unicodePathSeparators, 2 );
		if( pos == NotFound ) {
			AddPathSeparator( rawPath );
			return rawPath;
		}
		pos = rawPath.ReverseFindOneOf( unicodePathSeparators, pos + 1 );
		if( pos == NotFound ) {
			AddPathSeparator( rawPath );
			return rawPath;
		}
		return UnicodeStr( rawPath.Left( pos + 1 ) );
	}
	CUnicodeString result = GetDrive( rawPath );
	AddPathSeparator( result );
	return result;
}

CUnicodeString GetDrive( CUnicodePart name )
{
	const int length = name.Length();

	for( int i = 0; i < length; i++ ) {
		const auto ch = name[i];
		if( ch == L':' ) {
			return UnicodeStr( name.Left( i + 1 ) );
		} else if( isNameSeparator( ch ) ) {
			break;
		}
	}

	return CUnicodeString();
}

static const CUnicodeView nameSeparators = L"\\/";
CUnicodeString GetPath( CUnicodePart name )
{
	const int length = name.Length();

	const int nameEndPos = name.ReverseFindOneOf( nameSeparators );
	if( nameEndPos == NotFound ) {
		return CUnicodeString();
	}

	for( int i = 0; i < length; i++ ) {
		const auto ch = name[i];
		if( ch == L':' ) {
			return UnicodeStr( name.Mid( i + 1, nameEndPos - i ) );
		} else if( isNameSeparator( ch ) ) {
			return UnicodeStr( name.Left( nameEndPos + 1 ) );
		}
	}

	// Name separator is present in the path, previous loop should've been broken.
	assert( false );
	return CUnicodeString();
}

CUnicodeString GetDrivePath( CUnicodePart name )
{
	const int nameEndPos = name.ReverseFindOneOf( nameSeparators );
	if( nameEndPos == NotFound ) {
		return CUnicodeString();
	}

	return UnicodeStr( name.Left( nameEndPos + 1 ) );
}

CUnicodeString GetName( CUnicodePart name )
{
	const int length = name.Length();
	int nameEndPos = length;
	for( int i = nameEndPos - 1; i >= 0; i-- ) {
		const auto ch = name[i];
		if( ch == L'.' && nameEndPos == length ) {
			// The first dot is an extension separator.
			nameEndPos = i;
		} else if( isNameSeparator( ch ) ) {
			const int nameStartPos = i + 1;
			return UnicodeStr( name.Mid( nameStartPos, nameEndPos - nameStartPos ) );
		}
	}

	return UnicodeStr( name.Left( nameEndPos ) );
}

CUnicodeString GetExt( CUnicodePart name )
{
	const int length = name.Length();
	for( int i = length - 1; i >= 0; i-- ) {
		const auto ch = name[i];
		if( ch == L'.' ) {
			// Special tokens "." and ".." have no extension.
			if( ( length == 1 ) || ( length == 2 && name[0] == L'.' && i == 1 ) ) {
				return CUnicodeString();
			} else {
				return UnicodeStr( name.Mid( i ) );
			}
		} else if( isNameSeparator( ch ) ) {
			break;
		}
	}

	return CUnicodeString();
}

CUnicodeString GetNameExt( CUnicodePart name )
{
	const int length = name.Length();
	for( int i = length - 1; i >= 0; i-- ) {
		const auto ch = name[i];
		if( isNameSeparator( ch ) ) {
			return UnicodeStr( name.Mid( i + 1 ) );
		}
	}

	return UnicodeStr( name );
}

void AddPathSeparator( CUnicodeString& path )
{
	if( path.IsEmpty() ) {
		path += getNameSeparator();
		return;	
	}

	int separatorPos = path.Length() - 1;
	for( ; separatorPos > 0; separatorPos-- ) {
		if( isNameSeparator( path[separatorPos] ) ) {
			break;
		}
	}
	if( separatorPos == 0 && !isNameSeparator( path[separatorPos] ) ) {
		path += getNameSeparator();
	} else if( separatorPos < path.Length() - 1 ) {
		path += path[separatorPos];
	}
}

void NormalizePath( CUnicodeString& path )
{
	const int length = path.Length();
	if( length == 3 && isLetterLatin( path[0] ) && path[1] == L':' && isNameSeparator( path[2] ) ) {
		return;
	}
	if( length == 2 && isLetterLatin( path[0] ) && path[1] == L':' ) {
		path += L'\\';
	}
	if( length < 2 ) {
		return;
	}

	if( isNameSeparator( path.Last() ) ) {
		path.DeleteAt( path.Length() - 1 );
	}
}

void ForceBackSlashes( CUnicodeString& path )
{
	for( int i = 0; i < path.Length(); i++ ) {
		if( isNameSeparator( path[i] ) ) {
			path.ReplaceAt( i, L'\\' );
		}
	}
}

bool DirAccessible( CUnicodeView dir )
{
	try {
		if( dir.IsEmpty() ) {
			return false;
		}
		CUnicodeString fullDir = CreateFullPath( dir );
		if( fullDir.Length() < 3 ) {
			return false;
		}
		if( isNameSeparator( fullDir[0] ) ) {
			if( GetPathType( fullDir ) != PT_UNC ) {
				return false;
			}
		} else if( fullDir[1] != L':' ) {
			return false;
		}
		// Check the root catalog case.
		if( fullDir.Length() == 3 && fullDir[1] == ':' && isNameSeparator( fullDir[2] ) ) {
			const UINT driveType = GetDriveTypeW( fullDir.Ptr() ); 
			return driveType != DRIVE_NO_ROOT_DIR;
		}

		// Try and find at least one file in the directory. Every catalog but the root one has a file named "."
		CUnicodeString fileMask = MergeName( fullDir, L"*" );
		return FileExists( fileMask );

	} catch( const CException& ) {
		// If the exception occurred we assume the directory is inaccessible.
	}
	return false;
}

bool FileExists( CUnicodeView fileMask )
{
	if( fileMask.IsEmpty() ) {
		return false;
	}
	WIN32_FIND_DATAW findData;
	const UINT prevErrorMode = SetErrorMode( 0 );
	SetErrorMode( prevErrorMode | SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );
	HANDLE findResult = FindFirstFileW( fileMask.Ptr(), &findData );
	SetErrorMode( prevErrorMode );
	if( findResult == INVALID_HANDLE_VALUE ) {
		return false;
	}
	FindClose( findResult );
	return true;
}

bool CanOpenFile( CUnicodeView fileName, TFileReadWriteMode readWriteMode, TFileCreationMode createMode, TFileShareMode shareMode )
{
	if( fileName.IsEmpty() ) {
		return false;
	}
	CDynamicFile file;
	return file.TryOpen( fileName, readWriteMode, createMode, shareMode );
}

DWORD GetAttributes( CUnicodeView fileName )
{
	const DWORD attributes = GetFileAttributesW( fileName.Ptr() );
	checkLastFileError( attributes != INVALID_FILE_ATTRIBUTES, fileName );
	return attributes;
}

void SetAttributes( CUnicodeView fileName, DWORD attributes )
{
	checkLastFileError( SetFileAttributesW( fileName.Ptr(), attributes ) != 0, fileName );
}

void Rename( CUnicodeView fileName, CUnicodeView newFileName )
{
	checkLastFileError( ::MoveFileW( fileName.Ptr(), newFileName.Ptr() ) != 0, newFileName );
}

void Delete( CUnicodeView fileName )
{
	checkLastFileError( ::DeleteFileW( fileName.Ptr() ) != 0, fileName );
}

// Additional checks for the destination file to be copyable.
static void checkCanCopyTo( CUnicodeView fileName )
{
	if( FileExists( fileName ) ) {
		// Check if the destination file can be copied over.
		if( !CanOpenFile( fileName, FRWM_Write, FCM_OpenExisting, FMS_Exclusive ) ) {
			ThrowFileException( CFileException::FET_AccessDenied, fileName );
		}
	} else {
		// Check if the parent directory exists and is writable.
		const CUnicodeString fullPath = CreateFullPath( fileName );
		CUnicodeString folderName = GetDrivePath( fullPath );
		if( !DirAccessible( folderName ) ) {
			ThrowFileException( CFileException::FET_BadPath, fileName );
		}
	}
}

void Copy( CUnicodeView src, CUnicodeView dest )
{
	if( !::CopyFileW( src.Ptr(), dest.Ptr(), FALSE ) ) {
		checkLastFileError( CanOpenFile( src, FRWM_Read, FCM_OpenExisting, FSM_DenyWrite ), src );
		checkCanCopyTo( dest );
		if( FileExists( dest ) ) {
			// Deletion will help if dest has a hidden attribute.
			Delete( dest );
		}
		checkLastFileError( ::CopyFileW( src.Ptr(), dest.Ptr(), FALSE ) != 0, dest );
	}
}

void Move( CUnicodeView src, CUnicodeView dest )
{
	checkLastFileError( MoveFileExW( src.Ptr(), dest.Ptr(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH ) != 0, dest );
}

static bool deleteLastComponent( CUnicodeString& dir )
{
	if( dir.IsEmpty() ) {
		return false;
	}

	if( isNameSeparator( dir.Last() ) ) {
		dir.DeleteAt( dir.Length() - 1 );
		if( dir.IsEmpty() ) {
			return false;
		}
	}

	const int pos = findLastSeparator( dir );
	if( pos == NotFound ) {
		return false;
	}
	dir.DeleteFrom( pos );
	return true;
}

static int tryCreateDir( CUnicodeView dir ) 
{
	if( CreateDirectoryW( dir.Ptr(), 0 ) != 0 ) {
		return 0;
	}
	return GetLastError();
}

void CreateDir( CUnicodeView dir )
{
	DWORD errorCode = tryCreateDir( dir );
	if( errorCode == 0 ) {
		return;
	}
	// Check if the parent dir exists. Old windows system return ERROR_FILE_NOT_FOUND for UNC paths in this case.
	if( errorCode == ERROR_PATH_NOT_FOUND || errorCode == ERROR_FILE_NOT_FOUND ) {
		CUnicodeString parentDir = UnicodeStr( dir );
		if( deleteLastComponent( parentDir ) && !DirAccessible( parentDir ) ) {
			CreateDir( parentDir );
			errorCode = tryCreateDir( dir );
		}
	}
	if( errorCode != 0 ) {
		ThrowFileException( errorCode, dir );
	}
}

void DeleteDir( CUnicodeView dir )
{
	checkLastFileError( RemoveDirectoryW( dir.Ptr() ) != 0, dir );
}

void DeleteTree( CUnicodeView dir )
{
	CArray<CFileStatus> files;
	GetFilesInDir( dir, files, FIF_Directories | FIF_Files | FIF_Hidden );
	
	for( const auto& file : files ) {
		if( HasFlag( file.Attributes, FILE_ATTRIBUTE_DIRECTORY ) ) {
			DeleteTree( file.FullName );
		} else {
			if( HasFlag( file.Attributes, FILE_ATTRIBUTE_READONLY ) ) {
				SetAttributes( file.FullName, file.Attributes & ~FILE_ATTRIBUTE_READONLY );
			}
			Delete( file.FullName );
		}
	}

	const DWORD attributes = GetAttributes( dir );
	if( HasFlag( attributes, FILE_ATTRIBUTE_READONLY ) ) {
		SetAttributes( dir, attributes & ~FILE_ATTRIBUTE_READONLY );
	}
	DeleteDir( dir );
}

void CopyTree( CUnicodePart src, CUnicodeView dest )
{
	if( !DirAccessible( dest ) ) {
		CreateDir( dest );
	}

	CArray<CFileStatus> files;
	GetFilesInDir( src, files, FIF_Directories | FIF_Files | FIF_Hidden );

	for( const auto& file : files ) {
		const CUnicodeView srcName = file.FullName;
		const CUnicodeString destName = MergeName( dest, GetNameExt( srcName ) );
		if( HasFlag( file.Attributes, FILE_ATTRIBUTE_DIRECTORY ) ) {
			CopyTree( srcName, destName );
		} else {
			Copy( srcName, destName );
		}
	}
}

void MoveTree( CUnicodeView src, CUnicodeView dest )
{
	if( !DirAccessible( dest ) ) {
		CreateDir( dest );
	}

	CArray<CFileStatus> files;
	GetFilesInDir( src, files, FIF_Directories | FIF_Files | FIF_Hidden );

	for( const auto& file : files ) {
		const CUnicodeView srcName = file.FullName;
		const CUnicodeString destName = MergeName( dest, GetNameExt( srcName ) );
		if( HasFlag( file.Attributes, FILE_ATTRIBUTE_DIRECTORY ) ) {
			MoveTree( srcName, destName );
		} else {
			Move( srcName, destName );
		}
	}
	DeleteDir( src );
}

bool IsDirEmpty( CUnicodePart dir )
{
	CArray<CFileStatus> files;
	GetFilesInDir( dir, files, FIF_Directories | FIF_Files | FIF_Hidden );
	return files.IsEmpty();
}

static HANDLE startFileSearch( CUnicodeView fullPath, WIN32_FIND_DATAW& findData, CUnicodePart dir )
{
	const HANDLE result = FindFirstFileW( fullPath.Ptr(), &findData );
	if( result == INVALID_HANDLE_VALUE ) {
		const DWORD err = GetLastError();
		if( err != ERROR_NO_MORE_FILES && err != ERROR_FILE_NOT_FOUND ) {
			ThrowFileException( err, dir );
		}
	}
	return result;
}

static void finishFileSearch( HANDLE findHandle, CUnicodePart dir )
{
	DWORD err = GetLastError();
	if( err == ERROR_NO_MORE_FILES || err == ERROR_FILE_NOT_FOUND ) {
		checkLastFileError( FindClose( findHandle ) != 0, dir );
	} else {
		FindClose( findHandle );
		ThrowFileException( err, dir );
	}
}

int GetFileCount( CUnicodePart dir )
{
	const CUnicodeView allFilesMask = L"*";
	const CUnicodeString fullPath = MergeName( dir, allFilesMask );

	WIN32_FIND_DATAW findData;
	const HANDLE findHandle = startFileSearch( fullPath, findData, dir );
	if( findHandle == INVALID_HANDLE_VALUE ) {
		return 0;
	}

	int result = 0;
	do {
		if( HasFlag( findData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY ) ) {
			// Filter directories.
			continue;
		}
		const CUnicodeView currentName = findData.cFileName;
		if( currentName == L"." || currentName == L".." ) {
			// Filter the uninteresting folders.
			continue;
		}
		result++;
	} while( FindNextFileW( findHandle, &findData ) );

	finishFileSearch( findHandle, dir );
	return result;
}

static void getAllFilesInDir( CUnicodePart dir, CArray<CFileStatus>& fileList, CUnicodePart mask )
{
	fileList.Empty();
	WIN32_FIND_DATAW findData;
	const CUnicodeString fullPath = MergeName( dir, mask );

	const HANDLE findHandle = startFileSearch( fullPath, findData, dir );
	if( findHandle == INVALID_HANDLE_VALUE ) {
		return;
	}
	do {
		const CUnicodeView currentName = findData.cFileName;
		if( currentName == L"." || currentName == L".." ) {
			// Filter the uninteresting folders.
			continue;
		}
		CFileStatus currentFile;
		currentFile.FullName = MergePath( dir, currentName );
		currentFile.Attributes = findData.dwFileAttributes;
		currentFile.CreationTime = findData.ftCreationTime;
		currentFile.ModificationTime = findData.ftLastWriteTime;
		const auto longFileLength = ( ( ( unsigned long long ) findData.nFileSizeHigh ) << 32 ) | ( unsigned long long ) findData.nFileSizeLow;
		currentFile.Length = numeric_cast<__int64>( longFileLength );

		fileList.Add( move( currentFile ) );
	} while( FindNextFileW( findHandle, &findData ) );

	finishFileSearch( findHandle, dir );
}

static void getFilteredFilesInDir( CUnicodePart dir, CArray<CFileStatus>& fileList, CUnicodePart mask, DWORD flags )
{
	CArray<CFileStatus> allFiles;
	getAllFilesInDir( dir, allFiles, mask );

	for( auto& file : allFiles ) {
		const DWORD attribute = file.Attributes;
		if( !HasFlag( flags, FIF_Directories ) && HasFlag( attribute, FILE_ATTRIBUTE_DIRECTORY ) ) {
			continue;
		}
		if( !HasFlag( flags, FIF_Hidden ) && HasFlag( attribute, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ) ) {
			continue;
		}
		if( !HasFlag( flags, FIF_Files ) && !HasFlag( attribute, FILE_ATTRIBUTE_DIRECTORY ) ) {
			continue;
		}
		fileList.Add( move( file ) );
	}
}

static void getFilesByMultipleMasks( CUnicodePart dir, CArray<CFileStatus>& fileList, CUnicodePart masks, DWORD flags )
{
	for( CUnicodePart mask : masks.Split( L';' ) ) {
		getFilteredFilesInDir( dir, fileList, mask, flags );
		
	}
}

static void getFilesRecursive( CUnicodePart dir, CArray<CFileStatus>& fileList, CUnicodePart masks, DWORD flags )
{
	getFilesByMultipleMasks( dir, fileList, masks, flags );
	if( !HasFlag( flags, FIF_Recursive ) ) {
		return;
	}

	CArray<CFileStatus> subDirs;
	getFilteredFilesInDir( dir, subDirs, L"*", ( flags & FIF_Hidden ) | FIF_Directories );
	for( auto&& subDir : subDirs ) {
		CUnicodeString dirName = move( subDir.FullName );
		NormalizePath( dirName );
		getFilesRecursive( dirName, fileList, masks, flags );
	}
}

static void removeDuplicateFiles( CArray<CFileStatus>& fileList )
{
	fileList.QuickSort( LessByAction( &CFileStatus::FullName ) );

	for( int i = fileList.Size() - 1; i > 0; i-- ) {
		if( NamesEqual( fileList[i].FullName, fileList[i - 1].FullName ) ) {
			fileList.DeleteAt( i );
		}
	}
}

void GetFilesInDir( CUnicodePart dir, CArray<CFileStatus>& result, DWORD flags /*= FIF_Files */, CUnicodePart masks /*= L"*"*/ )
{
	assert( HasFlag( flags, FIF_Files | FIF_Directories ) );
	getFilesRecursive( dir, result, masks, flags );
	// Search by multiple mask can yield duplicates.
	removeDuplicateFiles( result );
}

CUnicodeString CreateUniqueName( CUnicodePart dir, CUnicodePart prefix, CUnicodeView extension )
{
	int suffix = 0;
	for( ;; ) {
		CUnicodeString newName = prefix + L'(' + UnicodeStr( suffix ) + L')';
		CUnicodeString newFullName = MergeName( dir, newName, extension );
		if( !FileExists( newFullName ) ) {
			return move( newFullName );
		}
		suffix++;
	}
}

CUnicodeString GetCurrentDir()
{
	const int bufferLength = GetCurrentDirectoryW( 0, 0 );
	checkLastFileError( bufferLength > 0, L"" );

	CUnicodeString result;
	auto buffer = result.CreateRawBuffer( bufferLength - 1 );
	const int length = GetCurrentDirectoryW( bufferLength, buffer );
	assert( length < bufferLength );
	buffer.Release( length );
	checkLastFileError( length != 0, L"" );
	return result;
}

void SetCurrentDir( CUnicodeView newDir )
{
	CUnicodeString path = CreateFullPath( newDir );
	checkLastFileError( SetCurrentDirectoryW( path.Ptr() ) != 0, path );
}

CUnicodeString GetExecutableName()
{
	int size = MAX_PATH;
	CUnicodeString shortExeName;
	for( ;; ) {
		auto resultPtr = shortExeName.CreateRawBuffer( size );
		const int length = ::GetModuleFileName( nullptr, resultPtr, size + 1 );
		resultPtr.Release( min( length, size ) );
		checkLastFileError( length != 0, CUnicodePart() );
		if( length <= size ) {
			break;
		}
		size *= 2;
	}
	// Convert the executable path name to long.
	return CreateLongPath( move( shortExeName ) );
}

CUnicodeString GetWindowsDir()
{
	const int length = GetWindowsDirectory( 0, 0 );
	checkLastFileError( length > 0, CUnicodePart() );

	CUnicodeString result;
	auto buffer = result.CreateRawBuffer( length - 1 );
	const int realLength = GetWindowsDirectory( buffer, length );
	buffer.Release( realLength );
	checkLastFileError( realLength != 0, CUnicodeString() );
	assert( realLength < length );
	return result;
}

CUnicodeString GetWindowsTempDir()
{
	const int length = ::GetTempPath( 0, 0 );
	checkLastFileError( length > 0, CUnicodePart() );

	CUnicodeString result;
	auto buffer = result.CreateRawBuffer( length - 1 );
	const int realLength = ::GetTempPath( length, buffer );
	buffer.Release( realLength );
	checkLastFileError( realLength != 0, CUnicodeString() );
	assert( realLength < length );
	NormalizePath( result );
	return result;
}

CUnicodeString REAPI GetEnvironmentVariable( CUnicodeView name )
{
	const int bufferFullSize = ::GetEnvironmentVariable( name.Ptr(), nullptr, 0 );
	checkLastError( bufferFullSize > 0 );
	const int bufferSize = bufferFullSize - 1;
	CUnicodeString result;
	auto resultBuffer = result.CreateRawBuffer( bufferSize );
	::GetEnvironmentVariable( name.Ptr(), resultBuffer, bufferFullSize );
	resultBuffer.Release( bufferSize );
	return move( result );
}

extern CUnicodeString SpecificUserAppDataPath;
extern CUnicodeString AllUsersAppDataPath;
extern CCriticalSection ApplicationDataSection;

static CUnicodeString getAppDataPathSimple( TAppDataPathType type )
{
	CCriticalSectionLock lock( ApplicationDataSection );
	switch( type ) {
	case ADPT_SpecificUser:
		return copy( SpecificUserAppDataPath );
	case ADPT_AllUsers:
		return copy( AllUsersAppDataPath );
	default:
		assert( false );
		return copy( SpecificUserAppDataPath );
	}
}

CUnicodeView GetAppDataPath( TAppDataPathType type )
{
	const CUnicodeString path = getAppDataPathSimple( type );
	try {
		if( !DirAccessible( path ) ) {
			CreateDir( path );
		}
	} catch( const CFileException& e ) {
		// ERROR_ALREADY_EXISTS should be ignored due to race conditions.
		if( e.ErrorCode() != ERROR_ALREADY_EXISTS ) {
			throw;
		}
	}
	return path;
}

void SetAppDataPath( TAppDataPathType type, CUnicodePart path )
{
	CCriticalSectionLock lock( ApplicationDataSection );
	switch( type ) {
	case ADPT_SpecificUser:
		SpecificUserAppDataPath = path;
		break;
	case ADPT_AllUsers:
		AllUsersAppDataPath = path;
		break;
	default:
		assert( false );
	}
}

static CUnicodeString getSpecialFolderLocation( int csIdl )
{
	ITEMIDLIST* pidl = 0;
	HRESULT hResult = SHGetFolderLocation( 0, csIdl, 0, 0, &pidl );
	assert( !FAILED( hResult ) );
	hResult;

	CUnicodeString path;
	auto buffer = path.CreateRawBuffer( MAX_PATH );
	if( SHGetPathFromIDListW( pidl, buffer ) != 0 ) {
		buffer.Release();
	} else {
		buffer.Release( 0 );
	}
	ILFree( pidl );
	AddPathSeparator( path );
	return path;
}

void SetAppDataRelativePath( CUnicodeView path )
{
	const CUnicodeString specificUserFolder = getSpecialFolderLocation( CSIDL_LOCAL_APPDATA );
	SetAppDataPath( ADPT_SpecificUser, MergePath( specificUserFolder, path ) );

	const CUnicodeString allUsersFolder = getSpecialFolderLocation( CSIDL_COMMON_APPDATA );
	SetAppDataPath( ADPT_AllUsers, MergePath( specificUserFolder, path ) );	
}

int GetDiskFreeSpace( CUnicodeView path )
{
	CUnicodeString root = GetRoot( path );
	ForceBackSlashes( root );

	ULARGE_INTEGER free;
	free.QuadPart = 0;

	checkLastFileError( GetDiskFreeSpaceExW( root.Ptr(), &free, 0, 0 ) != 0, path );

	return free.QuadPart > INT_MAX ? INT_MAX : numeric_cast<int>( free.QuadPart );
}

int GetCurrentDiskFreeSpace()
{
	return GetDiskFreeSpace( GetCurrentDir() );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace FileSystem.

}	// namespace Relib.
