#include <MemoryUtils.h>
#include <CriticalSection.h>
#include <Reutils.h>
#include <InlineStackAllocator.h>
#include <Array.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

void reportMemoryError( const wchar_t* format, ... )
{
	va_list argList;
	va_start( argList, format );

	const int startMemoryLength = 512;
	int memoryErrorLength = startMemoryLength;
	CFlexibleArray<wchar_t, startMemoryLength> errorStr;
	int count = 0;
	do {
		errorStr.IncreaseSizeNoInitialize( memoryErrorLength );
		count = _vsnwprintf_s( errorStr.Ptr(), memoryErrorLength, memoryErrorLength - 1, format, argList );
		memoryErrorLength *= 2;
	} while( count < 0 );

	va_end( argList );

	if( HasFlag( GetDebugFlags(), DF_ReportMemoryErrorsInMessageBox ) ) {
		const UINT result = ::MessageBoxW( 0, errorStr.Ptr(), L"", MB_OKCANCEL | MB_ICONERROR );
		if( result == IDCANCEL ) {
			ProgramBreakPoint();
		}
	} else {
		OutputDebugStringW( errorStr.Ptr() );
	}
}

//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

CVirtualMemoryDebugBlock::CVirtualMemoryDebugBlock( CVirtualMemoryDebugBlock* _next, int size, const char* fileName, int line ) :
	next( _next ),
	prev( 0 ),
	block( size, fileName, line )
{
	if( next != 0 ) {
		next->prev = this;
	}
}

static const wchar_t* virtualMemoryAllocatorName = L"virtual memory allocator";
void CVirtualMemoryDebugBlock::CheckFree()
{
	block.CheckFree( virtualMemoryAllocatorName );
}

void CVirtualMemoryDebugBlock::SetFree()
{
	if( next != 0 ) {
		next->prev = prev;
	}
	if( prev != 0 ) {
		prev->next = next;
	}
	block.SetFree( virtualMemoryAllocatorName );
}

int CVirtualMemoryDebugBlock::DebugBlockSize( int dataSize )
{
	return offsetof( CVirtualMemoryDebugBlock, block ) + CDebugMemoryBlock::DebugBlockSize( dataSize );
}

CVirtualMemoryDebugBlock* CVirtualMemoryDebugBlock::CreateFromData( void* ptr )
{
	void* result = reinterpret_cast<BYTE*>( CDebugMemoryBlock::CreateFromData( ptr ) ) - offsetof( CVirtualMemoryDebugBlock, block );
	return reinterpret_cast<CVirtualMemoryDebugBlock*>( result );
}

//Number of the next memory allocation.
static int nextAllocationNumber = 1;
static const int memoryTagUsed = 0x5D710FD2;
static const int memoryTagFree = 0xC380758B;
static const BYTE paddingByte = 0xED;
static const BYTE fillerByte = 0xAD;

extern CCriticalSection DebugAllocatorGlobalSection;

CDebugMemoryBlock::CDebugMemoryBlock( int _dataSize, const char* _fileName, int _line ) :
	memoryTag( memoryTagUsed ),
	dataSize( _dataSize ),
	fileName( _fileName ),
	line( _line )
{
	setInitialValues();
}

void CDebugMemoryBlock::setInitialValues()
{
	{
		CCriticalSectionLock lock( DebugAllocatorGlobalSection );
		allocationNumber = nextAllocationNumber;
		nextAllocationNumber++;
	}

	const int paddingSize = dataSizeWithPadding() - dataSize;
	assert( paddingSize > 0 );
	::memset( data + dataSize, paddingByte, paddingSize );
}

void CDebugMemoryBlock::ResetData( int _dataSize, const char* _fileName, int _line, const wchar_t* allocatorName )
{
	CheckFree( allocatorName );
	memoryTag = memoryTagUsed;
	dataSize = _dataSize;
	fileName = _fileName;
	line = _line;
	setInitialValues();
}

bool CDebugMemoryBlock::IsValidTag() const
{
	return memoryTag == memoryTagFree || memoryTag == memoryTagUsed;
}

static const wchar_t* doubleDeleteErrorStr = L"Memory block in %s is deleted twice! Address: %p\n";
static const wchar_t* memoryCorruptionErrorStr = L"Memory block in %s is corrupted! Address: %p\nFile: %S\nLine number: %d. Allocation number: %d.\n";
void CDebugMemoryBlock::SetFree( const wchar_t* allocatorName )
{
	switch( memoryTag ) {
	case memoryTagFree:
		RelibInternal::reportMemoryError( doubleDeleteErrorStr, allocatorName, data );
		break;
	case memoryTagUsed:
		checkDataPadding( allocatorName );
		break;
	default:
		RelibInternal::reportMemoryError( memoryCorruptionErrorStr, allocatorName, data, fileName, line, allocationNumber );
		break;
	}

	FillWithTrash();
	memoryTag = memoryTagFree;
	fileName = 0;
	line = 0;
}

static const wchar_t* undeletedMemoryErrorStr = L"Memory block in %s is not deleted! Address: %p\nFile: %S\nLine number: %d. Allocation number: %d.\n";
void CDebugMemoryBlock::CheckFree( const wchar_t* allocatorName ) const
{
	if( memoryTag == memoryTagUsed ) {
		RelibInternal::reportMemoryError( undeletedMemoryErrorStr, allocatorName, data, fileName, line, allocationNumber );
	}
}

void CDebugMemoryBlock::FillWithTrash()
{
	::memset( data, fillerByte, dataSizeWithPadding() );
}

CDebugMemoryBlock* CDebugMemoryBlock::CreateFromData( void* ptr )
{
	// Assume that ptr point to the start of data in the debug memory block.
	return reinterpret_cast<CDebugMemoryBlock*>( reinterpret_cast<BYTE*>( ptr ) - offsetof( CDebugMemoryBlock, data ) );
}

void CDebugMemoryBlock::CheckRegion( const wchar_t* allocatorName, const BYTE* startPtr, int size )
{
	for( const BYTE* ptr = startPtr; ptr - startPtr < size; ) {
		const CDebugMemoryBlock* debugBlock = reinterpret_cast<const CDebugMemoryBlock*>( ptr );
		debugBlock->CheckFree( allocatorName );
		ptr += debugBlock->DebugBlockSize( debugBlock->DataSize() );
	}
}

int CDebugMemoryBlock::dataSizeWithPadding() const
{
	return DebugBlockSize( dataSize ) - offsetof( CDebugMemoryBlock, data );
}

void CDebugMemoryBlock::checkDataPadding( const wchar_t* allocatorName ) const
{
	const int size = dataSizeWithPadding();
	for( int i = dataSize; i < size; i++ ) {
		if( data[i] != paddingByte ) {
			RelibInternal::reportMemoryError( memoryCorruptionErrorStr, allocatorName, data, fileName, line, allocationNumber );
		}
	}
}

#endif

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

