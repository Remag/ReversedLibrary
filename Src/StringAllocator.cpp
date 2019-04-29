#include <StringAllocator.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

extern CCriticalSection StringAllocatorSection;
template <class PoolType>
static auto createObjectInPool( PoolType& pool )
{
	CCriticalSectionLock lock( StringAllocatorSection );
	return pool.GetOrCreate();
}

static auto& getSmallAllocator( CStringAllocator& allocator )
{
	thread_local auto poolRef = createObjectInPool( allocator.GetSmallPool() );
	return poolRef.Value(); 
}

static auto& getMediumAllocator( CStringAllocator& allocator )
{
	thread_local auto poolRef = createObjectInPool( allocator.GetMediumPool() );
	return poolRef.Value();
}

static auto& getLargeAllocator( CStringAllocator& allocator )
{
	thread_local auto poolRef = createObjectInPool( allocator.GetLargePool() );
	return poolRef.Value();
}

//////////////////////////////////////////////////////////////////////////

CStringAllocator::CStringAllocator() 
{
	heapManager.Create( 0 );
	heapManager.SetLowFragmentation( true );
}

CRawBuffer CStringAllocator::AllocateSized( int realSize )
{
	if( realSize <= smallBlockSize ) {
		return getSmallAllocator( *this ).AllocateSized( smallBlockSize );
	} else if( realSize <= mediumBlockSize ) {
		return getMediumAllocator( *this ).AllocateSized( mediumBlockSize );
	} else if( realSize <= largeBlockSize ) {
		return getLargeAllocator( *this ).AllocateSized( largeBlockSize );
	} else {
		return heapManager.AllocateSized( realSize );
	}
}

void CStringAllocator::Free( CRawBuffer buffer )
{
	const int allocatedSize = buffer.Size();
	void* ptr = buffer.Ptr();
	if( allocatedSize <= smallBlockSize ) {
		getSmallAllocator( *this ).Free( ptr );
	} else if( allocatedSize <= mediumBlockSize ) {
		getMediumAllocator( *this ).Free( ptr );
	} else if( allocatedSize <= largeBlockSize ) {
		getLargeAllocator( *this ).Free( ptr );
	} else {
		heapManager.Free( ptr );
	}
}

#ifdef _DEBUG
CRawBuffer CStringAllocator::AllocateSized( int realSize, const char* fileName, int line )
{
	if( realSize <= smallBlockSize ) {
		return getSmallAllocator( *this ).AllocateSized( smallBlockSize, fileName, line );
	} else if( realSize <= mediumBlockSize ) {
		return getMediumAllocator( *this ).AllocateSized( mediumBlockSize, fileName, line );
	} else if( realSize <= largeBlockSize ) {
		return getLargeAllocator( *this ).AllocateSized( largeBlockSize, fileName, line );
	} else {
		return heapManager.AllocateSized( realSize, fileName, line );
	}
}
#endif

//////////////////////////////////////////////////////////////////////////

extern REAPI CStringAllocator StringAllocator;
CStringAllocator& GetStringAllocator()
{
	return StringAllocator;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
