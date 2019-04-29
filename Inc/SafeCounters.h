#pragma once
#include <Reassert.h>
#include <DynamicAllocators.h>
#include <AllocationStrategy.h>
#include <Atomic.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Thread-safe version of the strong reference counter.
template <class Allocator>
class CSafeStrongRefCounter : private CAllocationStrategy<Allocator> {
public:
	// Create a counter with a single reference.
	CSafeStrongRefCounter() : strongRefCount( 1 ) {}
	explicit CSafeStrongRefCounter( Allocator& _allocator ) : CAllocationStrategy( _allocator ), strongRefCount( 1 ) {}
	~CSafeStrongRefCounter()
		{ assert( strongRefCount.Load() == 0 ); }

	int GetRefCount() const
		{ return strongRefCount.Load(); }

	void AddStrongRef();
	bool ReleaseStrongRef();

	// Free counters and objects memory.
	void Expire();

private:
	CAtomic<int> strongRefCount;
	// Reference counter is stored in a somewhat unsafe manner. A counter flag with constant value is used for validation.
	const int counterFlag = safeCounterValidationValue;

	static const int safeCounterValidationValue = 0xFEEDFEED;

	// Copying is prohibited.
	CSafeStrongRefCounter( CSafeStrongRefCounter& ) = delete;
	void operator=( CSafeStrongRefCounter& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class Allocator>
void CSafeStrongRefCounter<Allocator>::AddStrongRef()
{
	const int countValue = strongRefCount.Load();
	assert( countValue >= 0 );
	assert( countValue < INT_MAX );
	assert( counterFlag == safeCounterValidationValue );
	strongRefCount.PreIncrement();
}

// Release a reference and return true if no references remain.
template <class Allocator>
bool CSafeStrongRefCounter<Allocator>::ReleaseStrongRef()
{
	const int countPrevValue = strongRefCount.PreDecrement();
	assert( countPrevValue >= 0 );
	assert( counterFlag == safeCounterValidationValue );
	return countPrevValue == 0;
}

template <class Allocator>
void CSafeStrongRefCounter<Allocator>::Expire()
{
	this->StrategyFree( this );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

