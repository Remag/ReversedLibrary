#pragma once
#include <Redefs.h>
// Map data accessors and iterators.

namespace Relib {
	
//////////////////////////////////////////////////////////////////////////

// Objects of this class are stored within CMap.
// External user gets only constant access to the key.
template <class KeyType, class ValueType>
class CMapData {
public:
	CMapData( const CMapData<KeyType, ValueType>& other, CExplicitCopyTag&& ) : key( copy( other.key ) ), value( copy( other.value ) ) {}
	CMapData() = default;

	const KeyType& Key() const
		{ return key; }
	ValueType& Value()
		{ return value; }
	const ValueType& Value() const
		{ return value; }

	// Index entries get to construct map data.
	template <class ContainedType>
	friend struct RelibInternal::CIndexEntry;

private:
	KeyType key;
	ValueType value;

	// General constructor is declared private so that this class will not register as copy constructible.
	template <class KeyArg, class... Args>
	explicit CMapData( KeyArg&& _key, Args&&... valueArgs ) : key( forward<KeyArg>( _key ) ), value( forward<Args>( valueArgs )... ) {}
};

//////////////////////////////////////////////////////////////////////////

template <class Key, class Value>
using CMapPos = RelibInternal::CIndexEntry<CMapData<Key, Value>>;

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Map iterator for values with the same key. Implements a bare minimum of methods that are required for range-based for loops.
template <class CompareKey, class KeyType, class ValueType, class Strategy, class Allocator>
class CMapKeyConstEnumerator {
public:
	CMapKeyConstEnumerator( const CHashIndex<CMapData<KeyType, ValueType>, Strategy, Allocator>& _hashIndex, CompareKey _key );
	CMapKeyConstEnumerator( const CHashIndex<CMapData<KeyType, ValueType>, Strategy, Allocator>& _hashIndex, CompareKey _key, CIndexEntry<CMapData<KeyType, ValueType>>* _indexPos ) :
		hashIndex( _hashIndex ), indexPos( _indexPos ), key( move( _key ) ) {}
	CMapKeyConstEnumerator( const CHashIndex<CMapData<KeyType, ValueType>, Strategy, Allocator>& _hashIndex, CIndexEntry<CMapData<KeyType, ValueType>>* _indexPos ) :
		hashIndex( _hashIndex ), indexPos( _indexPos ) {}
		
	const CMapData<KeyType, ValueType>& operator*() const
		{ return indexPos->Value; }
	// An increment operator.
	// Range-based for loops don't require the ++operator to return a value.
	void operator++()
		{ indexPos = nextPos( indexPos->NextInBucket ); }
		
	bool operator!=( const CMapKeyConstEnumerator<CompareKey, KeyType, ValueType, Strategy, Allocator>& other ) const
		{ return indexPos != other.indexPos; }

	auto begin() const
		{ return CMapKeyConstEnumerator<CompareKey, KeyType, ValueType, Strategy, Allocator>( hashIndex, move( *key ), indexPos ); }
	auto end() const
		{ return CMapKeyConstEnumerator<CompareKey, KeyType, ValueType, Strategy, Allocator>( hashIndex, nullptr ); }

protected:
	const CHashIndex<CMapData<KeyType, ValueType>, Strategy, Allocator>& hashIndex;
	CIndexEntry<CMapData<KeyType, ValueType>>* indexPos;
	COptional<CompareKey> key;

	CIndexEntry<CMapData<KeyType, ValueType>>* nextPos( CIndexEntry<CMapData<KeyType, ValueType>>* pos );
};

//////////////////////////////////////////////////////////////////////////

template <class CompareKey, class KeyType, class ValueType, class Strategy, class Allocator>
CMapKeyConstEnumerator<CompareKey, KeyType, ValueType, Strategy, Allocator>::CMapKeyConstEnumerator( const CHashIndex<CMapData<KeyType, ValueType>, Strategy, Allocator>& _hashIndex, CompareKey _key ) :
	hashIndex( _hashIndex ),
	key( move( _key ) )
{
	if( hashIndex.IsEmpty() ) {
		indexPos = nullptr;
	} else {
		const int hash = Strategy::HashKey( *key );
		indexPos = nextPos( *hashIndex.GetRootPosition( hash ) );
	}
}

template <class CompareKey, class KeyType, class ValueType, class Strategy, class Allocator>
CIndexEntry<CMapData<KeyType, ValueType>>* CMapKeyConstEnumerator<CompareKey, KeyType, ValueType, Strategy, Allocator>::nextPos( CIndexEntry<CMapData<KeyType, ValueType>>* pos )
{
	while( pos != nullptr ) {
		if( Strategy::IsEqual( pos->Value, *key ) ) {
			return pos;
		}
		pos = pos->NextInBucket;
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////

// Map iterator for constant access. Implements a bare minimum of methods that are required for range-based for loops.
template <class CompareKey, class KeyType, class ValueType, class Strategy, class Allocator>
class CMapKeyEnumerator : public CMapKeyConstEnumerator<CompareKey, KeyType, ValueType, Strategy, Allocator> {
public:
	using CMapKeyConstEnumerator<CompareKey, KeyType, ValueType, Strategy, Allocator>::CMapKeyConstEnumerator;

	CMapData<KeyType, ValueType>& operator*()
		{ return this->indexPos->Value; }

	auto begin()
		{ return CMapKeyEnumerator( this->hashIndex, move( *this->key ), this->indexPos ); }
	auto end()
		{ return CMapKeyEnumerator( this->hashIndex, nullptr ); }
};

//////////////////////////////////////////////////////////////////////////

// Hash strategy specialization for map data types.
template <class KeyType, class ValueType, class BaseHashStrategy>
class CMapHashStrategy {
public:
	template <class Key>
	static int HashKey( const Key& key )
		{ return BaseHashStrategy::HashKey( key ); }
	static int HashKey( const CMapData<KeyType, ValueType>& data )
		{ return BaseHashStrategy::HashKey( data.Key() ); }

	// Map data types are compared by their keys.
	static bool IsEqual( const CMapData<KeyType, ValueType>& leftKey, const CMapData<KeyType, ValueType>& rightKey )
		{ return BaseHashStrategy::IsEqual( leftKey.Key(), rightKey.Key() ); }
	template <class Key>
	static bool IsEqual( const Key& leftKey, const CMapData<KeyType, ValueType>& rightKey )
		{ return BaseHashStrategy::IsEqual( leftKey, rightKey.Key() ); }
	template <class Key>
	static bool IsEqual( const CMapData<KeyType, ValueType>& leftKey, const Key& rightKey )
		{ return BaseHashStrategy::IsEqual( leftKey.Key(), rightKey ); }
};

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.


