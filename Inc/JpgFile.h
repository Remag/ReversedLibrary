#pragma once
#ifndef RELIB_NO_JPEG

#include <Redefs.h>
#include <Errors.h>
#include <ImageFileUtils.h>

namespace Relib {

extern const CUnicodeView GeneralJpgFileError;
//////////////////////////////////////////////////////////////////////////

// Exception that occurs while trying to extract data from a JPEG file.
class REAPI CJpgException : public CFileWrapperException {
public:
	CJpgException( CUnicodePart fileName, CUnicodePart additionalInfo ) : CFileWrapperException( fileName, additionalInfo ) {}

	virtual CUnicodeString GetMessageTemplate() const override
		{ return UnicodeStr( GeneralJpgFileError ); }
};

//////////////////////////////////////////////////////////////////////////

// Mechanism for JPEG image compression and decompression.
class REAPI CJpgFile {
public:
	explicit CJpgFile( CUnicodePart fileName );

	// Read the contents of a JPEG file and decompress them as a JPEG image.
	void Read( CStaticImageData& result ) const;
	// Read directly from the provided array.
	static void ReadRawData( CArrayView<BYTE> jpgData, CStaticImageData& result );

private:
	CUnicodeString fileName;

	static void doReadRawData( CUnicodePart fileName, CArrayView<BYTE> pngData, CArray<CColor>& result, CVector2<int>& resultSize );
};

//////////////////////////////////////////////////////////////////////////

} // namespace Relib.

#endif // RELIB_NO_JPEG