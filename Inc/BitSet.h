#pragma once
#include <BitSetIteration.h>
#include <StackArray.h>
#include <TemplateUtils.h>

namespace Relib {

namespace RelibInternal {

	//////////////////////////////////////////////////////////////////////////

	// A set of integers.
	// BitSetStorage can be an arbitrary container type with random access.
	// Elem is a bit set element type. A static_cast to int is performed on it before all operations.
	template <class BitSetStorage, class Elem>
	class CBaseBitSet {
	private:
		using TBitsetWord = typename Types::ArrayElemType<BitSetStorage>::Result;
		static_assert( Types::IsSame<TBitsetWord, unsigned long>::Result || Types::IsSame<TBitsetWord, unsigned __int64>::Result, "BitsetStorage must be a container for 32/64 bit integer elements." );
		// Bit set constants.
		static const int bitsPerElement = CHAR_BIT * sizeof( TBitsetWord );

	public:
		typedef BitSetStorage TStorageType;

		CBaseBitSet();
		explicit CBaseBitSet( Elem element );
		CBaseBitSet( std::initializer_list<Elem> elemList );

		int Size() const
			{ return storage.BitSize();	}
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

		static TBitsetWord bitMask( Elem bit );
		TBitsetWord lastStorageMask() const;
		int index( Elem bit ) const;
		static int getTrailingZeroCount( TBitsetWord setPart );
		static int getLastNonZeroBit( TBitsetWord setPart );
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
		// Hamming weight problem.
		// Implementation taken from http://en.wikipedia.org/wiki/Hamming_weight.
		int totalCount = 0;
		if constexpr( sizeof( TBitsetWord ) == 4 ) {
			// 32 bit implementation.
			const unsigned m1 = 0x55555555;		// binary: 0101...
			const unsigned m2 = 0x33333333;		// binary: 00110011..
			const unsigned m4 = 0x0F0F0F0F;		// binary:  4 zeros,  4 ones ...
			const unsigned h01 = 0x01010101;	// the sum of 256 to the power of 0,1,2,3...

			for( auto setPart : storage ) {
				setPart -= ( ( setPart >> 1 ) & m1 );
				setPart = ( setPart & m2 ) + ( ( setPart >> 2 ) & m2 );
				totalCount += ( ( ( setPart + ( setPart >> 4 ) ) & m4 ) * h01 ) >> 24;
			}
		} else {
			staticAssert( sizeof( TBitsetWord ) == 8 );
			// 64 bit implementation.
			const uint64_t m1 = 0x5555555555555555;		// binary: 0101...
			const uint64_t m2 = 0x3333333333333333;		// binary: 00110011..
			const uint64_t m4 = 0x0f0f0f0f0f0f0f0f;		// binary:  4 zeros,  4 ones ...
			const uint64_t h01 = 0x0101010101010101;	// the sum of 256 to the power of 0,1,2,3...

			for( auto setPart : storage ) {
				setPart -= ( setPart >> 1 ) & m1;						   // put count of each 2 bits into those 2 bits
				setPart = ( setPart & m2 ) + ( ( setPart >> 2 ) & m2 );	   // put count of each 4 bits into those 4 bits
				setPart = ( setPart + ( setPart >> 4 ) ) & m4;			   // put count of each 8 bits into those 8 bits
				totalCount += ( setPart * h01 ) >> 56;
			}
		}
		return totalCount;
	}

	template <class BitSetStorage, class Elem>
	bool CBaseBitSet<BitSetStorage, Elem>::IsFilledWithZeroes() const
	{
		for( auto setPart : storage ) {
			if( setPart != 0 ) {
				return false;
			}
		}
		return true;
	}

	template <class BitSetStorage, class Elem>
	bool CBaseBitSet<BitSetStorage, Elem>::IsFilledWithOnes() const
	{
		const auto finalIndex = storage.StorageSize() - 1;
		for( int i = 0; i < finalIndex; i++ ) {
			if( storage[i] != ~static_cast<TBitsetWord>( 0 ) ) {
				return false;
			}
		}
		const auto finalMask = ~static_cast<TBitsetWord>( 0 ) & lastStorageMask();
		return storage[finalIndex] == finalMask;
	}

	template <class BitSetStorage, class Elem>
	void CBaseBitSet<BitSetStorage, Elem>::FillWithZeroes()
	{
		for( auto& setPart : storage ) {
			setPart = 0;
		}
	}

	template <class BitSetStorage, class Elem>
	void CBaseBitSet<BitSetStorage, Elem>::FillWithOnes()
	{
		for( auto& setPart : storage ) {
			setPart = ~static_cast<TBitsetWord>( 0 );
		}
		// Last body in the array is not filled to the end, some bits were unnecessarily flipped.
		storage[storage.StorageSize() - 1] &= lastStorageMask();
	}

	template <class BitSetStorage, class Elem>
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
		const auto subsetSize = subset.storage.StorageSize();
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
		for( int i = 0; i < storage.StorageSize(); i++ ) {
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
		for( int i = 0; i < storage.StorageSize(); i++ ) {
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
		for( int i = 0; i < storage.StorageSize(); i++ ) {
			storage[i] |= set.storage[i];
		}
		return *this;
	}

	template <class BitSetStorage, class Elem>
	CBaseBitSet<BitSetStorage, Elem>& CBaseBitSet<BitSetStorage, Elem>::operator&=( Elem element )
	{
		const int elemIndex = index( element );
		const auto newBody = storage[elemIndex] & bitMask( element );
		FillWithZeroes();
		storage[elemIndex] = newBody;
		return *this;
	}

	template <class BitSetStorage, class Elem>
	CBaseBitSet<BitSetStorage, Elem>& CBaseBitSet<BitSetStorage, Elem>::operator&=( const CBaseBitSet<BitSetStorage, Elem>& set )
	{
		for( int i = 0; i < storage.StorageSize(); i++ ) {
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
		for( int i = 0; i < storage.StorageSize(); i++ ) {
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
		for( int i = 0; i < storage.StorageSize(); i++ ) {
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

		const int lastIndex = storage.StorageSize() - 1;
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
		storage[storage.StorageSize() - 1] &= lastStorageMask();
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

		const int lastIndex = storage.StorageSize() - 1;
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
		for( int i = storage.StorageSize() - blockShift; i < storage.StorageSize(); i++ ) {
			storage[i] = 0;
		}
		return *this;
	}

	template <class BitSetStorage, class Elem>
	void CBaseBitSet<BitSetStorage, Elem>::Invert()
	{
		for( int i = 0; i < storage.StorageSize(); i++ ) {
			storage[i] = ~storage[i];
		}
		// Last body in the array is not filled to the end; some bit were unnecessarily flipped.
		storage[storage.StorageSize() - 1] &= lastStorageMask();
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
		auto setPart = storage[posIndex];
		const int posBit = pos % bitsPerElement;
		// Remove all the trailing 1s before the position.
		setPart &= ~static_cast<TBitsetWord>( 0 ) << posBit;
		if( setPart != 0 ) {
			return bitsPerElement * posIndex + getTrailingZeroCount( setPart );
		}

		// Check the remaining body parts.
		posIndex++;
		const int storageSize = storage.StorageSize();
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
		auto setPart = storage[posIndex];
		const int posBit = pos % bitsPerElement;
		// Remove all the leading 1s before the position.
		setPart &= ~static_cast<TBitsetWord>( 0 ) >> ( bitsPerElement - posBit - 1 );
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

	template <class BitSetStorage, class Elem>
	int CBaseBitSet<BitSetStorage, Elem>::FirstZero() const
	{
		if( !Has( 0 ) ) {
			return 0;
		} else {
			return NextZero( 0 );
		}
	}

	template <class BitSetStorage, class Elem>
	int CBaseBitSet<BitSetStorage, Elem>::LastZero() const
	{
		return PrevZero( Size() );
	}

	template <class BitSetStorage, class Elem>
	int CBaseBitSet<BitSetStorage, Elem>::NextZero( int pos ) const
	{
		assert( pos >= 0 );

		pos++;
		if( pos >= Size() ) {
			return NotFound;
		}

		// Check the first word.
		int posIndex = index( pos );
		auto setPart = ~storage[posIndex];
		const int posBit = pos % bitsPerElement;
		// Remove all the trailing 1s before the position.
		setPart &= ~static_cast<TBitsetWord>( 0 ) << posBit;
		if( setPart != 0 ) {
			return bitsPerElement * posIndex + getTrailingZeroCount( setPart );
		}

		// Check the remaining body parts.
		posIndex++;
		const int storageSize = storage.StorageSize();
		for( ; posIndex < storageSize; posIndex++ ) {
			setPart = ~storage[posIndex];
			if( setPart != 0 ) {
				return bitsPerElement * posIndex + getTrailingZeroCount( setPart );
			}
		}
		return NotFound;
	}

	template <class BitSetStorage, class Elem>
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
		auto setPart = ~storage[posIndex];
		const int posBit = pos % bitsPerElement;
		// Remove all the leading 1s before the position.
		setPart &= ~static_cast<TBitsetWord>( 0 ) >> ( bitsPerElement - posBit - 1 );
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
	typename CBaseBitSet<BitSetStorage, Elem>::TBitsetWord CBaseBitSet<BitSetStorage, Elem>::bitMask( Elem elem )
	{
		const int bit = static_cast<int>( elem );
		assert( bit >= 0 );
		return 1ULL << bit % bitsPerElement;
	}

	template <class BitSetStorage, class Elem>
	typename CBaseBitSet<BitSetStorage, Elem>::TBitsetWord CBaseBitSet<BitSetStorage, Elem>::lastStorageMask() const
	{
		return ~static_cast<TBitsetWord>( 0 ) >> ( CeilTo( Size(), bitsPerElement ) - Size() );
	}

	template <class BitSetStorage, class Elem>
	int CBaseBitSet<BitSetStorage, Elem>::index( Elem elem ) const
	{
		const int bit = static_cast<int>( elem );
		assert( bit >= 0 );
		return bit / bitsPerElement;
	}

	template <class BitSetStorage, class Elem>
	int CBaseBitSet<BitSetStorage, Elem>::getTrailingZeroCount( TBitsetWord setPart )
	{
		assert( setPart > 0 );
		DWORD result;
		if constexpr( sizeof( TBitsetWord ) == 4 ) {
			_BitScanForward( &result, setPart );
		} else {
			_BitScanForward64( &result, setPart );
		}
		return result;
	}

	template <class BitSetStorage, class Elem>
	int CBaseBitSet<BitSetStorage, Elem>::getLastNonZeroBit( TBitsetWord setPart )
	{
		assert( setPart > 0 );
		DWORD result;
		if constexpr( sizeof( TBitsetWord ) == 4 ) {
			_BitScanReverse( &result, setPart );
		} else {
			_BitScanReverse64( &result, setPart );
		}
		return result;
	}

	//////////////////////////////////////////////////////////////////////////

	// Calculate the size of a bitset with the given number of elements.
	template <int elementCount>
	constexpr inline int FindBitSetSize32()
	{
		return ( elementCount + CHAR_BIT * sizeof( unsigned long ) - 1 ) / ( CHAR_BIT * sizeof( unsigned long ) );
	}

	//////////////////////////////////////////////////////////////////////////

	template <class ElemType, int bitSetSize>
	class CStackBitSetStorage {
	public:
		using TElemType = ElemType;
		static const int storageSize = ( bitSetSize + CHAR_BIT * sizeof( ElemType ) - 1 ) / ( CHAR_BIT * sizeof( ElemType ) );

		auto& GetStorage()
			{ return stack; }
		auto& GetStorage() const
			{ return stack; }

		static int BitSize()
			{ return bitSetSize; }
		static int StorageSize()
			{ return CStackArray<ElemType, storageSize>::Size(); }

		ElemType& operator[]( int index )
			{ return stack[index]; }
		ElemType operator[]( int index ) const
			{ return stack[index]; }

		int HashKey() const
			{ return stack.HashKey(); }

		// Iteration support.
		ElemType* begin()
			{ return Relib::begin( stack ); }
		const ElemType* begin() const
			{ return Relib::begin( stack ); }

		ElemType* end()
			{ return Relib::end( stack ); }
		const ElemType* end() const
			{ return Relib::end( stack ); }

	private:
		CStackArray<ElemType, storageSize> stack;
	};


	template <int elementCount>
	struct CBestBitSetWord {
		using Result = typename Types::Conditional<elementCount <= 32, unsigned long, unsigned __int64>::Result;
	};

	template <int bitsetSize>
	using TBestBitSetStorage = CStackBitSetStorage<typename CBestBitSetWord<bitsetSize>::Result, bitsetSize>;

}	 // namespace RelibInternal.

 //////////////////////////////////////////////////////////////////////////

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

// Common bit set typedefs.
template <int bitsetSize>
using CBitSet = RelibInternal::CBaseBitSet<RelibInternal::TBestBitSetStorage<bitsetSize>, int>;
using CCharSet = RelibInternal::CBaseBitSet<RelibInternal::TBestBitSetStorage<256>, unsigned char>;

}	 // namespace Relib.
