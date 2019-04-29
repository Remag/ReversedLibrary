#pragma once
#include <Reassert.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

// Raw memory buffer common methods.
class CRawBufferOperations {
public:
	int Size() const
		{ return size; }
	bool IsEmpty() const
		{ return size == 0; }

	const void* Ptr() const
		{ return buffer; }

	// Convert the underlying buffer to a given type. Sizes must match.
	template <class T>
	const T& As() const;

	// Get a given type at a given offset.
	template <class T>
	const T& Get( int offset ) const;

protected:
	CRawBufferOperations() = default;
	CRawBufferOperations( const void* _buffer, int _size ) : buffer( _buffer ), size( _size ) {}
	
	void setBuffer( const void* newValue, int newSize );

	// Derivatives get write access.
	void* getWritableBuffer()
		{ return const_cast<void*>( buffer ); }

private:
	const void* buffer = nullptr;
	int size = 0;
};

//////////////////////////////////////////////////////////////////////////

inline void CRawBufferOperations::setBuffer( const void* newValue, int newSize )
{
	buffer = newValue;
	size = newSize;
}

template <class T>
const T& CRawBufferOperations::As() const
{
	assert( sizeof( T ) == size );
	assert( reinterpret_cast<size_t>( buffer ) % alignof( T ) == 0 );
	return *static_cast<const T*>( buffer );
}

template <class T>
const T& CRawBufferOperations::Get( int offset ) const
{
	const int endSize = sizeof( T ) + offset;
	assert( endSize <= size );
	const void* offsetBuffer = static_cast<const BYTE*>( buffer ) + offset;
	assert( reinterpret_cast<size_t>( offsetBuffer ) % alignof( T ) == 0 );
	return *static_cast<const T*>( offsetBuffer );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

// Raw memory buffer with constant-only access. This class doesn't own any memory but simply references it.
class CConstRawBuffer : public RelibInternal::CRawBufferOperations {
public:
	CConstRawBuffer() = default;
	CConstRawBuffer( const void* buffer, int size ) : CRawBufferOperations( buffer, size ) {}

	void SetBuffer( const void* newValue, int newSize )
		{ setBuffer( newValue, newSize ); }
	template <class T>
	void SetBuffer( const T* newValue )
		{ setBuffer( newValue, sizeof( T ) ); }
};

//////////////////////////////////////////////////////////////////////////

// Raw memory buffer. This class doesn't own any memory but simply references it.
class CRawBuffer : public RelibInternal::CRawBufferOperations {
public:
	CRawBuffer() = default;
	CRawBuffer( void* buffer, int size ) : CRawBufferOperations( buffer, size ) {}

	operator CConstRawBuffer() const
		{ return CConstRawBuffer( Ptr(), Size() ); }

	void SetBuffer( void* newValue, int newSize )
		{ setBuffer( newValue, newSize ); }
	template <class T>
	void SetBuffer( T* newValue )
		{ setBuffer( newValue, sizeof( T ) ); }

	using RelibInternal::CRawBufferOperations::Ptr;
	void* Ptr()
		{ return getWritableBuffer(); }

	// Convert the underlying buffer to a given type. Sizes must match.
	using RelibInternal::CRawBufferOperations::As;
	template <class T>
	T& As()
		{ return const_cast<T&>( CRawBufferOperations::As<T>() ); }

	// Get a given type at a given offset.
	using RelibInternal::CRawBufferOperations::Get;
	template <class T>
	T& Get( int offset )
		{ return const_cast<T&>( CRawBufferOperations::Get<T>( offset ) ); }

	// Copy/Move the given value on the buffer. 
	// Buffer is assumed to not be occupied.
	// The destructor for the copied value is assumed to be called externally.
	template <class T>
	T& Set( T newValue );
	template <class T>
	T& Set( T newValue, int offset );
};

//////////////////////////////////////////////////////////////////////////

template <class T>
T& CRawBuffer::Set( T newValue )
{
	assert( sizeof( T ) == Size() );
	assert( reinterpret_cast<size_t>( buffer ) % alignof( T ) == 0 );
	return *::new( getWritableBuffer() ) T( move( newValue ) );
}

template <class T>
T& CRawBuffer::Set( T newValue, int offset )
{
	const int endSize = sizeof( T ) + offset;
	assert( endSize <= Size() );
	void* offsetBuffer = static_cast<BYTE*>( getWritableBuffer() ) + offset;
	assert( reinterpret_cast<size_t>( offsetBuffer ) % alignof( T ) == 0 );
	return *::new( offsetBuffer ) T( move( newValue ) );
}

//////////////////////////////////////////////////////////////////////////

namespace Types {

// Corresponding type of a raw buffer class.
template <class T>
struct RawBufferType {
};

template <class T>
struct RawBufferType<T*> {
	typedef CRawBuffer Result;
};

template <class T>
struct RawBufferType<CPtrOwner<T>> {
	typedef CRawBuffer Result;
};

template <class T>
struct RawBufferType<CSharedPtr<T>> {
	typedef CRawBuffer Result;
};

template <class T>
struct RawBufferType<const T*> {
	typedef CConstRawBuffer Result;
};

}	// namespace Types.

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

