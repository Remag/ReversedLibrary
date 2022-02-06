#pragma once
#include <FileViews.h>
#include <BaseStringView.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// An owning file read class.
class CFileReader : public RelibInternal::CFileReadOperations {
public:
	CFileReader( CStringPart fileName, TFileCreationMode createMode )
		{ open( fileName, FRWM_Read, createMode, FSM_DenyNone, FILE_ATTRIBUTE_NORMAL ); }
	CFileReader( CFileReader&& other ) : RelibInternal::CFileReadOperations( other ) { other.detachFileHandle(); }
	CFileReader& operator=( CFileReader&& other )
		{ swapFileHandles( other ); return *this; }
	~CFileReader()
		{ close(); }

	// Conditional open operation. If the open operation fails, an invalid handle wrapper is returned.
	// Result can be checked for success with the IsOpen method.
	static CFileReader TryOpen( CStringPart fileName, TFileCreationMode createMode )
		{ return CFileReader( tryOpenHandle( fileName, FRWM_Read, createMode, FSM_DenyNone, FILE_ATTRIBUTE_NORMAL ) ); }

	operator CFileReadView() const
		{ return CFileReadView( getFileHandle() ); }

private:
	explicit CFileReader( HANDLE fileHandle ) : RelibInternal::CFileReadOperations( fileHandle ) {}

	// Copying is prohibited.
	CFileReader( CFileReader& ) = delete;
	void operator=( CFileReader& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

// An owning file write class.
class CFileWriter : public RelibInternal::CFileWriteOperations {
public:
	CFileWriter( CStringPart fileName, TFileCreationMode createMode )
		{ open( fileName, FRWM_Write, createMode, FSM_DenyNone, FILE_ATTRIBUTE_NORMAL ); }
	CFileWriter( CFileWriter&& other ) : RelibInternal::CFileWriteOperations( other ) { other.detachFileHandle(); }
	CFileWriter& operator=( CFileWriter&& other )
		{ swapFileHandles( other ); return *this; }
	~CFileWriter()
		{ close(); }
	// Conditional open operation. If the open operation fails, an invalid handle wrapper is returned.
	// Result can be checked for success with the IsOpen method.
	static CFileWriter TryOpen( CStringPart fileName, TFileCreationMode createMode )
		{ return CFileWriter( tryOpenHandle( fileName, FRWM_Write, createMode, FSM_DenyNone, FILE_ATTRIBUTE_NORMAL ) ); }

	operator CFileWriteView() const
		{ return CFileWriteView( getFileHandle() ); }

private:
	explicit CFileWriter( HANDLE fileHandle ) : RelibInternal::CFileWriteOperations( fileHandle ) {}

	// Copying is prohibited.
	CFileWriter( CFileWriter& ) = delete;
	void operator=( CFileWriter& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

// An owning file read-write class.
class CFileReadWriter : public RelibInternal::CFileReadWriteOperations {
public:
	CFileReadWriter( CStringPart fileName, TFileCreationMode createMode )
		{ open( fileName, FRWM_ReadWrite, createMode, FSM_DenyNone, FILE_ATTRIBUTE_NORMAL ); }
	CFileReadWriter( CFileReadWriter&& other ) : RelibInternal::CFileReadWriteOperations( other ) { other.detachFileHandle(); }
	CFileReadWriter& operator=( CFileReadWriter&& other )
		{ swapFileHandles( other ); return *this; }
	~CFileReadWriter()
		{ close(); }
	// Conditional open operation. If the open operation fails, an invalid handle wrapper is returned.
	// Result can be checked for success with the IsOpen method.
	static CFileReadWriter TryOpen( CStringPart fileName, TFileCreationMode createMode )
		{ return CFileReadWriter( tryOpenHandle( fileName, FRWM_ReadWrite, createMode, FSM_DenyNone, FILE_ATTRIBUTE_NORMAL ) ); }
	
	operator CFileReadView() const
		{ return CFileReadView( getFileHandle() ); }
	operator CFileWriteView() const
		{ return CFileWriteView( getFileHandle() ); }

private:
	explicit CFileReadWriter( HANDLE fileHandle ) : RelibInternal::CFileReadWriteOperations( fileHandle ) {}

	// Copying is prohibited.
	CFileReadWriter( CFileReadWriter& ) = delete;
	void operator=( CFileReadWriter& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

