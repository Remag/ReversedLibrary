#pragma once

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Range-based for loop support for bit sets.
// This enumerator iterates through set bits.
template <class BitSetType>
class CBitSetOneEnumerator {
public:
	explicit CBitSetOneEnumerator( BitSetType _bitset ) : bitset( _bitset ) {}

	CBitSetOneEnumerator begin()
		{ return CBitSetOneEnumerator( bitset, bitset.FirstOne() ); }
	CBitSetOneEnumerator end()
		{ return CBitSetOneEnumerator( bitset, NotFound ); }

	void operator++()
		{ pos = bitset.NextOne( pos ); }
	bool operator!=( CBitSetOneEnumerator other )
		{ return pos != other.pos; }
	int operator*()
		{ return pos; }

private:
	BitSetType bitset;
	int pos;

	CBitSetOneEnumerator( BitSetType _bitset, int _pos ) : bitset( _bitset ), pos( _pos ) {}
};

//////////////////////////////////////////////////////////////////////////

// Range-based for loop support for bit sets.
// This enumerator iterates through unset bits.
template <class BitSetType>
class CBitSetZeroEnumerator {
public:
	explicit CBitSetZeroEnumerator( BitSetType _bitset ) : bitset( _bitset ) {}

	CBitSetZeroEnumerator begin()
		{ return CBitSetZeroEnumerator( bitset, bitset.FirstZero() ); }
	CBitSetZeroEnumerator end()
		{ return CBitSetZeroEnumerator( bitset, NotFound ); }

	void operator++()
		{ pos = bitset.NextZero( pos ); }
	bool operator!=( CBitSetZeroEnumerator other )
		{ return pos != other.pos; }
	int operator*()
		{ return bitset[pos]; }

private:
	BitSetType bitset;
	int pos;

	CBitSetZeroEnumerator( BitSetType _bitset, int _pos ) : bitset( _bitset ), pos( _pos ) {}
};

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

