#pragma once
#include <ArrayData.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Uniform array storage with const-only access.
template <class T>
class CArrayView : public RelibInternal::CArrayConstData<T> {
public:
	CArrayView() = default;
	CArrayView( const T* buffer, int size ) : RelibInternal::CArrayConstData<T>( buffer, size ) {}
	template <int dim>
	CArrayView( const T ( &arr )[dim] ) : RelibInternal::CArrayConstData<T>( arr, dim ) {}
	CArrayView( std::initializer_list<T> initList ) : RelibInternal::CArrayConstData<T>( initList.begin(), static_cast<int>( initList.size() ) ) {}
	explicit CArrayView( const T& singleObject ) : RelibInternal::CArrayConstData<T>( &singleObject, 1 ) {}

	// Conversion to raw arrays.
	explicit operator CArrayView<BYTE>() const
		{ return CArrayView<BYTE>( reinterpret_cast<const BYTE*>( this->Ptr() ), sizeof( T ) * this->Size() ); }
};

//////////////////////////////////////////////////////////////////////////

// Uniform array storage. Provides common access operators. Resizing is not possible through this interface.
template <class T>
class CArrayBuffer : public RelibInternal::CArrayData<T> {
public:
	CArrayBuffer() = default;
	CArrayBuffer( T* buffer, int size ) : RelibInternal::CArrayData<T>( buffer, size ) {}
	template <int dim>
	CArrayBuffer( const T ( &arr )[dim] ) : RelibInternal::CArrayData<T>( arr, dim ) {}
	explicit CArrayBuffer( T& singleObject ) : RelibInternal::CArrayData<T>( &singleObject, 1 ) {}

	// Conversion to raw arrays.
	explicit operator CArrayBuffer<BYTE>()
		{ return CArrayBuffer<BYTE>( reinterpret_cast<BYTE*>( this->Ptr() ), sizeof( T ) * this->Size() ); }
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

