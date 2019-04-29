#pragma once
#include <Remath.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Strategy for exponentially growing an integer value.
// Can be used for growing memory buffers.
template <int minValue>
class CDefaultGrowStrategy {
public:
	// Grow for half the current size, but at least for the required amount.
	// Grow size is also bound by a constant value to stop frequent reallocations for small arrays.
	static int GrowValue( int oldValue, int minNewValue )
		{ return max( oldValue + oldValue / 2, minNewValue, minValue ); }

	// Use the suggested value.
	static int GetInitialValue( int preferedValue )
		{ return preferedValue; }
};

//////////////////////////////////////////////////////////////////////////

// Grow strategy for flexible arrays. Imposes a hard limit on initial value.
template <int minValue>
class CFlexibleArrayGrowStrategy : private CDefaultGrowStrategy<minValue> {
public:
	static int GrowValue( int oldValue, int minNewValue )
		{ return CDefaultGrowStrategy<minValue>::GrowValue( oldValue, minNewValue ); }

	static int GetInitialValue( int preferedValue )
		{ return max( minValue, preferedValue ); }
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

