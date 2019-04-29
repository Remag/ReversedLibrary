#pragma once
#include <Redefs.h>
#include <GeneralBlockAllocator.h>
#include <StaticAllocators.h>
#include <DynamicAllocators.h>
#include <ObjectPool.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Memory allocator for strings.
// Allocates smaller strings in thread local blocks.
class REAPI CStringAllocator {
public:
	CStringAllocator();

	auto& GetSmallPool()
		{ return smallBlockAllocators; }
	auto& GetMediumPool()
		{ return mediumBlockAllocators; }
	auto& GetLargePool()
		{ return largeBlockAllocators; }

	// Allocate memory of at least realSize.
	CRawBuffer AllocateSized( int realSize );
	// Free allocatedSize bytes of ptr memory.
	void Free( CRawBuffer buffer );

#ifdef _DEBUG
	CRawBuffer AllocateSized( int size, const char* fileName, int line );
#endif

private:
	static const int smallBlockSize = 32;
	static const int mediumBlockSize = 64;
	static const int largeBlockSize = 128;

	template <int blockSize>
	using TStringBlockAllocator = CGeneralBlockAllocator<blockSize, alignof( wchar_t ), CProcessHeap, CDynamicByteResizeStrategy<16 * blockSize>>;
	template <int blockSize>
	using TStringAllocatorPool = RelibInternal::CObjectPool<TStringBlockAllocator<blockSize>, 4, CProcessHeap, CProcessHeap>;

	// Block allocators are stored in a global pool.
	// Every thread gets its own allocator.
	// Since strings can travel between threads, blocks from one allocator might end up being freed on another one.
	// That's why block allocators are not destroyed and are instead reused when thread ends.
	// All allocators are destroyed at program cleanup.
	TStringAllocatorPool<smallBlockSize> smallBlockAllocators;
	TStringAllocatorPool<mediumBlockSize> mediumBlockAllocators;
	TStringAllocatorPool<largeBlockSize> largeBlockAllocators;

	// Large strings are stored in a global synchronized heap.
	CHeapAllocator heapManager;
};

//////////////////////////////////////////////////////////////////////////

REAPI CStringAllocator& GetStringAllocator();

// Static allocator that uses global string allocator.
class CStaticStringAllocator : public TStaticAllocator {
public:
	static void* Allocate( int size )
		{ return AllocateSized( size ).Ptr(); }
	static CRawBuffer AllocateSized( int size )
		{ return GetStringAllocator().AllocateSized( size ); }
	static void Free( CRawBuffer buffer )
		{ return GetStringAllocator().Free( buffer ); }

#ifdef _DEBUG
	static void* Allocate( int size, const char* fileName, int line )
		{ return AllocateSized( size, fileName, line ).Ptr(); }
	static CRawBuffer AllocateSized( int size, const char* fileName, int line )
		{ return GetStringAllocator().AllocateSized( size, fileName, line ); }
#endif
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

