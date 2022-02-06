#pragma once
#include <DynamicFile.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Class for viewing the contents of a mapped file. Owns the view buffer.
class REAPI CMappingReadView {
public:
	CMappingReadView() = default;
	// Views are movable.
	CMappingReadView( CMappingReadView&& other );
	CMappingReadView& operator=( CMappingReadView&& other );
	// Destructor that unmaps the view.
	~CMappingReadView();

	int Size() const
		{ return bufferSize; }
	// Constant access to the mapped buffer.
	const BYTE* GetBuffer() const
		{ return buffer; }
		
	// Flush the view to the hard drive.
	void Flush();
	// Close the view. Buffer is freed and set to 0.
	void Close();

	// Only the file mapping mechanism can create views.
	friend class CFileMapping;

private:
	// Mapped buffer.
	const BYTE* buffer = nullptr;
	// Size of the buffer.
	int bufferSize = 0;
	// Offset that needs to be subtracted from buffer to get the aligned allocation position.
	int allocationOffset = 0;
	
	CMappingReadView( const BYTE* buffer, int size, int allocationOffset );

	// Copying is prohibited.
	CMappingReadView( CMappingReadView& ) = delete;
	void operator=( CMappingReadView& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

// Class for changing the contents of a mapped file. Owns the view buffer.
class REAPI CMappingReadWriteView {
public:
	CMappingReadWriteView() = default;
	// Views are movable.
	CMappingReadWriteView( CMappingReadWriteView&& other );
	CMappingReadWriteView& operator=( CMappingReadWriteView&& other );
	// Destructor that unmaps the view.
	~CMappingReadWriteView();

	int Size() const
		{ return bufferSize; }
	// Access to the mapped buffer.
	BYTE* GetBuffer()
		{ return buffer; }
	// Constant access to the mapped buffer.
	const BYTE* GetBuffer() const
		{ return buffer; }

	// Flush the view to the hard drive.
	void Flush();
	// Close the view. Buffer is freed and set to 0.
	void Close();

	// Only the file mapping mechanism can create views.
	friend class CFileMapping;

private:
	// Mapped buffer.
	BYTE* buffer = nullptr;
	// Size of the buffer.
	int bufferSize = 0;
	// Offset that needs to be subtracted from buffer to get the aligned allocation position.
	int allocationOffset = 0;

	CMappingReadWriteView( BYTE* buffer, int size, int allocationOffset );

	// Copying is prohibited.
	CMappingReadWriteView( CMappingReadWriteView& ) = delete;
	void operator=( CMappingReadWriteView& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

// Mechanism for mapping an existing file to memory.
class REAPI CFileMapping {
public:
	// Mode, describing, whether we need write access to the mapped file.
	enum TMappingMode {
		MM_ReadOnly = PAGE_READONLY,
		MM_ReadWrite = PAGE_READWRITE
	};

	CFileMapping();
	// Create a mapping from an existing file.
	CFileMapping( CStringPart fileName, TMappingMode mode );
	CFileMapping( CStringPart fileName, TMappingMode mode, CUnicodeView mappingName );
	// Create a mapping in read/write mode from a file that might not exist.
	CFileMapping( CStringPart fileName, __int64 fileLength );
	CFileMapping( CStringPart fileName, __int64 fileLength, CUnicodeView mappingName );
	~CFileMapping();

	TMappingMode Mode() const
		{ return mode; }
	__int64 GetFileLength() const
		{ return file.GetLength(); }

	bool IsOpen() const
		{ return mappingHandle != nullptr; }
	// Open and map an existing file.
	void Open( CStringPart fileName, TMappingMode mode );
	void Open( CStringPart fileName, TMappingMode mode, CUnicodeView mappingName );
	// Open a mapping in read/write mode from a file that might not exist. File size is increased to minLength if necessary.
	void Open( CStringPart fileName, __int64 minLength );
	void Open( CStringPart fileName, __int64 fileLength, CUnicodeView mappingName );
	// Open an existing file mapping with the given name. Returns true if a mapping with this name was found.
	bool OpenExternal( CUnicodeView mappingName, TMappingMode mode );
	
	// Create the view of the whole file.
	CMappingReadView CreateReadView();
	CMappingReadWriteView CreateReadWriteView();
	// Create the view of a given region of the mapped file.
	CMappingReadView CreateReadView( __int64 offset, int length );
	CMappingReadWriteView CreateReadWriteView( __int64 offset, int length );
	;
	// Close the mapping. File will not be freed until all created views are closed as well.
	void Close();

private:
	// Mapped file.
	CDynamicFile file;
	// Handle of the mapping.
	HANDLE mappingHandle = nullptr;
	// Mapping mode.
	TMappingMode mode;

	void doOpenMapping( CStringPart fileName, TMappingMode mode, const wchar_t* mappingNamePtr );
	void doOpenMapping( CStringPart fileName, __int64 fileLength, const wchar_t* mappingNamePtr );
	void openMapping( __int64 minLength, const wchar_t* mappingNamePtr );
	void openView( DWORD viewMode, __int64 offset, int length, BYTE*& result, int& allocationOffset );
	static int getAllocationGranularity();

	// Copying is prohibited.
	CFileMapping( CFileMapping& ) = delete;
	void operator=( CFileMapping& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

} // namespace Relib.