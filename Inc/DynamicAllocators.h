#pragma once
#include <Allocator.h>
#include <Reassert.h>
#include <MemoryUtils.h>
#include <CriticalSection.h>
#include <Remath.h>
#include <BaseStackAllocator.h>
#include <RawBuffer.h>

namespace Relib {

// Size of preallocated block of memory.
static const int defaultAllocatorPageSize = 64 * 1024;
//////////////////////////////////////////////////////////////////////////

// Heap memory manager.
class REAPI CHeapAllocator : public TGlobalAllocator {
public:
	// Create an uninitialized heap.
	// Call to Create is needed to use it.
	CHeapAllocator();
	// Create and initialize the heap with the given options.
	explicit CHeapAllocator( DWORD options );
	~CHeapAllocator();

	// Create a heap with the given options. 
	// No multi threaded access is assumed and HEAP_NO_SERIALIZE is set by default.
	void Create( DWORD options = HEAP_NO_SERIALIZE );
	// Destroy the heap. Heap remains untouched if some blocks are unfreed and forcedDestroy is set to false.
	void Destroy( bool forcedDestroy = false );
	HANDLE Handle() const
		{ return heap; }

	// Set the heap in a low fragmentation mode.
	// Doesn't work with HEAP_NO_SERIALIZE flag.
	void SetLowFragmentation( bool set );

	// Output information about undeleted blocks.
	void Dump() const;

	int DataSize() const
		{ return totalSize; }

	CRawBuffer AllocateSized( int size );
	void Free( CRawBuffer ptr );

	void* Allocate( int size );
	void Free( void* ptr );

#ifdef _DEBUG
	CRawBuffer AllocateSized( int size, const char* fileName, int line );
	void* Allocate( int size, const char* fileName, int line );
#endif

private:
	HANDLE heap;
	int allocationCount;
	int totalSize;

	void destroy( bool forced );

	// Copying is prohibited.
	CHeapAllocator( const CHeapAllocator& ) = delete;
	void operator=( const CHeapAllocator& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

// Thread-safe template wrapper for any allocator.
template <class Allocator>
class CThreadSafeAllocator : public TGlobalAllocator {
public:
	template <class... Args>
	CThreadSafeAllocator( Args&&... args ) : allocator( forward<Args>( args )... ) {}
		
	CRawBuffer AllocateSized( int size );
	void Free( CRawBuffer ptr );

	void* Allocate( int size );
	void Free( void* ptr );

#ifdef _DEBUG
	CRawBuffer AllocateSized( int size, const char* fileName, int line );
	void* Allocate( int size, const char* fileName, int line );
#endif

private:
	Allocator allocator;
	CCriticalSection section;

	// Copying is prohibited.
	CThreadSafeAllocator( CThreadSafeAllocator& ) = delete;
	CThreadSafeAllocator( CThreadSafeAllocator&& ) = delete;
	void operator=( CThreadSafeAllocator& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

inline CRawBuffer CHeapAllocator::AllocateSized( int size )
{
	return { Allocate( size ), size };
}

inline void CHeapAllocator::Free( CRawBuffer buffer )
{
	assert( heap != 0 );
	void* ptr = buffer.Ptr();
	if( ptr == nullptr ) {
		return;
	}
#ifdef _DEBUG
	RelibInternal::CDebugMemoryBlock* block = RelibInternal::CDebugMemoryBlock::CreateFromData( ptr );
	block->SetFree( L"heap manager" );
	totalSize -= block->DataSize();
	const BOOL freeResult = ::HeapFree( Handle(), 0, block );
	assert( freeResult != 0 );
#else
	totalSize -= buffer.Size();
	const BOOL freeResult = ::HeapFree( Handle(), 0, ptr );
	freeResult;
	assert( freeResult != 0 );
#endif
	allocationCount--;
}

inline void* CHeapAllocator::Allocate( int size )
{
#ifdef _DEBUG
	return Allocate( size, "", 0 );
#else 
	void* result = ::HeapAlloc( Handle(), 0, size );
	checkMemoryError( result != nullptr );
	totalSize += size;
	allocationCount++;
	return result;
#endif
}

inline void CHeapAllocator::Free( void* ptr )
{
	assert( heap != 0 );
	if( ptr == nullptr ) {
		return;
	}
#ifdef _DEBUG
	RelibInternal::CDebugMemoryBlock* block = RelibInternal::CDebugMemoryBlock::CreateFromData( ptr );
	block->SetFree( L"heap manager" );
	totalSize -= block->DataSize();
	const BOOL freeResult = ::HeapFree( Handle(), 0, block );
	assert( freeResult != 0 );
#else
	const SIZE_T blockSize = ::HeapSize( Handle(), 0, ptr );
	if( blockSize != ( SIZE_T )( -1 ) ) {
		totalSize -= numeric_cast<int>( blockSize );
	}
	const BOOL freeResult = ::HeapFree( Handle(), 0, ptr );
	freeResult;
	assert( freeResult != 0 );
#endif
	allocationCount--;
}

//////////////////////////////////////////////////////////////////////////

template<class Allocator>
CRawBuffer CThreadSafeAllocator<Allocator>::AllocateSized( int size )
{
	CCriticalSectionLock lock( section );
	return allocator.Allocate( size );
}

template<class Allocator>
void* CThreadSafeAllocator<Allocator>::Allocate( int size )
{
	CCriticalSectionLock lock( section );
	return allocator.Allocate( size );
}

template<class Allocator>
void CThreadSafeAllocator<Allocator>::Free( CRawBuffer ptr )
{
	CCriticalSectionLock lock( section );
	allocator.Free( ptr );
}

template<class Allocator>
void CThreadSafeAllocator<Allocator>::Free( void* ptr )
{
	CCriticalSectionLock lock( section );
	allocator.Free( ptr );
}

#ifdef _DEBUG
template<class Allocator>
CRawBuffer CThreadSafeAllocator<Allocator>::AllocateSized( int size, const char* fileName, int line )
{
	CCriticalSectionLock lock( section );
	return allocator.Allocate( size, fileName, line );
}

template<class Allocator>
void* CThreadSafeAllocator<Allocator>::Allocate( int size, const char* fileName, int line )
{
	CCriticalSectionLock lock( section );
	return allocator.Allocate( size, fileName, line );
}
#endif

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.