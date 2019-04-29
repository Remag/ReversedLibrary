#include <ZipConverter.h>
#include <LibPng\zlib.h>

#include <ArrayBuffer.h>
#include <Array.h>
#include <Errors.h>
#include <StaticAllocators.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

const int minWriteChunk = 64 * 1024;
const int maxWriteChunk = 64 * 1024 * 1024;
extern const CError Err_ZlibInitError;
void CZipConverter::ZipData( CArrayView<BYTE> data, CArray<BYTE>& result )
{
	z_stream zStream;
	memset( &zStream, 0, sizeof( zStream ) );
	zStream.zalloc = allocationFunction;
	zStream.zfree = freeFunction;

	const auto initResult = deflateInit( &zStream, Z_DEFAULT_COMPRESSION );
	check( initResult == Z_OK, Err_ZlibInitError, initResult );

	zStream.avail_in = data.Size();
	zStream.next_in = const_cast<BYTE*>( data.Ptr() );
	const int writeChunk = Clamp( data.Size() / 4, minWriteChunk, maxWriteChunk );

	for( ;; ) {
		const int writePos = result.Size();
		result.IncreaseSize( result.Size() + writeChunk );
		zStream.avail_out = writeChunk;
		zStream.next_out = result.Ptr() + writePos;
		const auto deflateResult = deflate( &zStream, Z_FINISH );
		assert( deflateResult != Z_STREAM_ERROR );
		if( deflateResult == Z_STREAM_END ) {
			break;
		}
	}
	deflateEnd( &zStream );
}

const int minReadChunk = 64 * 1024;
const int maxReadChunk = 64 * 1024 * 1024;
extern const CError Err_ZlibInflateError;
void CZipConverter::UnzipData( CArrayView<BYTE> data, CArray<BYTE>& result )
{
	z_stream zStream;
	memset( &zStream, 0, sizeof( zStream ) );
	zStream.zalloc = allocationFunction;
	zStream.zfree = freeFunction;
	const auto initResult = inflateInit( &zStream );
	check( initResult == Z_OK, Err_ZlibInitError, initResult );

	zStream.avail_in = data.Size();
	zStream.next_in = const_cast<BYTE*>( data.Ptr() );
	const int readChunk = Clamp( data.Size(), minReadChunk, maxReadChunk );

		for( ;; ) {
		const int readPos = result.Size();
		result.IncreaseSize( result.Size() + readChunk );
		zStream.avail_out = readChunk;
		zStream.next_out = result.Ptr() + readPos;
		const auto inflateResult = inflate( &zStream, Z_NO_FLUSH );
		if( inflateResult == Z_STREAM_END ) {
			break;
		}
		if( inflateResult == Z_NEED_DICT || inflateResult == Z_DATA_ERROR || inflateResult == Z_MEM_ERROR ) {
			inflateEnd( &zStream );
			check( false, Err_ZlibInflateError, inflateResult );
		}
	}
	inflateEnd( &zStream );
}

void* CZipConverter::allocationFunction( void*, size_t itemCount, size_t itemSize )
{
	const auto byteCount = itemSize * itemCount;
	return CRuntimeHeap::Allocate( byteCount );
}

void CZipConverter::freeFunction( void*, void* ptr )
{
	CRuntimeHeap::Free( ptr );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
