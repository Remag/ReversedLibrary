#pragma once

namespace Relib {

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

// Iterator that links two containers together. Minimal support for range-based for loops.
template <class LeftIter, class RightIter>
class CLinkEnumerator {
public:
	CLinkEnumerator( LeftIter left, LeftIter _leftEnd, RightIter right, RightIter _rightEnd ) : leftCurrent( left ), leftEnd( _leftEnd ), rightCurrent( right ), rightEnd( _rightEnd ) {}
		
	auto begin() const
		{ return *this; }
	auto end() const
		{ return CLinkEnumerator( leftEnd, leftEnd, rightEnd, rightEnd ); }

	void operator++();
	decltype( auto ) operator*() const;
	bool operator!=( CLinkEnumerator<LeftIter, RightIter> other ) const
		{ return leftCurrent != other.leftCurrent || rightCurrent != other.rightCurrent; }

private:
	LeftIter leftCurrent;
	LeftIter leftEnd;
	RightIter rightCurrent;
	RightIter rightEnd;
};

template <class LeftIter, class RightIter>
void CLinkEnumerator<LeftIter, RightIter>::operator++()
{
	if( leftCurrent == leftEnd ) {
		++rightCurrent;
	} else {
		++leftCurrent;
	}
}

template <class LeftIter, class RightIter>
decltype( auto ) CLinkEnumerator<LeftIter, RightIter>::operator*() const
{
	return ( leftCurrent == leftEnd ) ? *rightCurrent : *leftCurrent;
}

//////////////////////////////////////////////////////////////////////////

// Link enumerator specialization for two iterators of the same type.
template <class Iter>
class CLinkEnumerator<Iter, Iter> {
public:
	CLinkEnumerator( Iter left, Iter _leftEnd, Iter _right, Iter _rightEnd ) 
		: current( left == _leftEnd ? _right : left ), leftEnd( _leftEnd ), rightBegin( _right ), rightEnd( _rightEnd ) {}
		
	auto begin() const
		{ return *this; }
	auto end() const
		{ return CLinkEnumerator( rightEnd, leftEnd, rightBegin, rightEnd ); }

	void operator++();
	decltype( auto ) operator*() const;
	bool operator!=( CLinkEnumerator<Iter, Iter> other ) const
		{ return current != other.current; }

private:
	Iter current;
	Iter leftEnd;
	Iter rightBegin;
	Iter rightEnd;
};

template <class Iter>
void CLinkEnumerator<Iter, Iter>::operator++()
{
	++current;
	if( current == leftEnd ) {
		current = rightBegin;
	}
}

template <class Iter>
decltype( auto ) CLinkEnumerator<Iter, Iter>::operator*() const
{
	return *current;
}

//////////////////////////////////////////////////////////////////////////

template <class LeftIter, class RightIter>
CLinkEnumerator<LeftIter, RightIter> CreateLinkEnumerator( LeftIter left, LeftIter leftEnd, RightIter right, RightIter rightEnd )
{
	return CLinkEnumerator<LeftIter, RightIter>( left, leftEnd, right, rightEnd );
}

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// Link two containers together for smooth iteration.
template <class LeftContainer, class RightContainer>
auto Link( LeftContainer& left, RightContainer& right )
{
	return RelibInternal::CreateLinkEnumerator( left.begin(), left.end(), right.begin(), right.end() );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

