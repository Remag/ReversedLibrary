#pragma once
#include <HashUtils.h>
#include <ExplicitCopy.h>
#include <BaseBlockAllocator.h>

namespace Relib {

namespace RelibInternal {

// An entry in the hash table index.
// Entries with the same hash value form a single linked list.
template <class T>
struct CIndexEntry {
	T Value;
	// Next position in the entry bucket.
	CIndexEntry<T>* NextInBucket = nullptr;

	template <class... ValueArgs>
	explicit CIndexEntry( ValueArgs&&... args ) : Value( forward<ValueArgs>( args )... ) {}
};

//////////////////////////////////////////////////////////////////////////

// Classes for accessing hash index values consequentially.
template <class T, class Startegy, class Allocator>
class CHashIndexConstIterator {
public:
	CHashIndexConstIterator( const RelibInternal::CHashIndex<T, Startegy, Allocator>& _hashIndex ) : hashIndex( _hashIndex ), indexPos( nullptr ) { nextIndexPos( 0 ); }
	CHashIndexConstIterator( const RelibInternal::CHashIndex<T, Startegy, Allocator>& _hashIndex, CIndexEntry<T>* indexPosPtr ) : hashIndex( _hashIndex ), indexPos( indexPosPtr ) {}

	const T& operator*() const
		{ return indexPos->Value; }
	// An increment operator.
	// Range-based for loops don't require the ++operator to return a value.
	void operator++();
		
	bool operator!=( const CHashIndexConstIterator<T, Startegy, Allocator>& other ) const
		{ return indexPos != other.indexPos; }

protected:
	const CHashIndex<T, Startegy, Allocator>& hashIndex;
	int index = 0;
	CIndexEntry<T>* indexPos;

	void nextIndexPos( int startPos );
};

template <class T, class Startegy, class Allocator>
void CHashIndexConstIterator<T, Startegy, Allocator>::operator++()
{
	if( indexPos->NextInBucket != nullptr ) {
		indexPos = indexPos->NextInBucket;
		return;
	}
	nextIndexPos( index + 1 );
}

//////////////////////////////////////////////////////////////////////////

template <class T, class Startegy, class Allocator>
void CHashIndexConstIterator<T, Startegy, Allocator>::nextIndexPos( int startPos )
{
	const int indexSize = hashIndex.IndexSize();
	for( int i = startPos; i < indexSize; i++ ) {
		const auto ptr = *hashIndex.GetTablePosition( i );
		if( ptr != nullptr ) {
			indexPos = ptr;
			index = i;
			return;
		}
	}
	indexPos = nullptr;
}

template <class T, class Startegy, class Allocator>
class CHashIndexIterator : public CHashIndexConstIterator<T, Startegy, Allocator> {
public:
	using CHashIndexConstIterator<T, Startegy, Allocator>::CHashIndexConstIterator;

	T& operator*()
		{ return this->indexPos->Value; }
};

//////////////////////////////////////////////////////////////////////////

// Class containing an index of hashed values.
// Index is hashed by KeyType and contains values of ContainedType.
// HashStrategy::HashKey is used to hash values of ContainedType.
template <class ContainedType, class HashStrategy, class Allocator>
class CHashIndex {
public:
	CHashIndex() = default;
	// Custom dynamic allocator.
	explicit CHashIndex( Allocator& allocator );
	CHashIndex( CHashIndex&& other );
	template <class OtherAllocator>
	CHashIndex( const CHashIndex<ContainedType, HashStrategy, OtherAllocator>& other, const CExplicitCopyTag& );

	CHashIndex& operator=( CHashIndex&& other );

	~CHashIndex();

	void InitializeIfEmpty();
	int IndexSize() const	
		{ return index.Size(); }
	int Size() const
		{ return elementCount; }
	bool IsEmpty() const
		{ return elementCount == 0; }
	void ReserveBuffer( int size );

	// Delete everything.
	void Empty();
	// Delete everything and free buffer.
	void FreeBuffer();

	CIndexEntry<ContainedType>** GetTablePosition( int indexPos )
		{ return &index[indexPos]; }
	CIndexEntry<ContainedType>** GetTablePosition( int indexPos ) const
		{ return const_cast<CHashIndex<ContainedType, HashStrategy, Allocator>*>( this )->GetTablePosition( indexPos ); }

	// Get position by hash.
	CIndexEntry<ContainedType>** GetRootPosition( int hash );
	CIndexEntry<ContainedType>** GetRootPosition( int hash ) const
		{ return const_cast<CHashIndex<ContainedType, HashStrategy, Allocator>*>( this )->GetRootPosition( hash ); }
	CIndexEntry<ContainedType>** NextIndexPosition( CIndexEntry<ContainedType>** pos ) const;
	bool IsPositionFree( CIndexEntry<ContainedType>** pos ) const;

	ContainedType& GetValue( CIndexEntry<ContainedType>** pos );
	const ContainedType& GetValue( CIndexEntry<ContainedType>** pos ) const
		{ return const_cast<CHashIndex<ContainedType, HashStrategy, Allocator>*>( this )->GetValue( pos ); }

	// Element insertion.
	template <class... Args>
	ContainedType& InsertEndOrRoot( int hash, CIndexEntry<ContainedType>** pos, Args&&... containedTypeArgs );
	template <class... Args>
	ContainedType& InsertRoot( int hash, CIndexEntry<ContainedType>** pos, Args&&... containedTypeArgs );

	// Deletion. Position is assumed to have an element.
	void DeleteEntry( CIndexEntry<ContainedType>** entry );

private:
	// Index that contains pointers to entries.
	// A position in the index corresponds to a unique hash key value.
	// Entries with the same hash for a linked list.
	typedef CIndexEntry<ContainedType> TIndexEntry;
	CArray<TIndexEntry*, Allocator> index;
	// Total number of elements in the table.
	int elementCount = 0;
	// Index size on the next recalculation.
	int nextIndexSize = 1;

	// Allocator that manages actual elements' position in memory.
	typedef RelibInternal::CBaseBlockAllocator<sizeof( TIndexEntry ), alignof( TIndexEntry ), 0 , Allocator, CDynamicElemResizeStrategy<TIndexEntry, 16>> TEntryAllocator;
	TEntryAllocator elementAllocator;
	typedef typename TEntryAllocator::TPage TAllocatorPage;

	void initialize();
	void clearIndex();

	static float getOptimalElementCount( int indexSize );
	void recalcIndex();
	void recalcEntry( BYTE* entryPtr );
	template <class Key>
	static int getRootPosition( const Key& key, int tableSize );
	static int getHashTablePosition( int hash, int tableSize );

	void deleteEntry( TIndexEntry* entry );
	template <class... Args>
	ContainedType& doInsertRoot( CIndexEntry<ContainedType>** pos, Args&&... containedTypeArgs );

	// Copying is prohibited.
	CHashIndex( CHashIndex& ) = delete;
	void operator=( CHashIndex& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class ContainedType, class HashStrategy, class Allocator>
CHashIndex<ContainedType, HashStrategy, Allocator>::CHashIndex( Allocator& allocator ) :
	index( allocator ),
	elementAllocator( allocator )
{
}

template <class ContainedType, class HashStrategy, class Allocator>
CHashIndex<ContainedType, HashStrategy, Allocator>::CHashIndex( CHashIndex&& other ) :
	elementCount( other.elementCount ),
	nextIndexSize( other.nextIndexSize ),
	index( move( other.index ) ),
	elementAllocator( move( other.elementAllocator ) )
{
	other.elementCount = 0;
}

template <class ContainedType, class HashStrategy, class Allocator>
template <class OtherAllocator>
CHashIndex<ContainedType, HashStrategy, Allocator>::CHashIndex( const CHashIndex<ContainedType, HashStrategy, OtherAllocator>& other, const CExplicitCopyTag& ) :
	elementCount( other.elementCount ),
	nextIndexSize( other.nextIndexSize )
{
	const int indexSize = other.index.Size();
	index.IncreaseSizeNoInitialize( indexSize );
	elementAllocator.Reserve( elementCount * sizeof( TIndexEntry ) );

	for( int i = 0; i < indexSize; i++ ) {
		TIndexEntry** newEntry = &index[i];
		const TIndexEntry* otherEntry = other.index[i];
		while( otherEntry != nullptr ) {
			void* newMemory = elementAllocator.AllocateBlock();
			TIndexEntry* newElem = ::new( newMemory ) TIndexEntry{ copy( otherEntry->Value ) };
			( *newEntry ) = newElem;
			newEntry = &newElem->NextInBucket;
			otherEntry = otherEntry->NextInBucket;
		}
		( *newEntry ) = nullptr;
	}	
}

template <class ContainedType, class HashStrategy, class Allocator>
CHashIndex<ContainedType, HashStrategy, Allocator>::~CHashIndex()
{
	FreeBuffer();
}

template <class ContainedType, class HashStrategy, class Allocator>
void CHashIndex<ContainedType, HashStrategy, Allocator>::InitializeIfEmpty()
{
	if( index.Size() == 0 ) {
		initialize();
	}
}

template <class ContainedType, class HashStrategy, class Allocator>
void CHashIndex<ContainedType, HashStrategy, Allocator>::initialize()
{
	assert( nextIndexSize > index.Size() );
	const int newSize = GetPrimeHashTableSize( nextIndexSize );
	index.IncreaseSizeNoInitialize( newSize );
	clearIndex();
	nextIndexSize = newSize + 1;
}

template <class ContainedType, class HashStrategy, class Allocator>
void CHashIndex<ContainedType, HashStrategy, Allocator>::clearIndex()
{
	::memset( index.Ptr(), 0, sizeof( TIndexEntry* ) * index.Size() );
}

template <class ContainedType, class HashStrategy, class Allocator>
void CHashIndex<ContainedType, HashStrategy, Allocator>::ReserveBuffer( int size )
{
	nextIndexSize = max( size, nextIndexSize );
	const int allocatedSize = elementAllocator.GetTotalSize();
	const int requiredSize = size * sizeof( TIndexEntry );
	if( allocatedSize < requiredSize ) {
		elementAllocator.HintNextPageSize( requiredSize - allocatedSize );
	}
}

template <class ContainedType, class HashStrategy, class Allocator>
float CHashIndex<ContainedType, HashStrategy, Allocator>::getOptimalElementCount( int indexSize )
{
	// The index uses a compile-time defined maximum load factor of 75%.
	return indexSize * 0.75f;
}

// Increase the size of the index to match the current hash table size and rehash everything.
template <class ContainedType, class HashStrategy, class Allocator>
void CHashIndex<ContainedType, HashStrategy, Allocator>::recalcIndex()
{
	initialize();

	// Go through all the allocated pages searching for active elements.
	int elementsToFind = elementCount;
	if( elementsToFind == 0 ) {
		return;
	}

	// Iterate through allocator elements consequentially.
	// The allocator is guaranteed to be densely filled at the time of recalculation.
	const int entrySize = sizeof( TIndexEntry );
	TAllocatorPage* currentPage = elementAllocator.GetCurrentPage();
	assert( currentPage != nullptr );
	for( auto page = currentPage->Next; page != nullptr; page = page->Next ) {
		const int pageEndPos = page->PageSize - entrySize;
		for( int pagePos = 0; pagePos <= pageEndPos; pagePos += entrySize ) {
			assert( elementsToFind > 0 );
			BYTE* entryPtr = page->PageData + pagePos;
			recalcEntry( entryPtr );
			elementsToFind--;
		}
	}

	// Current page might not be filled, handle it last.
	for( int pagePos = 0; elementsToFind > 0; pagePos += entrySize ) {
		BYTE* entryPtr = currentPage->PageData + pagePos;
		assert( pagePos <= currentPage->PageSize - entrySize );
		recalcEntry( entryPtr );
		elementsToFind--;
	}
}

template <class ContainedType, class HashStrategy, class Allocator>
void CHashIndex<ContainedType, HashStrategy, Allocator>::recalcEntry( BYTE* entryPtr )
{
	assert( entryPtr != nullptr );
	TIndexEntry* currentEntry = reinterpret_cast<TIndexEntry*>( entryPtr );
	const ContainedType& elem = currentEntry->Value;
	const int hashPos = getRootPosition( elem, index.Size() );
	auto& rootEntry = index[hashPos];
	currentEntry->NextInBucket = rootEntry;
	rootEntry = currentEntry;
}

template <class ContainedType, class HashStrategy, class Allocator>
ContainedType& CHashIndex<ContainedType, HashStrategy, Allocator>::GetValue( CIndexEntry<ContainedType>** pos )
{
	return ( *pos )->Value;
}

template <class ContainedType, class HashStrategy, class Allocator>
bool CHashIndex<ContainedType, HashStrategy, Allocator>::IsPositionFree( CIndexEntry<ContainedType>** pos ) const
{
	return ( *pos ) == nullptr;
}

template <class ContainedType, class HashStrategy, class Allocator>
void CHashIndex<ContainedType, HashStrategy, Allocator>::Empty()
{
	if( elementCount != 0 ) {
		for( auto& bucket : index ) {
			auto entry = bucket;
			bucket = nullptr;
			while( entry != nullptr ) {
				auto nextEntry = entry->NextInBucket;
				deleteEntry( entry );
				entry = nextEntry;
			}
		}
		elementCount = 0;
	}
}

template <class ContainedType, class HashStrategy, class Allocator>
void CHashIndex<ContainedType, HashStrategy, Allocator>::deleteEntry( TIndexEntry* entry )
{
	ContainedType& elem = entry->Value;
	elem;
	elem.~ContainedType();
	elementAllocator.Free( entry );
}

template <class ContainedType, class HashStrategy, class Allocator>
void CHashIndex<ContainedType, HashStrategy, Allocator>::FreeBuffer()
{
	if( elementCount != 0 ) {
		for( auto entry : index ) {
			while( entry != nullptr ) {
				ContainedType& elem = entry->Value;
				elem;
				elem.~ContainedType();
				entry = entry->NextInBucket;
			}
		}
	}
	index.FreeBuffer();
	elementAllocator.Reset();
}

template <class ContainedType, class HashStrategy, class Allocator>
CIndexEntry<ContainedType>** CHashIndex<ContainedType, HashStrategy, Allocator>::GetRootPosition( int hash )
{
	const int tableSize = index.Size();
	assert( tableSize > 0 );
	const int hashPos = getHashTablePosition( hash, tableSize );
	return &index[hashPos];
}

template <class ContainedType, class HashStrategy, class Allocator>
template <class Key>
int CHashIndex<ContainedType, HashStrategy, Allocator>::getRootPosition( const Key& key, int tableSize )
{
	const int hash = HashStrategy::HashKey( key );
	return getHashTablePosition( hash, tableSize );
}

template <class ContainedType, class HashStrategy, class Allocator>
int CHashIndex<ContainedType, HashStrategy, Allocator>::getHashTablePosition( int hash, int tableSize )
{
	return static_cast<unsigned>( hash ) % static_cast<unsigned>( tableSize );
}

template <class ContainedType, class HashStrategy, class Allocator>
CIndexEntry<ContainedType>** CHashIndex<ContainedType, HashStrategy, Allocator>::NextIndexPosition( CIndexEntry<ContainedType>** pos ) const
{
	return &( *pos )->NextInBucket;
}

template <class ContainedType, class HashStrategy, class Allocator>
template <class... Args>
ContainedType& CHashIndex<ContainedType, HashStrategy, Allocator>::InsertEndOrRoot( int hash, CIndexEntry<ContainedType>** endPos, Args&&... containedTypeArgs )
{
	assert( IsPositionFree( endPos ) );
	const float optimalElementCount = getOptimalElementCount( index.Size() );
	if( optimalElementCount < elementCount + 1 ) {
		// Recalculate index, find a new position for the element and put it there.
		recalcIndex();
		const auto newPos = GetRootPosition( hash );
		return doInsertRoot( newPos, forward<Args>( containedTypeArgs )... );
	} else {
		// No recalculation needed, simply insert the element.
		TIndexEntry* newElem = ::new( elementAllocator.AllocateBlock() ) TIndexEntry( forward<Args>( containedTypeArgs )... );
		*( endPos ) = newElem;
		elementCount++;
		return newElem->Value;
	}
}

template <class ContainedType, class HashStrategy, class Allocator>
template <class... Args>
ContainedType& CHashIndex<ContainedType, HashStrategy, Allocator>::InsertRoot( int hash, CIndexEntry<ContainedType>** pos, Args&&... containedTypeArgs )
{
	const float optimalElementCount = getOptimalElementCount( index.Size() );
	if( optimalElementCount < elementCount + 1 ) {
		// Recalculate index, find a new position for the element and put it there.
		recalcIndex();
		const auto newPos = GetRootPosition( hash );
		return doInsertRoot( newPos, forward<Args>( containedTypeArgs )... );
	} else {
		// No recalculation needed, simply insert the element.
		return doInsertRoot( pos, forward<Args>( containedTypeArgs )... );
	}
}

template <class ContainedType, class HashStrategy, class Allocator>
template <class... Args>
ContainedType& CHashIndex<ContainedType, HashStrategy, Allocator>::doInsertRoot( CIndexEntry<ContainedType>** pos, Args&&... containedTypeArgs )
{	
	TIndexEntry* newElem = ::new( elementAllocator.AllocateBlock() ) TIndexEntry( forward<Args>( containedTypeArgs )... );
	newElem->NextInBucket = *pos;
	*( pos ) = newElem;
	elementCount++;
	return newElem->Value;
}

template <class ContainedType, class HashStrategy, class Allocator>
void CHashIndex<ContainedType, HashStrategy, Allocator>::DeleteEntry( CIndexEntry<ContainedType>** pos )
{
	assert( pos != nullptr );
	auto& entry = *pos;
	const auto nextEntry = entry->NextInBucket;
	deleteEntry( entry );
	entry = nextEntry;
	elementCount--;
}

template <class ContainedType, class HashStrategy, class Allocator>
CHashIndex<ContainedType, HashStrategy, Allocator>& CHashIndex<ContainedType, HashStrategy, Allocator>::operator=( CHashIndex<ContainedType, HashStrategy, Allocator>&& other )
{
	FreeBuffer();
	index = move( other.index );
	nextIndexSize = other.nextIndexSize;
	elementAllocator = move( other.elementAllocator );
	elementCount = other.elementCount;
	other.elementCount = 0;
	other.nextIndexSize = 1;
	return *this;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

