#pragma once
#include <Redefs.h>
#include <PagedBitSet.h>
#include <Remath.h>
#include <CriticalSection.h>
#include <DynamicAllocators.h>
#include <GeneralBlockAllocator.h>

namespace Relib {

using CUnicodeSet = RelibInternal::CBaseBitSet<RelibInternal::CPagedStorage<USHRT_MAX + 1, 1024, RelibInternal::CUnicodeSetAllocator>, wchar_t>;

namespace RelibInternal {

// Allocator for the unicode set. Uses global block allocator.
class CUnicodeSetAllocator : public TStaticAllocator {
public:
	static void* Allocate( int size );
	static void Free( void* ptr );
#ifdef _DEBUG
	static void* Allocate( int size, const char*, int )
		{ return Allocate( size ); }
#endif
};

//////////////////////////////////////////////////////////////////////////

extern REAPI CThreadSafeAllocator<CGeneralBlockAllocator<CUnicodeSet::TStorageType::PageSizeInBytes>> UnicodeSetAllocator;
inline void* CUnicodeSetAllocator::Allocate( int size )
{
	return UnicodeSetAllocator.Allocate( size );
}

inline void CUnicodeSetAllocator::Free( void* ptr )
{
	return UnicodeSetAllocator.Free( ptr );
}

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////


}	// namespace Relib.

