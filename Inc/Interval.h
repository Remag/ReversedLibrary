#pragma once

namespace Relib {

// A numeric interval. Provides simple comparison methods.
// Type is assumed to be simple and is always passed by value.
template <class Type>
class CInterval {
public:
	explicit CInterval( Type value = Type{} ) : lower( value ), upper( value ) {}
	// It is asserted that lower <= upper.
	CInterval( Type lower, Type upper );

	int GetDelta() const
		{ return upper - lower; }

	Type GetLower() const
		{ return lower; }
	Type GetUpper() const
		{ return upper; }
	void SetLower( Type newLower )
		{ lower = newLower; }
	void SetUpper( Type newUpper)
		{ upper = newUpper; }

	// It is asserted that newLower <= newUpper.
	void Set( Type newLower, Type newUpper );
	void Offset( Type offset )
		{ lower += offset; upper += offset; }
	// Extend the interval to contain the given point.
	void Add( Type point );

	bool Has( Type val ) const;
	bool Has( const CInterval& other ) const;
	bool Intersects( const CInterval& other ) const;
	bool StrictIntersects( const CInterval& other ) const;

private:
	Type lower;
	Type upper;
};

//////////////////////////////////////////////////////////////////////////
template<class Type>
CInterval<Type>::CInterval( Type _lower, Type _upper ) :
	lower( _lower ),
	upper( _upper )
{
	assert( lower <= upper );
}

template<class Type>
void CInterval<Type>::Set( Type newLower, Type newUpper )
{
	assert( newLower <= newUpper );
	lower = newLower;
	upper = newUpper;
}

template<class Type>
void CInterval<Type>::Add( Type point )
{
	if( lower > point ) {
		lower = point;
	} else if( upper < point ) {
		upper = point;
	}
}

template<class Type>
bool CInterval<Type>::Has( Type val ) const
{
	return val >= lower && val <= upper;
}

template<class Type>
bool CInterval<Type>::Has( const CInterval& other ) const
{
	return other.lower >= lower && other.upper <= upper;
}

template<class Type>
bool CInterval<Type>::Intersects( const CInterval& other ) const
{
	return other.lower <= upper && other.upper >= lower;
}

template <class Type>
bool CInterval<Type>::StrictIntersects( const CInterval& other ) const
{
	return other.lower < upper && other.upper > lower;
}

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

// Interval iterator. Implements a bare minimum of methods that are required for range-based for loops.
template <class T>
class CIntervalIterator {
public:
	explicit CIntervalIterator( int start ) : currentPos( start ) {}

	int operator*()
		{ return currentPos; }
	
	// Increment operator.
	// Range-based for loops don't require the ++operator to return a value.
	void operator++()
		{ currentPos++; }
	// Inequality operator.
	bool operator!=( const CIntervalIterator<T>& other ) const
		{ return currentPos != other.currentPos; }
	bool operator!=( int otherPos ) const
		{ return currentPos != otherPos; }

private:
	int currentPos;
};

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// Range based for-loops support.
template <class T>
RelibInternal::CIntervalIterator<T> begin( const CInterval<T>& interval )
{
	return RelibInternal::CIntervalIterator<T>( interval.GetLower() );
}

template <class T>
RelibInternal::CIntervalIterator<T> end( const CInterval<T>& interval )
{
	return RelibInternal::CIntervalIterator<T>( interval.GetUpper() );
}

//////////////////////////////////////////////////////////////////////////

// Interval creation functions.
template <class T>
CInterval<T> Interval( T start, T end = start )
{
	return CInterval<T>( start, end );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.