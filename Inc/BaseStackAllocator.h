#pragma once
#include <Redefs.h>
#include <Remath.h>
#include <MemoryUtils.h>
#include <AllocationStrategy.h>
#include <CompressedPair.h>

namespace Relib {

template <class PageAllocator = CVirtualPageAllocator, class PageSizeStrategy = CStaticByteResizeStrategy<64 * 1024>>
struct CStackState;
template <class PageAllocator = CVirtualPageAllocator, class PageSizeStrategy = CStaticByteResizeStrategy<64 * 1024>>
class CStackStateOwner;
//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

// Basic stack functionality.
template <class PageAllocator, class PageSizeStrategy>
class CBaseStackAllocator : private CAllocationStrategy<PageAllocator> {
public:
	// Stack memory block.
	struct CStackPage {
		CStackPage* Next;	// pointer to the next page.
		int AllocatedSize;	// size of the page.
		int UsedDataSize;	// data that is currently used.
		alignas( AllocatorAlignment ) BYTE Data[1];	// actual data.
	};

	CBaseStackAllocator() = default;
	explicit CBaseStackAllocator( PageAllocator& allocator );
	CBaseStackAllocator( CBaseStackAllocator<PageAllocator, PageSizeStrategy>&& other );
	CBaseStackAllocator& operator=( CBaseStackAllocator<PageAllocator, PageSizeStrategy>&& other );
	~CBaseStackAllocator();

	// Access to the current page for memory checking.
	CStackPage* GetCurrentPage()
		{ return pageStrategyAndCurrentPage.Second(); }

	// Free page memory.
	void FreePages();

	// Reset the memory without dumping pages.
	void ResetWithoutDump();
	void ResetWithoutDump( CStackState<PageAllocator, PageSizeStrategy> state );
	
	CStackState<PageAllocator, PageSizeStrategy> GetState() const;

	// Allocation method.
	BYTE* Allocate( int size, int aligment );

private:
	static const int pageHeaderSize = offsetof( CStackPage, Data );
	typedef RelibInternal::CPageResizeStrategy<PageSizeStrategy, pageHeaderSize> TPageSizeStrategy;
	// Page that is currently being filled.
	CCompressedPair<TPageSizeStrategy, CStackPage*> pageStrategyAndCurrentPage{ {}, nullptr };
	// First page that is free from any data.
	CStackPage* firstFreePage = nullptr;

	TPageSizeStrategy& getPageStrategy()
		{ return pageStrategyAndCurrentPage.First(); }
	CStackPage* getCurrentPage() const
		{ return pageStrategyAndCurrentPage.Second(); }
	CStackPage*& getCurrentPage()
		{ return pageStrategyAndCurrentPage.Second(); }
		
	void detachData();
	void freePages( CStackPage* firstPage, CStackPage* lastPage, int dataSizeInLastBlock );

	void releaseMemory( CStackPage* startPage );
	bool canAllocateInCurrentPage( int size, int alignedOffset );
	void allocateNewPage( int size );
};

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// Stack state.
template <class PageAllocator, class PageSizeStrategy>
struct CStackState {
	typename RelibInternal::CBaseStackAllocator<PageAllocator, PageSizeStrategy>::CStackPage* Block;	// current page.
	int DataSizeInBlock;	// position in the page.
};

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

template <class PageAllocator, class PageSizeStrategy>
CBaseStackAllocator<PageAllocator, PageSizeStrategy>::CBaseStackAllocator( PageAllocator& allocator ) :
	CAllocationStrategy( allocator )
{
}

template <class PageAllocator, class PageSizeStrategy>
CBaseStackAllocator<PageAllocator, PageSizeStrategy>::CBaseStackAllocator( CBaseStackAllocator<PageAllocator, PageSizeStrategy>&& other ) :
	pageStrategyAndCurrentPage( move( other.pageStrategyAndCurrentPage ) ),
	firstFreePage( other.firstFreePage )
{
	other.detachData();
}

template <class PageAllocator, class PageSizeStrategy>
void CBaseStackAllocator<PageAllocator, PageSizeStrategy>::detachData()
{
	getPageStrategy() = TPageSizeStrategy{};
	getCurrentPage() = nullptr;
	firstFreePage = nullptr;
}

template <class PageAllocator, class PageSizeStrategy>
CBaseStackAllocator<PageAllocator, PageSizeStrategy>& CBaseStackAllocator<PageAllocator, PageSizeStrategy>::operator=( CBaseStackAllocator<PageAllocator, PageSizeStrategy>&& other )
{
	releaseMemory( getCurrentPage() );
	releaseMemory( firstFreePage );
	pageStrategyAndCurrentPage = move( other.pageStrategyAndCurrentPage );
	firstFreePage = other.firstFreePage;
	other.detachData();
	return *this;
}

template <class PageAllocator, class PageSizeStrategy>
CBaseStackAllocator<PageAllocator, PageSizeStrategy>::~CBaseStackAllocator()
{
	releaseMemory( getCurrentPage() );
	releaseMemory( firstFreePage );
}

template <class PageAllocator, class PageSizeStrategy>
void CBaseStackAllocator<PageAllocator, PageSizeStrategy>::FreePages()
{
	releaseMemory( getCurrentPage() );
	releaseMemory( firstFreePage );
	detachData();
}

template <class PageAllocator, class PageSizeStrategy>
void CBaseStackAllocator<PageAllocator, PageSizeStrategy>::ResetWithoutDump()
{
	freePages( getCurrentPage(), 0, 0 );
	getCurrentPage() = 0;
}

template <class PageAllocator, class PageSizeStrategy>
void CBaseStackAllocator<PageAllocator, PageSizeStrategy>::ResetWithoutDump( CStackState<PageAllocator, PageSizeStrategy> state )
{
	freePages( getCurrentPage(), state.Block, state.DataSizeInBlock );
	getCurrentPage() = state.Block;
}

template <class PageAllocator, class PageSizeStrategy>
CStackState<PageAllocator, PageSizeStrategy> CBaseStackAllocator<PageAllocator, PageSizeStrategy>::GetState() const
{
	CStackState<PageAllocator, PageSizeStrategy> state;
	state.Block = getCurrentPage();
	state.DataSizeInBlock = ( getCurrentPage() != nullptr ) ? getCurrentPage()->UsedDataSize : 0;
	return state;
}

template <class PageAllocator, class PageSizeStrategy>
BYTE* CBaseStackAllocator<PageAllocator, PageSizeStrategy>::Allocate( int size, int alignment )
{
	assert( size >= 0 );

	const int alignedOffset = getCurrentPage() == nullptr ? 0 : CeilTo( getCurrentPage()->UsedDataSize, alignment );
	BYTE* destPtr;
	if( !canAllocateInCurrentPage( size, alignedOffset ) ) {
		const int pageSize = getPageStrategy().NextPageSize();
		allocateNewPage( max( pageSize, size + pageHeaderSize ) );
		destPtr = getCurrentPage()->Data;
		getCurrentPage()->UsedDataSize = size;
	} else {
		destPtr = getCurrentPage()->Data + alignedOffset;
		getCurrentPage()->UsedDataSize = alignedOffset + size;
	}

	assert( reinterpret_cast<size_t>( destPtr ) % alignment == 0 );
	assert( getCurrentPage()->AllocatedSize >= getCurrentPage()->UsedDataSize );
	return destPtr;
}

template <class PageAllocator, class PageSizeStrategy>
bool CBaseStackAllocator<PageAllocator, PageSizeStrategy>::canAllocateInCurrentPage( int size, int alignedOffset )
{
	return getCurrentPage() != nullptr && size + alignedOffset <= getCurrentPage()->AllocatedSize;
}

// Free the memory of the chain of block starting from firstBlock.
template <class PageAllocator, class PageSizeStrategy>
void CBaseStackAllocator<PageAllocator, PageSizeStrategy>::releaseMemory( CStackPage* firstPage )
{
	CStackPage* next = nullptr;
	for( CStackPage* page = firstPage; page != nullptr; page = next ) {
		next = page->Next;
		this->StrategyFree( { page, page->AllocatedSize } );
	}
}

template <class PageAllocator, class PageSizeStrategy>
void CBaseStackAllocator<PageAllocator, PageSizeStrategy>::freePages( CStackPage* firstPage, CStackPage* lastPage, int dataSizeInLastPage )
{
	assert( lastPage != nullptr || dataSizeInLastPage == 0 );

	CStackPage* next = 0;
	for( CStackPage* block = firstPage; block != lastPage; block = next ) {
		next = block->Next;
		block->Next = firstFreePage;
		firstFreePage = block;
	}

	if( lastPage != nullptr ) {
		lastPage->UsedDataSize = dataSizeInLastPage;
	}
}

template <class PageAllocator, class PageSizeStrategy>
void CBaseStackAllocator<PageAllocator, PageSizeStrategy>::allocateNewPage( int sizeWithHeader )
{
	CStackPage* newPage;
	const int dataSize = sizeWithHeader - pageHeaderSize;
	if( firstFreePage == nullptr || dataSize > firstFreePage->AllocatedSize ) {
		auto ptr = RELIB_STRATEGY_ALLOCATE( sizeWithHeader );
		newPage = reinterpret_cast<CStackPage*>( ptr );
		newPage->AllocatedSize = dataSize;
	} else {
		newPage = firstFreePage;
		firstFreePage = firstFreePage->Next;
	}
	newPage->UsedDataSize = 0;
	newPage->Next = getCurrentPage();
	getCurrentPage() = newPage;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

