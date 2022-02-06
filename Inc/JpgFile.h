#pragma once
#include <Redefs.h>

#ifndef RELIB_NO_IMAGELIB

#include <Errors.h>
#include <ImageFileUtils.h>

namespace Relib {

extern const CStringView GeneralJpgFileError;
//////////////////////////////////////////////////////////////////////////

// Exception that occurs while trying to extract data from a JPEG file.
class REAPI CJpgException : public CFileWrapperException {
public:
	CJpgException( CStringPart fileName, CStringPart additionalInfo ) : CFileWrapperException( fileName, additionalInfo ) {}

	virtual CString GetMessageTemplate() const override
		{ return Str( GeneralJpgFileError ); }
};

//////////////////////////////////////////////////////////////////////////

// Mechanism for JPEG image compression and decompression.
class REAPI CJpgFile {
public:
	explicit CJpgFile( CStringPart fileName );

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

#endif // RELIB_NO_IMAGELIB