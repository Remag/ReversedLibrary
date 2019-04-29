#include <ActionOwner.h>
#include <StaticAllocators.h>
#include <MemoryUtils.h>
#include <Remath.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

extern REAPI CActionOwnerAllocator ActionOwnerAllocator;
REAPI CActionOwnerAllocator& GetActionOwnerAllocator()
{
	return ActionOwnerAllocator;
}

void* CActionOwnerAllocator::Allocate( int size )
{
	if( size <= smallActionOwnerBlockSize ) {
		CCriticalSectionLock lock( allocationLock );
		return RELIB_ALLOCATE( smallActionManager, size );
	} else if( size <= biggerBlockSize ) {
		CCriticalSectionLock lock( allocationLock );
		return RELIB_ALLOCATE( biggerActionManager, size );
	} else {
		return RELIB_STATIC_ALLOCATE( CRuntimeHeap, size );
	}
}

void CActionOwnerAllocator::Free( void* ptr, int allocSize )
{
	if( allocSize <= smallActionOwnerBlockSize ) {
		smallActionManager.Free( ptr );
	} else if( allocSize <= biggerBlockSize ) {
		biggerActionManager.Free( ptr );
	} else {
		CRuntimeHeap::Free( ptr );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

