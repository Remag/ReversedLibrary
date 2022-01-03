#ifndef RELIB_NO_JPEG

#include <JpgFile.h>
#include <FileOwners.h>

#include <LibJpeg/jpeglib.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CJpgFile::CJpgFile( CUnicodePart _fileName ) :
	fileName( _fileName )
{
}

void CJpgFile::Read( CStaticImageData& result ) const
{
	assert( result.Colors.IsEmpty() );

	CFileReader file( fileName, FCM_OpenExisting );
	CArray<BYTE> fileData;
	const int fileLength = file.GetLength32();
	fileData.IncreaseSize( fileLength );
	file.Read( fileData.Ptr(), fileLength );

	doReadRawData( fileName, fileData, result.Colors, result.ImageSize );
}

void CJpgFile::ReadRawData( CArrayView<BYTE> jpgData, CStaticImageData& result )
{
	doReadRawData( CUnicodePart(), jpgData, result.Colors, result.ImageSize ); 
}

// Internal exception to bypass default JPEG error handling.
class CJpgInternalException {};
static void handleDecompressionExit( j_common_ptr )
{
	throw CJpgInternalException();
}

void CJpgFile::doReadRawData( CUnicodePart fileName, CArrayView<BYTE> jpgData, CArray<CColor>& result, CVector2<int>& resultSize )
{
	jpeg_decompress_struct jpegInfo;
	jpeg_error_mgr defaultErrorHandler;
	jpegInfo.err = jpeg_std_error( &defaultErrorHandler );
	jpegInfo.err->error_exit = handleDecompressionExit;

	try {
		jpeg_create_decompress( &jpegInfo );
		// Function requires a non-const pointer for some reason.
		const auto nonConstJpgData = const_cast<BYTE*>( jpgData.Ptr() );
		jpeg_mem_src( &jpegInfo, nonConstJpgData, jpgData.Size() );
		jpeg_read_header( &jpegInfo, TRUE );
		jpegInfo.out_color_space = JCS_EXT_RGBA;
		jpeg_start_decompress( &jpegInfo );
		resultSize.X() = jpegInfo.output_width;
		resultSize.Y() = jpegInfo.output_height;
		const auto totalSize = jpegInfo.output_width * jpegInfo.output_height;
		result.IncreaseSize( totalSize );
		const auto rowStride = jpegInfo.output_width * jpegInfo.output_components;
		// Main struct requires to be converted for the allocation routine.
		const auto commonInfo = ( jpeg_common_struct* )( &jpegInfo );
		const auto buffer = ( *jpegInfo.mem->alloc_sarray )( commonInfo, JPOOL_IMAGE, rowStride, 1 );
		auto rawResult = static_cast<CArrayBuffer<BYTE>>( result.Buffer() );

		const auto totalByteSize = sizeof( CColor ) * totalSize;
		while( jpegInfo.output_scanline < jpegInfo.output_height ) {
			// Write rows bottom-to-top.
			jpeg_read_scanlines( &jpegInfo, buffer, 1 );
			const auto destPos = totalByteSize - jpegInfo.output_scanline * rowStride;
			::memcpy( rawResult.Ptr() + destPos, *buffer, rowStride );
		}
		jpeg_finish_decompress( &jpegInfo );
		jpeg_destroy_decompress( &jpegInfo );

	} catch( CJpgInternalException& ) {
		const auto errCode = jpegInfo.err->msg_code;
		const auto errMessage = jpegInfo.err->jpeg_message_table[errCode];
		jpeg_destroy_decompress( &jpegInfo );
		throw CJpgException( fileName, UnicodeStr( errMessage ) );
	}
}

//////////////////////////////////////////////////////////////////////////

} // namespace Relib.

#endif // RELIB_NO_JPEG