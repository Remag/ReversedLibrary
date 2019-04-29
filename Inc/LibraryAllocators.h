#pragma once
#include <StaticAllocators.h>

namespace Relib {

// Specific allocators used by Reversed library.
namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Manager that allocates memory using VirtualAlloc.
// Debug information is saved about the allocated pages.
class REAPI CVirtualAllocDynamicManager : public TGlobalAllocator {
public:
	CVirtualAllocDynamicManager();
	~CVirtualAllocDynamicManager();

	void* Allocate( int size );
	void Free( void* ptr );

	CRawBuffer AllocateSized( int size );
	void Free( CRawBuffer buffer );

#ifdef _DEBUG
	void* Allocate( int size, const char* fileName, int line );
	CRawBuffer AllocateSized( int size, const char* fileName, int line );
#endif

private:
#ifdef _DEBUG
	CCriticalSection virtualAllocatorLock;
	CVirtualMemoryDebugBlock* firstBlock;
#endif
};

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

