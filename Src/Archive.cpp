#include <Archive.h>
#include <Ptr.h>
#include <BaseString.h>
#include <StrConversions.h>
#include <FileViews.h>

namespace Relib {

extern const CError Err_BadArchiveVersion;

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////
	
CArchive::CArchive( int _bufferSize /*= minArchiveSize */ ) :
	bufferSize( max( _bufferSize, minArchiveSize ) ),
	archiveStartPos( 0 ),
	filePos( 0 ),
	fileLength( 0 ),
	arePosAndLengthActual( false ),
	currentBufferPos( 0 ),
	bufferLeftoverCount( 0 )
{
	buffer.ReserveBuffer( bufferSize );
	buffer.IncreaseSize( bufferSize );
}

void CArchive::flushReading( CFileReadView file )
{
	if( bufferLeftoverCount > 0 ) {
		// Synchronize current buffer position and current file position.
		const __int64 bufferLeftover64 = numeric_cast<__int64>( bufferLeftoverCount );
		file.Seek( -bufferLeftover64, FSP_Current );
		filePos -= bufferLeftover64;
	}
	currentBufferPos = 0;
	bufferLeftoverCount = 0;
}

void CArchive::flushWriting( CFileWriteView file  )
{
	if( currentBufferPos + bufferLeftoverCount > 0 ) {
		// Write the buffer in file.	
		file.Write( buffer.Ptr(), currentBufferPos + bufferLeftoverCount );
		// File length has increased unless we were writing in the middle of a file.
		fileLength = max( fileLength, filePos + currentBufferPos + bufferLeftoverCount );
		if( bufferLeftoverCount != 0 ) {
			// Synchronize file position and buffer position.
			file.Seek( -numeric_cast<__int64>( bufferLeftoverCount ), FSP_Current );
		}
		filePos += currentBufferPos;
	}
	currentBufferPos = 0;
	bufferLeftoverCount = 0;
}

void CArchive::skipReading( CFileReadView file, int byteCount )
{
	assert( byteCount >= 0 );
	if( byteCount == 0 ) {
		return;
	}

	if( byteCount < bufferLeftoverCount ) {
		// We will still be within the buffer after the skip.
		currentBufferPos += byteCount;
		bufferLeftoverCount -= byteCount;
	} else {
		file.Seek( numeric_cast<__int64>( byteCount - bufferLeftoverCount ), FSP_Current );
		filePos += byteCount - bufferLeftoverCount;
		currentBufferPos = 0;
		bufferLeftoverCount = 0;
	}
}

void CArchive::skipWriting( CFileWriteView file, int byteCount )
{
	assert( byteCount >= 0 );
	if( byteCount == 0 ) {
		return;
	}

	if( byteCount < bufferSize - currentBufferPos ) {
		currentBufferPos += byteCount;
		bufferLeftoverCount = max( bufferLeftoverCount - byteCount, 0 );
	} else {
		flushWriting( file );
		file.Seek( numeric_cast<__int64>( byteCount ), FSP_Current );
		filePos += byteCount;
		if( filePos > fileLength ) {
			fileLength = filePos;
			if( !arePosAndLengthActual ) {
				calcActualFilePos( file );
			}
			file.SetLength( fileLength );
		}
	}
}

void CArchive::abort()
{
	objectNamesDictionary.FreeBuffer();
	creationFunctionsBuffer.FreeBuffer();
}

int CArchive::readSmallValue( CFileReadView file )
{
	BYTE first;
	read( file, &first, sizeof( first ) );

	if( first != UCHAR_MAX ) {
		return first;
	} else {
		int result;
		read( file, &result, sizeof( result ) );
		return result;
	}
}

void CArchive::writeSmallValue( CFileWriteView file, int var )
{
	if( var >= 0 && var < UCHAR_MAX ) {
		const BYTE writeResult = numeric_cast<BYTE>( var );
		write( file, &writeResult, sizeof( writeResult ) );
	} else {
		const BYTE writePrefix = numeric_cast<BYTE>( UCHAR_MAX );
		const int writeResult = numeric_cast<int>( var );
		write( file, &writePrefix, sizeof( writePrefix ) );
		write( file, &writeResult, sizeof( writeResult ) );
	}
}

int CArchive::readVersion( CFileReadView file, int currentVersion )
{
	const int version = readSmallValue( file );
	check( version <= currentVersion, Err_BadArchiveVersion, file.GetFileName() );
	return version;
}

int CArchive::writeVersion( CFileWriteView file, int currentVersion )
{
	writeSmallValue( file, currentVersion );
	return currentVersion;
}

void CArchive::readOverBuffer( CFileReadView file, void* ptr, int size )
{
	BYTE* readPtr = reinterpret_cast<BYTE*>( ptr );
	if( bufferLeftoverCount > 0 ) {
		// Buffer is not empty, read from there first.
		memcpy( readPtr, buffer.Ptr() + currentBufferPos, bufferLeftoverCount );
		readPtr += bufferLeftoverCount;
		size -= bufferLeftoverCount;
		bufferLeftoverCount = 0;
	}
	currentBufferPos = 0;
	if( bufferSize <= size ) {
		// Size is bigger than buffer, read directly from file.
		const int readCount = file.Read( readPtr, size );
		if( readCount != size ) {
			ThrowFileException( CFileException::FET_EarlyEnd, file.GetFileName() );
		}
		filePos += readCount;
	} else {
		bufferLeftoverCount = file.Read( buffer.Ptr(), bufferSize );
		if( bufferLeftoverCount < size ) {
			ThrowFileException( CFileException::FET_EarlyEnd, file.GetFileName() );
		}
		filePos += bufferLeftoverCount;
		memcpy( readPtr, buffer.Ptr(), size );
		currentBufferPos += size;
		bufferLeftoverCount -= size;
	}
}

void CArchive::writeOverBuffer( CFileWriteView file, const void* ptr, int size )
{
	const BYTE* writePtr = reinterpret_cast<const BYTE*>( ptr );
	if( currentBufferPos > 0 ) {
		// Buffer is not empty, write as much as it can fit and flush.
		const int writeCount = bufferSize - currentBufferPos;
		memcpy( buffer.Ptr() + currentBufferPos, writePtr, writeCount );
		bufferLeftoverCount = 0;
		currentBufferPos = bufferSize;
		writePtr += writeCount;
		size -= writeCount;
		flushWriting( file );
	}
	if( bufferSize <= size ) {
		// Size is bigger than buffer, write directly to file.
		file.Write( writePtr, size );
		filePos += size;
	} else {
		memcpy( buffer.Ptr(), writePtr, size );
		currentBufferPos = size;
	}
	fileLength = max( fileLength, filePos );
	bufferLeftoverCount = 0;
}

// Archive can be created on the base of a file that is not empty.
// Usually this information is not important, but for some operations an actual length and position in the file are needed.
// In this case we perform these calculations.
void CArchive::calcActualFilePos( CFileWriteView file ) const
{
	assert( !arePosAndLengthActual );
	archiveStartPos = file.GetPosition() - filePos;
	assert( archiveStartPos >= 0 );
	filePos += archiveStartPos;
	fileLength = max( file.GetLength(), archiveStartPos + fileLength );
	arePosAndLengthActual = true;
}

// Write object's external name.
void CArchive::writeExternalName( CFileWriteView file, CUnicodeView name )
{
	const int nextIndex = objectNamesDictionary.Size() + 1;
	int index = objectNamesDictionary.GetOrCreate( name, nextIndex ).Value();
	if( index == nextIndex ) {
		// Encountered a new name, write the name itself.
		writeSmallValue( file, index << 1 );
		static_cast<CArchiveWriter&>( *this ) << Str( name );
	} else {
		// Name was previously written, simply write its index with a marked first bit.
		index <<= 1;
		index |= 1;
		writeSmallValue( file, index );
	}
}

// Read object's external name and get corresponding creation function.
const CBaseObjectCreationFunction* CArchive::readCreationFunctionPtr( int createIndex )
{
	if( createIndex == 0 ) {
		return 0;
	} else if( HasFlag( createIndex, 1 ) ) {
		// The first bit of createIndex is a flag that the class has been written before.
		const int objectIndex = createIndex >> 1;
		check( objectIndex > 0 && objectIndex <= creationFunctionsBuffer.Size(), Err_BadArchive );
		return creationFunctionsBuffer[objectIndex - 1];
	} else {
		// No flag is present, read the whole name and put it in the dictionary.
		CString newAnsiName;
		static_cast<CArchiveReader&>( *this ) >> newAnsiName;
		const CUnicodeString newName = UnicodeStr( newAnsiName );
		const CBaseObjectCreationFunction* newFunction = GetObjectCreationFunction( newName );
		creationFunctionsBuffer.Add( newFunction );
		return newFunction;
	}
}

CSharedPtr<ISerializable> CArchive::readObject( CFileReadView file )
{
	const int createIndex = readSmallValue( file );
	const CBaseObjectCreationFunction* createFunction = readCreationFunctionPtr( createIndex );
	if( createFunction == nullptr ) {
		return nullptr;
	} else {
		auto object = CreateSharedObject<ISerializable>( createFunction );
		object->Serialize( static_cast<CArchiveReader&>( *this ) );
		return object;
	}
}

void CArchive::writeObject( CFileWriteView file, const ISerializable* object )
{
	if( object == nullptr ) {
		writeSmallValue( file, 0 );
	} else {
		const auto objectName = GetExternalName( *object );
		writeExternalName( file, objectName );
		object->Serialize( static_cast<CArchiveWriter&>( *this ) );
	}
}

CPtrOwner<ISerializable> CArchive::readUniqueObject( CFileReadView file )
{
	const int createIndex = readSmallValue( file );
	const CBaseObjectCreationFunction* createFunction = readCreationFunctionPtr( createIndex );
	if( createFunction == 0 ) {
		return CPtrOwner<ISerializable>( nullptr );
	} else {
		CPtrOwner<ISerializable> object = CreateUniqueObject<ISerializable>( createFunction );
		object->Serialize( static_cast<CArchiveReader&>( *this ) );
		return move( object );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

 CArchiveReader::~CArchiveReader()
{
	 flushReading( file );
}

 //////////////////////////////////////////////////////////////////////////

 CArchiveWriter::~CArchiveWriter()
{
	 flushWriting( file );
}

 //////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

