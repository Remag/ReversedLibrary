#pragma once
#include <Reassert.h>
#include <Sort.h>

namespace Relib {

namespace RelibInternal {

template <class T>
class CArrayData;
//////////////////////////////////////////////////////////////////////////

// Base array constant view methods.
template <class T>
class CArrayConstData {
public:
	typedef T TElemType;
	
	int Size() const
		{ return size; }
	bool IsEmpty() const
		{ return Size() == 0; }

	CArrayView<T> View() const
		{ return CArrayView<T>( Ptr(), Size() ); }
	operator CArrayView<T>() const
		{ return View(); }

	const T* Ptr() const
		{ return buffer; }
	const T& operator[]( int pos ) const;
	const T& First() const
		{ return operator[]( 0 ); }
	const T& Last() const
		{ return operator[]( Size() - 1 ); }

	// Partition.
	CArrayView<T> Left( int count ) const
		{ return Mid( 0, count ); }
	CArrayView<T> Mid( int first ) const
		{ return Mid( first, Size() - first ); }
	CArrayView<T> Mid( int first, int count ) const;
	CArrayView<T> Right( int count ) const
		{ return Mid( Size() - count, count ); }

	// Check if the array is sorted using LessAction comparator.
	template <class LessAction>
	bool IsSorted( const LessAction& less ) const;

	// Range-based for loops support.
	const T* begin() const
		{ return buffer; } 
	const T* end() const 
		{ return buffer + size; }

	// Array data needs access to a non-const buffer.
	friend class CArrayData<T>;

protected:
	CArrayConstData() = default;
	CArrayConstData( const T* _buffer, int _size ) : 
		buffer( _buffer ), size( _size ) { assert( size >= 0 ); assert( buffer != nullptr || size == 0 ); }
	CArrayConstData( const CArrayConstData& ) = default;
	CArrayConstData& operator=( const CArrayConstData& ) = default;

	// Non-constant buffer is granted to accessors who require the access.
	T* getWritableBuffer()
		{ return const_cast<T*>( buffer ); }
		
	void setSizeValue( int newValue )
		{ size = newValue; }
	void setBufferValue( const T* newValue )
		{ buffer = newValue; }

private:
	// Raw buffer.
	const T* buffer = nullptr;
	// Number of elements that are stored in the buffer.
	int size = 0;
};

//////////////////////////////////////////////////////////////////////////

// Non constant array data.
template <class T>
class CArrayData {
public:
	typedef T TElemType;
	
	CArrayView<T> View() const
		{ return CArrayView<T>( Ptr(), Size() ); }
	operator CArrayView<T>() const
		{ return View(); }

	CArrayBuffer<T> Buffer()
		{ return CArrayBuffer<T>( data.getWritableBuffer(), Size() ); }
	
	int Size() const
		{ return data.Size(); }
	bool IsEmpty() const
		{ return Size() == 0; }

	T* Ptr()
		{ return data.getWritableBuffer(); }
	const T* Ptr() const
		{ return data.Ptr(); }

	const T& operator[]( int pos ) const
		{ return data[pos]; }
	T& operator[]( int pos )
		{ return const_cast<T&>( data[pos] ); }
	T& GetAt( int pos )
		{ return operator[]( pos ); }
	const T& GetAt( int pos ) const
		{ return operator[]( pos ); }

	const T& First() const
		{ return data.First(); }
	T& First()
		{ return operator[]( 0 ); }

	const T& Last() const
		{ return data.Last(); }
	T& Last()
		{ return operator[]( Size() - 1 ); }

	CArrayView<T> Left( int count ) const
		{ return data.Left( count ); }
	CArrayBuffer<T> Left( int count )
		{ return Mid( 0, count ); }
	CArrayView<T> Mid( int first ) const
		{ return data.Mid( first ); }
	CArrayBuffer<T> Mid( int first )
		{ return Mid( first, Size() - first ); }
	CArrayView<T> Right( int count ) const
		{ return data.Right( count ); }
	CArrayBuffer<T> Right( int count )
		{ return Mid( Size() - count, count ); }
	CArrayView<T> Mid( int first, int count ) const
		{ return data.Mid( first, count ); }
	CArrayBuffer<T> Mid( int first, int count );

	// Check if the array is sorted using LessAction comparator.
	template <class LessAction>
	bool IsSorted( const LessAction& less ) const
		{ return data.IsSorted( less ); }

	// Quick sorting algorithm. Uses a given comparison class to compare elements.
	template <class LessAction>
	void QuickSort( const LessAction& less );

	// Range-based for loops support.
	T* begin()
		{ return Ptr(); }
	const T* begin() const
		{ return Ptr(); }
	T* end()
		{ return Ptr() + Size(); }
	const T* end() const
		{ return Ptr() + Size(); }

protected:
	CArrayData() = default;
	CArrayData( T* buffer, int size );
	CArrayData( const CArrayData& ) = default;
	CArrayData& operator=( const CArrayData& ) = default;

	void setSizeValue( int newValue )
		{ data.setSizeValue( newValue ); }
	void setBufferValue( const T* newValue )
		{ data.setBufferValue( newValue ); }
	void destroyElements( int startPos, int endPos );

private:
	CArrayConstData<T> data;
};

//////////////////////////////////////////////////////////////////////////

template <class T>
const T& CArrayConstData<T>::operator[]( int pos ) const
{
	assert( pos >= 0 && pos < Size() );
	return *( buffer + pos );
}

template <class T>
CArrayView<T> CArrayConstData<T>::Mid( int first, int count ) const
{
	return CArrayView<T>( buffer + first, count );
}

template <class T>
template<class LessAction>
bool CArrayConstData<T>::IsSorted( const LessAction& less ) const
{
	for( int i = 0; i < size - 1; i++ ) {
		if( !less( buffer[i], buffer[i + 1] ) && less( buffer[i + 1], buffer[i] ) ) {
			return false;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

template <class T>
CArrayData<T>::CArrayData( T* buffer, int size ) :
	data( buffer, size )
{
}

template <class T>
CArrayBuffer<T> CArrayData<T>::Mid( int first, int count )
{
	assert( first >= 0 && count >= 0 );
	assert( first + count <= Size() );
	return CArrayBuffer<T>( Ptr() + first, count );
}

template <class T>
template<class LessAction>
void CArrayData<T>::QuickSort( const LessAction& less )
{
	Sort::QSort( data.getWritableBuffer(), Size(), less );
}

template <class T>
void CArrayData<T>::destroyElements( int firstPos, int lastPos )
{
	assert( lastPos < Size() );
	assert( firstPos >= 0 );
	for( int i = lastPos; i >= firstPos; i-- ) {
		Ptr()[i].~T();
	}
}

//////////////////////////////////////////////////////////////////////////

}	// RelibInternal.

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

