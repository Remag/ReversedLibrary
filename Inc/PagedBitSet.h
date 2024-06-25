#pragma once
#include <Redefs.h>
#include <BitSet.h>
#include <StackArray.h>
#include <PtrOwner.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

template <int bitSetSize, int pageSize, class Allocator>
class CPagedStorage;
//////////////////////////////////////////////////////////////////////////

// Iteration support for paged storage.
template<int bitSetSize, int pageSize, class Allocator>
class CPagedStorageIterator {
public:	
	CPagedStorageIterator( CPagedStorage<bitSetSize, pageSize, Allocator>& _storage, int _pos ) : storage( _storage ), pos( _pos ) {}

	void operator++()
		{ pos++; }

	DWORD& operator*() 
		{ return storage[pos]; }

	bool operator!=( CPagedStorageIterator<bitSetSize, pageSize, Allocator>& other ) const
		{ return pos != other.pos; }
	
private:
	CPagedStorage<bitSetSize, pageSize, Allocator>& storage;
	int pos;
};

template<int bitSetSize, int pageSize, class Allocator>
class CPagedStorageConstIterator {
public:	
	CPagedStorageConstIterator( const CPagedStorage<bitSetSize, pageSize, Allocator>& _storage, int _pos ) : storage( _storage ), pos( _pos ) {}

	void operator++()
	{ pos++; }

	const auto& operator*() const
	{ return storage[pos]; }

	bool operator!=( CPagedStorageConstIterator<bitSetSize, pageSize, Allocator>& other ) const
	{ return pos != other.pos; }

private:
	const CPagedStorage<bitSetSize, pageSize, Allocator>& storage;
	int pos;
};

//////////////////////////////////////////////////////////////////////////

template <int bitSetSize, int pageSize, class Allocator>
class CPagedStorage {
public:
	using TElemType = unsigned __int64;
	static const int bitsPerElement = CHAR_BIT * sizeof( TElemType );
	static const int realPageSize = ( ( pageSize + bitsPerElement - 1 ) / bitsPerElement ) * bitsPerElement;
	using TPage = CBitSet<realPageSize>;
	static const int PageSizeInBytes = sizeof( TPage );
	static const int PagesCount = ( bitSetSize + realPageSize - 1 ) / realPageSize;

	CStackArray<CPtrOwner<TPage, Allocator>, PagesCount>& Pages()
		{ return pages; }

	CPagedStorage() = default;
	CPagedStorage( const CPagedStorage& other );
	CPagedStorage( CPagedStorage&& other );
	CPagedStorage& operator=( CPagedStorage other );

	static int BitSize()
		{ return bitSetSize; }
	static int StorageSize()
		{ return PagesCount * realPageSize / bitsPerElement; }
	void Empty();

	TElemType& operator[]( int index );
	TElemType operator[]( int index ) const;

	int HashKey() const;

	// Iteration support.
	CPagedStorageIterator<bitSetSize, pageSize, Allocator> begin()
		{ return CPagedStorageIterator<bitSetSize, pageSize, Allocator>( *this, 0 ); }
	CPagedStorageConstIterator<bitSetSize, pageSize, Allocator> begin() const
		{ return CPagedStorageConstIterator<bitSetSize, pageSize, Allocator>( *this, 0 ); }

	CPagedStorageIterator<bitSetSize, pageSize, Allocator> end()
		{ return CPagedStorageIterator<bitSetSize, pageSize, Allocator>( *this, StorageSize() ); }
	CPagedStorageConstIterator<bitSetSize, pageSize, Allocator> end() const
		{ return CPagedStorageConstIterator<bitSetSize, pageSize, Allocator>( *this, StorageSize() ); }

private:
	CStackArray<CPtrOwner<TPage, Allocator>, PagesCount> pages;
};

//////////////////////////////////////////////////////////////////////////

template <int bitSetSize, int pageSize, class Allocator>
CPagedStorage<bitSetSize, pageSize, Allocator>::CPagedStorage( const CPagedStorage<bitSetSize, pageSize, Allocator>& other )
{
	for( int i = 0; i < PagesCount; i++ ) {
		if( other.pages[i] != 0 ) {
			pages[i] = CreateOwner<TPage, Allocator>( other );
		}
	}
}

template <int bitSetSize, int pageSize, class Allocator>
CPagedStorage<bitSetSize, pageSize, Allocator>::CPagedStorage( CPagedStorage<bitSetSize, pageSize, Allocator>&& other ) :
	pages( move( other.pages ) )
{
}

template <int bitSetSize, int pageSize, class Allocator>
CPagedStorage<bitSetSize, pageSize, Allocator>& CPagedStorage<bitSetSize, pageSize, Allocator>::operator=( CPagedStorage<bitSetSize, pageSize, Allocator> other )
{
	swap( *this, other );
	return *this;
}

template <int bitSetSize, int pageSize, class Allocator>
void CPagedStorage<bitSetSize, pageSize, Allocator>::Empty()
{
	for( auto& page : pages ) {
		page.Release();
	}
}

template <int bitSetSize, int pageSize, class Allocator>
unsigned __int64& CPagedStorage<bitSetSize, pageSize, Allocator>::operator[]( int index )
{
	static const int pageSizeInWords = realPageSize / bitsPerElement;

	const int pageIndex = index / pageSizeInWords;
	const int posInPage = index % pageSizeInWords;
	if( pages[pageIndex] == nullptr ) {
		pages[pageIndex] = CreateOwner<TPage, Allocator>();
	}
	return pages[pageIndex]->GetStorage()[posInPage];
}

template <int bitSetSize, int pageSize, class Allocator>
unsigned __int64 CPagedStorage<bitSetSize, pageSize, Allocator>::operator[]( int index ) const
{
	static const int pageSizeInWords = realPageSize / bitsPerElement;

	const int pageIndex = index / pageSizeInWords;
	const int posInPage = index % pageSizeInWords;
	return pages[pageIndex] == nullptr ? 0 : pages[pageIndex]->GetStorage()[posInPage];
}

template <int bitSetSize, int pageSize, class Allocator>
int CPagedStorage<bitSetSize, pageSize, Allocator>::HashKey() const
{
	int result = 0;
	// Start from the end.
	// This way hash starts changing only when we find the first element.
	// All empty pages at the end are ignored.
	for( int i = PagesCount - 1; i >= 0; i-- ) {
		if( pages[i] == 0 ) {
			result += result << 5;
		} else {
			result += ( result << 5 ) + pages[i]->HashKey();
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

// Swap specialization.
template <int bitSetSize, int pageSize, class Allocator>
void swap( RelibInternal::CPagedStorage<bitSetSize, pageSize, Allocator>& first, RelibInternal::CPagedStorage<bitSetSize, pageSize, Allocator>& second )
{
	assert( first.PagesCount == second.PagesCount );
	swap( first.Pages(), second.Pages() );
}

//////////////////////////////////////////////////////////////////////////

template <int bitSetSize, int pageSize, class Allocator = CRuntimeHeap>
using CPagedBitSet = RelibInternal::CBaseBitSet<RelibInternal::CPagedStorage<bitSetSize, pageSize, Allocator>, int>;

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

