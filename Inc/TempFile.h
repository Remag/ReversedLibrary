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
CString REAPI GetTempDir();
void REAPI SetTempDir( CString newDir );

// Creates a new temporary file in a designated folder.
CString REAPI New();
// Creates a new temporary file in a specific folder.
CString REAPI NewInDir( CStringPart dir );

// Delete the file. It must be closed before deletion.
void REAPI Delete( CStringPart name );
// A given file will not be deleted on library deinitialization.
void REAPI MakePermanent( CStringPart name, CStringPart permanentName );

// Creates a temporary directory which will be deleted on cleanup.
CUnicodeString REAPI NewDir();
// Creates a temporary directory in a specific folder.
CUnicodeString REAPI NewDirInDir( CStringPart name );
// Delete the directory and all the files it contains.
void REAPI DeleteDir( CStringPart name );
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
	explicit CTempFile( CStringPart folder );
	CTempFile( CTempFile&& other );
	CTempFile& operator=( CTempFile&& other );
	~CTempFile();

	void Delete();
	void MakePermanent( CStringPart permanentName );

private:
	CString name;

	void openTempFile( CStringPart fileName );
	CString createNewTempFile();
	CString createNewTempFile( CStringPart dirName );
	CString openUniqueFile( CStringPart dir );
	void deleteTempFile();
	static int findTempFile( CStringPart name );

	static CString getTempDir();
	static void filterException( const CFileException& e );

	// Copying is prohibited.
	CTempFile( const CTempFile& ) = delete;
	void operator=( const CTempFile& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
