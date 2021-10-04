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

void CJpgFile::Read( CArray<CColor>& result, CVector2<int>& resultSize ) const
{
	assert( result.IsEmpty() );

	CFileReader file( fileName, FCM_OpenExisting );
	CArray<BYTE> fileData;
	const int fileLength = file.GetLength32();
	fileData.IncreaseSize( fileLength );
	file.Read( fileData.Ptr(), fileLength );

	doReadRawData( fileName, fileData, result, resultSize );
}

void CJpgFile::ReadRawData( CArrayView<BYTE> jpgData, CArray<CColor>& result, CVector2<int>& resultSize )
{
	doReadRawData( CUnicodePart(), jpgData, result, resultSize ); 
}

// Internal exception to bypass default JPEG error handling.
class CJpgInternalException {};
static void handleDecompressionExit( j_common_ptr jpegInfo )
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
		jpeg_start_decompress( &jpegInfo );
		resultSize.X() = jpegInfo.output_width;
		resultSize.Y() = jpegInfo.output_height;
		result.IncreaseSize( resultSize.X() * resultSize.Y() );
		const auto rowStride = jpegInfo.output_width * jpegInfo.output_components;
		// Main struct requires to be converted for the allocation routine.
		const auto commonInfo = ( jpeg_common_struct* )( &jpegInfo );
		const auto buffer = ( *jpegInfo.mem->alloc_sarray )( commonInfo, JPOOL_IMAGE, rowStride, 1 );
		auto rawResult = static_cast<CArrayBuffer<BYTE>>( result.Buffer() );

		while( jpegInfo.output_scanline < jpegInfo.output_height ) {
			const auto destPos = jpegInfo.output_scanline * rowStride;
			jpeg_read_scanlines( &jpegInfo, buffer, 1 );
			::memcpy( rawResult.Ptr() + destPos, buffer, rowStride );
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