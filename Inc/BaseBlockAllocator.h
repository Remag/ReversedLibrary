#pragma once
#include <AllocationStrategy.h>
#include <CompressedPair.h>
#include <MemoryUtils.h>

namespace Relib {

namespace RelibInternal {

// Allocated memory page.
template <int alignment>
struct CBlockMemoryPage {
	CBlockMemoryPage<alignment>* Next;	// next page.
	int PageSize;	// total byte size of the page.
	alignas( alignment ) BYTE PageData[1];	// actual data.
};

//////////////////////////////////////////////////////////////////////////

// General purpose block allocator.
// Bock memory is returned at blockOffset of its size.
// PageAllocator must provide pages that are aligned at least to blockAlignment.
// PageSizeStrategy must provide a size for the next page in its NextPageSize method.
template <int blockSize, int blockAlignment, int blockOffset, class PageAllocator, class PageSizeStrategy>
class CBaseBlockAllocator : private CAllocationStrategy<PageAllocator> {
public:
	// Header of a free block.
	struct CFreeBlockHeader {
		CFreeBlockHeader* Next;	// pointer to the next free block in the list.
	};

	static const int physicalBlockAlignment = max( blockAlignment, static_cast<int>( alignof( CFreeBlockHeader ) ) );
	typedef RelibInternal::CBlockMemoryPage<physicalBlockAlignment> TPage;

	template <class... Args>
	explicit CBaseBlockAllocator( Args&&... allocatorArgs ); 
	~CBaseBlockAllocator();

	CBaseBlockAllocator( CBaseBlockAllocator&& other );
	CBaseBlockAllocator& operator=( CBaseBlockAllocator&& other );

	static int BlockSize()
		{ return blockDataSize; }
	static int GetPhysicalBlockSize()
		{ return physicalBlockSize; }

	TPage* GetCurrentPage()
		{ return getCurrentPage(); }

	CAllocationStrategy<PageAllocator>& GetPageAllocationStrategy()
		{ return *this; }

	int GetTotalSize() const;

	// Ensure that the allocator has at least byteCount allocated memory.
	void Reserve( int byteCount );
	// Set the size of the next page directly. Only dynamic page strategies support this operation.
	void HintNextPageSize( int newSize );

	void* AllocateBlock();
	void Free( void* buffer );

	void Reset();

private:
	// Real size with alignment / debug information.
	static const int blockDataSize = max( blockSize, static_cast<int>( sizeof( CFreeBlockHeader ) ) );
	static const int physicalBlockSize = CeilTo( blockSize, physicalBlockAlignment );
	static const int pageHeaderSize = offsetof( TPage, PageData );

	typedef RelibInternal::CPageResizeStrategy<PageSizeStrategy, pageHeaderSize> TPageSizeStrategy;
	RelibInternal::CCompressedPair<TPageSizeStrategy, TPage*> pageStrategyAndCurrentPage;
	CFreeBlockHeader* firstBlock = nullptr;

	TPageSizeStrategy& getPageStrategy()
		{ return pageStrategyAndCurrentPage.First(); }
	TPage*& getCurrentPage()
		{ return pageStrategyAndCurrentPage.Second(); }
	TPage* getCurrentPage() const
		{ return pageStrategyAndCurrentPage.Second(); }

	void allocatePage();
	void allocatePage( int pageSize );
	void releaseMemory( TPage* firstPage );

	// Copying is prohibited.
	CBaseBlockAllocator( const CBaseBlockAllocator& other ) = delete;
	void operator=( const CBaseBlockAllocator& other ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <int blockSize, int blockAlignment, int blockOffset, class PageAllocator, class PageSizeStrategy>
template <class... Args>
CBaseBlockAllocator<blockSize, blockAlignment, blockOffset, PageAllocator, PageSizeStrategy>::CBaseBlockAllocator( Args&&... allocatorArgs ) : 
	CAllocationStrategy<PageAllocator>( forward<Args>( allocatorArgs )... ),
	pageStrategyAndCurrentPage( {}, nullptr )
{
	staticAssert( offsetof( TPage, PageData ) % physicalBlockAlignment == 0 );
	staticAssert( blockSize > 0 );
}

template <int blockSize, int blockAlignment, int blockOffset, class PageAllocator, class PageSizeStrategy>
CBaseBlockAllocator<blockSize, blockAlignment, blockOffset, PageAllocator, PageSizeStrategy>::CBaseBlockAllocator( CBaseBlockAllocator&& other ) :
	CAllocationStrategy<PageAllocator>( other ),
	pageStrategyAndCurrentPage( other.pageStrategyAndCurrentPage ),
	firstBlock( other.firstBlock )
{
	other.firstBlock = nullptr;
	other.getCurrentPage() = nullptr;
	other.getPageStrategy() = TPageSizeStrategy{};
}

template <int blockSize, int blockAlignment, int blockOffset, class PageAllocator, class PageSizeStrategy>
CBaseBlockAllocator<blockSize, blockAlignment, blockOffset, PageAllocator, PageSizeStrategy>& CBaseBlockAllocator<blockSize, blockAlignment, blockOffset, PageAllocator, PageSizeStrategy>::operator=( 
	CBaseBlockAllocator&& other )
{
	Reset();
	pageStrategyAndCurrentPage = other.pageStrategyAndCurrentPage;
	firstBlock = other.firstBlock;
	other.firstBlock = nullptr;
	other.getCurrentPage() = nullptr;
	other.getPageStrategy() = TPageSizeStrategy{};
	return *this;
}

template <int blockSize, int blockAlignment, int blockOffset, class PageAllocator, class PageSizeStrategy>
CBaseBlockAllocator<blockSize, blockAlignment, blockOffset, PageAllocator, PageSizeStrategy>::~CBaseBlockAllocator()
{
	releaseMemory( getCurrentPage() );
}

template <int blockSize, int blockAlignment, int blockOffset, class PageAllocator, class PageSizeStrategy>
int CBaseBlockAllocator<blockSize, blockAlignment, blockOffset, PageAllocator, PageSizeStrategy>::GetTotalSize() const
{
	int totalPageSize = 0;
	for( TPage* page = getCurrentPage(); page != nullptr; page = page->Next ) {
		totalPageSize += page->PageSize;
	}

	return totalPageSize;
}

template <int blockSize, int blockAlignment, int blockOffset, class PageAllocator, class PageSizeStrategy>
void CBaseBlockAllocator<blockSize, blockAlignment, blockOffset, PageAllocator, PageSizeStrategy>::Reserve( int byteCount )
{
	const int totalPageSize = GetTotalSize();
	if( totalPageSize < byteCount ) {
		allocatePage( byteCount - totalPageSize );
	}
}

template <int blockSize, int blockAlignment, int blockOffset, class PageAllocator, class PageSizeStrategy>
void CBaseBlockAllocator<blockSize, blockAlignment, blockOffset, PageAllocator, PageSizeStrategy>::HintNextPageSize( int newSize )
{
	getPageStrategy().HintNextPageSize( newSize );
}

template <int blockSize, int blockAlignment, int blockOffset, class PageAllocator, class PageSizeStrategy>
void* CBaseBlockAllocator<blockSize, blockAlignment, blockOffset, PageAllocator, PageSizeStrategy>::AllocateBlock()
{
	if( firstBlock == nullptr ) {
		allocatePage();
	} 
	void* result = firstBlock;
	assert( result != nullptr );
	firstBlock = firstBlock->Next;
	return result;
}

template <int blockSize, int blockAlignment, int blockOffset, class PageAllocator, class PageSizeStrategy>
void CBaseBlockAllocator<blockSize, blockAlignment, blockOffset, PageAllocator, PageSizeStrategy>::allocatePage()
{
	const int pageSize = getPageStrategy().NextPageSize();
	allocatePage( pageSize );
}

template <int blockSize, int blockAlignment, int blockOffset, class PageAllocator, class PageSizeStrategy>
void CBaseBlockAllocator<blockSize, blockAlignment, blockOffset, PageAllocator, PageSizeStrategy>::allocatePage( int pageSize )
{
	staticAssert( pageHeaderSize % physicalBlockAlignment == 0 );
	assert( firstBlock == nullptr );
	auto pageBuffer = RELIB_STRATEGY_ALLOCATE_SIZED( pageSize );
	TPage* newPage = reinterpret_cast<TPage*>( pageBuffer.Ptr() );

	newPage->Next = getCurrentPage();
	const int pageDataSize = pageBuffer.Size() - pageHeaderSize;
	newPage->PageSize = pageDataSize;
	getCurrentPage() = newPage;

	// Separate the new page into blocks.
	BYTE* dataPtr = newPage->PageData;
	const int maxPagePos = pageDataSize - physicalBlockSize;
	firstBlock = reinterpret_cast<CFreeBlockHeader*>( dataPtr + blockOffset );
	CFreeBlockHeader* block = firstBlock;
	for( int pagePos = physicalBlockSize; pagePos <= maxPagePos; pagePos += physicalBlockSize ) {
		block->Next = reinterpret_cast<CFreeBlockHeader*>( dataPtr + pagePos + blockOffset );
		block = block->Next;
	}
	block->Next = nullptr;
}

template <int blockSize, int blockAlignment, int blockOffset, class PageAllocator, class PageSizeStrategy>
void CBaseBlockAllocator<blockSize, blockAlignment, blockOffset, PageAllocator, PageSizeStrategy>::Reset()
{
	releaseMemory( getCurrentPage() );

	firstBlock = nullptr;
	getPageStrategy() = TPageSizeStrategy{};
	getCurrentPage() = nullptr;
}

template <int blockSize, int blockAlignment, int blockOffset, class PageAllocator, class PageSizeStrategy>
void CBaseBlockAllocator<blockSize, blockAlignment, blockOffset, PageAllocator, PageSizeStrategy>::Free( void* ptr )
{
	assert( ptr != nullptr );
	CFreeBlockHeader* newFreeBlock = reinterpret_cast<CFreeBlockHeader*>( ptr );
	newFreeBlock->Next = firstBlock;
	firstBlock = newFreeBlock;
}

template <int blockSize, int blockAlignment, int blockOffset, class PageAllocator, class PageSizeStrategy>
void CBaseBlockAllocator<blockSize, blockAlignment, blockOffset, PageAllocator, PageSizeStrategy>::releaseMemory( TPage* firstPage )
{
	TPage* next = nullptr;
	for( TPage* page = firstPage; page != nullptr; page = next ) {
		next = page->Next;
		CAllocationStrategy<PageAllocator>::StrategyFree( { page, page->PageSize + pageHeaderSize } );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

