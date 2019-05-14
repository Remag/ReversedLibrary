#pragma once
#include <Remath.h>
#include <BaseString.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

enum TFileCreationMode : unsigned {
	FCM_OpenExisting = OPEN_EXISTING,
	FCM_CreateOrOpen = OPEN_ALWAYS,
	FCM_CreateAlways = CREATE_ALWAYS
};

enum TFileReadWriteMode : unsigned {
	FRWM_Read = GENERIC_READ,
	FRWM_Write = GENERIC_WRITE,
	FRWM_ReadWrite = GENERIC_READ | GENERIC_WRITE
};

enum TFileShareMode : unsigned {
	FSM_DenyNone = FILE_SHARE_WRITE | FILE_SHARE_READ,
	FSM_DenyWrite = FILE_SHARE_READ,
	FSM_DenyRead = FILE_SHARE_WRITE,
	FMS_Exclusive = 0
};

// Text encoding used in files.
enum TFileTextEncoding : unsigned {
	FTE_Undefined,
	FTE_UTF16LittleEndian,
	FTE_UTF16BigEndian,
	FTE_UTF8
};

// Start point for Seek method.
enum TFileSeekPosition : unsigned {
	FSP_Begin = FILE_BEGIN,
	FSP_Current = FILE_CURRENT,
	FSP_End = FILE_END
};

// Detailed file information.
struct CFileStatus {
	__int64 Length = 0;
	CUnicodeString FullName;
	FILETIME CreationTime;
	FILETIME ModificationTime;
	DWORD Attributes = 0;

	CFileStatus() = default;
	CFileStatus( const CFileStatus& other, const CExplicitCopyTag& )
		: Length( other.Length ), FullName( copy( other.FullName ) ), CreationTime( other.CreationTime ), ModificationTime( other.ModificationTime ), Attributes( other.Attributes )
	{
	}
};

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

class REAPI CFileReadWriteOperations {
public:
	CFileReadWriteOperations() = default;
	explicit CFileReadWriteOperations( HANDLE _fileHandle ) : fileHandle( _fileHandle ) {}

	// Get the full path to the opened file.
	CUnicodeString GetFileName() const;

	bool IsOpen() const;
	CFileStatus GetStatus() const;

	// Low level read-write methods.

	// Write contents of the buffer to the file.
	void Write( const void* buffer, int bytesCount );

	// Read contents of the file. Returns the number of bytes read.
	int Read( void* buffer, int maxBytesCount );
	// Safe read function. Throws a file exception if it is impossible to read the specified number of bytes.
	void ReadExact( void* buffer, int bytesCount );
	// Read a potential byte order mark and the rest of the file as a sequence of bytes.
	// File position is asserted to be at the start of the file.
	// Result contains a potential BOM, the actual file content starts at strStartPos;
	TFileTextEncoding ReadByteString( CArray<BYTE>& result, int& strStartPos );

	__int64 GetLength() const;
	int GetLength32() const;
	void SetLength( __int64 newLength );

	// Check if the EOF is reached.
	bool IsEndOfFile() const
		{ return GetPosition() == GetLength(); }
	
	// Get current position in the file.
	__int64 GetPosition() const;
	// Set the position.
	__int64 Seek( __int64 offset, TFileSeekPosition from );
	__int64 SeekToBegin()
		{ return Seek( 0, FSP_Begin ); }
	__int64 SeekToEnd()
		{ return Seek( 0, FSP_End ); }

protected:
	HANDLE getFileHandle() const
		{ return fileHandle; }
	void detachFileHandle()
		{ fileHandle = INVALID_HANDLE_VALUE; }
	void swapFileHandles( CFileReadWriteOperations& other )
		{ swap( fileHandle, other.fileHandle ); }

	// Open method that throws an exception on failure.
	void open( CUnicodeView fileName, TFileReadWriteMode readWriteMode, TFileCreationMode createMode, TFileShareMode shareMode, DWORD attributes );
	// Open method that returns false on failure.
	bool tryOpen( CUnicodeView fileName, TFileReadWriteMode readWriteMode, TFileCreationMode createMode, TFileShareMode shareMode, DWORD attributes );
	// Open method that returns an invalid handle on failure.
	static HANDLE tryOpenHandle( CUnicodeView fileName, TFileReadWriteMode readWriteMode, TFileCreationMode createMode, TFileShareMode shareMode, DWORD attributes );

	// Save pending changes.
	void flush();
	// Close the handle.
	void close();

	static TFileTextEncoding findFileEncoding( CArrayView<BYTE> fileContents, int& bomOffset );

private:
	HANDLE fileHandle = INVALID_HANDLE_VALUE;

	void create( CUnicodeView fileName, DWORD accessMode, DWORD shareMode, SECURITY_ATTRIBUTES* security, DWORD createMode, DWORD attributes );
	bool tryCreate( CUnicodeView fileName, DWORD accessMode, DWORD shareMode, SECURITY_ATTRIBUTES* security, DWORD createMode, DWORD attributes );
	void doCreate( CUnicodeView fileName, DWORD accessMode, DWORD shareMode, SECURITY_ATTRIBUTES* security, DWORD createMode, DWORD attributes );
	static HANDLE doCreateHandle( CUnicodeView fileName, DWORD accessMode, DWORD shareMode, SECURITY_ATTRIBUTES* security, DWORD createMode, DWORD attributes );

	void throwException() const;
	void throwException( CUnicodePart fileName ) const;
};

//////////////////////////////////////////////////////////////////////////

// Generic read operations.
// File owning handle will need access to an extended operation set from CReadWriteOperations so protected inheritance is used.
class CFileReadOperations : protected CFileReadWriteOperations {
public:
	using CFileReadWriteOperations::CFileReadWriteOperations;

	using CFileReadWriteOperations::GetFileName;
	using CFileReadWriteOperations::IsEndOfFile;
	using CFileReadWriteOperations::IsOpen;
	using CFileReadWriteOperations::GetStatus;
	using CFileReadWriteOperations::GetLength;
	using CFileReadWriteOperations::GetLength32;
	using CFileReadWriteOperations::GetPosition;
	using CFileReadWriteOperations::Seek;
	using CFileReadWriteOperations::SeekToBegin;
	using CFileReadWriteOperations::SeekToEnd;
	using CFileReadWriteOperations::Read;
	using CFileReadWriteOperations::ReadExact;
	using CFileReadWriteOperations::ReadByteString;
};

// Generic write operations.
// File owning handle will need access to an extended operation set from CReadWriteOperations so protected inheritance is used.
class CFileWriteOperations : protected CFileReadWriteOperations {
public:
	using CFileReadWriteOperations::CFileReadWriteOperations;

	using CFileReadWriteOperations::GetFileName;
	using CFileReadWriteOperations::IsEndOfFile;
	using CFileReadWriteOperations::IsOpen;
	using CFileReadWriteOperations::GetStatus;
	using CFileReadWriteOperations::GetLength;
	using CFileReadWriteOperations::GetLength32;
	using CFileReadWriteOperations::GetPosition;
	using CFileReadWriteOperations::Seek;
	using CFileReadWriteOperations::SeekToBegin;
	using CFileReadWriteOperations::SeekToEnd;
	using CFileReadWriteOperations::Write;
	using CFileReadWriteOperations::SetLength;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal

namespace File {

//////////////////////////////////////////////////////////////////////////

// ANSI read-write methods.
// Put the contents of the given file in the result string. Newline symbols are intact.
CString REAPI ReadText( CUnicodeView fileName );
CString REAPI ReadText( CFileReadView file );
// Write text from the given string.
void REAPI WriteText( CUnicodeView fileName, CStringPart text );
void REAPI WriteText( CFileWriteView file, CStringPart text );

// Unicode read-write methods.
// Put the contents of the given file in the result string. Non unicode files are converted to unicode with the given codePage. Newline symbols are intact.
CUnicodeString REAPI ReadUnicodeText( CUnicodeView fileName, UINT codePage = CP_ACP );
CUnicodeString REAPI ReadUnicodeText( CFileReadView file, UINT codePage = CP_ACP );
// Write text in the unicode encoding.
void REAPI WriteUnicodeText( CUnicodeView fileName, CUnicodePart text, TFileTextEncoding encoding = FTE_UTF16LittleEndian );
void REAPI WriteUnicodeText( CFileWriteView file, CUnicodePart text, TFileTextEncoding encoding = FTE_UTF16LittleEndian );

//////////////////////////////////////////////////////////////////////////

}	// namespace File.

}	// namespace Relib.

