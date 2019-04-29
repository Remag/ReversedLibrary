#pragma once

#include <Redefs.h>
#include <BaseString.h>
#include <BaseStringView.h>
#include <FileOperations.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Global functions for managing the temporary files.
namespace TempFile {

//////////////////////////////////////////////////////////////////////////

// Get the designated temporary dir folder.
CUnicodeString REAPI GetTempDir();
void REAPI SetTempDir( CUnicodeString newDir );

// Creates a new temporary file in a designated folder.
CUnicodeString REAPI New();
// Creates a new temporary file in a specific folder.
CUnicodeString REAPI NewInDir( CUnicodeView dir );

// Delete the file. It must be closed before deletion.
void REAPI Delete( CUnicodeView name );
// A given file will not be deleted on library deinitialization.
void REAPI MakePermanent( CUnicodeView name, CUnicodeView permanentName );

// Creates a temporary directory which will be deleted on cleanup.
CUnicodeString REAPI NewDir();
// Creates a temporary directory in a specific folder.
CUnicodeString REAPI NewDirInDir( CUnicodeView name );
// Delete the directory and all the files it contains.
void REAPI DeleteDir( CUnicodeView name );
// The directory and its contents will not be deleted on cleanup. 
void REAPI MakeDirPermanent( CUnicodeString name, CUnicodeString permanentName );

void REAPI DeleteAll();
void REAPI Reset();

//////////////////////////////////////////////////////////////////////////

}	// namespace TempFile.

//////////////////////////////////////////////////////////////////////////

// Class that creates a temp file when constructed and deletes it in destructor.
class REAPI CTempFile : public RelibInternal::CFileReadWriteOperations {
public:
	CTempFile();
	explicit CTempFile( CUnicodeView folder );
	CTempFile( CTempFile&& other );
	CTempFile& operator=( CTempFile&& other );
	~CTempFile();

	void Delete();
	void MakePermanent( CUnicodeView permanentName );

private:
	CUnicodeString name;

	void openTempFile( CUnicodeView fileName );
	CUnicodeString createNewTempFile();
	CUnicodeString createNewTempFile( CUnicodeView dirName );
	CUnicodeString openUniqueFile( CUnicodeView dir );
	void deleteTempFile();
	static int findTempFile( CUnicodeView name );

	static CUnicodeString getTempDir();
	static void filterException( const CFileException& e );

	// Copying is prohibited.
	CTempFile( const CTempFile& ) = delete;
	void operator=( const CTempFile& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
