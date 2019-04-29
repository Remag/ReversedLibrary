#pragma once
#include <MemoryUtils.h>
#include <AllocationStrategy.h>
#include <ArrayBuffer.h>
#include <ArrayData.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// An array that initializes its size once at construction.
// Can be used to contain immovable types.
template <class Elem, class Allocator>
class CStaticArray : private CAllocationStrategy<Allocator>, public RelibInternal::CArrayData<Elem> {
public:
	typedef Elem TElemType;

	// Create an array and allocate a buffer of the given size.
	// Empty elements are not created.
	explicit CStaticArray( int maxSize = 0 );
	// Create an array with the custom dynamic allocator.
	explicit CStaticArray( Allocator& allocator, int maxSize = 0 );
	// Arrays can be constructed from rvalues implicitly.
	// Implicit copying is prohibited, use CopyFrom.
	CStaticArray( CStaticArray&& other );

	~CStaticArray()
		{ FreeBuffer(); }

	operator CArrayBuffer<Elem>()
		{ return this->Buffer(); }

	int Capacity() const
		{ return maxSize; }
	bool IsFull() const
		{ return this->Size() == Capacity(); }

	// Contents manipulation.

	// Construct an Elem with given arguments and add it to array.
	template <class ...AddArgs>
	Elem& Add( AddArgs&&... args );

	// Free the previous buffer and set a new one with a given size.
	// Old elements are not copied.
	void ResetBuffer( int newBufferSize );
	// Reset the buffer and fill it with zero initialized elements.
	void ResetSize( int newSize );
	// Reset the buffer and fill it with uninitialized elements.
	void ResetSizeNoInitialize( int newSize );
	void DeleteLast( int count = 1 );

	// Destroy all elements. Buffer is not freed.
	void Empty();
	// Delete all the elements and free the buffer.
	void FreeBuffer();

	// Move objects to another array.
	// Destination is emptied before all operations.
	CStaticArray& operator=( CStaticArray&& other );

private:
	int maxSize = 0;

	void createNewBuffer( int bufferSize );

	// Copying is prohibited.
	CStaticArray( CStaticArray& ) = delete;
	void operator=( CStaticArray& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class Elem, class Allocator>
CStaticArray<Elem, Allocator>::CStaticArray( int maxSize /*= 0 */ )
{
	createNewBuffer( maxSize );
}

template <class Elem, class Allocator>
CStaticArray<Elem, Allocator>::CStaticArray( Allocator& allocator, int maxSize /*= 0 */ ) :
	CAllocationStrategy<Allocator>( allocator )
{
	createNewBuffer( maxSize );
}

template <class Elem, class Allocator>
void CStaticArray<Elem, Allocator>::createNewBuffer( int bufferSize )
{
	assert( bufferSize >= this->Size() );
	assert( bufferSize <= INT_MAX / sizeof( Elem ) );
	
	Elem* newBuffer = ( bufferSize > 0 )
		? static_cast<Elem*>( RELIB_STRATEGY_ALLOCATE( bufferSize * sizeof( Elem ) ) )
		: 0;

	this->setBufferValue( newBuffer );
	maxSize = bufferSize;
}

template <class Elem, class Allocator>
CStaticArray<Elem, Allocator>::CStaticArray( CStaticArray<Elem, Allocator>&& other ) :
	CAllocationStrategy<Allocator>( move( other ) ),
	RelibInternal::CArrayData<Elem>( other ),
	maxSize( other.maxSize )
{
	other.setBufferValue( nullptr );
	other.setSizeValue( 0 );
	other.maxSize = 0;
}

template <class Elem, class Allocator>
template <class ...AddArgs>
Elem& CStaticArray<Elem, Allocator>::Add( AddArgs&&... args )
{
	const int newSize = this->Size() + 1;
	assert( newSize <= maxSize );
	// Call the needed constructor.
	Elem* result = ::new( this->Ptr() + this->Size() ) Elem{ forward<AddArgs>( args )... };
	this->setSizeValue( this->Size() + 1 );
	return *result;
}
	
template <class Elem, class Allocator>
void CStaticArray<Elem, Allocator>::ResetBuffer( int newSize )
{
	assert( newSize >= 0 );
	FreeBuffer();
	createNewBuffer( newSize );
}

template <class Elem, class Allocator>
void CStaticArray<Elem, Allocator>::ResetSize( int newSize )
{
	ResetBuffer( newSize );
	// Default constructor is called for new elements.
	// Elements are zero initialized.
	for( int i = 0; i < newSize; i++ ) {
		new( this->Ptr() + i ) Elem();
	}
	this->setSizeValue( newSize );
}

template <class Elem, class Allocator>
void CStaticArray<Elem, Allocator>::ResetSizeNoInitialize( int newSize )
{
	staticAssert( Types::IsPOD<Elem>::Result );
	ResetBuffer( newSize );
	// Elements are not initialized.
	for( int i = 0; i < newSize; i++ ) {
		new( this->Ptr() + i ) Elem;
	}
	this->setSizeValue( newSize );
}

template <class Elem, class Allocator>
void CStaticArray<Elem, Allocator>::DeleteLast( int count /*= 1 */ )
{
	assert( count >= 0 );
	assert( count <= this->Size() );
	if( count == 0 ) {
		return;
	}
	const int newSize = this->Size() - count;
	for( int i = this->Size() - 1; i >= newSize; i-- ) {
		this->Ptr()[i].~Elem();
	}
	this->setSizeValue( newSize );
}

template <class Elem, class Allocator>
void CStaticArray<Elem, Allocator>::Empty()
{
	this->destroyElements( 0, this->Size() - 1 );
	this->setSizeValue( 0 );
}

template <class Elem, class Allocator>
void CStaticArray<Elem, Allocator>::FreeBuffer()
{
	Empty();
	CAllocationStrategy<Allocator>::StrategyFree( this->Ptr() );
	this->setBufferValue( nullptr );
	maxSize = 0;
}

template <class Elem, class Allocator>
CStaticArray<Elem, Allocator>& CStaticArray<Elem, Allocator>::operator=( CStaticArray<Elem, Allocator>&& other )
{
	FreeBuffer();
	this->setBufferValue( other.Ptr() );
	this->setSizeValue( other.Size() );
	maxSize = other.maxSize;
	
	other.setBufferValue( nullptr );
	other.setSizeValue( 0 );
	other.maxSize = 0;

	CAllocationStrategy<Allocator>::operator=( move( other ) );
	return *this;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

