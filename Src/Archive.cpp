#include <Archive.h>
#include <Ptr.h>
#include <BaseString.h>
#include <StrConversions.h>
#include <FileViews.h>
#include <FileOwners.h>
#include <MessageLog.h>
#include <MessageUtils.h>
#include <ZipConverter.h>

namespace Relib {

extern const CError Err_BadArchiveVersion;

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////
	
CArchive::CArchive() :
	currentBufferPos( 0 )
{
}

void CArchive::increaseBuffer( int bufferSize )
{
	buffer.IncreaseSizeNoInitialize( bufferSize );
}

void CArchive::attachBuffer( CArray<BYTE> newBuffer )
{
	currentBufferPos = 0;
	buffer = move( newBuffer );
}

CArray<BYTE> CArchive::detachBuffer()
{
	currentBufferPos = 0;
	return move( buffer );
}

void CArchive::skip( int byteCount )
{
	assert( byteCount >= 0 );
	currentBufferPos += byteCount;
}

int CArchive::readSmallValue()
{
	BYTE first;
	read( &first, sizeof( first ) );

	if( first != UCHAR_MAX ) {
		return first;
	} else {
		int result;
		read( &result, sizeof( result ) );
		return result;
	}
}

void CArchive::writeSmallValue( int var )
{
	if( var >= 0 && var < UCHAR_MAX ) {
		const auto writeResult = numeric_cast<BYTE>( var );
		write( &writeResult, sizeof( writeResult ) );
	} else {
		const auto writePrefix = numeric_cast<BYTE>( UCHAR_MAX );
		const auto writeResult = numeric_cast<int>( var );
		write( &writePrefix, sizeof( writePrefix ) );
		write( &writeResult, sizeof( writeResult ) );
	}
}

int CArchive::readVersion( int currentVersion )
{
	const auto version = readSmallValue();
	check( version <= currentVersion, Err_BadArchiveVersion );
	return version;
}

int CArchive::writeVersion( int currentVersion )
{
	writeSmallValue( currentVersion );
	return currentVersion;
}

// Write object's external name.
void CArchive::writeExternalName( CUnicodeView name )
{
	const auto nextIndex = objectNamesDictionary.Size() + 1;
	const auto index = objectNamesDictionary.GetOrCreate( name, nextIndex ).Value();
	if( index == nextIndex ) {
		// Encountered a new name, write the name itself.
		writeSmallValue( index << 1 );
		static_cast<CArchiveWriter&>( *this ) << Str( name );
	} else {
		// Name was previously written, simply write its index with a marked first bit.
		writeSmallValue( ( index << 1 ) | 1 );
	}
}

// Read object's external name and get corresponding creation function.
const CBaseObjectCreationFunction* CArchive::readCreationFunctionPtr( int createIndex )
{
	if( createIndex == 0 ) {
		return 0;
	} else if( HasFlag( createIndex, 1 ) ) {
		// The first bit of createIndex is a flag that the class has been written before.
		const auto objectIndex = createIndex >> 1;
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

CSharedPtr<ISerializable> CArchive::readObject()
{
	const int createIndex = readSmallValue();
	const CBaseObjectCreationFunction* createFunction = readCreationFunctionPtr( createIndex );
	if( createFunction == nullptr ) {
		return nullptr;
	} else {
		auto object = CreateSharedObject<ISerializable>( createFunction );
		object->Serialize( static_cast<CArchiveReader&>( *this ) );
		return object;
	}
}

void CArchive::writeObject( const ISerializable* object )
{
	if( object == nullptr ) {
		writeSmallValue( 0 );
	} else {
		const auto objectName = GetExternalName( *object );
		writeExternalName( objectName );
		object->Serialize( static_cast<CArchiveWriter&>( *this ) );
	}
}

CPtrOwner<ISerializable> CArchive::readUniqueObject( CFileReadView file )
{
	const int createIndex = readSmallValue();
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

static const BYTE fileArchivePrefix = 0xFA;
static const BYTE binaryArchivePrefix = 0xBA;
static const BYTE compressedArchivePrefix = 0xCA;
CArchiveReader::CArchiveReader( CUnicodeView fileName ) :
	CArchiveReader( CFileReader( fileName, FCM_OpenExisting ) )
{
}

CArchiveReader::CArchiveReader( CFileReadView file )
{
	const auto length = file.GetLength32();
	auto& buffer = getBuffer();
	buffer.IncreaseSizeNoInitialize( length );
	file.Read( buffer.Ptr(), length );

	handleArchiveFlags();
}

CArchiveReader::CArchiveReader( CArray<BYTE> fileData )
{
	attachBuffer( move( fileData ) );
	handleArchiveFlags();
}

void CArchiveReader::handleArchiveFlags()
{
	BYTE archiveFlag;
	( *this ) >> archiveFlag;
	if( archiveFlag == compressedArchivePrefix ) {
		CZipConverter zipper;
		CArray<BYTE> unzippedData;
		const auto zippedData = getBuffer().Mid( sizeof( archiveFlag ) );
		zipper.UnzipData( zippedData, unzippedData );
		attachBuffer( move( unzippedData ) );
	} else {
		check( archiveFlag == fileArchivePrefix || archiveFlag == binaryArchivePrefix, Err_BadArchive );
	}
}

//////////////////////////////////////////////////////////////////////////

CArchiveWriter::CArchiveWriter( int bufferSize )
{
	const auto flagByteSize = sizeof( fileArchivePrefix );
	increaseBuffer( bufferSize + flagByteSize );
	skip( flagByteSize );
}

extern const CUnicodeView UncommitedArchiveError;
CArchiveWriter::~CArchiveWriter()
{
	 if( getBufferSize() > 0 ) {
		 Log::Error( UncommitedArchiveError, this );
	 }
}

 void CArchiveWriter::FlushToFile( CUnicodeView fileName )
 {
	 CFileWriter file( fileName, FCM_CreateAlways );
	 auto buffer = detachBuffer();
	 writeArchiveFlag( fileArchivePrefix, buffer );
	 file.Write( buffer.Ptr(), buffer.Size() );
 }

 void CArchiveWriter::FlushToCompressedFile( CUnicodeView fileName )
 {
	 const auto flagSize = sizeof( compressedArchivePrefix );
	 CFileWriter file( fileName, FCM_CreateAlways );
	 CZipConverter zipper;
	 auto buffer = detachBuffer();
	 CArray<BYTE> zippedBuffer;
	 zippedBuffer.IncreaseSizeNoInitialize( flagSize );
	 writeArchiveFlag( compressedArchivePrefix, zippedBuffer );
	 zipper.ZipData( buffer.Mid( flagSize ), zippedBuffer );
	 file.Write( zippedBuffer.Ptr(), zippedBuffer.Size() );
 }

 CArray<BYTE> CArchiveWriter::FlushToByteString()
 {
	 auto buffer = detachBuffer();
	 writeArchiveFlag( binaryArchivePrefix, buffer );
	 return buffer;
 }

 void CArchiveWriter::writeArchiveFlag( BYTE flagValue, CArray<BYTE>& dest ) const
 {
	 ::memcpy( dest.Ptr(), &flagValue, sizeof( flagValue ) );
 }

 //////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

