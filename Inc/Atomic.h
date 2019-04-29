#pragma once
#include <Redefs.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Atomic wrapper for lock-free programming.
// Wraps std::atomic.
template <class T>
class CAtomic {
public:
	CAtomic() = default;
	explicit CAtomic( T initValue ) : atomicValue( initValue ) {}

	// Load and store operations.
	T Load() const
		{ return atomicValue.load(); }
	void Store( T value )
		{ atomicValue.store( value ); }
	// Exchange operation. Previous value is returned.
	T Exchange( T value )
		{ atomicValue.exchange( value ); }

	// Atomic increment/decrement operations.
	T PostIncrement()
		{ return atomicValue++; }
	T PostDecrement()
		{ return atomicValue--; }
	T PreIncrement()
		{ return ++atomicValue; }
	T PreDecrement()
		{ return --atomicValue; }

	// CompareExchange operation. 
	// Compares this->Load() with expected and stores desired if equal. If not equal, changes expected to the load result.
	// Returns true if the underlying value has changed.
	// Weak version is allowed to act as if this->Load() != expected even when these values are equal.
	bool CompareExchangeWeak( T& expected, T desired )
		{ return atomicValue.compare_exchange_weak( expected, desired ); }
	bool CompareExchangeStrong( T& expected, T desired )
		{ return atomicValue.compare_exchange_strong( expected, desired ); }

private:
	std::atomic<T> atomicValue;
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

