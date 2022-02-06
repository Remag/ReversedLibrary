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
	explicit CFileCollection( CStringPart folderName );
	// Create a collection from raw binary data.
	explicit CFileCollection( CArray<BYTE> binaryData );

	// Write the collection to a folder. Name conflicts are resolved by overwriting the files.
	void WriteToFolder( CStringPart folderName );
	

	int GetFileCount() const
		{ return unitInfo.Size(); }
	CStringPart GetFileName( int filePos ) const;
	CArrayView<BYTE> GetFileData( int filePos ) const;
	void SetFileName( int filePos, CStringPart newName );
	void DeleteFile( int filePos );

	// Delete unnecessary information from binary data.
	void RepackBinaryData();
	CArrayView<BYTE> GetBinaryData() const
		{ return fileData; }

private:
	// File collection binary data.
	CArray<BYTE> fileData;

	struct CFileUnitData {
		CString RelativePath;
		int DataOffset;
		int DataSize;
	};

	// Data of the collection units.
	CArray<CFileUnitData> unitInfo;

	void parseFolderData( CStringPart folderName );
	void checkDataError( bool condition );
	void parseFileData();
	int parseFileUnit( int unitOffset );
	void parseFileUnit( CStringPart fileName, int fileLength, CStringPart folderName );
	CStringPart findRelativeFileName( CStringPart fileName, CStringPart parentFolderName ) const;
	void writeFileUnit( const CFileUnitData& data, CStringPart folder );
	void writeIntToData( int value );
	void writeStringToData( CStringPart str );
	void updateFileUnit( CFileUnitData& unit, CArrayView<BYTE> data );
};


//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

