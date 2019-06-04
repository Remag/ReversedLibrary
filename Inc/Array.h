#pragma once
#include <TemplateUtils.h>
#include <ArrayData.h>
#include <AllocationStrategy.h>
#include <GrowStrategy.h>
#include <StaticAllocators.h>
#include <ExplicitCopy.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A list that provides random access to its values.
// The values are stored consecutively in a preallocated memory buffer. 
// Adding to a full array results in a buffer resize operation : a larger buffer is allocated and old values are moved into it.
// GrowStrategy controls the size of the new buffer.
// Allocator controls the allocation method.
template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
class CArray : private CAllocationStrategy<Allocator>, public RelibInternal::CArrayData<Elem> {
public:
	// Default constructor for simple array storage type.
	CArray();
	// Construct an array with a specific allocator.
	explicit CArray( Allocator& memoryAllocator );
	// Explicit construction from another container.
	template <class Container>
	CArray( const Container& other, const CExplicitCopyTag& );

	// Move construction.
	CArray( CArray&& other );

	~CArray();

	operator CArrayBuffer<Elem>()
		{ return this->Buffer(); }

	// Destroy all elements. Buffer is not freed.
	void Empty();

	// Buffer size information.
	int Capacity() const
		{ return bufferSize; }

	// Size manipulation.
	void ReserveBuffer( int newBufferSize );
	void IncreaseSize( int newSize );
	// Change an arrays size without zero initializing the elements.
	// Only arrays of POD elements can use this method.
	void IncreaseSizeNoInitialize( int newSize );

	// Contents manipulation.

	// Construct an Elem with given arguments and add it to array.
	template <class... AddArgs>
	Elem& Add( AddArgs&&... args );
	// Add arrays asserting that capacity will not be exceeded.
	template <class... AddArgs>
	Elem& AddWithinCapacity( AddArgs&&... args );

	// Redaction.
	// Insert using an arbitrary constructor.
	template <class... Args>
	Elem& InsertAt( int pos, Args&&... elemArgs );
	void DeleteAt( int pos, int count = 1 );
	void DeleteLast( int count = 1 );
	// Find all array elements for which shouldSwap is true and delete them.
	template <class BoolFunc>
	void DeleteMatching( const BoolFunc& shouldSwap );

	// Delete all the elements and free the buffer.
	void FreeBuffer();

	// Move objects from another array.
	// Destination is emptied before all operations.
	const CArray& operator=( CArray&& other );

private:
	// Current size of the preallocated buffer in elements.
	int bufferSize = 0;

	Elem* allocateBuffer( int size );

	void grow( int newSize );
	void growAt( int pos, int newSize );
	void reallocateBuffer( int newBufferSize );
	
	static void internalMoveElementsLeft( Elem* src, int srcPos, int destPos, int count );
	static void doInternalMoveElementsLeft( Elem* src, Elem* dest, int count, Types::TrueType triviallyCopyableTag );
	static void doInternalMoveElementsLeft( Elem* src, Elem* dest, int count, Types::FalseType triviallyCopyableTag );

	static void internalMoveElementsRight( Elem* src, int srcPos, int destPos, int count );
	static void doInternalMoveElementsRight( Elem* src, Elem* dest, int count, Types::TrueType triviallyCopyableTag );
	static void doInternalMoveElementsRight( Elem* src, Elem* dest, int count, Types::FalseType triviallyCopyableTag );

	static void bitwiseInternalMove( Elem* src, Elem* dest, int count );

	static void externalMoveElements( Elem* src, Elem* dest, int count );
	static void doExternalMoveElements( Elem* src, Elem* dest, int count, Types::TrueType triviallyCopyableTag );
	static void doExternalMoveElements( Elem* src, Elem* dest, int count, Types::FalseType triviallyCopyableTag );

	static void invokeMoveContructor( Elem* src, Elem* dest );

	Elem* detachBuffer();

	// Copying is prohibited.
	CArray( const CArray& ) = delete;
	CArray& operator=( const CArray& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
CArray<Elem, Allocator, GrowStrategy>::CArray()
{
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
CArray<Elem, Allocator, GrowStrategy>::CArray( Allocator& memoryAllocator ) :
	CAllocationStrategy<Allocator>( memoryAllocator )
{
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
template <class Container>
CArray<Elem, Allocator, GrowStrategy>::CArray( const Container& src, const CExplicitCopyTag& )
{
	const int srcSize = src.Size();
	const int srcBufferSize = GrowStrategy::GetInitialValue( srcSize );
	auto destBuffer = allocateBuffer( srcBufferSize );
	int i = 0;
	for( const auto& elem : src ) {
		::new( destBuffer + i ) Elem( copy( elem ) );
		i++;
	}

	bufferSize = srcBufferSize;
	this->setBufferValue( destBuffer );
	this->setSizeValue( srcSize );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
Elem* CArray<Elem, Allocator, GrowStrategy>::allocateBuffer( int size )
{
	const int byteSize = sizeof( Elem ) * size;
	return reinterpret_cast<Elem*>( RELIB_STRATEGY_ALLOCATE( byteSize ) );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
CArray<Elem, Allocator, GrowStrategy>::CArray( CArray<Elem, Allocator, GrowStrategy>&& other ) :
	CAllocationStrategy<Allocator>( move( other ) )
{
	bufferSize = other.bufferSize;
	this->setSizeValue( other.Size() );
	this->setBufferValue( other.detachBuffer() );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
Elem* CArray<Elem, Allocator, GrowStrategy>::detachBuffer()
{
	auto result = this->Ptr();
	this->setBufferValue( nullptr );
	this->setSizeValue( 0 );
	bufferSize = 0;
	return result;
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
CArray<Elem, Allocator, GrowStrategy>::~CArray()
{
	FreeBuffer();
	// Set a bogus value for array size to detect errors.
	this->setSizeValue( NotFound );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::Empty()
{
	this->destroyElements( 0, this->Size() - 1 );
	this->setSizeValue( 0 );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::ReserveBuffer( int newBufferSize )
{
	assert( newBufferSize >= 0 );
	if( newBufferSize > Capacity() ) {
		const int realBufferSize = GrowStrategy::GetInitialValue( newBufferSize );
		reallocateBuffer( realBufferSize );
	}
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::IncreaseSize( int newSize )
{
	assert( newSize >= 0 );
	const int size = this->Size();
	assert( size <= newSize );
	grow( newSize );
	// Default constructor is called for new elements.
	// Elements are zero initialized.
	auto buffer = this->Ptr();
	for( int i = size; i < newSize; i++ ) {
		::new( buffer + i ) Elem{};
	}
	this->setSizeValue( newSize );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::IncreaseSizeNoInitialize( int newSize )
{
	staticAssert( Types::IsPOD<Elem>::Result );
	assert( newSize >= 0 );
	const int size = this->Size();
	assert( size <= newSize );
	grow( newSize );
	auto buffer = this->Ptr();
	for( int i = size; i < newSize; i++ ) {
		::new( buffer + i ) Elem;
	}
	this->setSizeValue( newSize );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
template <class ...AddArgs>
Elem& CArray<Elem, Allocator, GrowStrategy>::Add( AddArgs&&... args )
{
	const int size = this->Size();
	const int newSize = size + 1;
	grow( newSize );
	// Call the needed constructor.
	Elem* result = ::new( this->Ptr() + size ) Elem{ forward<AddArgs>( args )... };
	this->setSizeValue( newSize );
	return *result;
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
template <class ...AddArgs>
Elem& CArray<Elem, Allocator, GrowStrategy>::AddWithinCapacity( AddArgs&&... args )
{
	const int size = this->Size();
	const int newSize = size + 1;
	assert( newSize <= Capacity() );
	// Call the needed constructor.
	Elem* result = ::new( this->Ptr() + size ) Elem{ forward<AddArgs>( args )... };
	this->setSizeValue( newSize );
	return *result;
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
template <class... Args>
Elem& CArray<Elem, Allocator, GrowStrategy>::InsertAt( int pos, Args&&... elemArgs )
{
	assert( pos >= 0 && pos <= this->Size() );
	const int newSize = this->Size() + 1;
	growAt( pos, newSize );
	auto result = ::new( this->Ptr() + pos ) Elem{ forward<Args>( elemArgs )... };
	this->setSizeValue( newSize );
	return *result;
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::DeleteAt( int pos, int count /*= 1 */ )
{
	assert( count >= 0 && count <= this->Size() );
	assert( pos >= 0 && pos <= this->Size() - count );
	this->destroyElements( pos, pos + count - 1 );
	const int newSize = this->Size() - count;
	internalMoveElementsLeft( this->Ptr(), pos + count, pos, newSize - pos );
	this->setSizeValue( newSize );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::DeleteLast( int count /*= 1 */ )
{
	const int size = this->Size();
	assert( count >= 0 );
	assert( count <= size );
	const int newSize = size - count;
	this->destroyElements( newSize, size - 1 );
	this->setSizeValue( newSize );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
template <class BoolFunc>
void CArray<Elem, Allocator, GrowStrategy>::DeleteMatching( const BoolFunc& shouldSwap )
{
	const auto buffer = this->Ptr();
	const int count = this->Size();

	// Find the first matching element.
	for( int i = 0; i < count; i++ ) {
		if( shouldSwap( buffer[i] ) ) {
			// The first element is found, move through the rest of the array sector by sector and move the accordingly.
			buffer[i].~Elem();
			int moveStartPos = i;
			int moveCount = 1;
			for( int j = moveStartPos + moveCount; j < count; j++ ) {
				if( shouldSwap( buffer[j] ) ) {
					buffer[j].~Elem();
					internalMoveElementsLeft( buffer, moveStartPos + moveCount, moveStartPos, j - moveStartPos - moveCount );
					moveStartPos = j - moveCount;
					moveCount++;
				}
			}
			// Move the last segment.
			const auto newCount = count - moveCount;
			internalMoveElementsLeft( buffer, moveStartPos + moveCount, moveStartPos, newCount - moveStartPos );
			this->setSizeValue( newCount );
			return;
		}
	}
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::FreeBuffer()
{
	Empty();
	CAllocationStrategy<Allocator>::StrategyFree( this->Ptr() );
	bufferSize = 0;
	this->setBufferValue( nullptr );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
const CArray<Elem, Allocator, GrowStrategy>& CArray<Elem, Allocator, GrowStrategy>::operator=( CArray<Elem, Allocator, GrowStrategy>&& other )
{
	assert( &other != this );
	FreeBuffer();
	CAllocationStrategy<Allocator>::operator=( move( other ) );
	bufferSize = other.bufferSize;
	this->setSizeValue( other.Size() );
	this->setBufferValue( other.detachBuffer() );
	return *this;
}

// Grows the array's buffer size exponentially.
template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::grow( int newSize )
{
	assert( newSize >= 0 );
	assert( this->Size() >= 0 );
	const int storageSize = Capacity();
	if( newSize > storageSize ) {
		const int newBufferSize = GrowStrategy::GrowValue( storageSize, newSize );
		reallocateBuffer( newBufferSize );
	}
}

// Grows the array freeing space for ( newSize - size ) elements on position pos.
template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::growAt( int pos, int newSize )
{
	const int size = this->Size();
	assert( newSize > size );
	assert( pos >= 0 && pos <= size );
	grow( newSize );
	internalMoveElementsRight( this->Ptr(), pos, pos + newSize - size, size - pos );
}

// Reallocate the buffer to a new continuous chunk of memory.
template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::reallocateBuffer( int newBufferSize )
{
	const int size = this->Size();
	assert( newBufferSize > size );
	assert( newBufferSize <= INT_MAX / sizeof( Elem ) );
	assert( size >= 0 );
	
	Elem* newBuffer = allocateBuffer( newBufferSize );
	Elem* oldBuffer = this->Ptr();

	externalMoveElements( oldBuffer, newBuffer, size );
	CAllocationStrategy<Allocator>::StrategyFree( oldBuffer );
	this->setBufferValue( newBuffer );
	bufferSize = newBufferSize;
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::internalMoveElementsLeft( Elem* src, int srcPos, int destPos, int count )
{
	assert( destPos <= srcPos );
	doInternalMoveElementsLeft( src + srcPos, src + destPos, count, Types::HasTrivialCopyConstructor<Elem>() );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::doInternalMoveElementsLeft( Elem* src, Elem* dest, int count, Types::TrueType )
{
	bitwiseInternalMove( src, dest, count );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::doInternalMoveElementsLeft( Elem* src, Elem* dest, int count, Types::FalseType )
{
	for( int i = 0; i < count; i++ ) {
		invokeMoveContructor( src + i, dest + i );
	}
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::internalMoveElementsRight( Elem* src, int srcPos, int destPos, int count )
{
	assert( destPos >= srcPos );
	doInternalMoveElementsRight( src + srcPos, src + destPos, count, Types::HasTrivialCopyConstructor<Elem>() );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::doInternalMoveElementsRight( Elem* src, Elem* dest, int count, Types::TrueType )
{
	bitwiseInternalMove( src, dest, count );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::doInternalMoveElementsRight( Elem* src, Elem* dest, int count, Types::FalseType )
{
	for( int i = count - 1; i >= 0; i-- ) {
		invokeMoveContructor( src + i, dest + i );
	}
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::bitwiseInternalMove( Elem* src, Elem* dest, int count )
{
	if( count > 0 ) {
		::memmove( dest, src, count * sizeof( Elem ) );
	}
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::externalMoveElements( Elem* src, Elem* dest, int count )
{
	doExternalMoveElements( src, dest, count, Types::HasTrivialCopyConstructor<Elem>() );
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::doExternalMoveElements( Elem* src, Elem* dest, int count, Types::TrueType )
{
	if( count > 0 ) {
		::memcpy( dest, src, count * sizeof( Elem ) );
	}
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::doExternalMoveElements( Elem* src, Elem* dest, int count, Types::FalseType )
{
	for( int i = 0; i < count; i++ ) {
		invokeMoveContructor( src + i, dest + i );
	}
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
void CArray<Elem, Allocator, GrowStrategy>::invokeMoveContructor( Elem* src, Elem* dest )
{
	assert( src != nullptr );
	assert( dest != nullptr );
	// Call the move constructor.
	::new( dest ) Elem( move( *src ) );
	// Destroy the original.
	src->~Elem();
}

//////////////////////////////////////////////////////////////////////////

template <class Elem, int stackSize, class Allocator = CRuntimeHeap>
using CFlexibleArray = CArray<Elem, CInlineStackAllocator<Allocator, stackSize * sizeof( Elem ), alignof( Elem )>, CFlexibleArrayGrowStrategy<stackSize>>;

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
