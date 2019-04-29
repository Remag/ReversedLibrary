#pragma once

#include <Redefs.h>
#include <Array.h>
#include <HashTableBase.h>
#include <HashUtils.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// The hash table template.
// Uses HashStrategy::HashKey to hash values.
// Uses HashStrategy::IsEqual to compare values.
template<class Elem, class HashStrategy = CDefaultHash<Elem>, class Allocator = CRuntimeHeap>
class CHashTable {
public:
	typedef Elem ElemType;
	
	CHashTable() = default;
	// Custom dynamic allocator.
	explicit CHashTable( Allocator& allocator );
	// Explicit copy constructor.
	template <class Container>
	CHashTable( const Container& other, const CExplicitCopyTag& );
	template <class OtherAllocator>
	CHashTable( const CHashTable<Elem, HashStrategy, OtherAllocator>& other, const CExplicitCopyTag& );
	// Move constructor.
	CHashTable( CHashTable&& other );

	~CHashTable();

	int Size() const
		{ return hashIndex.Size(); }
	void ReserveBuffer( int size );
	bool IsEmpty() const
		{ return hashIndex.IsEmpty(); }
	
	// Getting, checking and setting values.
	// Set a new value. If an equal value is already set, do nothing and return false.
	template <class Key>
	bool Set( Key&& elem );
	// Get an existing value or create it using key as the constructor argument.
	template <class Key>
	const Elem& GetOrCreateValue( Key&& key );
	// Get an existing value or create it using the specified creation function.
	template <class Key, class Creator>
	const Elem& GetOrCreateValue( Key&& key, const Creator& createFunc );
	// Get a hash table element that IsEqual to key. Return nullptr if no element is found.
	template <class Key>
	const Elem* Get( const Key& key ) const;

	// Redaction.
	template <class Key>
	void Delete( const Key& elem );
	// Delete all.
	void Empty();
	// Delete all and free buffer.
	void FreeBuffer();

	// Moving objects from another hash table.
	// Destination is emptied before all operations.
	const CHashTable& operator=( CHashTable&& other );
		
	template <class Key>
	bool HasValue( const Key& elem ) const
		{ return Get( elem ) != nullptr; }

	// Range-based for loops support.
	auto begin() const
		{ return RelibInternal::CHashIndexConstIterator<Elem, HashStrategy, Allocator>( hashIndex ); }
	auto end() const
		{ return RelibInternal::CHashIndexConstIterator<Elem, HashStrategy, Allocator>( hashIndex, nullptr ); }

private:
	// Base index of the table.
	RelibInternal::CHashIndex<Elem, HashStrategy, Allocator> hashIndex;

	// Copying is prohibited.
	CHashTable( const CHashTable& ) = delete;
	void operator=( const CHashTable& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class Elem, class HashStrategy, class Allocator>
CHashTable<Elem, HashStrategy, Allocator>::CHashTable( Allocator& allocator ) :
	hashIndex( allocator )
{
}

template <class Elem, class HashStrategy, class Allocator>
template <class Container>
CHashTable<Elem, HashStrategy, Allocator>::CHashTable( const Container& other, const CExplicitCopyTag& )
{
	for( const auto& elem : other ) {
		Set( elem );
	}
}

template <class Elem, class HashStrategy, class Allocator>
template <class OtherAllocator>
CHashTable<Elem, HashStrategy, Allocator>::CHashTable( const CHashTable<Elem, HashStrategy, OtherAllocator>& other, const CExplicitCopyTag& ) :
	hashIndex( copy( other.hashIndex ) )
{
}

template <class Elem, class HashStrategy, class Allocator>
CHashTable<Elem, HashStrategy, Allocator>::CHashTable( CHashTable&& other ) :
	hashIndex( move( other.hashIndex ) )
{
}

template <class Elem, class HashStrategy, class Allocator>
CHashTable<Elem, HashStrategy, Allocator>::~CHashTable()
{
	FreeBuffer();
}

template <class Elem, class HashStrategy, class Allocator>
void CHashTable<Elem, HashStrategy, Allocator>::ReserveBuffer( int size )
{
	hashIndex.ReserveBuffer( size );
}

template <class Elem, class HashStrategy, class Allocator>
template <class Key>
bool CHashTable<Elem, HashStrategy, Allocator>::Set( Key&& elem )
{
	hashIndex.InitializeIfEmpty();
	const int hash = HashStrategy::HashKey( elem );
	auto entry = hashIndex.GetRootPosition( hash );
	while( !hashIndex.IsPositionFree( entry ) ) {
		if( HashStrategy::IsEqual( elem, hashIndex.GetValue( entry ) ) ) {
			hashIndex.GetValue( entry ) = forward<Key>( elem );
			return false;
		} else {
			entry = hashIndex.NextIndexPosition( entry );
		}
	}

	hashIndex.InsertEndOrRoot( hash, entry, forward<Key>( elem ) );
	return true;
}

template <class Elem, class HashStrategy, class Allocator>
template <class Key>
const Elem& CHashTable<Elem, HashStrategy, Allocator>::GetOrCreateValue( Key&& key )
{
	hashIndex.InitializeIfEmpty();
	const int hash = HashStrategy::HashKey( key );
	auto entry = hashIndex.GetRootPosition( hash );
	while( !hashIndex.IsPositionFree( entry ) ) {
		if( HashStrategy::IsEqual( key, hashIndex.GetValue( entry ) ) ) {
			return hashIndex.GetValue( entry );
		} else {
			entry = hashIndex.NextIndexPosition( entry );
		}
	}

	return hashIndex.InsertEndOrRoot( hash, entry, forward<Key>( key ) );
}

template <class Elem, class HashStrategy, class Allocator>
template <class Key, class Creator>
const Elem& CHashTable<Elem, HashStrategy, Allocator>::GetOrCreateValue( Key&& key, const Creator& createFunc )
{
	hashIndex.InitializeIfEmpty();
	const int hash = HashStrategy::HashKey( key );
	auto entry = hashIndex.GetRootPosition( hash );
	while( !hashIndex.IsPositionFree( entry ) ) {
		if( HashStrategy::IsEqual( key, hashIndex.GetValue( entry ) ) ) {
			return hashIndex.GetValue( entry );
		} else {
			entry = hashIndex.NextIndexPosition( entry );
		}
	}

	return hashIndex.InsertEndOrRoot( hash, entry, createFunc( forward<Key>( key ) ) );
}

template <class Elem, class HashStrategy, class Allocator>
template <class Key>
const Elem* CHashTable<Elem, HashStrategy, Allocator>::Get( const Key& key ) const
{
	if( hashIndex.IsEmpty() ) {
		return nullptr;
	}

	const int hash = HashStrategy::HashKey( key );
	auto entry = hashIndex.GetRootPosition( hash );
	while( !hashIndex.IsPositionFree( entry ) ) {
		if( HashStrategy::IsEqual( key, hashIndex.GetValue( entry ) ) ) {
			return &hashIndex.GetValue( entry );
		} else {
			entry = hashIndex.NextIndexPosition( entry );
		}
	}

	return nullptr;
}

template <class Elem, class HashStrategy, class Allocator>
template <class Key>
void CHashTable<Elem, HashStrategy, Allocator>::Delete( const Key& key )
{
	if( hashIndex.IsEmpty() ) {
		return;
	}

	const int hash = HashStrategy::HashKey( key );
	auto entry = hashIndex.GetRootPosition( hash );
	while( !hashIndex.IsPositionFree( entry ) ) {
		if( HashStrategy::IsEqual( key, hashIndex.GetValue( entry ) ) ) {
			hashIndex.DeleteEntry( entry );
			return;
		} else {
			entry = hashIndex.NextIndexPosition( entry );
		}
	}
}

template<class Elem, class HashStrategy, class Allocator>
void CHashTable<Elem, HashStrategy, Allocator>::Empty()
{
	hashIndex.Empty();
}

template<class Elem, class HashStrategy, class Allocator>
void CHashTable<Elem, HashStrategy, Allocator>::FreeBuffer()
{
	hashIndex.FreeBuffer();
}

template<class Elem, class HashStrategy, class Allocator>
const CHashTable<Elem, HashStrategy, Allocator>& CHashTable<Elem, HashStrategy, Allocator>::operator=( CHashTable<Elem, HashStrategy, Allocator>&& other )
{
	other.hashIndex.Move( hashIndex );
	return *this;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
