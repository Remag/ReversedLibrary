#pragma once

#include <Redefs.h>
#include <Array.h>
#include <HashTableBase.h>
#include <MapUtils.h>
#include <TemplateUtils.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// The map template.
// Uses HashStrategy::HashKey to hash values.
// Uses HashStrategy::IsEqual to compare values.
// The class is implemented in almost the same way as CHashTable.
template<class KeyType, class ValueType, class HashStrategy = CDefaultHash<KeyType>, class Allocator = CRuntimeHeap>
class CMap {
public:
	typedef KeyType KeyElemType;
	typedef ValueType ValueElemType;
	typedef RelibInternal::CMapHashStrategy<KeyType, ValueType, HashStrategy> TMapHashStrategy;

	CMap() = default;
	// Custom dynamic allocator.
	explicit CMap( Allocator& allocator );
	// Explicit copy constructor.
	template <class Container>
	CMap( const Container& other, const CExplicitCopyTag& );
	template <class OtherAllocator>
	CMap( const CMap<KeyType, ValueType, HashStrategy, OtherAllocator>& other, const CExplicitCopyTag& );
	CMap( CMap&& other );
	~CMap();

	int IndexSize() const
		{ return hashIndex.IndexSize(); }
	int Size() const
		{ return hashIndex.Size(); }
	bool IsEmpty() const
		{ return Size() == 0; }
	void ReserveBuffer( int size );
	
	// Sets the value for the given key. All previous values associated with this key are destroyed.
	// Returns a reference to a newly created value.
	template <class Key, class... Args>
	CMapData<KeyType, ValueType>& Set( Key&& key, Args&&... valueArgs );
	// Adds the value for the given key. All other values are still there.
	// Returns a reference to a newly created value.
	template <class Key, class... Args>
	CMapData<KeyType, ValueType>& Add( Key&& key, Args&&... valueArgs );

	// Get the value. If it's not present, null is returned.
	template <class Key>
	ValueType* Get( const Key& key );
	template <class Key>
	const ValueType* Get( const Key& key ) const;
	// Get the value. It must be present in the map.
	template <class Key>
	ValueType& operator[]( const Key& key );
	template <class Key>
	const ValueType& operator[]( const Key& key ) const;

	// Get the value for the given key, or create one with given arguments.
	template <class Key, class... Args>
	CMapData<KeyType, ValueType>& GetOrCreate( Key&& key, Args&&... valueArgs );

	// Check if a key-value pair with a given key is present in this map.
	template <class Key>
	bool Has( const Key& key ) const;

	// Redaction.
	template <class Key>
	void Delete( const Key& key );
	// Delete all.
	void Empty();
	// Delete all and free buffer.
	void FreeBuffer();

	// Iteration.
	auto begin()
		{ return RelibInternal::CHashIndexIterator<CMapData<KeyType, ValueType>, TMapHashStrategy, Allocator>( hashIndex ); }
	auto begin() const
		{ return RelibInternal::CHashIndexConstIterator<CMapData<KeyType, ValueType>, TMapHashStrategy, Allocator>( hashIndex ); }
	auto end()
		{ return RelibInternal::CHashIndexIterator<CMapData<KeyType, ValueType>, TMapHashStrategy, Allocator>( hashIndex, nullptr ); }
	auto end() const
		{ return RelibInternal::CHashIndexConstIterator<CMapData<KeyType, ValueType>, TMapHashStrategy, Allocator>( hashIndex, nullptr ); }
	// Iteration through values with the same key.
	template <class Key>
	auto AllValues( Key&& key ) 
		{ return RelibInternal::CMapKeyEnumerator<typename Types::PureType<Key>::Result, KeyType, ValueType, TMapHashStrategy, Allocator>( hashIndex, forward<Key>( key ) ); }
	template <class Key>
	auto AllValues( Key&& key ) const
		{ return RelibInternal::CMapKeyConstEnumerator<typename Types::PureType<Key>::Result, KeyType, ValueType, TMapHashStrategy, Allocator>( hashIndex, forward<Key>( key ) ); }
	
	// Copying and moving objects to another hash table.
	// Destination is emptied before all operations.
	const CMap& operator=( CMap&& other );
		
private:
	RelibInternal::CHashIndex<CMapData<KeyType, ValueType>, TMapHashStrategy, Allocator> hashIndex;

	template <class Key>
	ValueType* doGet( const Key& key );
	
	// Copying is prohibited.
	CMap( CMap& ) = delete;
	void operator=( CMap& ) = delete;
};

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
CMap<KeyType, ValueType, HashStrategy, Allocator>::CMap( Allocator& allocator ) :
	hashIndex( allocator )
{
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
template <class Container>
CMap<KeyType, ValueType, HashStrategy, Allocator>::CMap( const Container& src, const CExplicitCopyTag& )
{
	for( const auto& elem : src ) {
		Add( elem );
	}
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
template <class OtherAllocator>
CMap<KeyType, ValueType, HashStrategy, Allocator>::CMap( const CMap<KeyType, ValueType, HashStrategy, OtherAllocator>& src, const CExplicitCopyTag& ) :
	hashIndex( copy( src.hashIndex ) )
{
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
CMap<KeyType, ValueType, HashStrategy, Allocator>::CMap( CMap<KeyType, ValueType, HashStrategy, Allocator>&& other ) :
	hashIndex( move( other.hashIndex ) )
{
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
CMap<KeyType, ValueType, HashStrategy, Allocator>::~CMap()
{
	FreeBuffer();
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
void CMap<KeyType, ValueType, HashStrategy, Allocator>::ReserveBuffer( int size )
{
	hashIndex.ReserveBuffer( size );
}

template <class KeyType, class ValueType, class HashStrategy, class Allocator>
template <class Key, class... Args>
CMapData<KeyType,ValueType>& CMap<KeyType, ValueType, HashStrategy, Allocator>::Set( Key&& key, Args&&... valueArgs )
{
	hashIndex.InitializeIfEmpty();
	const int hash = HashStrategy::HashKey( key );
	auto entry = hashIndex.GetRootPosition( hash );
	while( !hashIndex.IsPositionFree( entry ) ) {
		if( TMapHashStrategy::IsEqual( key, hashIndex.GetValue( entry ) ) ) {
			hashIndex.DeleteEntry( entry );
		} else {
			entry = hashIndex.NextIndexPosition( entry );
		}
	}
	return hashIndex.InsertEndOrRoot( hash, entry, forward<Key>( key ), forward<Args>( valueArgs )... );
}

template <class KeyType, class ValueType, class HashStrategy, class Allocator>
template <class Key, class... Args>
CMapData<KeyType, ValueType>& CMap<KeyType, ValueType, HashStrategy, Allocator>::Add( Key&& key, Args&&... valueArgs )
{
	hashIndex.InitializeIfEmpty();
	const int hash = HashStrategy::HashKey( key );
	auto entry = hashIndex.GetRootPosition( hash );
	return hashIndex.InsertRoot( hash, entry, forward<Key>( key ), forward<Args>( valueArgs )... );
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
template <class Key>
ValueType* CMap<KeyType, ValueType, HashStrategy, Allocator>::Get( const Key& key )
{
	if( hashIndex.IsEmpty() ) {
		return nullptr;
	}
	return doGet( key );
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
template <class Key>
const ValueType* CMap<KeyType, ValueType, HashStrategy, Allocator>::Get( const Key& key ) const
{
	return const_cast<CMap<KeyType, ValueType, HashStrategy, Allocator>*>( this )->Get( key );
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
template <class Key>
ValueType* CMap<KeyType, ValueType, HashStrategy, Allocator>::doGet( const Key& key )
{
	const int hash = HashStrategy::HashKey( key );
	auto entry = hashIndex.GetRootPosition( hash );
	while( !hashIndex.IsPositionFree( entry ) ) {
		if( TMapHashStrategy::IsEqual( key, hashIndex.GetValue( entry ) ) ) {
			return &hashIndex.GetValue( entry ).Value();
		} else {
			entry = hashIndex.NextIndexPosition( entry );
		}
	}

	return nullptr;
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
template <class Key>
ValueType& CMap<KeyType, ValueType, HashStrategy, Allocator>::operator[]( const Key& key )
{
	auto result = doGet( key );
	assert( result != nullptr );
	return *result;
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
template <class Key>
const ValueType& CMap<KeyType, ValueType, HashStrategy, Allocator>::operator[]( const Key& key ) const
{
	return const_cast<CMap<KeyType, ValueType, HashStrategy, Allocator>*>( this )->operator[]( key );
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
template <class Key, class... Args>
CMapData<KeyType, ValueType>& CMap<KeyType, ValueType, HashStrategy, Allocator>::GetOrCreate( Key&& key, Args&&... valueArgs )
{
	hashIndex.InitializeIfEmpty();
	const int hash = HashStrategy::HashKey( key );
	auto entry = hashIndex.GetRootPosition( hash );
	while( !hashIndex.IsPositionFree( entry ) ) {
		if( TMapHashStrategy::IsEqual( key, hashIndex.GetValue( entry ) ) ) {
			return hashIndex.GetValue( entry );
		} else {
			entry = hashIndex.NextIndexPosition( entry );
		}
	}

	return hashIndex.InsertEndOrRoot( hash, entry, forward<Key>( key ), forward<Args>( valueArgs )... );
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
template <class Key>
bool CMap<KeyType, ValueType, HashStrategy, Allocator>::Has( const Key& key ) const
{
	return Get( key ) != nullptr;
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
template <class Key>
void CMap<KeyType, ValueType, HashStrategy, Allocator>::Delete( const Key& key )
{
	if( !hashIndex.IsEmpty() ) {
		const int hash = HashStrategy::HashKey( key );
		auto entry = hashIndex.GetRootPosition( hash );
		while( !hashIndex.IsPositionFree( entry ) ) {
			if( TMapHashStrategy::IsEqual( key, hashIndex.GetValue( entry ) ) ) {
				hashIndex.DeleteEntry( entry );
			} else {
				entry = hashIndex.NextIndexPosition( entry );
			}
		}
	}
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
void CMap<KeyType, ValueType, HashStrategy, Allocator>::Empty()
{
	hashIndex.Empty();
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
void CMap<KeyType, ValueType, HashStrategy, Allocator>::FreeBuffer()
{
	hashIndex.FreeBuffer();
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
const CMap<KeyType, ValueType, HashStrategy, Allocator>& CMap<KeyType, ValueType, HashStrategy, Allocator>::operator=( CMap<KeyType, ValueType, HashStrategy, Allocator>&& other )
{
	hashIndex = move( other.hashIndex );
	return *this;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
