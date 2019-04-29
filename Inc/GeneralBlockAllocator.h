#pragma once
#include <BaseBlockAllocator.h>
#include <Allocator.h>
#include <MemoryUtils.h>
#include <StaticAllocators.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Memory pool. Allocates blocks of given size and alignment.
// Provides constant time allocations, but the block size never changes.
template <int blockSize, int blockAlignment = AllocatorAlignment, class PageAllocator = CVirtualPageAllocator, class PageSizeStrategy = TDefaultPageResizeStrategy>
class CGeneralBlockAllocator : public TGlobalAllocator {
public:
	CGeneralBlockAllocator() = default;
	~CGeneralBlockAllocator();
	
	static int BlockSize()
		{ return blockSize; }

	// Ensure that the allocator has at least byteCount allocated memory.
	void Reserve( int byteCount )
		{ baseAllocator.Reserve( byteCount ); }

	// Free all memory.
	void Reset();
	void ResetWithoutDump();

	void* Allocate( int size );
	CRawBuffer AllocateSized( int size );
	void Free( CRawBuffer buffer );
	void Free( void* buffer );

#ifdef _DEBUG
	void* Allocate( int size, const char* fileName, int line );
	CRawBuffer AllocateSized( int size, const char* fileName, int line );
#endif

private:
	// Real sizes with debug information.
#ifdef _DEBUG
	static const int physicalBlockAlignment = max( blockAlignment, static_cast<int>( alignof( RelibInternal::CDebugMemoryBlock ) ) );
	static const int physicalBlockSize = RelibInternal::CDebugMemoryBlock::DebugBlockSize( blockSize );
	static const int blockOffset = RelibInternal::CDebugMemoryBlock::GetDataOffset();
#else
	static const int physicalBlockAlignment = blockAlignment;
	static const int physicalBlockSize = blockSize;
	static const int blockOffset = 0;
#endif

	typedef RelibInternal::CBaseBlockAllocator<physicalBlockSize, physicalBlockAlignment, blockOffset, PageAllocator, PageSizeStrategy> TBaseAllocator;
	TBaseAllocator baseAllocator; 
#ifdef _DEBUG
	// Pointer to the furthest pointer allocated in the current page.
	BYTE* latestPointerInCurrentPage = nullptr;

	static bool isPointerInPage( void* ptr, typename TBaseAllocator::TPage* page );
	void saveLatestPagePointer( BYTE* newPtr );
#endif
	void dump();

	// Copying is prohibited.
	CGeneralBlockAllocator( CGeneralBlockAllocator& ) = delete;
	void operator=( CGeneralBlockAllocator& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <int blockSize, int blockAlignment, class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= TDefaultPageResizeStrategy*/>
CGeneralBlockAllocator<blockSize, blockAlignment, PageAllocator, PageSizeStrategy>::~CGeneralBlockAllocator()
{
	dump();
}

template <int blockSize, int blockAlignment, class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= TDefaultPageResizeStrategy*/>
void CGeneralBlockAllocator<blockSize, blockAlignment, PageAllocator, PageSizeStrategy>::Reset()
{
	dump();
	ResetWithoutDump();
}

template <int blockSize, int blockAlignment /*= AllocatorAlignment*/, class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= TDefaultPageResizeStrategy*/>
void CGeneralBlockAllocator<blockSize, blockAlignment, PageAllocator, PageSizeStrategy>::ResetWithoutDump()
{
	baseAllocator.Reset();
#ifdef _DEBUG
	latestPointerInCurrentPage = nullptr;
#endif 
}

template <int blockSize, int blockAlignment, class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= TDefaultPageResizeStrategy*/>
void CGeneralBlockAllocator<blockSize, blockAlignment, PageAllocator, PageSizeStrategy>::dump()
{
#ifdef _DEBUG
	const auto currentPage = baseAllocator.GetCurrentPage();
	if( currentPage == nullptr ) {
		return;
	}

	assert( isPointerInPage( latestPointerInCurrentPage, currentPage ) );
	RelibInternal::CDebugMemoryBlock::CheckRegion( L"block allocator", currentPage->PageData, latestPointerInCurrentPage - currentPage->PageData );
	for( auto page = currentPage->Next; page != nullptr; page = page->Next ) {
		const int allocatedInPage = FloorTo( page->PageSize, baseAllocator.GetPhysicalBlockSize() );
		RelibInternal::CDebugMemoryBlock::CheckRegion( L"block allocator", page->PageData, allocatedInPage );
	}
#endif
}

template <int blockSize, int blockAlignment, class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= TDefaultPageResizeStrategy*/>
void* CGeneralBlockAllocator<blockSize, blockAlignment, PageAllocator, PageSizeStrategy>::Allocate( int size )
{
	return AllocateSized( size ).Ptr();
}

template <int blockSize, int blockAlignment, class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= TDefaultPageResizeStrategy*/>
CRawBuffer CGeneralBlockAllocator<blockSize, blockAlignment, PageAllocator, PageSizeStrategy>::AllocateSized( int size )
{
	assert( size <= blockSize );
	size;
#ifdef _DEBUG
	return AllocateSized( size, "", 0 );
#else
	return { baseAllocator.AllocateBlock(), blockSize };
#endif
}

template <int blockSize, int blockAlignment, class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= TDefaultPageResizeStrategy*/>
void CGeneralBlockAllocator<blockSize, blockAlignment, PageAllocator, PageSizeStrategy>::Free( CRawBuffer buffer )
{
	assert( buffer.Ptr() == nullptr || buffer.Size() == blockSize );
	Free( buffer.Ptr() );
}

template <int blockSize, int blockAlignment, class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= TDefaultPageResizeStrategy*/>
void CGeneralBlockAllocator<blockSize, blockAlignment, PageAllocator, PageSizeStrategy>::Free( void* ptr )
{
	if( ptr == nullptr ) {
		return;
	}
#ifdef _DEBUG
	RelibInternal::CDebugMemoryBlock* debugBlock = RelibInternal::CDebugMemoryBlock::CreateFromData( ptr );
	debugBlock->SetFree( L"block allocator" );
#endif
	baseAllocator.Free( ptr );
}

#ifdef _DEBUG

template <int blockSize, int blockAlignment, class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= TDefaultPageResizeStrategy*/>
void* CGeneralBlockAllocator<blockSize, blockAlignment, PageAllocator, PageSizeStrategy>::Allocate( int size, const char* fileName, int line )
{
	return AllocateSized( size, fileName, line ).Ptr();
}

template <int blockSize, int blockAlignment, class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= TDefaultPageResizeStrategy*/>
CRawBuffer CGeneralBlockAllocator<blockSize, blockAlignment, PageAllocator, PageSizeStrategy>::AllocateSized( int size, const char* fileName, int line )
{
	assert( size <= blockSize );
	BYTE* ptr = static_cast<BYTE*>( baseAllocator.AllocateBlock() );
	saveLatestPagePointer( ptr );
	RelibInternal::CDebugMemoryBlock* block = ::new( ptr - blockOffset ) RelibInternal::CDebugMemoryBlock( blockSize, fileName, line );
	return { block->Data(), blockSize };
}

template <int blockSize, int blockAlignment /*= AllocatorAlignment*/, class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= TDefaultPageResizeStrategy*/>
bool CGeneralBlockAllocator<blockSize, blockAlignment, PageAllocator, PageSizeStrategy>::isPointerInPage( void* ptr, typename TBaseAllocator::TPage* page )
{
	assert( page != nullptr );
	// Strict pointer ordering is assumed.
	const void* pageBegin = page->PageData;
	const void* pageEnd = page->PageData + page->PageSize;
	return ptr >= pageBegin && ptr < pageEnd;
}

template <int blockSize, int blockAlignment /*= AllocatorAlignment*/, class PageAllocator /*= CVirtualPageAllocator*/, class PageSizeStrategy /*= TDefaultPageResizeStrategy*/>
void CGeneralBlockAllocator<blockSize, blockAlignment, PageAllocator, PageSizeStrategy>::saveLatestPagePointer( BYTE* newPtr )
{
	const auto currentPage = baseAllocator.GetCurrentPage();
	if( currentPage == nullptr ){
		// Page may be null when a block from another allocator was freed on this one.
		// This is not necessarily a bug. For example, a group of thread-local string allocators does it.
		return;
	}

	if( isPointerInPage( newPtr, currentPage ) ) {
		if( !isPointerInPage( latestPointerInCurrentPage, currentPage ) || newPtr > latestPointerInCurrentPage ) {
			latestPointerInCurrentPage = newPtr;
		}
	}
}

#endif

//////////////////////////////////////////////////////////////////////////

// Block allocator for a single class entity.
template <class T, class PageAllocator = CVirtualPageAllocator, class PageSizeStrategy = TDefaultPageResizeStrategy>
using CBlockAllocator = CGeneralBlockAllocator<sizeof( T ), alignof( T ), PageAllocator, PageSizeStrategy>;

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

