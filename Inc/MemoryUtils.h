#pragma once
#include <Remath.h>
#include <CriticalSection.h>

//////////////////////////////////////////////////////////////////////////

namespace Relib {

// Default alignment used for dynamic allocators.
static const int AllocatorAlignment = MEMORY_ALLOCATION_ALIGNMENT;
//////////////////////////////////////////////////////////////////////////

// Private allocators and functions used by the library.
namespace RelibInternal {

static const int DefaultMemoryPageSize = 64 * 1024;

// Print the information about memory errors.
// Strings and message handlers might be unusable by the time of this report, only standard output is used.
void REAPI reportMemoryError( const wchar_t* format, ... );

//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// Memory block that is created by allocators in debug mode. Contains additional information for memory leaks detection.
class REAPI CDebugMemoryBlock {
public:
	CDebugMemoryBlock( int dataSize, const char* fileName, int line );

	void ResetData( int dataSize, const char* fileName, int line, const wchar_t* allocatorName );

	void* Data()
		{ return data; }
	int DataSize() const
		{ return dataSize; }

	bool IsValidTag() const;
	void SetFree( const wchar_t* allocatorName );
	void CheckFree( const wchar_t* allocatorName ) const;
	// Fill block with trash to make corruption detection easier.
	void FillWithTrash();

	// Size of the debug globe with dataSize of actual data.
	static constexpr int DebugBlockSize( int dataSize );
	static constexpr int GetDataOffset();
	static CDebugMemoryBlock* CreateFromData( void* ptr );
	// Check if the region has been freed correctly.
	static void CheckRegion( const wchar_t* allocatorName, const BYTE* startPtr, long long size );

	// Class needs access to data to get its offset in constexpr-usable format.
	friend struct CDebugMemoryBlockOffsetOf;

private:
	// Tag that specifies current status of the block.
	int memoryTag;
	// Size of the block excluding the header and alignment.
	int dataSize;
	// Source of the allocation.
	const char* fileName;
	// Line of the allocation.
	int line;
	// Allocation count.
	int allocationNumber;
	// Useful data.
	alignas( AllocatorAlignment ) BYTE data[1];

	
	void setInitialValues();
	int dataSizeWithPadding() const;
	void checkDataPadding( const wchar_t* allocatorName ) const;
};

struct CDebugMemoryBlockOffsetOf {
	static const int Result = offsetof( CDebugMemoryBlock, data );
};

//////////////////////////////////////////////////////////////////////////

// Debug block used by virtual memory allocator.
class REAPI CVirtualMemoryDebugBlock {
public:
	CVirtualMemoryDebugBlock( CVirtualMemoryDebugBlock* next, int size, const char* fileName, int line );

	void* Data()
		{ return block.Data(); }
	CVirtualMemoryDebugBlock* Next()
		{ return next; }

	void CheckFree();
	void SetFree();

	static int DebugBlockSize( int dataSize );
	static CVirtualMemoryDebugBlock* CreateFromData( void* ptr );

private:
	CVirtualMemoryDebugBlock* prev;
	CVirtualMemoryDebugBlock* next;
	CDebugMemoryBlock block;
};

//////////////////////////////////////////////////////////////////////////

inline constexpr int CDebugMemoryBlock::DebugBlockSize( int dataSize )
{
	staticAssert( CDebugMemoryBlockOffsetOf::Result % AllocatorAlignment == 0 );
	// Additional AllocatorAlignment bytes are added to the end of data to detect corruption.
	return CDebugMemoryBlockOffsetOf::Result + CeilTo( dataSize, AllocatorAlignment ) + AllocatorAlignment;
}

inline constexpr int CDebugMemoryBlock::GetDataOffset()
{
	return CDebugMemoryBlockOffsetOf::Result;
}

#endif

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// Page resize strategies.

// Allocate a single page size with the given value.
template <int pageSize>
class CStaticByteResizeStrategy {};

// Allocated a single page size for the blockCount of given block sizes.
template <int blockSize, int blockCount>
class CStaticBlockResizeStrategy {};

// Allocated a single page size that can hold a given amount of elements.
template <class Elem, int startElemCount>
class CStaticElemResizeStrategy {};

// Allocate an exponentially growing page size starting from the given value in bytes.
template <int startPageSize>
class CDynamicByteResizeStrategy {};

// Allocate an exponentially growing page size starting with the value that can hold a given amount of blocks.
template <int blockSize, int blockCount>
class CDynamicBlockResizeStrategy {};

// Allocate an exponentially growing page size starting with the value that can hold a given amount of elements.
template <class Elem, int startElemCount>
class CDynamicElemResizeStrategy {};

typedef CStaticByteResizeStrategy<RelibInternal::DefaultMemoryPageSize> TDefaultPageResizeStrategy;

namespace RelibInternal {

// Resize strategy implementations with page header size information.
template <class Strategy, int pageHeaderSize>
class CPageResizeStrategy {};

//////////////////////////////////////////////////////////////////////////

template <int pageSize, int pageHeaderSize>
class CPageResizeStrategy<CStaticByteResizeStrategy<pageSize>, pageHeaderSize> {
public:
	static constexpr int NextPageSize()
		{ return pageSize; }
	static constexpr void Reset() {}
};

//////////////////////////////////////////////////////////////////////////

template <int elemSize, int elemCount, int pageHeaderSize>
class CPageResizeStrategy<CStaticBlockResizeStrategy<elemSize, elemCount>, pageHeaderSize> {
public:
	static const int PageByteSize = elemCount * elemSize;

	static int NextPageSize()
		{ return PageByteSize + pageHeaderSize; }
	static void Reset() {}
};

//////////////////////////////////////////////////////////////////////////

template <class Elem, int elemCount, int pageHeaderSize>
class CPageResizeStrategy<CStaticElemResizeStrategy<Elem, elemCount>, pageHeaderSize> : public CPageResizeStrategy<CStaticBlockResizeStrategy<sizeof( Elem ), elemCount>, pageHeaderSize> {};

//////////////////////////////////////////////////////////////////////////

template <int startPageSize, int pageHeaderSize>
class CPageResizeStrategy<CDynamicByteResizeStrategy<startPageSize>, pageHeaderSize> {
public:
	int NextPageSize();
	void HintNextPageSize( int pageSize )
		{ currentPageSize = min( pageSize, maxPageByteSize ); }

	void Reset()
		{ currentPageSize = startPageSize; }

private:
	static const int maxPageByteSize = 64 * 1024;

	int currentPageSize = startPageSize;
};

template <int startPageSize, int pageHeaderSize>
int CPageResizeStrategy<CDynamicByteResizeStrategy<startPageSize>, pageHeaderSize>::NextPageSize()
{
	const int result = max( currentPageSize, pageHeaderSize );
	HintNextPageSize( currentPageSize * 2 );
	return result;
}

//////////////////////////////////////////////////////////////////////////

template <int blockSize, int startBlockCount, int pageHeaderSize>
class CPageResizeStrategy<CDynamicBlockResizeStrategy<blockSize, startBlockCount>, pageHeaderSize> {
public:
	int NextPageSize();
	void HintNextPageSize( int pageSize )
		{ currentPageSize = min( pageSize, maxPageByteSize - pageHeaderSize ); }

	void Reset()
		{ currentPageSize = minPageByteSize; }

private:
	static const int minPageByteSize = blockSize * startBlockCount;
	static const int maxPageByteSize = 64 * 1024;

	int currentPageSize = minPageByteSize;
};

template <int blockSize, int startBlockCount, int pageHeaderSize>
int CPageResizeStrategy<CDynamicBlockResizeStrategy<blockSize, startBlockCount>, pageHeaderSize>::NextPageSize()
{
	const int result = pageHeaderSize + currentPageSize;
	HintNextPageSize( currentPageSize * 2 );
	return result;
}

//////////////////////////////////////////////////////////////////////////

template <class Elem, int startElemCount, int pageHeaderSize>
class CPageResizeStrategy<CDynamicElemResizeStrategy<Elem, startElemCount>, pageHeaderSize> : public CPageResizeStrategy<CDynamicBlockResizeStrategy<sizeof( Elem ), startElemCount>, pageHeaderSize> {};

}	// namespace RelibInternal.

////////////////////////////////////////////////////////////////////////////

// Memory error handling functions.
void REAPI ThrowMemoryException();
void REAPI checkMemoryError( bool condition );

//////////////////////////////////////////////////////////////////////////

// Allocation macros.

#ifdef _DEBUG
// Manual allocation. Memory should be freed with the Free method.
#define RELIB_ALLOCATE( allocator, size ) ( allocator ).Allocate( ( size ), __FILE__, __LINE__ )
#define RELIB_STATIC_ALLOCATE( staticAllocator, size ) ( staticAllocator::Allocate )( ( size ), __FILE__, __LINE__ )
#define RELIB_STRATEGY_ALLOCATE( size ) this->StrategyAllocate( ( size ), __FILE__, __LINE__ )
#define RELIB_STRATEGY_ALLOCATE_SIZED( size ) this->StrategyAllocateSized( ( size ), __FILE__, __LINE__ )
#define RELIB_EXTERNAL_STRATEGY_ALLOCATE_SIZED( strategy, size ) ( strategy ).StrategyAllocateSized( ( size ), __FILE__, __LINE__ )
#else
#define RELIB_ALLOCATE( allocator, size ) ( allocator ).Allocate( ( size ) )
#define RELIB_STATIC_ALLOCATE( staticAllocator, size ) ( staticAllocator::Allocate )( ( size ) )
#define RELIB_STRATEGY_ALLOCATE( size ) this->StrategyAllocate( ( size ) )
#define RELIB_STRATEGY_ALLOCATE_SIZED( size ) this->StrategyAllocateSized( ( size ) )
#define RELIB_EXTERNAL_STRATEGY_ALLOCATE_SIZED( strategy, size ) ( strategy ).StrategyAllocateSized( ( size ) )
#endif

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.