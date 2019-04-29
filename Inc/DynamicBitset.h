#pragma once
#include <BitSet.h>
#include <Array.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Storage that can stretch out.
template <class ContainerType>
class CDynamicBitSetStorage {
private:
	static const int bitsPerElement = CHAR_BIT * sizeof( DWORD );
	
public:
	typedef typename ContainerType::TElemType TElemType;

	CDynamicBitSetStorage(); 
	CDynamicBitSetStorage( const CDynamicBitSetStorage& other );
	CDynamicBitSetStorage( CDynamicBitSetStorage&& other );
	CDynamicBitSetStorage& operator=( CDynamicBitSetStorage other );

	ContainerType& GetStorage()
		{ return storage; }
	ContainerType& GetStorage() const
		{ return storage; }

	int Size() const
		{ return storage.Size(); }
	void SetSize( int newBitSize );
	void Empty();

	DWORD& operator[]( int index );
	DWORD operator[]( int index ) const;

	int HashKey() const;

	// Iteration support.
	DWORD* begin()
		{ return storage.begin();	}
	const DWORD* begin() const
		{ return storage.begin();	}

	DWORD* end()
		{ return storage.end(); }
	const DWORD* end() const
		{ return storage.end(); }

private:
	ContainerType storage;
};

//////////////////////////////////////////////////////////////////////////

template <class ContainerType>
CDynamicBitSetStorage<ContainerType>::CDynamicBitSetStorage()
{
}

template <class ContainerType>
CDynamicBitSetStorage<ContainerType>::CDynamicBitSetStorage( const CDynamicBitSetStorage<ContainerType>& other )
{
	storage = copy( other.storage );
}

template <class ContainerType>
CDynamicBitSetStorage<ContainerType>::CDynamicBitSetStorage( CDynamicBitSetStorage<ContainerType>&& other ) :
	storage( move( other.storage ) )
{
}

template <class ContainerType>
CDynamicBitSetStorage<ContainerType>& CDynamicBitSetStorage<ContainerType>::operator=( CDynamicBitSetStorage<ContainerType> other )
{
	swap( *this, other );
	return *this;
}

template <class ContainerType>
void CDynamicBitSetStorage<ContainerType>::SetSize( int newBitSize )
{
	const auto newByteSize = ( newBitSize + bitsPerElement - 1 ) / bitsPerElement;
	if( newByteSize > storage.Size() ) {
		storage.IncreaseSize( newByteSize );
	}
}

template <class ContainerType>
void CDynamicBitSetStorage<ContainerType>::Empty()
{
	storage.Empty();
}

template <class ContainerType>
DWORD& CDynamicBitSetStorage<ContainerType>::operator[]( int index )
{
	if( storage.Size() <= index ) {
		storage.IncreaseSize( index + 1 );
	}
	return storage[index];
}

template <class ContainerType>
DWORD CDynamicBitSetStorage<ContainerType>::operator[]( int index ) const
{
	if( storage.Size() <= index ) {
		return 0;
	}
	return storage[index];
}

template <class ContainerType>
int CDynamicBitSetStorage<ContainerType>::HashKey() const
{
	int result = 0;
	for( DWORD word : storage ) {
		result += ( result << 5 ) + word;
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// Serialization functions.
template <class ContainerType>
CArchiveReader& operator>>( CArchiveReader& archive, RelibInternal::CDynamicBitSetStorage<ContainerType>& set )
{
	set.Empty();
	archive >> set.GetStorage();
	return archive;
}

template <class ContainerType>
CArchiveWriter& operator<<( CArchiveWriter& archive, const RelibInternal::CDynamicBitSetStorage<ContainerType>& set )
{
	archive << set.GetStorage();
	return archive;
}

// Swap specialization.
template <class ContainerType>
void swap( RelibInternal::CDynamicBitSetStorage<ContainerType>& first, RelibInternal::CDynamicBitSetStorage<ContainerType>& second )
{
	swap( first.GetStorage(), second.GetStorage() );
}

//////////////////////////////////////////////////////////////////////////

template <class Allocator = CRuntimeHeap>
using CDynamicBitSet = RelibInternal::CBaseBitSet<RelibInternal::CDynamicBitSetStorage<CArray<DWORD, Allocator>>, int>;
template <int bitSize, class Allocator = CRuntimeHeap>
using CFlexibleBitSet = RelibInternal::CBaseBitSet<RelibInternal::CDynamicBitSetStorage<CFlexibleArray<DWORD, ( bitSize + CHAR_BIT * sizeof( DWORD ) - 1 ) / ( CHAR_BIT * sizeof( DWORD ) ), Allocator>>, int>;

}	// namespace Relib.

