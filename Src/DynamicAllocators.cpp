#include <DynamicAllocators.h>
#include <StaticAllocators.h>
#include <MemoryUtils.h>
#include <LibraryAllocators.h>
#include <Reassert.h>
#include <Reutils.h>

namespace Relib {

// Amount of memory that is allocated with VirtualAlloc with no regard to the actual memory manager.
static const int initialHeapSize = 128 * 1024;
//////////////////////////////////////////////////////////////////////////

// Function pointer to the set information function.
typedef BOOL ( WINAPI *THeapSetInformation )( HANDLE, HEAP_INFORMATION_CLASS, VOID*, SIZE_T );

CHeapAllocator::CHeapAllocator() :
	heap( 0 ),
	allocationCount( 0 ),
	totalSize( 0 )
{
}

CHeapAllocator::CHeapAllocator( DWORD options ) :
	CHeapAllocator()
{
	Create( options );
}

CHeapAllocator::~CHeapAllocator()
{
	if( heap != 0 ) {
		Dump();
		destroy( true );
	}
}

void CHeapAllocator::Create( DWORD options /*= HEAP_NO_SERIALIZE */ )
{
	assert( heap == 0 );
	heap = ::HeapCreate( options, initialHeapSize, 0 );
	checkMemoryError( heap != 0 );
	allocationCount = 0;
}

void CHeapAllocator::Destroy( bool forcedDestroy /*= false */ )
{
	if( heap != 0 ) {
		Dump();
		destroy( forcedDestroy );
	}
}

void CHeapAllocator::SetLowFragmentation( bool isSet )
{
	assert( heap != 0 );

	THeapSetInformation heapSetInformation = reinterpret_cast<THeapSetInformation>( GetProcAddress( GetModuleHandle( L"kernel32.dll" ), "HeapSetInformation" ) );
	if( heapSetInformation == nullptr ) {
		// Function is not supported on the system.
		return;
	}
	ULONG lowFragmentationFlag = isSet ? 2 : 0;
	heapSetInformation( heap, HeapCompatibilityInformation, &lowFragmentationFlag, sizeof( lowFragmentationFlag ) );
}

void CHeapAllocator::Dump() const
{
	assert( heap != 0 );
	if( allocationCount == 0 ) {
		return;
	}

#ifdef _DEBUG
	PROCESS_HEAP_ENTRY entry;
	entry.lpData = 0;
	::HeapLock( heap );
	while( ::HeapWalk( heap, &entry ) != 0 ) {
		if( !HasFlag( entry.wFlags, PROCESS_HEAP_ENTRY_BUSY ) ) {
			continue;
		}
		RelibInternal::CDebugMemoryBlock* block = reinterpret_cast<RelibInternal::CDebugMemoryBlock*>( entry.lpData );
		if( block->IsValidTag() ) {
			block->CheckFree( L"heap manager" );
		} else {
			RelibInternal::reportMemoryError( L"Corrupted memory block in heap manager! Address: %p\n", entry.lpData );
		}
	}
	::HeapUnlock( heap );
#endif
}

#ifdef _DEBUG

CRawBuffer CHeapAllocator::AllocateSized( int size, const char* fileName, int line )
{
	return { Allocate( size, fileName, line ), size };
}

void* CHeapAllocator::Allocate( int size, const char* fileName, int line )
{
	assert( Handle() != 0 );
	const int realSize = RelibInternal::CDebugMemoryBlock::DebugBlockSize( size );
	void* ptr = ::HeapAlloc( Handle(), 0, realSize );
	checkMemoryError( ptr != nullptr );
	RelibInternal::CDebugMemoryBlock* block = ::new( ptr ) RelibInternal::CDebugMemoryBlock( size, fileName, line );
	allocationCount++;
	totalSize += size;
	return block->Data();
}

#endif

void CHeapAllocator::destroy( bool forced )
{
	assert( heap != 0 );
	if( allocationCount == 0 || forced ) {
		const BOOL destroyResult = ::HeapDestroy( heap );
		destroyResult;
		assert( destroyResult != 0 );
		heap = 0;
		allocationCount = 0;
		totalSize = 0;
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

