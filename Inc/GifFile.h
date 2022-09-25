#pragma once
#include <Redefs.h>

#ifndef RELIB_NO_IMAGELIB

#include <ImageFileUtils.h>
#include <Errors.h>
#include <DynamicBitset.h>

//////////////////////////////////////////////////////////////////////////

namespace Relib {

namespace RelibInternal {
struct CGiffDecodeData;
}

extern const CStringView GeneralGifFileError;
//////////////////////////////////////////////////////////////////////////

// Exception that occurs while trying to extract data from a GIF file.
class REAPI CGifException : public CFileWrapperException {
public:
	CGifException( CStringPart fileName, CStringPart additionalInfo ) : CFileWrapperException( fileName, additionalInfo ) {}

	virtual CString GetMessageTemplate() const override
		{ return Str( GeneralGifFileError ); }
};

// Convert 1 and 0 delay to 10 delay.

//////////////////////////////////////////////////////////////////////////

// Mechanism for GIF file decompression.
class REAPI CGifFile {
public:
	explicit CGifFile( CStringPart fileName );

	// Read the contents of the GIF file and decompress them as a 32bit RGBA image.
	// Image pixel dimensions are returned in resultSize.
	void Read( CAnimatedImageData& result ) const;

	// Read directly from the provided array.
	static void ReadRawData( CArrayView<BYTE> gifData, CAnimatedImageData& result );

private:
	CString fileName;

	static void doReadRawData( CStringPart fileName, CArrayView<BYTE> gifData, CAnimatedImageData& result );
	static void readGifFrames( RelibInternal::CGiffDecodeData& decodeData, CArray<CImageFrameData>& result );
	static void copyColorData( CArrayView<BYTE> frameData, int width, int height, const CDynamicBitSet<>& transparencyMask, CArray<CColor>& result );
	static CColor createColor( CArrayView<BYTE> frameColors, int framePos );
	static int getFrameDelay( RelibInternal::CGiffDecodeData& decodeData );
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

#endif // RELIB_NO_IMAGELIB
