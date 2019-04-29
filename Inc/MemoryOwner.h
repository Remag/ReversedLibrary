#pragma once
#include <AllocationStrategy.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Class that owns raw memory.
template <class Allocator = CRuntimeHeap>
class CMemoryOwner : private CAllocationStrategy<Allocator> {
public:
	CMemoryOwner() = default;
	explicit CMemoryOwner( void* _ptr ) : ptr( _ptr ) {}
	CMemoryOwner( void* _ptr, Allocator& allocator ) : CAllocationStrategy<Allocator>( allocator ), ptr( _ptr ) {}
	CMemoryOwner( CMemoryOwner&& other ) : ptr( other.ptr ) { other.ptr = nullptr; }
	CMemoryOwner& operator=( CMemoryOwner&& other );
	~CMemoryOwner();

	void* Ptr()
		{ return ptr; }
	const void* Ptr() const
		{ return ptr; }
	void Detach()
		{ ptr = nullptr; }

private:
	void* ptr = nullptr;

	void free();

	// Copying is prohibited.
	CMemoryOwner( CMemoryOwner& ) = delete;
	void operator=( CMemoryOwner& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class Allocator /*= CRuntimeHeap*/>
CMemoryOwner<Allocator>& CMemoryOwner<Allocator>::operator=( CMemoryOwner<Allocator>&& other )
{
	free();
	ptr = other.ptr;
	other.ptr = nullptr;
	return *this;
}

template <class Allocator>
void CMemoryOwner<Allocator>::free()
{
	if( ptr != nullptr ) {
		CAllocationStrategy<Allocator>::StrategyFree( ptr );
	}
}

template <class Allocator>
CMemoryOwner<Allocator>::~CMemoryOwner()
{
	free();
}

}	// namespace Relib.

