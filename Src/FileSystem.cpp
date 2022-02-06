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
	
static bool isLetterLatin( char letter )
{
	return ( letter >= 'A' && letter <= 'Z' ) || ( letter >= 'a' && letter <= 'z' );
}

static bool checkDrive( CStringPart drive )
{
	return drive.Length() == 2 && isLetterLatin( drive[0] ) && drive[1] == ':';
}

static bool isValidSymbol( wchar_t symbol )
{
	return !InvalidFileNameSymbols.Has( symbol );
}

static bool checkComponent( CStringPart str )
{
	if( str.IsEmpty() ) {
		return false;
	}
	bool nonSpaceSymbolFound = false;
	for( auto c : str ) {
		if( !isValidSymbol( c ) ) {
			return false;
		}
		if( !CString::IsCharWhiteSpace( c ) ) {
			nonSpaceSymbolFound = true;
		}
	}
	// Valid path cannot only consist of spaces.
	return nonSpaceSymbolFound;
}

bool IsNameValid( CStringPart name )
{
	CArray<CStringPart> components;
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

static char getNameSeparator()
{
	return '\\';
}

static bool isNameSeparator( char symbol )
{
	return symbol == '\\' || symbol == '/';
}

int CompareNames( CStringPart leftName, CStringPart rightName )
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

bool NamesEqual( CStringPart leftName, CStringPart rightName )
{
	return CompareNames( leftName, rightName ) == 0;
}

TPathType GetPathType( CStringPart path )
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

static int findLastSeparator( CStringPart path )
{
	for( int i = path.Length() - 1; i >= 0; i-- ) {
		if( isNameSeparator( path[i] ) ) {
			return i;
		}
	}
	return NotFound;
}

void SplitName( CStringView fullName, CString& drive, CString& dir, CString& name, CString& ext )
{
	const int length = fullName.Length();
	_splitpath_s( fullName.Ptr(), drive.CreateRawBuffer( _MAX_DRIVE ), _MAX_DRIVE + 1, dir.CreateRawBuffer( length ), length + 1, 
		name.CreateRawBuffer( length ), length + 1, ext.CreateRawBuffer( length ), length + 1 );
}

TPathType SplitName( CStringPart path, CArray<CStringPart>& components )
{
	const TPathType pathType = GetPathType( path );
	components.Empty();
	
	while( !path.IsEmpty() ) {
		const int pos = findLastSeparator( path );
		CStringPart newComponent;
		if( pos == NotFound ) {
			newComponent = path;
			path = CStringPart();
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

CString MergeName( CStringPart driveDir, CStringPart nameExt )
{
	return MergeName( driveDir, GetName( nameExt ), GetExt( nameExt ) );
}

CString MergeName( CStringPart driveDir, CStringView name, CStringView ext )
{
	CString rawDriveDir = Str( driveDir );
	if( !rawDriveDir.IsEmpty() ) {
		AddPathSeparator( rawDriveDir );
	}
	return MergeName( GetDrive( rawDriveDir ), GetPath( rawDriveDir ), name, ext );
}

CString MergeName( CStringView drive, CStringView dir, CStringView name, CStringView ext )
{
	CString result;
	const int bufferSize = max( MAX_PATH, drive.Length() + dir.Length() + name.Length() + ext.Length() + 3 );
	_makepath_s( result.CreateRawBuffer( bufferSize ), bufferSize + 1, drive.Ptr(), dir.Ptr(), name.Ptr(), ext.Ptr() );
	return result;
}

static CString mergePath( CStringPart dir, CStringPart relativePath )
{
	if( dir.IsEmpty() ) {
		return Str( relativePath );
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
			auto dirCopy = Str( dir );
			AddPathSeparator( dirCopy );
			return move( dirCopy ) + relativePath;
		}
		case 1:
			return dir + relativePath;
		case 2:
			return dir.Left( dir.Length() - 1 ) + relativePath;
		default:
			assert( false );
			return CString();
	}
}

CString MergePath( CStringPart dir, CStringPart relativePath )
{
	switch( GetPathType( relativePath ) ) {
		case PT_UNC:
		case PT_Absolute:
			// RelativePath is already absolute.
			return Str( relativePath );
		case PT_Relative:
			return mergePath( dir, relativePath );
		case PT_RelativeFromRoot:
			return mergePath( GetDrive( dir ), relativePath );
		case PT_RelativeWithDrive:
			return CreateFullPath( relativePath );
		default:
			assert( false );
			return CString();
	}
}

void AddExtIfNone( CString& fullName, CStringView extNoPeriod )
{
	CString drive;
	CString dir;
	CString name;
	CString oldExt;
	SplitName( fullName, drive, dir, name, oldExt );
	if( oldExt.IsEmpty() ) {
		fullName = MergeName( drive, dir, name, extNoPeriod );
	}
}

void ReplaceExt( CString& fullName, CStringView extNoPeriod )
{
	CString drive;
	CString dir;
	CString name;
	CString oldExt;
	SplitName( fullName, drive, dir, name, oldExt );
	fullName = MergeName( drive, dir, name, extNoPeriod );
}

CUnicodeString CreateFullUnicodePath( CUnicodeView path )
{
	CUnicodeString result;
	int bufferLength = MAX_PATH + 1;
	auto resPtr = result.CreateRawBuffer( bufferLength - 1 );
	int length = ::GetFullPathName( path.Ptr(), bufferLength, resPtr, nullptr );
	if( length > bufferLength ) {
		bufferLength = length;
		resPtr.Release( 0 );
		resPtr = result.CreateRawBuffer( bufferLength - 1 );
		length = ::GetFullPathName( path.Ptr(), bufferLength, resPtr, nullptr );
	}
	assert( length < bufferLength );
	resPtr.Release( length );
	return result;
}

CString CreateFullPath( CStringPart path )
{
	CUnicodeString unicodePath = UnicodeStr( path );
	return Str( CreateFullUnicodePath( unicodePath ) );
}

CString CreateFullPath( CStringPart dir, CStringPart relativePath )
{
	return CreateFullPath( MergePath( dir, relativePath ) );
}

// Generate an exception from the fileName if condition is false.
static void checkLastFileError( bool condition, CStringPart fileName )
{
	if( !condition ) {
		ThrowFileException( GetLastError(), fileName );
	}
}

static CUnicodeString createLongUnicodePath( CUnicodeView path )
{
	CUnicodeString result;
	int bufferLength = MAX_PATH + 1;
	auto resPtr = result.CreateRawBuffer( bufferLength - 1 );
	int length = ::GetLongPathName( path.Ptr(), resPtr, bufferLength );
	if( length > bufferLength ) {
		bufferLength = length;
		resPtr.Release( 0 );
		resPtr = result.CreateRawBuffer( bufferLength - 1 );
		length = ::GetLongPathName( path.Ptr(), resPtr, bufferLength );
	}
	assert( length < bufferLength );
	resPtr.Release( length );
	return result;
}

CString CreateLongPath( CStringPart path )
{
	const auto unicodePath = UnicodeStr( path );
	return Str( createLongUnicodePath( unicodePath ) );
}

static const char* pathSeparators = "\\/";
CString GetRoot( CStringPart path )
{
	CString rawPath = Str( path );
	if( GetPathType( rawPath ) == PT_UNC ) {
		int pos = rawPath.FindOneOf( pathSeparators, 2 );
		if( pos == NotFound ) {
			AddPathSeparator( rawPath );
			return rawPath;
		}
		pos = rawPath.ReverseFindOneOf( pathSeparators, pos + 1 );
		if( pos == NotFound ) {
			AddPathSeparator( rawPath );
			return rawPath;
		}
		return Str( rawPath.Left( pos + 1 ) );
	}
	CString result = GetDrive( rawPath );
	AddPathSeparator( result );
	return result;
}

CString GetDrive( CStringPart name )
{
	const int length = name.Length();

	for( int i = 0; i < length; i++ ) {
		const auto ch = name[i];
		if( ch == L':' ) {
			return Str( name.Left( i + 1 ) );
		} else if( isNameSeparator( ch ) ) {
			break;
		}
	}

	return CString();
}

static const CStringView nameSeparators = "\\/";
CString GetPath( CStringPart name )
{
	const int length = name.Length();

	const int nameEndPos = name.ReverseFindOneOf( nameSeparators );
	if( nameEndPos == NotFound ) {
		return CString();
	}

	for( int i = 0; i < length; i++ ) {
		const auto ch = name[i];
		if( ch == L':' ) {
			return Str( name.Mid( i + 1, nameEndPos - i ) );
		} else if( isNameSeparator( ch ) ) {
			return Str( name.Left( nameEndPos + 1 ) );
		}
	}

	// Name separator is present in the path, previous loop should've been broken.
	assert( false );
	return CString();
}

CString GetDrivePath( CStringPart name )
{
	const int nameEndPos = name.ReverseFindOneOf( nameSeparators );
	if( nameEndPos == NotFound ) {
		return CString();
	}

	return Str( name.Left( nameEndPos + 1 ) );
}

CString GetName( CStringPart name )
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
			return Str( name.Mid( nameStartPos, nameEndPos - nameStartPos ) );
		}
	}

	return Str( name.Left( nameEndPos ) );
}

CString GetExt( CStringPart name )
{
	const int length = name.Length();
	for( int i = length - 1; i >= 0; i-- ) {
		const auto ch = name[i];
		if( ch == L'.' ) {
			// Special tokens "." and ".." have no extension.
			if( ( length == 1 ) || ( length == 2 && name[0] == L'.' && i == 1 ) ) {
				return CString();
			} else {
				return Str( name.Mid( i ) );
			}
		} else if( isNameSeparator( ch ) ) {
			break;
		}
	}

	return CString();
}

CString GetNameExt( CStringPart name )
{
	const int length = name.Length();
	for( int i = length - 1; i >= 0; i-- ) {
		const auto ch = name[i];
		if( isNameSeparator( ch ) ) {
			return Str( name.Mid( i + 1 ) );
		}
	}

	return Str( name );
}

void AddPathSeparator( CString& path )
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

void NormalizePath( CString& path )
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

void ForceBackSlashes( CString& path )
{
	for( int i = 0; i < path.Length(); i++ ) {
		if( isNameSeparator( path[i] ) ) {
			path.ReplaceAt( i, L'\\' );
		}
	}
}

bool DirAccessible( CStringPart dir )
{
	try {
		if( dir.IsEmpty() ) {
			return false;
		}
		CString fullDir = CreateFullPath( dir );
		if( fullDir.Length() < 3 ) {
			return false;
		}
		if( isNameSeparator( fullDir[0] ) ) {
			if( GetPathType( fullDir ) != PT_UNC ) {
				return false;
			}
		} else if( fullDir[1] != ':' ) {
			return false;
		}
		// Check the root catalog case.
		if( fullDir.Length() == 3 && fullDir[1] == ':' && isNameSeparator( fullDir[2] ) ) {
			const UINT driveType = GetDriveTypeA( fullDir.Ptr() ); 
			return driveType != DRIVE_NO_ROOT_DIR;
		}

		// Try and find at least one file in the directory. Every catalog but the root one has a file named "."
		CString fileMask = MergeName( fullDir, "*" );
		return FileExists( fileMask );

	} catch( const CException& ) {
		// If the exception occurred we assume the directory is inaccessible.
	}
	return false;
}

bool FileExists( CStringPart fileMask )
{
	const auto unicodeMask = UnicodeStr( fileMask );
	if( fileMask.IsEmpty() ) {
		return false;
	}
	WIN32_FIND_DATA findData;
	const UINT prevErrorMode = SetErrorMode( 0 );
	SetErrorMode( prevErrorMode | SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );
	HANDLE findResult = ::FindFirstFile( unicodeMask.Ptr(), &findData );
	SetErrorMode( prevErrorMode );
	if( findResult == INVALID_HANDLE_VALUE ) {
		return false;
	}
	FindClose( findResult );
	return true;
}

bool CanOpenFile( CStringPart fileName, TFileReadWriteMode readWriteMode, TFileCreationMode createMode, TFileShareMode shareMode )
{
	if( fileName.IsEmpty() ) {
		return false;
	}
	CDynamicFile file;
	return file.TryOpen( fileName, readWriteMode, createMode, shareMode );
}

DWORD GetAttributes( CStringPart fileName )
{
	const auto unicodeName = UnicodeStr( fileName );
	const DWORD attributes = GetFileAttributesW( unicodeName.Ptr() );
	checkLastFileError( attributes != INVALID_FILE_ATTRIBUTES, fileName );
	return attributes;
}

void SetAttributes( CStringPart fileName, DWORD attributes )
{
	const auto unicodeName = UnicodeStr( fileName );
	checkLastFileError( SetFileAttributesW( unicodeName.Ptr(), attributes ) != 0, fileName );
}

void Rename( CStringPart fileName, CStringPart newFileName )
{
	const auto unicodeOld = UnicodeStr( fileName );
	const auto unicodeNew = UnicodeStr( newFileName );
	checkLastFileError( ::MoveFileW( unicodeOld.Ptr(), unicodeNew.Ptr() ) != 0, newFileName );
}

void Delete( CStringPart fileName )
{
	const auto unicodeName = UnicodeStr( fileName );
	checkLastFileError( ::DeleteFileW( unicodeName.Ptr() ) != 0, fileName );
}

// Additional checks for the destination file to be copyable.
static void checkCanCopyTo( CStringPart fileName )
{
	if( FileExists( fileName ) ) {
		// Check if the destination file can be copied over.
		if( !CanOpenFile( fileName, FRWM_Write, FCM_OpenExisting, FMS_Exclusive ) ) {
			ThrowFileException( CFileException::FET_AccessDenied, fileName );
		}
	} else {
		// Check if the parent directory exists and is writable.
		const CString fullPath = CreateFullPath( fileName );
		CString folderName = GetDrivePath( fullPath );
		if( !DirAccessible( folderName ) ) {
			ThrowFileException( CFileException::FET_BadPath, fileName );
		}
	}
}

void Copy( CStringPart src, CStringPart dest )
{
	const auto unicodeSrc = UnicodeStr( src );
	const auto unicodeDest = UnicodeStr( dest );

	if( !::CopyFileW( unicodeSrc.Ptr(), unicodeDest.Ptr(), FALSE ) ) {
		checkLastFileError( CanOpenFile( src, FRWM_Read, FCM_OpenExisting, FSM_DenyWrite ), src );
		checkCanCopyTo( dest );
		if( FileExists( dest ) ) {
			// Deletion will help if dest has a hidden attribute.
			Delete( dest );
		}
		checkLastFileError( ::CopyFileW( unicodeSrc.Ptr(), unicodeDest.Ptr(), FALSE ) != 0, dest );
	}
}

void Move( CStringPart src, CStringPart dest )
{
	const auto unicodeSrc = UnicodeStr( src );
	const auto unicodeDest = UnicodeStr( dest );
	checkLastFileError( MoveFileExW( unicodeSrc.Ptr(), unicodeDest.Ptr(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH ) != 0, dest );
}

static bool deleteLastComponent( CString& dir )
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

void CreateDir( CStringPart dir )
{
	const auto unicodeDir = UnicodeStr( dir );
	DWORD errorCode = tryCreateDir( unicodeDir );
	if( errorCode == 0 ) {
		return;
	}
	// Check if the parent dir exists. Old windows system return ERROR_FILE_NOT_FOUND for UNC paths in this case.
	if( errorCode == ERROR_PATH_NOT_FOUND || errorCode == ERROR_FILE_NOT_FOUND ) {
		CString parentDir = Str( unicodeDir );
		if( deleteLastComponent( parentDir ) && !DirAccessible( parentDir ) ) {
			CreateDir( parentDir );
			errorCode = tryCreateDir( unicodeDir );
		}
	}
	if( errorCode != 0 ) {
		ThrowFileException( errorCode, dir );
	}
}

void DeleteDir( CStringPart dir )
{
	const auto unicodeDir = UnicodeStr( dir );
	checkLastFileError( RemoveDirectoryW( unicodeDir.Ptr() ) != 0, dir );
}

void DeleteTree( CStringPart dir )
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

void CopyTree( CStringPart src, CStringView dest )
{
	if( !DirAccessible( dest ) ) {
		CreateDir( dest );
	}

	CArray<CFileStatus> files;
	GetFilesInDir( src, files, FIF_Directories | FIF_Files | FIF_Hidden );

	for( const auto& file : files ) {
		const CStringView srcName = file.FullName;
		const CString destName = MergeName( dest, GetNameExt( srcName ) );
		if( HasFlag( file.Attributes, FILE_ATTRIBUTE_DIRECTORY ) ) {
			CopyTree( srcName, destName );
		} else {
			Copy( srcName, destName );
		}
	}
}

void MoveTree( CStringView src, CStringView dest )
{
	if( !DirAccessible( dest ) ) {
		CreateDir( dest );
	}

	CArray<CFileStatus> files;
	GetFilesInDir( src, files, FIF_Directories | FIF_Files | FIF_Hidden );

	for( const auto& file : files ) {
		const CStringView srcName = file.FullName;
		const CString destName = MergeName( dest, GetNameExt( srcName ) );
		if( HasFlag( file.Attributes, FILE_ATTRIBUTE_DIRECTORY ) ) {
			MoveTree( srcName, destName );
		} else {
			Move( srcName, destName );
		}
	}
	DeleteDir( src );
}

bool IsDirEmpty( CStringPart dir )
{
	CArray<CFileStatus> files;
	GetFilesInDir( dir, files, FIF_Directories | FIF_Files | FIF_Hidden );
	return files.IsEmpty();
}

static HANDLE startFileSearch( CStringView fullPath, WIN32_FIND_DATAW& findData, CStringPart dir )
{
	const auto unicodePath = UnicodeStr( fullPath );
	const auto result = ::FindFirstFile( unicodePath.Ptr(), &findData );
	if( result == INVALID_HANDLE_VALUE ) {
		const DWORD err = GetLastError();
		if( err != ERROR_NO_MORE_FILES && err != ERROR_FILE_NOT_FOUND ) {
			ThrowFileException( err, dir );
		}
	}
	return result;
}

static void finishFileSearch( HANDLE findHandle, CStringPart dir )
{
	DWORD err = GetLastError();
	if( err == ERROR_NO_MORE_FILES || err == ERROR_FILE_NOT_FOUND ) {
		checkLastFileError( ::FindClose( findHandle ) != 0, dir );
	} else {
		::FindClose( findHandle );
		ThrowFileException( err, dir );
	}
}

int GetFileCount( CStringPart dir )
{
	const CStringView allFilesMask = "*";
	const auto fullPath = MergeName( dir, allFilesMask );

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
	} while( ::FindNextFile( findHandle, &findData ) );

	finishFileSearch( findHandle, dir );
	return result;
}

static void getAllFilesInDir( CStringPart dir, CArray<CFileStatus>& fileList, CStringPart mask )
{
	fileList.Empty();
	WIN32_FIND_DATAW findData;
	const CString fullPath = MergeName( dir, mask );

	const HANDLE findHandle = startFileSearch( fullPath, findData, dir );
	if( findHandle == INVALID_HANDLE_VALUE ) {
		return;
	}
	do {
		const CUnicodeView currentName = findData.cFileName;
		if( currentName == L"." || currentName == L".." ) {
			// Filter uninteresting folders.
			continue;
		}
		CFileStatus currentFile;
		currentFile.FullName = MergePath( dir, Str( currentName ) );
		currentFile.Attributes = findData.dwFileAttributes;
		currentFile.CreationTime = findData.ftCreationTime;
		currentFile.ModificationTime = findData.ftLastWriteTime;
		const auto longFileLength = ( ( ( unsigned long long ) findData.nFileSizeHigh ) << 32 ) | ( unsigned long long ) findData.nFileSizeLow;
		currentFile.Length = numeric_cast<__int64>( longFileLength );

		fileList.Add( move( currentFile ) );
	} while( FindNextFileW( findHandle, &findData ) );

	finishFileSearch( findHandle, dir );
}

static void getFilteredFilesInDir( CStringPart dir, CArray<CFileStatus>& fileList, CStringPart mask, DWORD flags )
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

static void getFilesByMultipleMasks( CStringPart dir, CArray<CFileStatus>& fileList, CStringPart masks, DWORD flags )
{
	for( auto mask : masks.Split( ';' ) ) {
		getFilteredFilesInDir( dir, fileList, mask, flags );
		
	}
}

static void getFilesRecursive( CStringPart dir, CArray<CFileStatus>& fileList, CStringPart masks, DWORD flags )
{
	getFilesByMultipleMasks( dir, fileList, masks, flags );
	if( !HasFlag( flags, FIF_Recursive ) ) {
		return;
	}

	CArray<CFileStatus> subDirs;
	getFilteredFilesInDir( dir, subDirs, "*", ( flags & FIF_Hidden ) | FIF_Directories );
	for( auto&& subDir : subDirs ) {
		CString dirName = move( subDir.FullName );
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

void GetFilesInDir( CStringPart dir, CArray<CFileStatus>& result, DWORD flags /*= FIF_Files */, CStringPart masks /*= "*"*/ )
{
	assert( HasFlag( flags, FIF_Files | FIF_Directories ) );
	getFilesRecursive( dir, result, masks, flags );
	// Search by multiple mask can yield duplicates.
	removeDuplicateFiles( result );
}

CString CreateUniqueName( CStringPart dir, CStringPart prefix, CStringView extension )
{
	int suffix = 0;
	for( ;; ) {
		auto newName = prefix + '(' + Str( suffix ) + ')';
		auto newFullName = MergeName( dir, newName, extension );
		if( !FileExists( newFullName ) ) {
			return move( newFullName );
		}
		suffix++;
	}
}

CString GetCurrentDir()
{
	const int bufferLength = ::GetCurrentDirectoryW( 0, nullptr );
	checkLastFileError( bufferLength > 0, "" );

	CUnicodeString result;
	auto buffer = result.CreateRawBuffer( bufferLength - 1 );
	const int length = ::GetCurrentDirectoryW( bufferLength, buffer );
	assert( length < bufferLength );
	buffer.Release( length );
	checkLastFileError( length != 0, "" );
	return Str( result );
}

void SetCurrentDir( CStringPart newDir )
{
	auto path = CreateFullUnicodePath( UnicodeStr( newDir ) );
	checkLastFileError( ::SetCurrentDirectoryW( path.Ptr() ) != 0, newDir );
}

CString GetExecutableName()
{
	int size = MAX_PATH;
	CUnicodeString shortExeName;
	for( ;; ) {
		auto resultPtr = shortExeName.CreateRawBuffer( size );
		const int length = ::GetModuleFileName( nullptr, resultPtr, size + 1 );
		resultPtr.Release( min( length, size ) );
		checkLastFileError( length != 0, CStringPart() );
		if( length <= size ) {
			break;
		}
		size *= 2;
	}
	// Convert the executable path name to long.
	return Str( createLongUnicodePath( move( shortExeName ) ) );
}

CString GetWindowsDir()
{
	const int length = ::GetWindowsDirectory( 0, 0 );
	checkLastFileError( length > 0, CStringPart() );

	CUnicodeString result;
	auto buffer = result.CreateRawBuffer( length - 1 );
	const int realLength = ::GetWindowsDirectory( buffer, length );
	buffer.Release( realLength );
	checkLastFileError( realLength != 0, CString() );
	assert( realLength < length );
	return Str( result ); 
}

CString GetWindowsTempDir()
{
	const int length = ::GetTempPath( 0, 0 );
	checkLastFileError( length > 0, CStringPart() );

	CUnicodeString unicodeResult;
	auto buffer = unicodeResult.CreateRawBuffer( length - 1 );
	const int realLength = ::GetTempPath( length, buffer );
	buffer.Release( realLength );
	checkLastFileError( realLength != 0, CString() );
	assert( realLength < length );
	auto result = Str( unicodeResult );
	NormalizePath( result );
	return result; 
}

CString GetEnvironmentVariable( CStringPart name )
{
	const auto unicodeName = UnicodeStr( name );
	const int bufferFullSize = ::GetEnvironmentVariable( unicodeName.Ptr(), nullptr, 0 );
	checkLastError( bufferFullSize > 0 );
	const int bufferSize = bufferFullSize - 1;
	CUnicodeString result;
	auto resultBuffer = result.CreateRawBuffer( bufferSize );
	::GetEnvironmentVariable( unicodeName.Ptr(), resultBuffer, bufferFullSize );
	resultBuffer.Release( bufferSize );
	return Str( result );
}

extern CString SpecificUserAppDataPath;
extern CString AllUsersAppDataPath;
extern CCriticalSection ApplicationDataSection;

static CString getAppDataPathSimple( TAppDataPathType type )
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

CStringView GetAppDataPath( TAppDataPathType type )
{
	const CString path = getAppDataPathSimple( type );
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

void SetAppDataPath( TAppDataPathType type, CStringPart path )
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

static CString getSpecialFolderLocation( int csIdl )
{
	ITEMIDLIST* pidl = 0;
	HRESULT hResult = SHGetFolderLocation( 0, csIdl, 0, 0, &pidl );
	assert( !FAILED( hResult ) );
	hResult;

	CUnicodeString path;
	auto buffer = path.CreateRawBuffer( MAX_PATH );
	if( ::SHGetPathFromIDListW( pidl, buffer ) != 0 ) {
		buffer.Release();
	} else {
		buffer.Release( 0 );
	}
	::ILFree( pidl );
	auto result = Str( path );
	AddPathSeparator( result );
	return result;
}

void SetAppDataRelativePath( CStringView path )
{
	const CString specificUserFolder = getSpecialFolderLocation( CSIDL_LOCAL_APPDATA );
	SetAppDataPath( ADPT_SpecificUser, MergePath( specificUserFolder, path ) );

	const CString allUsersFolder = getSpecialFolderLocation( CSIDL_COMMON_APPDATA );
	SetAppDataPath( ADPT_AllUsers, MergePath( specificUserFolder, path ) );	
}

int GetDiskFreeSpace( CStringPart path )
{
	auto root = GetRoot( path );
	ForceBackSlashes( root );

	ULARGE_INTEGER free;
	free.QuadPart = 0;

	const auto unicodeRoot = UnicodeStr( root );
	checkLastFileError( ::GetDiskFreeSpaceEx( unicodeRoot.Ptr(), &free, 0, 0 ) != 0, path );

	return free.QuadPart > INT_MAX ? INT_MAX : numeric_cast<int>( free.QuadPart );
}

int GetCurrentDiskFreeSpace()
{
	return GetDiskFreeSpace( GetCurrentDir() );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace FileSystem.

}	// namespace Relib.
