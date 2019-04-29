#pragma once
#include <Allocator.h>
#include <MemoryUtils.h>
#include <RawBuffer.h>
// Memory allocators with static Alloc and Free methods.

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Allocator that uses MSVCRT runtime heap.
class CRuntimeHeap : public TStaticAllocator {
public:
	static CRawBuffer AllocateSized( int size );
	static void* Allocate( int size );
	static void Free( CRawBuffer buffer );
	static void Free( void* ptr );

#ifdef _DEBUG
	static CRawBuffer AllocateSized( int size, const char* fileName, int line );
	static void* Allocate( int size, const char* fileName, int line );
#endif
};

//////////////////////////////////////////////////////////////////////////

// Allocator that uses MSVCRT runtime heap to allocate aligned memory.
template <int alignment>
class CAlignedRuntimeHeap : public TStaticAllocator {
public:
	static CRawBuffer AllocateSized( int size );
	static void* Allocate( int size );
	static void Free( CRawBuffer buffer );
	static void Free( void* ptr );

#ifdef _DEBUG
	static CRawBuffer AllocateSized( int size, const char* fileName, int line );
	static void* Allocate( int size, const char* fileName, int line );
#endif
};

//////////////////////////////////////////////////////////////////////////

// Allocator that uses the process heap.
class REAPI CProcessHeap : public TStaticAllocator {
public:
	static CRawBuffer AllocateSized( int size );
	static void* Allocate( int size );
	static void Free( CRawBuffer ptr );
	static void Free( void* ptr );

#ifdef _DEBUG
	static CRawBuffer AllocateSized( int size, const char* fileName, int line );
	static void* Allocate( int size, const char* fileName, int line );
#endif
};

//////////////////////////////////////////////////////////////////////////

// Allocator that uses a thread local heap manager.
// The user must guarantee that memory allocated with this class does not leave the thread boundaries.
// Otherwise heap corruption will occur.
class REAPI CThreadHeap : public TStaticAllocator {
public:
	static CRawBuffer AllocateSized( int size );
	static void* Allocate( int size );
	static void Free( CRawBuffer ptr );
	static void Free( void* ptr );

#ifdef _DEBUG
	static CRawBuffer AllocateSized( int size, const char* fileName, int line );
	static void* Allocate( int size, const char* fileName, int line );
#endif
};

//////////////////////////////////////////////////////////////////////////

// Allocator that uses virtual allocation manager.
class REAPI CVirtualPageAllocator : public TStaticAllocator {
public:
	static CRawBuffer AllocateSized( int size );
	static void* Allocate( int size );
	static void Free( CRawBuffer ptr );
	static void Free( void* ptr );

#ifdef _DEBUG
	static CRawBuffer AllocateSized( int size, const char* fileName, int line );
	static void* Allocate( int size, const char* fileName, int line );
#endif
};

// Get the heap local to the calling thread.
REAPI CHeapAllocator& GetThreadHeap();

//////////////////////////////////////////////////////////////////////////

// Allocator that uses two different allocators depending on size.
template <class SmallAllocator, class LargeAllocator, int sizeCutoff>
class CSizeConditionalAllocator : public TStaticAllocator {
public:
	static void* Allocate( int size )
		{ static_assert( false, "Unsized allocation not supported for this allocator." ); return nullptr; }

	static CRawBuffer AllocateSized( int size )
		{ return size < sizeCutoff ? SmallAllocator::AllocateSized( size ) : LargeAllocator::AllocateSized( size ); }

	static void Free( CRawBuffer buffer )
		{ return buffer.Size() < sizeCutoff ? SmallAllocator::Free( buffer ) : LargeAllocator::Free( buffer ); }

#ifdef _DEBUG
	static CRawBuffer AllocateSized( int size, const char* fileName, int line )
		{ return size < sizeCutoff ? SmallAllocator::AllocateSized( size, fileName, line ) : LargeAllocator::AllocateSized( size, fileName, line ); }
#endif
};

//////////////////////////////////////////////////////////////////////////

inline CRawBuffer CRuntimeHeap::AllocateSized( int size )
{
	return { Allocate( size ), size };
}

inline void* CRuntimeHeap::Allocate( int size )
{
	void* result = ::malloc( size );
	checkMemoryError( result != nullptr );
	return result;
}

inline void CRuntimeHeap::Free( CRawBuffer buffer )
{
	::free( buffer.Ptr() );
}

inline void CRuntimeHeap::Free( void* ptr )
{
	::free( ptr );
}

#ifdef _DEBUG

inline CRawBuffer CRuntimeHeap::AllocateSized( int size, const char* fileName, int line )
{
	return { Allocate( size, fileName, line ), size };
}

inline void* CRuntimeHeap::Allocate( int size, const char* fileName, int line )
{
	void* result = _malloc_dbg( size, _NORMAL_BLOCK, fileName, line );
	checkMemoryError( result != nullptr );
	return result;
}

#endif

//////////////////////////////////////////////////////////////////////////

template <int alignment>
CRawBuffer CAlignedRuntimeHeap<alignment>::AllocateSized( int size )
{
	return { AllocateSized( size ), size };
}

template <int alignment>
void* CAlignedRuntimeHeap<alignment>::Allocate( int size )
{
	void* result = ::_aligned_malloc( size, alignment );
	checkMemoryError( result != nullptr );
	return result;
}

template <int alignment>
void CAlignedRuntimeHeap<alignment>::Free( CRawBuffer buffer )
{
	::_aligned_free( buffer.Ptr() );
}

template <int alignment>
void CAlignedRuntimeHeap<alignment>::Free( void* ptr )
{
	::_aligned_free( ptr );
}

#ifdef _DEBUG

template <int alignment>
CRawBuffer CAlignedRuntimeHeap<alignment>::AllocateSized( int size, const char* fileName, int line )
{
	return { Allocate( size, fileName, line ), size };
}

template <int alignment>
void* CAlignedRuntimeHeap<alignment>::Allocate( int size, const char* fileName, int line )
{
	void* result = _aligned_malloc_dbg( size, alignment, fileName, line );
	checkMemoryError( result != nullptr );
	return result;
}

#endif

//////////////////////////////////////////////////////////////////////////

} // namespace Relib.