#pragma once
#include <Redefs.h>
#include <Array.h>
#include <ArrayBuffer.h>
#include <BaseString.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Binary data of multiple files stitched together.
class REAPI CFileCollection {
public:
	static constexpr int GetMagicNumber()
		{ return 1223218893; }

	// Create a collection from folder.
	explicit CFileCollection( CUnicodeView folderName );
	// Create a collection from raw binary data.
	explicit CFileCollection( CArray<BYTE> binaryData );

	// Write the collection to a folder. Name conflicts are resolved by overwriting the files.
	void WriteToFolder( CUnicodeView folderName );
	

	int GetFileCount() const
		{ return unitInfo.Size(); }
	CUnicodeView GetFileName( int filePos ) const;
	CArrayView<BYTE> GetFileData( int filePos ) const;
	void SetFileName( int filePos, CUnicodePart newName );
	void DeleteFile( int filePos );

	// Delete unnecessary information from binary data.
	void RepackBinaryData();
	CArrayView<BYTE> GetBinaryData() const
		{ return fileData; }

private:
	// File collection binary data.
	CArray<BYTE> fileData;

	struct CFileUnitData {
		CUnicodeString RelativePath;
		int DataOffset;
		int DataSize;
	};

	// Data of the collection units.
	CArray<CFileUnitData> unitInfo;

	void parseFolderData( CUnicodeView folderName );
	void checkDataError( bool condition );
	void parseFileData();
	int parseFileUnit( int unitOffset );
	void parseFileUnit( CUnicodeView fileName, int fileLength, CUnicodeView folderName );
	CUnicodeView findRelativeFileName( CUnicodeView fileName, CUnicodeView parentFolderName ) const;
	void writeFileUnit( const CFileUnitData& data, CUnicodeView folder );
	void writeIntToData( int value );
	void writeStringToData( CUnicodeView str );
	void updateFileUnit( CFileUnitData& unit, CArrayView<BYTE> data );
};


//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

