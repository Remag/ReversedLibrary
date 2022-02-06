#pragma once
#include <FileViews.h>
#include <BaseStringPart.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// File handle wrapper that can change the underlying file and has an extended operation set.
class CDynamicFile : public RelibInternal::CFileReadWriteOperations {
public:
	CDynamicFile() = default;
	CDynamicFile( CDynamicFile&& other ) : RelibInternal::CFileReadWriteOperations( other ) { other.detachFileHandle(); }
	CDynamicFile& operator=( CDynamicFile&& other )
		{ swapFileHandles( other ); return *this; }
	~CDynamicFile()
		{ close(); }

	operator CFileReadView() const
		{ return CFileReadView( getFileHandle() ); }
	operator CFileWriteView() const
		{ return CFileWriteView( getFileHandle() ); }
	operator CFileReadWriteView() const
		{ return CFileReadWriteView( getFileHandle() ); }

	// Get the WinAPI file handle.
	HANDLE Handle() const
		{ return getFileHandle(); }

	// Open and create methods that throw an exception on failure.
	void Open( CStringPart fileName, TFileReadWriteMode readWriteMode, TFileCreationMode createMode, TFileShareMode shareMode )
		{ return open( fileName, readWriteMode, createMode, shareMode, FILE_ATTRIBUTE_NORMAL ); }
	void Open( CStringPart fileName, TFileReadWriteMode readWriteMode, TFileCreationMode createMode, TFileShareMode shareMode, DWORD attributes )
		{ return open( fileName, readWriteMode, createMode, shareMode, attributes ); }
	// Open and create methods that return false on failure.
	bool TryOpen( CStringPart fileName, TFileReadWriteMode readWriteMode, TFileCreationMode createMode, TFileShareMode shareMode )
		{ return tryOpen( fileName, readWriteMode, createMode, shareMode, FILE_ATTRIBUTE_NORMAL ); }
	bool TryOpen( CStringPart fileName, TFileReadWriteMode readWriteMode, TFileCreationMode createMode, TFileShareMode shareMode, DWORD attributes )
		{ return tryOpen( fileName, readWriteMode, createMode, shareMode, attributes ); }

	// Save pending changes.
	void Flush()
		{ return flush(); }
	// Close the handle.
	void Close()
		{ return close(); }

private:
	// Copying is prohibited.
	CDynamicFile( CDynamicFile& ) = delete;
	void operator=( CDynamicFile& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

