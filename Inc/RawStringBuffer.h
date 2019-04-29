#pragma once
#include <Redefs.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Editable string buffer with fixed length. Gives access to the underlying editable pointer for functions that require C-style access.
// Access to the underlying string is prohibited while the raw buffer exists.
template <class StrType>
class CRawStringBuffer {
public:
	CRawStringBuffer() = default;
	explicit CRawStringBuffer( CBaseString<StrType>& _str ) : str( &_str ) {}
	CRawStringBuffer( CRawStringBuffer<StrType>&& other );
	CRawStringBuffer& operator=( CRawStringBuffer<StrType>&& other );

	// Set the string size if the buffer hasn't been released manually.
	~CRawStringBuffer();

	StrType* Ptr();
	operator StrType*()
		{ return Ptr(); }

	// Find the size of the new buffer and set the correct size in the owner.
	// Buffer can only be manually released once.
	// Automatic release is performed in the destructor if necessary.
	void Release();
	// Release the buffer with a known size.
	void Release( int strSize );

private:
	// Owner of the buffer.
	CBaseString<StrType>* str = nullptr;

	void doRelease();

	// Copying is prohibited.
	CRawStringBuffer( CRawStringBuffer& ) = delete;
	void operator=( CRawStringBuffer& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class StrType>
CRawStringBuffer<StrType>::CRawStringBuffer( CRawStringBuffer<StrType>&& other ) :
	str( other.str )
{
	other.str = nullptr;
}
	
template <class StrType>
CRawStringBuffer<StrType>& CRawStringBuffer<StrType>::operator=( CRawStringBuffer<StrType>&& other )
{
	swap( str, other.str );
	return *this;
}

template <class StrType>
CRawStringBuffer<StrType>::~CRawStringBuffer()
{
	if( str != nullptr ) {
		doRelease();
	}
}

template <class StrType>
void CRawStringBuffer<StrType>::doRelease()
{
	str->setBufferLength( CStringOperations<StrType>::Length( str->Ptr() ) );
}

template <class StrType>
StrType* CRawStringBuffer<StrType>::Ptr()
{
	assert( str != nullptr );
	return const_cast<StrType*>( str->Ptr() );
}

template <class StrType>
void CRawStringBuffer<StrType>::Release()
{
	assert( str != nullptr );
	doRelease();
	str = nullptr;
}

template <class StrType>
void CRawStringBuffer<StrType>::Release( int strLength )
{
	assert( strLength >= 0 );
	assert( str != nullptr );
	assert( str->getRequiredSize( strLength ) <= str->allocatedSize );
	str->markBufferLength( strLength );
	str = nullptr;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

