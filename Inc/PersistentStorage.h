#pragma once
#include <Redefs.h>
#include <AllocationStrategy.h>
#include <Array.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A container for persistent data. Pointers to each element are never invalidated until container reset.
// Dynamic addition of arbitrary amount of elements is supported. 
// Internal structure is similar to deque.
// The size of a chunk in elements is defined by groupSize.
// GeneralAllocator is used to allocate additional utility structures that are not the chunks.
// GroupAllocator is used to create memory for the chunks.
template <class Elem, int groupSize, class GeneralAllocator = CRuntimeHeap, class GroupAllocator = CRuntimeHeap>
class CPersistentStorage : private CAllocationStrategy<GroupAllocator> {
public:
	CPersistentStorage() = default;
	// Specifying custom dynamic allocators.
	explicit CPersistentStorage( GroupAllocator& allocator );
	CPersistentStorage( GeneralAllocator& generalAllocator, GroupAllocator& groupAllocator );
	// Explicit construction from another container.
	template <class Container>
	CPersistentStorage( const Container& other, const CExplicitCopyTag& );

	// Implicit copying is prohibited, use the explicit copy mechanism.
	CPersistentStorage( CPersistentStorage&& other );

	~CPersistentStorage()
		{ FreeBuffer(); }

	int Size() const
		{ return size; }
	bool IsEmpty() const
		{ return Size() == 0; }

	// Fill the storage with default values up to a given size value.
	void IncreaseSize( int newSize );
	void IncreaseSizeNoInitialize( int newSize );
	
	// Element access.
	Elem& operator[]( int pos );
	const Elem& operator[]( int pos ) const
		{ return const_cast<CPersistentStorage*>( this )->operator[]( pos ); }
	Elem& First();
	const Elem& First() const
		{ return const_cast<CPersistentStorage*>( this )->First(); }
	Elem& Last();
	const Elem& Last() const
		{ return const_cast<CPersistentStorage*>( this )->Last(); }

	// Raw access to chunks.
	int GetGroupCount() const;
	Elem* GetGroupPtr( int groupPos );
	const Elem* GetGroupPtr( int pos ) const
		{ return const_cast<CPersistentStorage*>( this )->GetGroupPtr( pos ); }

	// Size manipulation.
	void ReserveBuffer( int newElemCount );

	// Contents manipulation.

	// Construct an Elem with given arguments and add it to storage
	template <class ...AddArgs>
	Elem& Add( AddArgs&&... args );

	// Delete all the elements. Buffer size is unchanged.
	void Empty();
	// Delete all the elements and free the buffer.
	void FreeBuffer();

	// Copying and moving objects to another storage.
	// Destination is emptied before all operations.
	const CPersistentStorage& operator=( CPersistentStorage&& other );

private:
	// An array of subarrays, each pointing to a chunk in the deque.
	CArray<Elem*, GeneralAllocator> groups;
	// Element count.
	int size = 0;

	Elem& getAt( unsigned pos );
	void growBuffer( int minGrowth );
	void reserveGroups( int newElemCount );
	int capacity() const;
};

//////////////////////////////////////////////////////////////////////////

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::CPersistentStorage( GroupAllocator& allocator ) :
	CAllocationStrategy<GroupAllocator>( allocator )
{
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::CPersistentStorage( GeneralAllocator& generalAllocator, GroupAllocator& groupAllocator ) :
	CAllocationStrategy<GroupAllocator>( groupAllocator ),
	groups( generalAllocator )
{
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
template <class Container>
CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::CPersistentStorage( const Container& other, const CExplicitCopyTag& ) :
	size( other.Size() )
{
	reserveGroups( size );

	int i = 0;
	for( const auto& elem : other ) {
		const int groupIndex = i / groupSize;
		const int groupOffset = i % groupSize;
		Elem* destElem = groups[groupIndex] + groupOffset;
		::new( destElem ) Elem( other[i] );
		i++;
	}
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::CPersistentStorage( CPersistentStorage&& other ) :
	CAllocationStrategy<GroupAllocator>( move( other ) ),
	groups( move( other.groups ) ),
	size( other.size )
{
	other.size = 0;
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
void CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::growBuffer( int minGrowth )
{
	const int newElemCount = minGrowth - ( capacity() - size );
	reserveGroups( newElemCount );
}

template <class Elem, int groupSize, class GeneralAllocator, class GroupAllocator>
void CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::reserveGroups( int newElemCount )
{
	static const int groupByteSize = groupSize * sizeof( Elem );

	if( newElemCount > 0 ) {
		const int newGroupCount = Ceil( newElemCount, groupSize );
		for( int i = 0; i < newGroupCount; i++ ) {
			groups.Add( static_cast<Elem*>( RELIB_STATIC_ALLOCATE( GroupAllocator, groupByteSize ) ) );
		}
	}
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
int CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::capacity() const
{
	return groups.Size() * groupSize;
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
void CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::IncreaseSize( int newSize )
{
	assert( newSize >= size );
	ReserveBuffer( newSize );

	for( int i = size; i < newSize; i++ ) {
		const int lastGroupIndex = i / groupSize;
		const int lastGroupOffset = i % groupSize;

		::new( groups[lastGroupIndex] + lastGroupOffset ) Elem();
	}
	size = newSize;
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
void CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::IncreaseSizeNoInitialize( int newSize )
{
	assert( newSize >= size );
	ReserveBuffer( newSize );

	for( int i = size; i < newSize; i++ ) {
		const int lastGroupIndex = i / groupSize;
		const int lastGroupOffset = i % groupSize;

		::new( groups[lastGroupIndex] + lastGroupOffset ) Elem;
	}
	size = newSize;
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
Elem& CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::operator[]( int pos )
{
	assert( pos >= 0 && pos < Size() );
	// A hint to the compiler to ensure modulo operator optimization for power of two group sizes.
	return getAt( static_cast<unsigned>( pos ) );
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
Elem& CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::getAt( unsigned pos )
{
	const int groupIndex = pos / groupSize;
	const int groupOffset = pos % groupSize;
	return groups[groupIndex][groupOffset];
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
Elem& CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::First()
{
	assert( !IsEmpty() );
	return groups[0][0];
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
Elem& CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::Last()
{
	assert( !IsEmpty() );
	return this->operator[]( Size() - 1 );
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
int CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::GetGroupCount() const
{
	return Ceil( size, groupSize );
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
Elem* CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::GetGroupPtr( int groupPos )
{
	assert( groupPos >= 0 && size > 0 && groupPos * groupSize < size );
	return groups[groupPos];
}

 template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
void CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::ReserveBuffer( int newElemCount )
{
	assert( newElemCount >= 0 );
	growBuffer( newElemCount - size );
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
template <class ...AddArgs>
Elem& CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::Add( AddArgs&&... args )
{
	const int lastGroupOffset = size % groupSize;
	if( lastGroupOffset == 0 ) {
		growBuffer( 1 );
	}
	const int lastGroupIndex = size / groupSize;
	Elem* result = ::new( groups[lastGroupIndex] + lastGroupOffset ) Elem( forward<AddArgs>( args )... );
	size++;
	return *result;
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
void CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::Empty()
{
	for( int i = 0; i < size; i++ ) {
		const int groupIndex = i / groupSize;
		const int groupOffset = i % groupSize;
		Elem* elem = groups[groupIndex] + groupOffset;
		// Optimizer thinks that elem is not referenced for some reason.
		elem;
		elem->~Elem();
	}
	size = 0;
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
void CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::FreeBuffer()
{
	Empty();
	for( auto group : groups ) {
		this->StrategyFree( group );
	}
	groups.FreeBuffer();
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
const CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>& CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>::operator=( 
	CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>&& other )
{
	assert( &other != this );
	FreeBuffer();

	groups = move( other.groups );
	size = other.size;
	other.size = 0;
	CAllocationStrategy::operator=( move( other ) );

	return *this;
}

//////////////////////////////////////////////////////////////////////////

// Range-based for loops support.
namespace RelibInternal {

template <class Elem, int groupSize, class GeneralAllocator, class GroupAllocator>
class CPersistentStorageIterator {
public:
	CPersistentStorageIterator( CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>& _storage, int _pos ) : storage( _storage ), pos( _pos ) {}

	void operator++()
		{ pos++; }
	Elem& operator*()
		{ return storage[pos]; }
	bool operator!=( const CPersistentStorageIterator& other ) const
		{ return pos != other.pos; }

private:
	CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>& storage;
	int pos;
};

template <class Elem, int groupSize, class GeneralAllocator, class GroupAllocator>
class CPersistentStorageConstIterator {
public:
	CPersistentStorageConstIterator( const CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>& _storage, int _pos ) : storage( _storage ), pos( _pos ) {}

	void operator++()
		{ pos++; }
	const Elem& operator*() const
		{ return storage[pos]; }
	bool operator!=( const CPersistentStorageConstIterator& other ) const
		{ return pos != other.pos; }

private:
	const CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>& storage;
	int pos;
};

}

template <class Elem, int groupSize, class GeneralAllocator, class GroupAllocator>
RelibInternal::CPersistentStorageIterator<Elem, groupSize, GeneralAllocator, GroupAllocator> begin( CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>& storage )
{
	return RelibInternal::CPersistentStorageIterator<Elem, groupSize, GeneralAllocator, GroupAllocator>( storage, 0 );
}

template <class Elem, int groupSize, class GeneralAllocator, class GroupAllocator>
RelibInternal::CPersistentStorageIterator<Elem, groupSize, GeneralAllocator, GroupAllocator> end( CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>& storage )
{
	return RelibInternal::CPersistentStorageIterator<Elem, groupSize, GeneralAllocator, GroupAllocator>( storage, storage.Size() );
}

template <class Elem, int groupSize, class GeneralAllocator, class GroupAllocator>
RelibInternal::CPersistentStorageConstIterator<Elem, groupSize, GeneralAllocator, GroupAllocator> begin( const CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>& storage )
{
	return RelibInternal::CPersistentStorageConstIterator<Elem, groupSize, GeneralAllocator, GroupAllocator>( storage, 0 );
}

template <class Elem, int groupSize, class GeneralAllocator, class GroupAllocator>
RelibInternal::CPersistentStorageConstIterator<Elem, groupSize, GeneralAllocator, GroupAllocator> end( const CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>& storage )
{
	return RelibInternal::CPersistentStorageConstIterator<Elem, groupSize, GeneralAllocator, GroupAllocator>( storage, storage.Size() );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

