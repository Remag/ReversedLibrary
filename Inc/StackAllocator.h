#pragma once
#include <BaseStackAllocator.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Stack memory manager.
template <class PageAllocator = CVirtualPageAllocator, class PageSizeStrategy = CStaticByteResizeStrategy<64 * 1024>>
class CStackAllocator : public TGlobalAllocator {
public:
	typedef PageAllocator TPageAllocator;
	typedef PageSizeStrategy TPageSizeStrategy;

	CStackAllocator() = default;
	~CStackAllocator();

	// Free all the memory.
	void FreePages();

	void Reset();
	// Reset to the given state.
	void Reset( const CStackState<PageAllocator, PageSizeStrategy>& state );
	void ResetWithoutDump();
	void ResetWithoutDump( const CStackState<PageAllocator, PageSizeStrategy>& state );

	CStackState<PageAllocator, PageSizeStrategy> GetState() const
		{ return baseAllocator.GetState(); }

	void* AllocateAligned( int size, int alignment );

	void* Allocate( int size );
	CRawBuffer AllocateSized( int size );
	void Free( CRawBuffer buffer );
	void Free( void* buffer );

#ifdef _DEBUG
	void* Allocate( int size, const char* fileName, int line );
	CRawBuffer AllocateSized( int size, const char* fileName, int line );
#endif

private:
	typedef RelibInternal::CBaseStackAllocator<PageAllocator, PageSizeStrategy> TBaseAllocator;

	TBaseAllocator baseAllocator;

	void dumpPages( const typename TBaseAllocator::CStackPage* firstPage, const typename TBaseAllocator::CStackPage* lastPage, int dataSizeInLastPage ) const;
	void fillPagesWithTrash( typename TBaseAllocator::CStackPage* firstPage, typename TBaseAllocator::CStackPage* lastPage, int dataSizeInLastPage );

	// Copying is prohibited.
	CStackAllocator( const CStackAllocator& ) = delete;
	void operator=( const CStackAllocator& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class PageAllocator, class PageSizeStrategy>
CStackAllocator<PageAllocator, PageSizeStrategy>::~CStackAllocator()
{
#ifdef _DEBUG
	dumpPages( baseAllocator.GetCurrentPage(), nullptr, 0 );
#endif
}

template <class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= CStaticByteResizeStrategy<64 * 1024>*/>
void CStackAllocator<PageAllocator, PageSizeStrategy>::FreePages()
{
#ifdef _DEBUG
	dumpPages( baseAllocator.GetCurrentPage(), nullptr, 0 );
#endif
	baseAllocator.FreePages();
}

template <class PageAllocator, class PageSizeStrategy>
void CStackAllocator<PageAllocator, PageSizeStrategy>::Reset()
{
#ifdef _DEBUG
	dumpPages( baseAllocator.GetCurrentPage(), nullptr, 0 );
	fillPagesWithTrash( baseAllocator.GetCurrentPage(), nullptr, 0 );
#endif
	baseAllocator.ResetWithoutDump();
}

template <class PageAllocator, class PageSizeStrategy>
void CStackAllocator<PageAllocator, PageSizeStrategy>::Reset( const CStackState<PageAllocator, PageSizeStrategy>& state )
{
#ifdef	_DEBUG
	dumpPages( baseAllocator.GetCurrentPage(), state.Block, state.DataSizeInBlock );
	fillPagesWithTrash( baseAllocator.GetCurrentPage(), state.Block, state.DataSizeInBlock );
#endif
	baseAllocator.ResetWithoutDump( state );
}

template <class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= CStaticByteResizeStrategy<64 * 1024>*/>
void CStackAllocator<PageAllocator, PageSizeStrategy>::ResetWithoutDump()
{
	baseAllocator.ResetWithoutDump();
}

template <class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= CStaticByteResizeStrategy<64 * 1024>*/>
void CStackAllocator<PageAllocator, PageSizeStrategy>::ResetWithoutDump( const CStackState<PageAllocator, PageSizeStrategy>& state )
{
	baseAllocator.ResetWithoutDump( state );
}

template <class PageAllocator, class PageSizeStrategy>
CRawBuffer CStackAllocator<PageAllocator, PageSizeStrategy>::AllocateSized( int size )
{
	return { Allocate( size ), size };
}

template <class PageAllocator, class PageSizeStrategy>
void CStackAllocator<PageAllocator, PageSizeStrategy>::Free( CRawBuffer buffer )
{
	return Free( buffer.Ptr() );
}

#ifdef _DEBUG

template <class PageAllocator, class PageSizeStrategy>
void* CStackAllocator<PageAllocator, PageSizeStrategy>::Allocate( int size )
{
	return Allocate( size, "", 0 );
}

template <class PageAllocator, class PageSizeStrategy>
void* CStackAllocator<PageAllocator, PageSizeStrategy>::Allocate( int size, const char* fileName, int line )
{
	assert( size >= 0 );
	const int realSize = RelibInternal::CDebugMemoryBlock::DebugBlockSize( size );
	assert( realSize % AllocatorAlignment == 0 );
	void* ptr = baseAllocator.Allocate( realSize, AllocatorAlignment );

	RelibInternal::CDebugMemoryBlock* block = ::new( ptr ) RelibInternal::CDebugMemoryBlock( size, fileName, line );
	return block->Data();
}

template <class PageAllocator, class PageSizeStrategy>
CRawBuffer CStackAllocator<PageAllocator, PageSizeStrategy>::AllocateSized( int size, const char* fileName, int line )
{
	return { Allocate( size, fileName, line ), size };
}

template <class PageAllocator, class PageSizeStrategy>
void CStackAllocator<PageAllocator, PageSizeStrategy>::Free( void* ptr )
{
	if( ptr == nullptr ) {
		return;
	}

	const wchar_t* stackManagerName = L"stack manager";
	RelibInternal::CDebugMemoryBlock* block = RelibInternal::CDebugMemoryBlock::CreateFromData( ptr );
	block->SetFree( stackManagerName );
}

// Dump the range of the pages.
// lastPage and dataSizeInLastPage mark the spot where the dumping stops.
template <class PageAllocator, class PageSizeStrategy>
void CStackAllocator<PageAllocator, PageSizeStrategy>::dumpPages( const typename TBaseAllocator::CStackPage* firstPage, const typename TBaseAllocator::CStackPage* lastPage, 
	int dataSizeInLastPage ) const
{
	assert( lastPage != nullptr || dataSizeInLastPage == 0 );
	const wchar_t* stackManagerName = L"stack manager";
	for( const TBaseAllocator::CStackPage* page = firstPage; page != lastPage; page = page->Next ) {
		RelibInternal::CDebugMemoryBlock::CheckRegion( stackManagerName, page->Data, page->UsedDataSize );
	}

	// Check the top of the last block.
	if( lastPage != 0 && dataSizeInLastPage < lastPage->UsedDataSize ) {
		RelibInternal::CDebugMemoryBlock::CheckRegion( stackManagerName, lastPage->Data + dataSizeInLastPage, lastPage->UsedDataSize - dataSizeInLastPage );
	}
}

template <class PageAllocator, class PageSizeStrategy>
void CStackAllocator<PageAllocator, PageSizeStrategy>::fillPagesWithTrash( typename TBaseAllocator::CStackPage* firstPage, typename TBaseAllocator::CStackPage* lastPage,
	int dataSizeInLastPage )
{
	assert( lastPage != nullptr || dataSizeInLastPage == 0 );

	for( typename TBaseAllocator::CStackPage* page = firstPage; page != lastPage; page = page->Next ) {
		BYTE* startPtr = page->Data;
		for( BYTE* ptr = startPtr; 
			ptr - startPtr < page->UsedDataSize;
			ptr += RelibInternal::CDebugMemoryBlock::DebugBlockSize( reinterpret_cast<RelibInternal::CDebugMemoryBlock*>( ptr )->DataSize() ) ) 
		{
			reinterpret_cast<RelibInternal::CDebugMemoryBlock*>( ptr )->FillWithTrash();
		}
	}

	// Fill the top of the last block.
	if( lastPage != nullptr && dataSizeInLastPage < lastPage->UsedDataSize ) {
		BYTE* startPtr = lastPage->Data + dataSizeInLastPage;
		for( BYTE* ptr = startPtr;
			ptr - startPtr < lastPage->AllocatedSize; 
			ptr += RelibInternal::CDebugMemoryBlock::DebugBlockSize( reinterpret_cast<RelibInternal::CDebugMemoryBlock*>( ptr )->DataSize() ) )
		{
			reinterpret_cast<RelibInternal::CDebugMemoryBlock*>( ptr )->FillWithTrash();
		}
	}
}

#else

template <class PageAllocator, class PageSizeStrategy>
void* CStackAllocator<PageAllocator, PageSizeStrategy>::Allocate( int size )
{
	return baseAllocator.Allocate( size, AllocatorAlignment );
}

template <class PageAllocator, class PageSizeStrategy>
void CStackAllocator<PageAllocator, PageSizeStrategy>::Free( void* )
{
}

#endif

template <class PageAllocator, class PageSizeStrategy>
void* CStackAllocator<PageAllocator, PageSizeStrategy>::AllocateAligned( int size, int alignment )
{
	return baseAllocator.Allocate( size, alignment );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

