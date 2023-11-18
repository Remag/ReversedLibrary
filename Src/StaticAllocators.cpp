#include <DynamicAllocators.h>
#include <LibraryAllocators.h>
#include <RawBuffer.h>
#include <Reassert.h>
#include <StaticAllocators.h>

namespace Relib {

namespace RelibInternal {
	extern CVirtualAllocDynamicManager VirtualMemoryAllocator;
}

//////////////////////////////////////////////////////////////////////////

void* CRuntimeHeap::Allocate( int size )
{
	void* result = ::malloc( size );
	checkMemoryError( result != nullptr );
	return result;
}

void CRuntimeHeap::Free( void* ptr )
{
	::free( ptr );
}

void* CRuntimeHeap::AllocatedAligned( int size, int alignment )
{
	void* result = ::_aligned_malloc( size, alignment );
	checkMemoryError( result != nullptr );
	return result;
}

void CRuntimeHeap::FreeAligned( void* ptr )
{
	::_aligned_free( ptr );
}

#ifdef _DEBUG

void* CRuntimeHeap::Allocate( int size, const char* fileName, int line )
{
	void* result = ::_malloc_dbg( size, _NORMAL_BLOCK, fileName, line );
	checkMemoryError( result != nullptr );
	return result;
}

void* CRuntimeHeap::AllocateAligned( int size, int alignment, const char* fileName, int line )
{
	void* result = ::_aligned_malloc_dbg( size, alignment, fileName, line );
	checkMemoryError( result != nullptr );
	return result;
}

#endif

//////////////////////////////////////////////////////////////////////////

CRawBuffer CProcessHeap::AllocateSized( int size )
{
	return { Allocate( size ), size };
}

void* CProcessHeap::Allocate( int size )
{
	void* result = ::HeapAlloc( ::GetProcessHeap(), 0, size );
	checkMemoryError( result != nullptr );
	return result;
}

void CProcessHeap::Free( CRawBuffer buffer )
{
	return Free( buffer.Ptr() );
}

void CProcessHeap::Free( void* ptr )
{
	const BOOL freeResult = ::HeapFree( GetProcessHeap(), 0, ptr );
	assert( freeResult != 0 );
}

#ifdef _DEBUG

CRawBuffer CProcessHeap::AllocateSized( int size, const char*, int )
{
	return CProcessHeap::AllocateSized( size );
}

void* CProcessHeap::Allocate( int size, const char*, int )
{
	return CProcessHeap::Allocate( size );
}

#endif

//////////////////////////////////////////////////////////////////////////

CRawBuffer CThreadHeap::AllocateSized( int size )
{
	return GetThreadHeap().AllocateSized( size );
}

void* CThreadHeap::Allocate( int size )
{
	return GetThreadHeap().Allocate( size );
}

void CThreadHeap::Free( CRawBuffer buffer )
{
	GetThreadHeap().Free( buffer );
}

void CThreadHeap::Free( void* ptr )
{
	GetThreadHeap().Free( ptr );
}

#ifdef _DEBUG

CRawBuffer CThreadHeap::AllocateSized( int size, const char* fileName, int line )
{
	return GetThreadHeap().AllocateSized( size, fileName, line );
}

void* CThreadHeap::Allocate( int size, const char* fileName, int line )
{
	return GetThreadHeap().Allocate( size, fileName, line );
}

#endif

REAPI CHeapAllocator& GetThreadHeap()
{
	thread_local CHeapAllocator heapAllocator( HEAP_NO_SERIALIZE );
	return heapAllocator;
}

//////////////////////////////////////////////////////////////////////////

CRawBuffer CVirtualPageAllocator::AllocateSized( int size )
{
	return RelibInternal::VirtualMemoryAllocator.AllocateSized( size );
}

void* CVirtualPageAllocator::Allocate( int size )
{
	return RelibInternal::VirtualMemoryAllocator.Allocate( size );
}

void CVirtualPageAllocator::Free( CRawBuffer ptr )
{
	return RelibInternal::VirtualMemoryAllocator.Free( ptr );
}

void CVirtualPageAllocator::Free( void* ptr )
{
	return RelibInternal::VirtualMemoryAllocator.Free( ptr );
}

#ifdef _DEBUG

CRawBuffer CVirtualPageAllocator::AllocateSized( int size, const char* fileName, int line )
{
	return RelibInternal::VirtualMemoryAllocator.AllocateSized( size, fileName, line );
}

void* CVirtualPageAllocator::Allocate( int size, const char* fileName, int line )
{
	return RelibInternal::VirtualMemoryAllocator.Allocate( size, fileName, line );
}

#endif

//////////////////////////////////////////////////////////////////////////

}	 // namespace Relib.
