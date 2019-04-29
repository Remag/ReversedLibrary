#pragma once

namespace Relib {

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

template <class T, class ...Types>
CInterval<T> calculateMinMax( T currentMin, T currentMax, T next, Types ...rest )
{
	if( next >= currentMax ) {
		return calculateMinMax( currentMin, next, rest... );
	}
	if( next <= currentMin ) {
		return calculateMinMax( next, currentMax, rest... );
	}
	return calculateMinMax( currentMin, currentMax, rest... );
}

template <class T>
CInterval<T> calculateMinMax( T currentMin, T currentMax )
{
	return CInterval<T>( currentMin, currentMax );
}

}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

