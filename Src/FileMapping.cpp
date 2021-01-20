#include <FileMapping.h>
#include <Errors.h>
#include <StrConversions.h>
#include <MessageLog.h>

namespace Relib {
	
//////////////////////////////////////////////////////////////////////////

CMappingReadView::CMappingReadView( CMappingReadView&& other ) :
	buffer( other.buffer ),
	bufferSize( other.bufferSize ),
	allocationOffset( other.allocationOffset )
{
	other.buffer = nullptr;
	other.bufferSize = 0;
	other.allocationOffset = 0;
}

CMappingReadView& CMappingReadView::operator=( CMappingReadView&& other )
{
	swap( buffer, other.buffer );
	swap( bufferSize, other.bufferSize );
	swap( allocationOffset, other.allocationOffset );
	return *this;
}

CMappingReadView::CMappingReadView( const BYTE* _buffer, int size, int _allocationOffset ) :
	buffer( _buffer ),
	bufferSize( size ),
	allocationOffset( _allocationOffset )
{
}

CMappingReadView::~CMappingReadView()
{
	Close();
}

void CMappingReadView::Flush()
{
	const void* allocatedBuffer = buffer - allocationOffset;
	const bool result = ::FlushViewOfFile( allocatedBuffer, 0 ) != 0;
	checkLastError( result );
}

void CMappingReadView::Close()
{
	if( buffer != 0 ) {
		const void* allocatedBuffer = buffer - allocationOffset;
		const bool result = ::UnmapViewOfFile( allocatedBuffer ) != 0;
		checkLastError( result );
		buffer = 0;
	}
}

//////////////////////////////////////////////////////////////////////////

CMappingReadWriteView::CMappingReadWriteView( CMappingReadWriteView&& other ) :
	buffer( other.buffer ),
	bufferSize( other.bufferSize ),
	allocationOffset( other.allocationOffset )
{
	other.buffer = nullptr;
	other.bufferSize = 0;
	other.allocationOffset = 0;
}

CMappingReadWriteView& CMappingReadWriteView::operator=( CMappingReadWriteView&& other )
{
	swap( buffer, other.buffer );
	swap( bufferSize, other.bufferSize );
	swap( allocationOffset, other.allocationOffset );
	return *this;
}

CMappingReadWriteView::CMappingReadWriteView( BYTE* _buffer, int size, int _allocationOffset ) :
	buffer( _buffer ),
	bufferSize( size ),
	allocationOffset( _allocationOffset )
{
}

CMappingReadWriteView::~CMappingReadWriteView()
{
	Close();
}

void CMappingReadWriteView::Flush()
{
	const void* allocatedBuffer = buffer - allocationOffset;
	const bool result = ::FlushViewOfFile( allocatedBuffer, 0 ) != 0;
	checkLastError( result );
}

void CMappingReadWriteView::Close()
{
	if( buffer != 0 ) {
		const void* allocatedBuffer = buffer - allocationOffset;
		const bool result = ::UnmapViewOfFile( allocatedBuffer ) != 0;
		checkLastError( result );
		buffer = 0;
	}
}

//////////////////////////////////////////////////////////////////////////

CFileMapping::CFileMapping() :
	mappingHandle( 0 )
{
}

CFileMapping::CFileMapping( CUnicodeView fileName, TMappingMode _mode )
{
	Open( fileName, _mode );
}

CFileMapping::CFileMapping( CUnicodeView fileName, TMappingMode _mode, CUnicodeView mappingName )
{
	Open( fileName, _mode, mappingName );
}

CFileMapping::CFileMapping( CUnicodeView fileName, __int64 fileLength )
{
	Open( fileName, fileLength );
}

CFileMapping::CFileMapping( CUnicodeView fileName, __int64 fileLength, CUnicodeView mappingName )
{
	Open( fileName, fileLength, mappingName);
}

CFileMapping::~CFileMapping()
{
	try {
		Close();
	} catch( const CException& e ) {
		Log::Exception( e );
	}
}

void CFileMapping::Open( CUnicodeView fileName, TMappingMode _mode )
{
	doOpenMapping( fileName, _mode, nullptr );
}

void CFileMapping::Open( CUnicodeView fileName, TMappingMode _mode, CUnicodeView mappingName )
{
	doOpenMapping( fileName, _mode, mappingName.Ptr() );
}

void CFileMapping::Open( CUnicodeView fileName, __int64 fileLength )
{
	doOpenMapping( fileName, fileLength, nullptr );
}

void CFileMapping::Open( CUnicodeView fileName, __int64 fileLength, CUnicodeView mappingName )
{
	doOpenMapping( fileName, fileLength, mappingName.Ptr() );
}

void CFileMapping::doOpenMapping( CUnicodeView fileName, TMappingMode _mode, const wchar_t* mappingNamePtr )
{
	assert( !IsOpen() );
	mode = _mode;
	const auto readMode = ( mode == MM_ReadOnly ) ? FRWM_Read : FRWM_ReadWrite;
	file.Open( fileName, readMode, FCM_OpenExisting, FSM_DenyNone );
	openMapping( 0, mappingNamePtr );
}

void CFileMapping::doOpenMapping( CUnicodeView fileName, __int64 fileLength, const wchar_t* mappingNamePtr )
{
	assert( !IsOpen() );
	assert( fileLength >= 0 );
	mode = MM_ReadWrite;
	file.Open( fileName, FRWM_ReadWrite, FCM_CreateOrOpen, FSM_DenyNone );
	openMapping( fileLength, mappingNamePtr );
}

bool CFileMapping::OpenExternal( CUnicodeView mappingName, TMappingMode _mode )
{
	assert( !IsOpen() );
	mode = _mode;
	const auto externalOpenFlags = mode == MM_ReadOnly ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE;
	mappingHandle = ::OpenFileMapping( externalOpenFlags, FALSE, mappingName.Ptr() );
	return mappingHandle != nullptr;
}

CMappingReadView CFileMapping::CreateReadView()
{
	assert( IsOpen() );

	BYTE* buffer;
	int allocationOffset;
	openView( FILE_MAP_READ, 0, 0, buffer, allocationOffset );
	return CMappingReadView( buffer, NotFound, allocationOffset );
}

CMappingReadWriteView CFileMapping::CreateReadWriteView()
{
	assert( IsOpen() );
	assert( mode == MM_ReadWrite );

	BYTE* buffer;
	int allocationOffset;
	openView( FILE_MAP_WRITE, 0, 0, buffer, allocationOffset );
	return CMappingReadWriteView( buffer, NotFound, allocationOffset );
}

CMappingReadView CFileMapping::CreateReadView( __int64 offset, int length )
{
	assert( IsOpen() );
	assert( offset >= 0 && length >= 0 );

	BYTE* buffer;
	int allocationOffset;
	openView( FILE_MAP_READ, offset, length, buffer, allocationOffset );
	return CMappingReadView( buffer, length, allocationOffset );
}

CMappingReadWriteView CFileMapping::CreateReadWriteView( __int64 offset, int length )
{
	assert( IsOpen() );
	assert( offset >= 0 && length >= 0 );
	assert( mode == MM_ReadWrite );

	BYTE* buffer;
	int allocationOffset;
	openView( FILE_MAP_WRITE, offset, length, buffer, allocationOffset );
	return CMappingReadWriteView( buffer, length, allocationOffset );
}

void CFileMapping::Close()
{
	if( mappingHandle != nullptr ) {
		checkLastError( ::CloseHandle( mappingHandle ) != 0 );
		mappingHandle = nullptr;
	}
	file.Close();
}

void CFileMapping::openMapping( __int64 minLength, const wchar_t* mappingNamePtr )
{
	assert( file.IsOpen() );
	const int lowWord = numeric_cast<int>( minLength & 0xFFFFFFFF );
	const int highWord = numeric_cast<int>( ( minLength & 0xFFFFFFFF00000000 ) >> 32 );
	mappingHandle = ::CreateFileMapping( file.Handle(), 0, mode, highWord, lowWord, mappingNamePtr );
	checkLastError( mappingHandle != nullptr );
}

void CFileMapping::openView( DWORD viewMode, __int64 offset, int length, BYTE*& result, int& allocationOffset )
{
	// Offset must be aligned to allocation granularity.
	const int granularity = getAllocationGranularity();
	const __int64 alignedOffset = ( offset / granularity ) * granularity;
	allocationOffset = numeric_cast<int>( offset - alignedOffset );
	const int alignedLength = ( length == 0 ) ? 0 : length + allocationOffset;
	
	const int lowWord = numeric_cast<int>( alignedOffset & 0xFFFFFFFF );
	const int highWord = numeric_cast<int>( ( alignedOffset & 0xFFFFFFFF00000000 ) >> 32 );
	void* alignedResult = ::MapViewOfFile( mappingHandle, viewMode, highWord, lowWord, alignedLength );
	checkLastError( alignedResult != 0 );
	result = reinterpret_cast<BYTE*>( alignedResult ) + allocationOffset;
}

int CFileMapping::getAllocationGranularity()
{
	static int allocationGranularity = 0;
	if( allocationGranularity == 0 ) {
		SYSTEM_INFO systemInfo;
		::GetSystemInfo( &systemInfo );
		allocationGranularity = systemInfo.dwAllocationGranularity;
		assert( allocationGranularity != 0 );
	}
	return allocationGranularity;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

