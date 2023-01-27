#include <Redefs.h>

#ifndef RELIB_NO_IMAGELIB

#include <PngFile.h>

#include <BaseStringPart.h>
#include <BaseStringView.h>
#include <ArrayBuffer.h>
#include <StrConversions.h>
#include <FileOwners.h>

#include <libpng\png.h>
#include <zlib\zlib.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

static void throwPngException( const png_image& image, CStringPart fileName )
{
	const CStringView messageStr = image.message;
	throw CPngException( fileName, messageStr );
}

//////////////////////////////////////////////////////////////////////////

// Utility class that handles proper closing of libpng file resources.
class CPngLibFileReader {
public:
	CPngLibFileReader( CStringPart fileName, CArrayView<BYTE> fileData, png_image& image );
	~CPngLibFileReader();

	void FinalizeReading( CArray<CColor>& result, int rowStride );

private:
	CStringPart fileName;
	png_image* image = nullptr;

	// Copying is prohibited.
	CPngLibFileReader( CPngLibFileReader& ) = delete;
	void operator=( CPngLibFileReader& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

CPngLibFileReader::CPngLibFileReader( CStringPart _fileName, CArrayView<BYTE> fileData, png_image& _image ) : 
	image( &_image ),
	fileName( _fileName )
{
	if( png_image_begin_read_from_memory( &_image, fileData.Ptr(), fileData.Size() ) == 0 ) {
		throwPngException( _image, fileName );
	}
}

CPngLibFileReader::~CPngLibFileReader()
{
	if( image != nullptr ) {
		png_image_free( image );
	}
}

void CPngLibFileReader::FinalizeReading( CArray<CColor>& result, int rowStride )
{
	const auto imageCpy = image;
	assert( imageCpy != nullptr );
	image = nullptr;
	// Image is saved as bottom-up, so negative row stride is passed.
	if( png_image_finish_read( imageCpy, nullptr, result.Ptr(), -rowStride, nullptr ) == 0 ) {
		throwPngException( *imageCpy, fileName );
	}
}

//////////////////////////////////////////////////////////////////////////

CPngFile::CPngFile( CStringPart _fileName ) :
	fileName( _fileName )
{
}

void CPngFile::Read( CStaticImageData& result ) const
{
	assert( result.Colors.IsEmpty() );

	CFileReader file( fileName, FCM_OpenExisting );
	CArray<BYTE> fileData;
	const int fileLength = file.GetLength32();
	fileData.IncreaseSize( fileLength );
	file.Read( fileData.Ptr(), fileLength );

	doReadRawData( fileName, fileData, result );
}

void CPngFile::ReadRawData( CArrayView<BYTE> pngData, CStaticImageData& result )
{
	doReadRawData( CStringPart(), pngData, result ); 
}

void CPngFile::doReadRawData( CStringPart fileName, CArrayView<BYTE> pngData, CStaticImageData& result )
{
	png_image pngImage;
	::memset( &pngImage,0, sizeof( pngImage ) );
	pngImage.version = PNG_IMAGE_VERSION;

	CPngLibFileReader reader( fileName, pngData, pngImage );
	pngImage.format = PNG_FORMAT_RGBA;
	result.Colors.IncreaseSize( pngImage.height * pngImage.width );
	const auto rowStride = PNG_IMAGE_ROW_STRIDE( pngImage );
	reader.FinalizeReading( result.Colors, rowStride );
	result.ImageSize.X() = pngImage.width;
	result.ImageSize.Y() = pngImage.height;
}

void CPngFile::Write( CArrayView<CColor> file, CVector2<int> imageSize )
{
	// Image is assumed to be bottom up, so negative row stride is passed.
	doWrite( file.Ptr(), PNG_FORMAT_RGBA, -4 * imageSize.X(), imageSize );
}

void CPngFile::Write( CArrayView<BYTE> file, TPngColorFormat format, int rowStride, CVector2<int> imageSize )
{
	const auto libFormat = findLibPngFormat( format );
	doWrite( file.Ptr(), libFormat, rowStride, imageSize );
}

unsigned CPngFile::findLibPngFormat( TPngColorFormat format ) const
{
	staticAssert( PCF_EnumCount == 4 );
	switch( format ) {
		case PCF_BGR:
			return PNG_FORMAT_BGR;
		case PCF_BGRA:
			return PNG_FORMAT_BGRA;
		case PCF_RGB:
			return PNG_FORMAT_RGB;
		case PCF_RGBA:
			return PNG_FORMAT_RGBA;
		default:
			assert( false );
			return PNG_FORMAT_RGB;
	}
}

void CPngFile::doWrite( const void* fileData, unsigned pngLibFormat, int rowStride, CVector2<int> imageSize )
{
	png_image pngImage;
	::memset( &pngImage,0, sizeof( pngImage ) );
	pngImage.version = PNG_IMAGE_VERSION;
	pngImage.opaque = nullptr;
	pngImage.width = imageSize.X();
	pngImage.height = imageSize.Y();
	pngImage.format = pngLibFormat;
	pngImage.flags = 0;
	pngImage.colormap_entries = 0;

	CArray<BYTE> compressedData;
	const auto upperSize = PNG_IMAGE_PNG_SIZE_MAX( pngImage );
	compressedData.IncreaseSize( static_cast<int>( upperSize ) );
	png_alloc_size_t bufferSize = upperSize;
	if( png_image_write_to_memory( &pngImage, compressedData.Ptr(), &bufferSize, 0, fileData, rowStride, nullptr ) == 0 ) {
		// The upper size may be calculated incorrectly in extreme cases. Handle this situation.
		if( bufferSize > upperSize ) {
			compressedData.IncreaseSize( static_cast<int>( bufferSize ) );
			if( png_image_write_to_memory( &pngImage, compressedData.Ptr(), &bufferSize, 0, fileData, rowStride, nullptr ) != 0 ) {
				writeCompressedData( compressedData, static_cast<int>( bufferSize ) );
				return;
			}
		}
		throwPngException( pngImage, fileName );
	}
	writeCompressedData( compressedData, static_cast<int>( bufferSize ) );
}

void CPngFile::writeCompressedData( CArray<BYTE>& compressedData, int dataSize ) const
{
	assert( compressedData.Size() >= dataSize );
	compressedData.DeleteLast( compressedData.Size() - dataSize );
	CFileWriter file( fileName, FCM_CreateAlways );
	file.Write( compressedData.Ptr(), compressedData.Size() );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

#endif // RELIB_NO_IMAGELIB