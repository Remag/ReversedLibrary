#pragma once
#include <Comparators.h>

namespace Relib {

// Container search functions.
//////////////////////////////////////////////////////////////////////////

// Searching with comparison operator.
template <class Container, class TargetType, class Comparator>
int SearchPos( const Container& container, const TargetType& elem, const Comparator& comp, int startPos = 0 )
{
	const int size = container.Size();
	for( int i = startPos; i < size; i++ ) {
		if( comp( container[i], elem ) ) {
			return i;
		}
	}
	return NotFound;
}

// Search using an equality operator comparison.
template <class Container, class TargetType>
int SearchPos( const Container& container, const TargetType& elem, int startPos = 0 )
{
	return SearchPos( container, elem, Equal(), startPos );
}

// Find an index in a sorted array, where elem should be inserted in order to retain the sorted property.
// If there are elements that are equal to elem, the end return point depends on the LessAction::operator() result.
// If the result for equal elements is false, index of the first of equal elements is returned.
// Otherwise, index of the first greater element is returned.
template<class Container, class TargetType, class LessAction>
int SearchSortedPos( const Container& container, const TargetType& elem, const LessAction& less )
{
	int first = 0;
	int last = container.Size();
	while( first < last ) {
		const int mid = ( first + last ) >> 1;
		if( less( container[mid], elem ) ) {
			first = mid + 1;
		} else {
			last = mid;
		}
	}
	return first;
}

// Binary search in a sorted container.
template <class Container, class TargetType, class LessAction, class Comparator>
int BinarySearchPos( const Container& container, const TargetType& elem, const LessAction& isLess, const Comparator& isEqual )
{
	const int insertionPos = SearchSortedPos( container, elem, isLess );
	if( insertionPos < container.Size() && isEqual( container[insertionPos], elem ) ) {
		return insertionPos;
	} else if( insertionPos > 0 && isEqual( container[insertionPos - 1], elem ) ) {
		return insertionPos - 1;
	} else {
		return NotFound;
	}
}

// Binary search using an equality operator comparison.
template <class Container, class TargetType, class LessAction>
int BinarySearchPos( const Container& container, const TargetType& elem, const LessAction& compClass )
{
	return BinarySearchPos( container, elem, compClass, Equal() );
}

template <class Container, class TargetType, class Comparator>
bool Has( const Container& container, const TargetType& elem, const Comparator& comp )
{
	return SearchPos( container, elem, comp ) != NotFound;
}

template <class Container, class TargetType>
bool Has( const Container& container, const TargetType& elem )
{
	return SearchPos( container, elem, Equal() ) != NotFound;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

