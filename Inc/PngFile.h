#pragma once
#include <Redefs.h>
#include <ImageFileUtils.h>
#include <Errors.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Acceptable PNG file formats.
enum TPngColorFormat {
	PCF_BGR,
	PCF_BGRA,
	PCF_RGB,
	PCF_RGBA,
	PCF_EnumCount
};

extern const CUnicodeView GeneralPngFileError;
//////////////////////////////////////////////////////////////////////////

// Exception that occurs while trying to extract data from a PNG file.
class REAPI CPngException : public CFileWrapperException {
public:
	CPngException( CUnicodePart fileName, CUnicodePart additionalInfo ) : CFileWrapperException( fileName, additionalInfo ) {}

	virtual CUnicodeString GetMessageTemplate() const override
	{ return UnicodeStr( GeneralPngFileError ); }
};

//////////////////////////////////////////////////////////////////////////

// Mechanism for PNG file compression and decompression.
class REAPI CPngFile {
public:
	explicit CPngFile( CUnicodePart fileName );

	// Read the contents of the PNG file and decompress them as a 32bit RGBA image.
	// Image pixel dimensions are returned in resultSize.
	void Read( CStaticImageData& result ) const;
	// Compress the given result into a 32bit RGBA PNG file.
	// imageSize specifies the size of the given file data in pixels.
	void Write( CArrayView<CColor> file, CVector2<int> imageSize );
	// Write from custom color data. rowStride indicates the size in bytes between image rows. Negative row stride indicates a bottom-up image.
	void Write( CArrayView<BYTE> file, TPngColorFormat format, int rowStride, CVector2<int> imageSize );

	// Read directly from the provided array.
	static void ReadRawData( CArrayView<BYTE> pngData, CStaticImageData& result );

private:
	CUnicodeString fileName;

	unsigned findLibPngFormat( TPngColorFormat format ) const;
	void doWrite( const void* fileData, unsigned pngLibFormat, int rowStride, CVector2<int> imageSize );
	void writeCompressedData( CArray<BYTE>& compressedData, int dataSize ) const;

	static void doReadRawData( CUnicodePart fileName, CArrayView<BYTE> pngData, CStaticImageData& result );
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

