#include <TempFile.h>
#include <Array.h>
#include <BaseString.h>
#include <FileSystem.h>
#include <Errors.h>
#include <CriticalSection.h>
#include <RandomGenerator.h>
#include <Reutils.h>
#include <StrConversions.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

extern const CStringView TempFilePrefix;
extern const CStringView TempFileExt;

extern CCriticalSection TempFileLock;
extern CArray<CString, CProcessHeap> TempFileNames;
extern CRandomGenerator TempFileSuffixGenerator;
extern const CError Err_CreateTempFile;

//////////////////////////////////////////////////////////////////////////

CTempFile::CTempFile() :
	name( createNewTempFile() )
{
}

CTempFile::CTempFile( CStringPart folder )	:
	name( createNewTempFile( folder ) )
{
}

CString CTempFile::createNewTempFile()
{
	return createNewTempFile( getTempDir() );
}

CString CTempFile::createNewTempFile( CStringPart dirName )
{
	auto newName = openUniqueFile( dirName );
	CCriticalSectionLock lock( TempFileLock );
	TempFileNames.Add( copy( newName ) );
	return newName;
}

CString CTempFile::openUniqueFile( CStringPart dir )
{
	check( FileSystem::DirAccessible( dir ), Err_CreateTempFile, dir );
	auto suffix = TempFileSuffixGenerator.RandomNumber( 0, 0xFFFF );
	auto tempName = FileSystem::MergeName( dir, TempFilePrefix + Str( suffix, 16 ), TempFileExt );

	for( ;; ) {
		try {
			openTempFile( tempName );
			return tempName;
		} catch( const CFileException& e ) {
			filterException( e );
			suffix++;
			tempName = FileSystem::MergeName( dir, TempFilePrefix + Str( suffix, 16 ), TempFileExt );
		}
	}
	assert( false );
	return CString();
}

void CTempFile::openTempFile( CStringPart fileName )
{
	open( fileName, FRWM_ReadWrite, FCM_CreateAlways, FSM_DenyNone, FILE_ATTRIBUTE_TEMPORARY );
}

// Exceptions like FileExist need to be handled internally. Others need to be rethrown.
// Filter the incoming exception and rethrow if necessary.
void CTempFile::filterException( const CFileException& e )
{
	const auto eType = e.Type();
	auto fileDir = FileSystem::GetDrivePath( e.FileName() );
	// Recreate an exception with the temp folder name.
	switch( eType ) {
		case CFileException::FET_DiskFull:
		case CFileException::FET_HardwareError:
			throw CFileException( eType, move( fileDir ) );
		default:
			break;
	}
}

CString CTempFile::getTempDir()
{
	CString result;
	try {
		result = FileSystem::GetWindowsTempDir();
	} catch( const CException& ) {
	}
	if( !FileSystem::DirAccessible( result ) ) {
		result = FileSystem::GetWindowsDir();
	}
	return result;
}

CTempFile::CTempFile( CTempFile&& other ) :
	CFileReadWriteOperations( move( other ) ),
	name( move( other.name ) )
{
}

CTempFile& CTempFile::operator=( CTempFile&& other )
{
	swapFileHandles( other );
	swap( name, other.name );
	return *this;
}

CTempFile::~CTempFile()
{
	Delete();
}

void CTempFile::Delete()
{
	close();
	if( !name.IsEmpty() ) {
		deleteTempFile();
		name.Empty();
	}
}

void CTempFile::deleteTempFile()
{
	CCriticalSectionLock lock( TempFileLock );
	const int filePos = findTempFile( name );
	assert( filePos != NotFound );

	try {
		FileSystem::Delete( TempFileNames[filePos] );
		TempFileNames.DeleteAt( filePos );
	} catch( const CException& ) {
		const CUnicodeView tempFileTemplate = L"Warning: Temporary file cannot be deleted. File name: %0\n";
		auto outputStr = tempFileTemplate.SubstParam( TempFileNames[filePos] );
		::OutputDebugString( outputStr.Ptr() );
	}
}

void CTempFile::MakePermanent( CStringPart permanentName )
{
	close();
	const auto fullName = FileSystem::CreateFullPath( name );
	const auto fullDest = FileSystem::CreateFullPath( permanentName );

	CCriticalSectionLock lock( TempFileLock );
	const int pos = findTempFile( fullName );
	assert( pos != NotFound );

	if( !FileSystem::NamesEqual( fullName, fullDest ) ) {
		FileSystem::Move( fullName, fullDest );
	}
	TempFileNames.DeleteAt( pos );
	name.Empty();
}

int CTempFile::findTempFile( CStringPart name )
{
	for( int i = 0; i < TempFileNames.Size(); i++ ) {
		if( FileSystem::NamesEqual( name, TempFileNames[i] ) ) {
			return i;
		}
	}
	return NotFound;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.