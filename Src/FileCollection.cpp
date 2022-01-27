#include <FileCollection.h>
#include <FileSystem.h>
#include <FileOwners.h>
#include <Errors.h>
#include <BaseStringView.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CFileCollection::CFileCollection( CUnicodeView folderName )
{
	parseFolderData( folderName );
}

void CFileCollection::parseFolderData( CUnicodeView folderName )
{
	const auto fullName = FileSystem::CreateFullPath( folderName );
	CArray<CFileStatus> folderFiles;
	FileSystem::GetFilesInDir( fullName, folderFiles, FileSystem::FIF_Files | FileSystem::FIF_Recursive );
	writeIntToData( GetMagicNumber() );
	writeIntToData( folderFiles.Size() );

	for( const auto& file : folderFiles ) {
		parseFileUnit( file.FullName, static_cast<int>( file.Length ), fullName );
	}
}

void CFileCollection::parseFileUnit( CUnicodeView fileName, int fileLength, CUnicodeView folderName )
{
	const auto relName = findRelativeFileName( fileName, folderName );
	writeStringToData( relName );
	writeIntToData( fileLength );

	CFileReader file( fileName, FCM_OpenExisting );
	const int fileDataOffset = fileData.Size();
	fileData.IncreaseSize( fileData.Size() + fileLength );
	file.Read( fileData.Ptr() + fileDataOffset, fileLength );

	auto& fileUnit = unitInfo.Add();
	fileUnit.RelativePath = relName;
	fileUnit.DataOffset = fileDataOffset;
	fileUnit.DataSize = fileLength;
}

CUnicodeView CFileCollection::findRelativeFileName( CUnicodeView fileName, CUnicodeView parentFolderName ) const
{
	assert( fileName.HasPrefix( parentFolderName ) );
	const auto fileSuffix = fileName.Mid( parentFolderName.Length() );
	if( fileSuffix[0] == L'\\' || fileSuffix[0] == L'/' ) {
		return fileSuffix.Mid( 1 );
	}
	return fileSuffix;
}

void CFileCollection::writeIntToData( int value )
{
	const int offset = fileData.Size();
	fileData.IncreaseSize( offset + sizeof( int ) );
	::memcpy( fileData.Ptr() + offset, &value, sizeof( int ) );
}

void CFileCollection::writeStringToData( CUnicodeView str )
{
	const int fileNameOffset = fileData.Size();
	const int relNameByteSize = ( str.Length() + 1 ) * sizeof( wchar_t );
	fileData.IncreaseSize( fileNameOffset + relNameByteSize );
	::memcpy( fileData.Ptr() + fileNameOffset, str.Ptr(), relNameByteSize );
}

CFileCollection::CFileCollection( CArray<BYTE> binaryData ) :
	fileData( move( binaryData ) )
{
	parseFileData();
}

extern const CError Err_BadCollectionData;
void CFileCollection::checkDataError( bool condition )
{
	check( condition, Err_BadCollectionData );
}

void CFileCollection::parseFileData()
{
	int dataOffset = 2 * sizeof( int );
	checkDataError( fileData.Size() >= dataOffset );
	const int magic = *reinterpret_cast<int*>( fileData.Ptr() );
	checkDataError( magic == GetMagicNumber() );
	const int fileCount = *reinterpret_cast<int*>( fileData.Ptr() + sizeof( int ) );
	for( int i = 0; i < fileCount; i++ ) {
		dataOffset = parseFileUnit( dataOffset );
	}
}

int CFileCollection::parseFileUnit( int unitOffset )
{
	const auto dataSize = fileData.Size();
	checkDataError( dataSize > unitOffset );
	const wchar_t* relPathBuffer = reinterpret_cast<const wchar_t*>( fileData.Ptr() + unitOffset );
	const CUnicodeView relPathStr( relPathBuffer );
	const int fileSizeOffset = unitOffset + ( relPathStr.Length() + 1 ) * sizeof( wchar_t );
	checkDataError( dataSize > fileSizeOffset );
	const auto fileSize = *reinterpret_cast<int*>( fileData.Ptr() + fileSizeOffset );
	auto& newUnit = unitInfo.Add();
	newUnit.RelativePath = relPathStr;
	newUnit.DataOffset = fileSizeOffset + 4;
	newUnit.DataSize = fileSize;
	const auto result = fileSizeOffset + 4 + fileSize;
	checkDataError( dataSize >= result );
	return result;
}

void CFileCollection::WriteToFolder( CUnicodeView folderName )
{
	for( const auto& unit : unitInfo ) {
		writeFileUnit( unit, folderName );
	}
}

CUnicodeView CFileCollection::GetFileName( int filePos ) const
{
	return unitInfo[filePos].RelativePath;
}

CArrayView<BYTE> CFileCollection::GetFileData( int filePos ) const
{
	return fileData.Mid( unitInfo[filePos].DataOffset, unitInfo[filePos].DataSize );
}

void CFileCollection::SetFileName( int filePos, CUnicodePart newName )
{
	unitInfo[filePos].RelativePath = newName;
}

void CFileCollection::DeleteFile( int filePos )
{
	unitInfo.DeleteAt( filePos );
}

void CFileCollection::RepackBinaryData()
{
	CArray<BYTE> oldData;
	swap( fileData, oldData );
	writeIntToData( GetMagicNumber() );
	writeIntToData( unitInfo.Size() );

	for( auto& unit : unitInfo ) {
		updateFileUnit( unit, oldData );
	}
}

void CFileCollection::updateFileUnit( CFileUnitData& unit, CArrayView<BYTE> data )
{
	writeStringToData( unit.RelativePath );
	writeIntToData( unit.DataSize );

	const auto fileOffset = fileData.Size();
	fileData.IncreaseSize( fileOffset + unit.DataSize );
	memcpy( fileData.Ptr() + fileOffset, data.Ptr() + unit.DataOffset, unit.DataSize );
	unit.DataOffset = fileOffset;
}

void CFileCollection::writeFileUnit( const CFileUnitData& data, CUnicodeView folder )
{
	const auto fileName = FileSystem::MergePath( folder, data.RelativePath );
	const auto dirName = FileSystem::GetPath( fileName );
	if( !FileSystem::DirAccessible( dirName ) ) {
		FileSystem::CreateDir( dirName );
	}
	CFileWriter file( fileName, FCM_CreateAlways );
	const auto fileBuffer = fileData.Mid( data.DataOffset, data.DataSize );
	file.Write( fileBuffer.Ptr(), fileBuffer.Size() );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
