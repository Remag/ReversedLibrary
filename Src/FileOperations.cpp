#include <FileOperations.h>
#include <BaseString.h>
#include <BaseStringView.h>
#include <Errors.h>
#include <FileSystem.h>
#include <FileViews.h>
#include <FileOwners.h>

namespace Relib {

namespace RelibInternal {

// Standard unicode tags. Certain files place one of them at the start of the file to specify its encoding.
extern const char Utf16LEFileTag[2] = { char( 0xFFU ), char( 0xFEU ) };
extern const char Utf16BEFileTag[2] = { char( 0xFEU ), char( 0xFFU ) };
extern const char Utf8FileTag[3] = { char( 0xEFU ), char( 0xBBU ), char( 0xBFU ) };
//////////////////////////////////////////////////////////////////////////

CUnicodeString CFileReadWriteOperations::GetFileName() const
{
	assert( IsOpen() );
	CUnicodeString result;
	int bufferLength = MAX_PATH;
	auto resPtr = result.CreateRawBuffer( bufferLength );
	int length = ::GetFinalPathNameByHandle( fileHandle, resPtr, bufferLength, 0 );
	if( length > bufferLength ) {
		bufferLength = length;
		resPtr.Release( 0 );
		resPtr = result.CreateRawBuffer( bufferLength );
		length = ::GetFinalPathNameByHandle( fileHandle, resPtr, bufferLength, 0 );
	}
	assert( length <= bufferLength );
	resPtr.Release( length );
	return result;
}

void CFileReadWriteOperations::Write( const void* buffer, int bytesCount )
{
	assert( IsOpen() );
	if( bytesCount == 0 ) {
		return;
	}
	assert( buffer != nullptr );
	assert( bytesCount > 0 );

	DWORD bytesWritten;
	if( ::WriteFile( fileHandle, buffer, bytesCount, &bytesWritten, 0 ) == 0 ) {
		throwException();
	}
}

int CFileReadWriteOperations::Read( void* buffer, int maxBytesCount )
{
	assert( IsOpen() );
	if( maxBytesCount == 0 ) {
		return 0;
	}
	assert( buffer != nullptr );
	assert( maxBytesCount > 0 );

	DWORD bytesRead;
	if( ::ReadFile( fileHandle, buffer, maxBytesCount, &bytesRead, 0 ) == 0 ) {
		throwException();
	}
	return numeric_cast<int>( bytesRead );
}

void CFileReadWriteOperations::ReadExact( void* buffer, int bytesCount )
{
	if( Read( buffer, bytesCount ) != bytesCount ) {
		ThrowFileException( CFileException::FET_EarlyEnd, GetFileName() );
	}
}

TFileTextEncoding CFileReadWriteOperations::ReadByteString( CArray<BYTE>& result, int& strStartPos )
{
	assert( result.IsEmpty() );
	const int length = GetLength32();
	result.IncreaseSizeNoInitialize( length + 2 );
	ReadExact( result.Ptr(), length );
	result[length] = 0;
	result[length + 1] = 0;
	return findFileEncoding( result, strStartPos );
}

bool CFileReadWriteOperations::IsOpen() const
{
	return fileHandle != INVALID_HANDLE_VALUE;
}

void CFileReadWriteOperations::open( CUnicodeView fileName, TFileReadWriteMode readWriteMode, TFileCreationMode createMode, TFileShareMode shareMode, DWORD attributes )
{
	assert( !IsOpen() );
	create( fileName, readWriteMode, shareMode, nullptr, createMode, attributes );
}

bool CFileReadWriteOperations::tryOpen( CUnicodeView fileName, TFileReadWriteMode readWriteMode, TFileCreationMode createMode, TFileShareMode shareMode, DWORD attributes )
{
	assert( !IsOpen() );
	return tryCreate( fileName, readWriteMode, shareMode, nullptr, createMode, attributes );
}

HANDLE CFileReadWriteOperations::tryOpenHandle( CUnicodeView fileName, TFileReadWriteMode readWriteMode, TFileCreationMode createMode, TFileShareMode shareMode, DWORD attributes )
{
	return doCreateHandle( fileName, readWriteMode, createMode, nullptr, shareMode, attributes );
}

__int64 CFileReadWriteOperations::GetPosition() const
{
	assert( IsOpen() );

	LARGE_INTEGER offset;
	offset.QuadPart = 0;
	LARGE_INTEGER newPos;
	const auto result = ::SetFilePointerEx( fileHandle, offset, &newPos, FILE_CURRENT );
	if( result == 0 ) {
		throwException();
	}
	return newPos.QuadPart;
}

__int64 CFileReadWriteOperations::Seek( __int64 offset, TFileSeekPosition from )
{
	assert( IsOpen() );

	LARGE_INTEGER largeOffset;
	largeOffset.QuadPart = offset;
	LARGE_INTEGER newPos;
	const auto setResult = ::SetFilePointerEx( fileHandle, largeOffset, &newPos, from );
	if( setResult == 0 ) {
		throwException();
	}
	return newPos.QuadPart;
}

CFileStatus CFileReadWriteOperations::getStatus() const
{
	assert( IsOpen() );
	CFileStatus result;
	result.FullName = GetFileName();
	result.Attributes = FileSystem::GetAttributes( result.FullName );
	result.Length = GetLength();

	::GetFileTime( fileHandle, &result.CreationTime, nullptr, &result.ModificationTime );
	return result;
}

__int64 CFileReadWriteOperations::GetLength() const
{
	assert( IsOpen() );

	ULARGE_INTEGER length;
	length.LowPart = ::GetFileSize( fileHandle, &length.HighPart );
	DWORD err = ::GetLastError();
	if( length.LowPart == INVALID_FILE_SIZE && err != NO_ERROR ) {
		ThrowFileException( err, GetFileName() );
	}
	return length.QuadPart;
}

int CFileReadWriteOperations::GetLength32() const
{
	const __int64 length = GetLength();
	assert( length >= 0 );
	if( length > INT_MAX ) {
		ThrowFileException( CFileException::FET_FileTooBig, GetFileName() );
	}
	return static_cast<int>( length );
}

void CFileReadWriteOperations::SetLength( __int64 newLength )
{
	assert( IsOpen() );
	assert( newLength >= 0 );

	Seek( newLength, FSP_Begin );
	if( ::SetEndOfFile( fileHandle ) == 0 ) {
		throwException();
	}
}

void CFileReadWriteOperations::flush()
{
	assert( IsOpen() );

	if( ::FlushFileBuffers( fileHandle ) == 0 ) {
		throwException();
	}
}

void CFileReadWriteOperations::close()
{
	if( !IsOpen() ) {
		return;
	}
	::CloseHandle( fileHandle );
	fileHandle = INVALID_HANDLE_VALUE;
}

TFileTextEncoding CFileReadWriteOperations::findFileEncoding( CArrayView<BYTE> fileContents, int& bomOffset )
{
	const int size = fileContents.Size();
	const BYTE* ptr = fileContents.Ptr();
	const int Utf16LETagSize = _countof( Utf16LEFileTag );
	if( size >= Utf16LETagSize && ::memcmp( ptr, Utf16LEFileTag, sizeof( Utf16LEFileTag ) ) == 0 ) {
		bomOffset = Utf16LETagSize;
		return FTE_UTF16LittleEndian;
	}
	const int Utf16BETagSize = _countof( Utf16BEFileTag );
	if( size >= Utf16BETagSize && ::memcmp( ptr, Utf16BEFileTag, sizeof( Utf16BEFileTag ) ) == 0 ) {
		bomOffset = Utf16BETagSize;
		return FTE_UTF16BigEndian;
	}
	const int Utf8TagSize = _countof( Utf8FileTag );
	if( size >= Utf8TagSize && ::memcmp( ptr, Utf8FileTag, sizeof( Utf8FileTag ) ) == 0 ) {
		bomOffset = Utf8TagSize;
		return FTE_UTF8;
	}

	bomOffset = 0;
	return FTE_Undefined;
}

void CFileReadWriteOperations::create( CUnicodeView fileName, DWORD accessMode, DWORD shareMode, SECURITY_ATTRIBUTES* security, DWORD createMode, DWORD attributes )
{
	doCreate( fileName, accessMode, shareMode, security, createMode, attributes );
	if( !IsOpen() ) {
		throwException( fileName );
	}
}

bool CFileReadWriteOperations::tryCreate( CUnicodeView fileName, DWORD accessMode, DWORD shareMode, SECURITY_ATTRIBUTES* security, DWORD createMode, DWORD attributes )
{
	doCreate( fileName, accessMode, shareMode, security, createMode, attributes );
	return IsOpen();
}

void CFileReadWriteOperations::doCreate( CUnicodeView fileName, DWORD accessMode, DWORD shareMode, SECURITY_ATTRIBUTES* security, DWORD createMode, DWORD attributes )
{
	assert( !IsOpen() );
	fileHandle = doCreateHandle( fileName, accessMode, shareMode, security, createMode, attributes );
}

HANDLE CFileReadWriteOperations::doCreateHandle( CUnicodeView fileName, DWORD accessMode, DWORD shareMode, SECURITY_ATTRIBUTES* security, DWORD createMode, DWORD attributes )
{
	CUnicodeString fullName = FileSystem::CreateFullPath( fileName );
	return ::CreateFile( fullName.Ptr(), accessMode, shareMode, security, createMode, attributes, 0 );
}

void CFileReadWriteOperations::throwException() const
{
	// Getting the file name overwrites last error, get it beforehand.
	const int errorCode = ::GetLastError();
	const auto fileName = GetFileName();
	ThrowFileException( errorCode, fileName );
}

void CFileReadWriteOperations::throwException( CUnicodePart fileName ) const
{
	ThrowFileException( ::GetLastError(), fileName );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal

namespace File {

//////////////////////////////////////////////////////////////////////////

// Read file contents as an ANSI text.
static CString readAnsi( CFileReadView file, int fileLength )
{
	CString result;
	auto textPtr = result.CreateRawBuffer( fileLength );
	file.ReadExact( textPtr, fileLength );
	textPtr.Release( fileLength );
	return result;
}

CString ReadText( CUnicodeView fileName )
{
	CFileReader file( fileName, FCM_OpenExisting );
	return ReadText( file );
}

CString ReadText( CFileReadView file )
{
	const int length = file.GetLength32();
	return readAnsi( file, length );
}

void WriteText( CUnicodeView fileName, CStringPart text )
{
	CFileWriter file( fileName, FCM_OpenExisting );
	WriteText( file, text );
}

void WriteText( CFileWriteView file, CStringPart text )
{
	file.Write( text.begin(), text.Length() );
}

// Try to read one of the unicodeTags.
static TFileTextEncoding readUnicodeTag( CFileReadView file )
{
	assert( file.IsOpen() );
	BYTE utf16Buffer[_countof( RelibInternal::Utf16LEFileTag )];
	const int utf16ReadCount = file.Read( utf16Buffer, sizeof( utf16Buffer ) );
	if( utf16ReadCount != sizeof( utf16Buffer ) ) {
		// Couldn't read the smaller tag, return an undefined encoding.
		file.SeekToBegin();
		return FTE_Undefined;
	}

	if( ::memcmp( utf16Buffer, RelibInternal::Utf16LEFileTag, sizeof( RelibInternal::Utf16LEFileTag ) ) == 0 ) {
		return FTE_UTF16LittleEndian;
	} else if( ::memcmp( utf16Buffer, RelibInternal::Utf16BEFileTag, sizeof( RelibInternal::Utf16BEFileTag ) ) == 0 ) {
		return FTE_UTF16BigEndian;
	}

	// UTF16 tag wasn't found, try to find a UTF8 tag.
	BYTE utf8Buffer[_countof( RelibInternal::Utf8FileTag )];
	staticAssert( sizeof( utf8Buffer ) > sizeof( utf16Buffer ) );
	::memcpy( utf8Buffer, utf16Buffer, sizeof( utf16Buffer ) );
	const int utf8ReadCount = file.Read( utf8Buffer + sizeof( utf16Buffer ), sizeof( utf8Buffer ) - sizeof( utf16Buffer ) );
	if( utf16ReadCount + utf8ReadCount == sizeof( RelibInternal::Utf8FileTag ) && ::memcmp( utf8Buffer, RelibInternal::Utf8FileTag, sizeof( RelibInternal::Utf8FileTag ) ) == 0 ) {
		return FTE_UTF8;
	}
	// No unicode tag was found, restore the file position and return an undefined encoding.
	file.SeekToBegin();
	return FTE_Undefined;
}

// Read file contents in UTF16 encoding.
static CUnicodeString readUtf16LE( CFileReadView file )
{
	// The first two bytes in the file is the UTF16 tag, they are not counted as a symbol.
	const int stringByteLength = file.GetLength32() - 2;
	const int symbolCount = stringByteLength / sizeof( wchar_t );
	CUnicodeString result;
	auto resultPtr = result.CreateRawBuffer( symbolCount );
	file.ReadExact( resultPtr, symbolCount * sizeof( wchar_t ) );
	resultPtr.Release( symbolCount );
	return result;
}

// Swaps the neighbor bytes in the given buffer.
static void swapBytes( CUnicodeString& str )
{
	staticAssert( sizeof( wchar_t ) == 2 );

	const int length = str.Length();
	const BYTE* castedConstBuffer = reinterpret_cast<const BYTE*>( str.Ptr() );
	BYTE* castedBuffer = const_cast<BYTE*>( castedConstBuffer );
	for( int i = 0; i < length; i++ ) {
		swap( castedBuffer[2 * i], castedBuffer[2 * i + 1] );
	}
}

// Read file contents as an ANSI string and put it in the unicode string in the given encoding.
static CUnicodeString readAndConvertAnsi( CFileReadView file, int fileLength, UINT codePage )
{
	CString ansiResult = readAnsi( file, fileLength );
	return UnicodeStr( ansiResult, codePage );
}

// Read file contents in UTF8 encoding.
static CUnicodeString readUtf8( CFileReadView file )
{
	const int length = file.GetLength32() - sizeof( RelibInternal::Utf8FileTag );
	return readAndConvertAnsi( file, length, CP_UTF8 );
}

CUnicodeString ReadUnicodeText( CUnicodeView fileName, UINT codePage )
{
	CFileReader file( fileName, FCM_OpenExisting );
	return ReadUnicodeText( file, codePage );
}

CUnicodeString ReadUnicodeText( CFileReadView file, UINT codePage )
{
	const auto fileEncoding = readUnicodeTag( file );
	switch( fileEncoding ) {
		case FTE_UTF16BigEndian: {
			CUnicodeString result = readUtf16LE( file );
			swapBytes( result );
			return result;
		}
		case FTE_UTF16LittleEndian:
			return readUtf16LE( file );
		case FTE_UTF8:
			return readUtf8( file );
		case FTE_Undefined: {
			const int length = file.GetLength32();
			return readAndConvertAnsi( file, length, codePage );
		}
		default:
			assert( false );
			return CUnicodeString();
	}
}

// Write the given text string in the file with the UTF16 encoding.
static void writeUtf16LE( CFileWriteView file, CUnicodePart text )
{
	const int length = text.Length();
	file.Write( text.begin(), length * sizeof( wchar_t ) );
}

void WriteUnicodeText( CUnicodeView fileName, CUnicodePart text, TFileTextEncoding encoding )
{
	CFileWriter file( fileName, FCM_OpenExisting );
	WriteUnicodeText( file, text, encoding );
}

void WriteUnicodeText( CFileWriteView file, CUnicodePart text, TFileTextEncoding encoding )
{
	switch( encoding ) {
		case FTE_UTF16BigEndian: {
			file.Write( RelibInternal::Utf16BEFileTag, sizeof( RelibInternal::Utf16BEFileTag ) );
			CUnicodeString textCopy = UnicodeStr( text );
			swapBytes( textCopy );
			writeUtf16LE( file, textCopy );
			break;
		}
		case FTE_UTF16LittleEndian:
			file.Write( RelibInternal::Utf16LEFileTag, sizeof( RelibInternal::Utf16LEFileTag ) );
			writeUtf16LE( file, text );
			break;
		case FTE_UTF8: {
			const CString utf8String = Str( text, CP_UTF8 );
			file.Write( RelibInternal::Utf8FileTag, sizeof( RelibInternal::Utf8FileTag ) );
			file.Write( utf8String.Ptr(), utf8String.Length() );
			break;
		}
		case FTE_Undefined:
		default:
			assert( false );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace File

}	// namespace Relib.
