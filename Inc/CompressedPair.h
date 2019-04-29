#pragma once

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Pair of elements, first of which is of empty size.
template <class EmptyFirstType, class SecondType>
class CCompressedPair : private EmptyFirstType {
public:
	CCompressedPair() = default;
	CCompressedPair( EmptyFirstType first, SecondType _second ) : EmptyFirstType( move( first ) ), second( move( _second ) ) {}

	EmptyFirstType& First() 
		{ return *this; }
	const EmptyFirstType& First() const
		{ return *this; }
	SecondType& Second()
		{ return second; }
	const SecondType& Second() const
		{ return second; }

private:
	SecondType second;
};


//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

