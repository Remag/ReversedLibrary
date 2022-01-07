#pragma once
#include <StackArray.h>
#include <BitSetIteration.h>
#include <TemplateUtils.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// A set of integers. 
// BitSetStorage can be an arbitrary container type with random access.
// Elem is a bit set element type. A static_cast to int is performed on it before all operations.
template<class BitSetStorage, class Elem>
class CBaseBitSet {
private:
	static_assert( Types::IsSame<typename Types::ArrayElemType<BitSetStorage>::Result, DWORD>::Result, "BitsetStorage must be a container for DWORD elements." );
	// Bit set constants.
	static const int bitsPerElement = CHAR_BIT * sizeof( DWORD );

public:
	typedef BitSetStorage TStorageType;

	CBaseBitSet();
	explicit CBaseBitSet( Elem element );
	CBaseBitSet( std::initializer_list<Elem> elemList );

	int Size() const
		{ return storage.Size() * bitsPerElement; }
	// Change bit set size. This operation only makes sense for dynamic bit sets.
	void ReserveBuffer( int newBitSize );	

	// Low level access.
	BitSetStorage& GetStorage()
		{ return storage; }
	const BitSetStorage& GetStorage() const
		{ return storage; }
	void SetStorage( BitSetStorage newStorage )
		{ storage = move( newStorage ); }

	int ElementsCount() const;
	bool IsFilledWithZeroes() const;
	bool IsFilledWithOnes() const;
	void FillWithZeroes();
	void FillWithOnes();

	bool Has( Elem element ) const;
	bool HasAll( const CBaseBitSet& subset ) const;
	bool Intersects( const CBaseBitSet& other ) const;
	bool operator==( const CBaseBitSet& other ) const;
	bool operator!=( const CBaseBitSet& other ) const
		{ return !( *this == other ); }
	
	// Redaction.
	void Set( Elem element, bool flag );
	CBaseBitSet<BitSetStorage, Elem> operator~() const;
	CBaseBitSet<BitSetStorage, Elem> operator|( Elem element ) const;
	CBaseBitSet<BitSetStorage, Elem> operator|( CBaseBitSet set ) const;
	CBaseBitSet<BitSetStorage, Elem> operator&( Elem element ) const;
	CBaseBitSet<BitSetStorage, Elem> operator&( CBaseBitSet set ) const;
	CBaseBitSet<BitSetStorage, Elem> operator^( Elem element ) const;
	CBaseBitSet<BitSetStorage, Elem> operator^( CBaseBitSet set ) const;
	CBaseBitSet<BitSetStorage, Elem> operator-( Elem element ) const;
	CBaseBitSet<BitSetStorage, Elem> operator-( const CBaseBitSet& set ) const;
	CBaseBitSet<BitSetStorage, Elem> operator<<( int shift ) const;
	CBaseBitSet<BitSetStorage, Elem> operator>>( int shift ) const;

	CBaseBitSet<BitSetStorage, Elem>& operator|=( Elem element );
	CBaseBitSet<BitSetStorage, Elem>& operator|=( const CBaseBitSet& set );
	CBaseBitSet<BitSetStorage, Elem>& operator&=( Elem element );
	CBaseBitSet<BitSetStorage, Elem>& operator&=( const CBaseBitSet& set );
	CBaseBitSet<BitSetStorage, Elem>& operator^=( Elem element );
	CBaseBitSet<BitSetStorage, Elem>& operator^=( const CBaseBitSet& set );
	CBaseBitSet<BitSetStorage, Elem>& operator-=( Elem element );
	CBaseBitSet<BitSetStorage, Elem>& operator-=( const CBaseBitSet& set );
	CBaseBitSet<BitSetStorage, Elem>& operator<<=( int shift );
	CBaseBitSet<BitSetStorage, Elem>& operator>>=( int shift );

	void Invert();
	
	// Iteration.
	int FirstOne() const;
	int LastOne() const;
	int NextOne( int pos ) const;
	int PrevOne( int pos ) const;
	CBitSetOneEnumerator<CBaseBitSet<BitSetStorage, Elem>> Ones() const
		{ return RelibInternal::CBitSetOneEnumerator<CBaseBitSet<BitSetStorage, Elem>>( *this ); }

	int FirstZero() const;
	int LastZero() const;
	int NextZero( int pos ) const;
	int PrevZero( int pos ) const;
	CBitSetZeroEnumerator<CBaseBitSet<BitSetStorage, Elem>> Zeros() const
		{ return RelibInternal::CBitSetZeroEnumerator<CBaseBitSet<BitSetStorage, Elem>>( *this ); }

	// Hashing.
	int HashKey() const;

private:
	BitSetStorage storage;

	static DWORD bitMask( Elem bit );
	DWORD lastBodyMask() const;
	int index( Elem bit ) const;
	static int getTrailingZeroCount( DWORD setPart );
	static int getLastNonZeroBit( DWORD setPart );
};

//////////////////////////////////////////////////////////////////////////

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem>::CBaseBitSet()
{
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem>::CBaseBitSet( Elem element )
{
	*this |= element;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem>::CBaseBitSet( std::initializer_list<Elem> elemList )
{
	for( Elem elem : elemList ) {
		*this |= elem;
	}
}

template <class BitSetStorage, class Elem>
int CBaseBitSet<BitSetStorage, Elem>::ElementsCount() const
{
	staticAssert( sizeof( DWORD ) == 4 );
	// Hamming weight problem.
	// Implementation taken from http://en.wikipedia.org/wiki/Hamming_weight.
	DWORD totalCount = 0;
	for( DWORD setPart : storage ) {
		setPart = setPart - ( ( setPart >> 1 ) & 0x55555555 );
		setPart = ( setPart & 0x33333333 ) + ( ( setPart >> 2 ) & 0x33333333 );
		totalCount += ( ( ( setPart + ( setPart >> 4 ) ) & 0x0F0F0F0F ) * 0x01010101 ) >> 24;
	}
	return totalCount;
}

template <class BitSetStorage, class Elem>
bool CBaseBitSet<BitSetStorage, Elem>::IsFilledWithZeroes() const
{
	for( DWORD setPart : storage ) {
		if( setPart != 0 ) {
			return false;
		}
	}
	return true;
}

template<class BitSetStorage, class Elem>
bool CBaseBitSet<BitSetStorage, Elem>::IsFilledWithOnes() const
{
	for( DWORD setPart : storage ) {
		if( setPart != ~static_cast<DWORD>( 0 ) ) {
			return false;
		}
	}
	return true;
}

template <class BitSetStorage, class Elem>
void CBaseBitSet<BitSetStorage, Elem>::FillWithZeroes()
{
	for( DWORD& setPart : storage ) {
		setPart = 0;
	}
}

template <class BitSetStorage, class Elem>
void CBaseBitSet<BitSetStorage, Elem>::FillWithOnes()
{
	for( DWORD& setPart : storage ) {
		setPart = ~static_cast<DWORD>( 0 );
	}
}

template<class BitSetStorage, class Elem>
void CBaseBitSet<BitSetStorage, Elem>::ReserveBuffer( int newBitSize )
{
	storage.ReserveBuffer( newBitSize );
}

template <class BitSetStorage, class Elem>
bool CBaseBitSet<BitSetStorage, Elem>::Has( Elem element ) const
{
	return ( storage[index( element )] & bitMask( element ) ) != 0;
}

template <class BitSetStorage, class Elem>
bool CBaseBitSet<BitSetStorage, Elem>::HasAll( const CBaseBitSet<BitSetStorage, Elem>& subset ) const
{
	const auto subsetSize = subset.storage.Size();
	for( int i = 0; i < subsetSize; i++ ) {
		// Check if subset has bits that are not present in the body.
		if( ( ~storage[i] & subset.storage[i] ) != 0 ) {
			return false;
		}
	}
	return true;
}

template <class BitSetStorage, class Elem>
bool CBaseBitSet<BitSetStorage, Elem>::Intersects( const CBaseBitSet<BitSetStorage, Elem>& other ) const
{
	for( int i = 0; i < storage.Size(); i++ ) {
		// Check if bodies have a common bit.
		if( ( storage[i] & other.storage[i] ) != 0 ) {
			return true;
		}
	}
	return false;
}

template <class BitSetStorage, class Elem>
bool CBaseBitSet<BitSetStorage, Elem>::operator==( const CBaseBitSet<BitSetStorage, Elem>& other ) const
{
	for( int i = 0; i < storage.Size(); i++ ) {
		if( storage[i] != other.storage[i] ) {
			return false;
		}
	}
	return true;
}

template <class BitSetStorage, class Elem>
void CBaseBitSet<BitSetStorage, Elem>::Set( Elem element, bool flag )
{
	const int elem = static_cast<int>( element );
	if( flag ) {
		*this |= elem;
	} else {
		*this -= elem;
	}
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem> CBaseBitSet<BitSetStorage, Elem>::operator~() const
{
	CBaseBitSet<BitSetStorage, Elem> setCopy( *this );
	setCopy.Invert();
	return setCopy;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem> CBaseBitSet<BitSetStorage, Elem>::operator|( Elem element ) const
{
	return CBaseBitSet<BitSetStorage, Elem>( *this ) |= element;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem> CBaseBitSet<BitSetStorage, Elem>::operator|( CBaseBitSet<BitSetStorage, Elem> set ) const
{
	return set |= *this;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem> CBaseBitSet<BitSetStorage, Elem>::operator&( Elem element ) const
{
	return CBaseBitSet<BitSetStorage, Elem>( *this ) &= element;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem> CBaseBitSet<BitSetStorage, Elem>::operator&( CBaseBitSet<BitSetStorage, Elem> set ) const
{
	return set &= *this;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem> CBaseBitSet<BitSetStorage, Elem>::operator^( Elem element ) const
{
	return CBaseBitSet<BitSetStorage, Elem>( *this ) ^= element;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem> CBaseBitSet<BitSetStorage, Elem>::operator^( CBaseBitSet<BitSetStorage, Elem> set ) const
{
	return set ^= *this;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem> CBaseBitSet<BitSetStorage, Elem>::operator-( Elem element ) const
{
	return CBaseBitSet<BitSetStorage, Elem>( *this ) -= element;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem> CBaseBitSet<BitSetStorage, Elem>::operator-( const CBaseBitSet<BitSetStorage, Elem>& set ) const
{
	return CBaseBitSet<BitSetStorage, Elem>( *this ) -= set;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem> CBaseBitSet<BitSetStorage, Elem>::operator<<( int shift ) const
{
	return CBaseBitSet<BitSetStorage, Elem>( *this ) <<= shift;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem> CBaseBitSet<BitSetStorage, Elem>::operator>>( int shift ) const
{
	return CBaseBitSet<BitSetStorage, Elem>( *this ) >>= shift;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem>& CBaseBitSet<BitSetStorage, Elem>::operator|=( Elem element )
{
	storage[index( element )] |= bitMask( element );
	return *this;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem>& CBaseBitSet<BitSetStorage, Elem>::operator|=( const CBaseBitSet<BitSetStorage, Elem>& set )
{
	for( int i = 0; i < storage.Size(); i++ ) {
		storage[i] |= set.storage[i];
	}
	return *this;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem>& CBaseBitSet<BitSetStorage, Elem>::operator&=( Elem element )
{
	const int elemIndex = index( element );
	const int newBody = storage[elemIndex] & bitMask( element );
	FillWithZeroes();
	storage[elemIndex] = newBody;
	return *this;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem>& CBaseBitSet<BitSetStorage, Elem>::operator&=( const CBaseBitSet<BitSetStorage, Elem>& set )
{
	for( int i = 0; i < storage.Size(); i++ ) {
		storage[i] &= set.storage[i];
	}
	return *this;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem>& CBaseBitSet<BitSetStorage, Elem>::operator^=( Elem element )
{
	storage[index( element )] ^= bitMask( element );
	return *this;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem>& CBaseBitSet<BitSetStorage, Elem>::operator^=( const CBaseBitSet<BitSetStorage, Elem>& set )
{
	for( int i = 0; i < storage.Size(); i++ ) {
		storage[i] ^= set.storage[i];
	}
	return *this;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem>& CBaseBitSet<BitSetStorage, Elem>::operator-=( Elem element )
{
	storage[index( element )] &= ~bitMask( element );
	return *this;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem>& CBaseBitSet<BitSetStorage, Elem>::operator-=( const CBaseBitSet<BitSetStorage, Elem>& set )
{
	for( int i = 0; i < storage.Size(); i++ ) {
		storage[i] &= ~set.storage[i];
	}
	return *this;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem>& CBaseBitSet<BitSetStorage, Elem>::operator<<=( int shift )
{
	assert( shift >= 0 );

	if( shift >= Size() ) {
		FillWithZeroes();
		return *this;
	}

	const int lastIndex = storage.Size() - 1;
	const int blockShift = index( shift );
	const int innerLeftShift = shift % bitsPerElement;

	if( innerLeftShift == 0 ) {
		// No inner shift is required, just move the blocks.
		for( int i = lastIndex - blockShift; i >= 0; i-- ) {
			storage[blockShift + i] = storage[i];
		}
	} else {
		const int innerRightShift = bitsPerElement - innerLeftShift;
		for( int i = lastIndex - blockShift; i > 0; i-- ) {
			// innerRightShift is always less than bitsPerElement, no undefined behavior happens here.
			storage[blockShift + i] = ( storage[i] << innerLeftShift ) | ( storage[i - 1] >> innerRightShift );
		}
		storage[blockShift] = storage[0] << innerLeftShift;
	}
	// Zero in the left and right tails of the result.
	for( int i = 0; i < blockShift; i++ ) {
		storage[i] = 0;
	}
	storage[storage.Size() - 1] &= lastBodyMask();
	return *this;
}

template <class BitSetStorage, class Elem>
CBaseBitSet<BitSetStorage, Elem>& CBaseBitSet<BitSetStorage, Elem>::operator>>=( int shift )
{
	assert( shift >= 0 );
	if( shift >= Size() ) {
		FillWithZeroes();
		return *this;
	}

	const int lastIndex = storage.Size() - 1;
	const int blockShift = index( shift );
	const int innerRightShift = shift % bitsPerElement;

	if( innerRightShift == 0 ) {
		// No inner shift is required, just move the blocks.
		for( int i = blockShift; i <= lastIndex; i++ ) {
			storage[i - blockShift] = storage[i];
		}
	} else {
		const auto innerLeftShift = bitsPerElement - innerRightShift;
		for( int i = blockShift; i < lastIndex; i++ ) {
			// innerRightShift is always less than bitsPerElement, no undefined behavior happens here.
			storage[i - blockShift] = ( storage[i] >> innerRightShift ) | ( storage[i + 1] << innerLeftShift );
		}
		storage[lastIndex - blockShift] = storage[lastIndex] >> innerRightShift;
	}
	for( int i = storage.Size() - blockShift; i < storage.Size(); i++ ) {
		storage[i] = 0;
	}
	return *this;
}

template <class BitSetStorage, class Elem>
void CBaseBitSet<BitSetStorage, Elem>::Invert()
{
	for( int i = 0; i < storage.Size(); i++ ) {
		storage[i] = ~storage[i];
	}
	// Last body in the array is not filled to the end; some bit were unnecessarily flipped.
	storage[storage.Size() - 1] &= lastBodyMask();
}

template <class BitSetStorage, class Elem>
int CBaseBitSet<BitSetStorage, Elem>::FirstOne() const
{
	if( Has( 0 ) ) {
		return 0;
	} else {
		return NextOne( 0 );
	}
}

template <class BitSetStorage, class Elem>
int CBaseBitSet<BitSetStorage, Elem>::LastOne() const
{
	return PrevOne( Size() );
}

template <class BitSetStorage, class Elem>
int CBaseBitSet<BitSetStorage, Elem>::NextOne( int pos ) const
{
	assert( pos >= 0 );

	pos++;
	if( pos >= Size() ) {
		return NotFound;
	}

	// Check the first word.
	int posIndex = index( pos );
	DWORD setPart = storage[posIndex];
	const int posBit = pos % bitsPerElement;
	// Remove all the trailing 1s before the position.
	setPart &= ~static_cast<DWORD>( 0 ) << posBit;
	if( setPart != 0 ) {
		return bitsPerElement * posIndex + getTrailingZeroCount( setPart );
	}

	// Check the remaining body parts.
	posIndex++;
	const int storageSize = storage.Size();
	for( ; posIndex < storageSize; posIndex++ ) {
		setPart = storage[posIndex];
		if( setPart != 0 ) {
			return bitsPerElement * posIndex + getTrailingZeroCount( setPart );
		}
	}
	return NotFound;
}

template <class BitSetStorage, class Elem>
int CBaseBitSet<BitSetStorage, Elem>::PrevOne( int pos ) const
{
	assert( pos >= 0 );
	assert( pos <= Size() );

	if( pos == 0 ) {
		return NotFound;
	}
	pos--;
	// Check the first word.
	int posIndex = index( pos );
	DWORD setPart = storage[posIndex];
	const int posBit = pos % bitsPerElement;
	// Remove all the leading 1s before the position.
	setPart &= ~static_cast<DWORD>( 0 ) >> ( bitsPerElement - posBit - 1 );
	if( setPart != 0 ) {
		return bitsPerElement * posIndex + getLastNonZeroBit( setPart );
	}

	// Check the remaining body parts.
	posIndex--;
	for( ; posIndex >= 0; posIndex-- ) {
		setPart = storage[posIndex];
		if( setPart != 0 ) {
			return bitsPerElement * posIndex + getLastNonZeroBit( setPart );
		}
	}
	return NotFound;
}

template<class BitSetStorage, class Elem>
int CBaseBitSet<BitSetStorage, Elem>::FirstZero() const
{
	if( !Has( 0 ) ) {
		return 0;
	} else {
		return NextZero( 0 );
	}
}

template<class BitSetStorage, class Elem>
int CBaseBitSet<BitSetStorage, Elem>::LastZero() const
{
	return PrevZero( Size() );
}

template<class BitSetStorage, class Elem>
int CBaseBitSet<BitSetStorage, Elem>::NextZero( int pos ) const
{
	assert( pos >= 0 );

	pos++;
	if( pos >= Size() ) {
		return NotFound;
	}

	// Check the first word.
	int posIndex = index( pos );
	DWORD setPart = ~storage[posIndex];
	const int posBit = pos % bitsPerElement;
	// Remove all the trailing 1s before the position.
	setPart &= ~static_cast<DWORD>( 0 ) << posBit;
	if( setPart != 0 ) {
		return bitsPerElement * posIndex + getTrailingZeroCount( setPart );
	}

	// Check the remaining body parts.
	posIndex++;
	const int storageSize = storage.Size();
	for( ; posIndex < storageSize; posIndex++ ) {
		setPart = ~storage[posIndex];
		if( setPart != 0 ) {
			return bitsPerElement * posIndex + getTrailingZeroCount( setPart );
		}
	}
	return NotFound;
}

template<class BitSetStorage, class Elem>
int CBaseBitSet<BitSetStorage, Elem>::PrevZero( int pos ) const
{
	assert( pos >= 0 );
	assert( pos <= Size() );

	if( pos == 0 ) {
		return NotFound;
	}
	pos--;
	// Check the first word.
	int posIndex = index( pos );
	DWORD setPart = ~storage[posIndex];
	const int posBit = pos % bitsPerElement;
	// Remove all the leading 1s before the position.
	setPart &= ~static_cast<DWORD>( 0 ) >> ( bitsPerElement - posBit - 1 );
	if( setPart != 0 ) {
		return bitsPerElement * posIndex + getLastNonZeroBit( setPart );
	}

	// Check the remaining body parts.
	posIndex--;
	for( ; posIndex >= 0; posIndex-- ) {
		setPart = ~storage[posIndex];
		if( setPart != 0 ) {
			return bitsPerElement * posIndex + getLastNonZeroBit( setPart );
		}
	}
	return NotFound;
}

template <class BitSetStorage, class Elem>
inline int CBaseBitSet<BitSetStorage, Elem>::HashKey() const
{
	return storage.HashKey();
}

template <class BitSetStorage, class Elem>
DWORD CBaseBitSet<BitSetStorage, Elem>::bitMask( Elem elem )
{
	const int bit = static_cast<int>( elem );
	assert( bit >= 0 );
	return 1 << bit % bitsPerElement;
}

template <class BitSetStorage, class Elem>
DWORD CBaseBitSet<BitSetStorage, Elem>::lastBodyMask() const
{
	return ~static_cast<DWORD>( 0 ) >> ( CeilTo( Size(), bitsPerElement ) - Size() );
}

template <class BitSetStorage, class Elem>
int CBaseBitSet<BitSetStorage, Elem>::index( Elem elem ) const
{
	const int bit = static_cast<int>( elem );
	assert( bit >= 0 );
	return bit / bitsPerElement;
}

template <class BitSetStorage, class Elem>
int CBaseBitSet<BitSetStorage, Elem>::getTrailingZeroCount( DWORD setPart )
{
	assert( setPart > 0 );
	DWORD result;
	_BitScanForward( &result, setPart );
	return result;
}

template <class BitSetStorage, class Elem>
int CBaseBitSet<BitSetStorage, Elem>::getLastNonZeroBit( DWORD setPart )
{
	assert( setPart > 0 );
	DWORD result;
	_BitScanReverse( &result, setPart );
	return result;
}

//////////////////////////////////////////////////////////////////////////

} // namespace RelibInternal.

template <class BitSetStorage, class Elem>
CArchiveReader& operator>>( CArchiveReader& archive, RelibInternal::CBaseBitSet<BitSetStorage, Elem>& set )
{
	archive >> set.GetStorage();
}

template <class BitSetStorage, class Elem>
CArchiveWriter& operator<<( CArchiveWriter& archive, const RelibInternal::CBaseBitSet<BitSetStorage, Elem>& set )
{
	archive << set.GetStorage();
}

//////////////////////////////////////////////////////////////////////////

// Calculate the size of a bitset with the given number of elements.
constexpr inline int FindBitSetSize( int elementCount )
{
	return ( elementCount + CHAR_BIT * sizeof( DWORD ) - 1 ) / ( CHAR_BIT * sizeof( DWORD ) );
}

// Common bit set typedefs.
template <int bitsetSize>
using CBitSet = RelibInternal::CBaseBitSet<CStackArray<DWORD, FindBitSetSize( bitsetSize )>, int>;
using CCharSet = RelibInternal::CBaseBitSet<CStackArray<DWORD, 8>, unsigned char>;
using CExternalBitSet = RelibInternal::CBaseBitSet<DWORD*, int>;
using CExternalConstBitSet = RelibInternal::CBaseBitSet<const DWORD*, int>;

}	// namespace Relib.

