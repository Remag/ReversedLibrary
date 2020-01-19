#pragma once

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A simple array wrapper that performs bound checks in debug mode.
template <class Elem, int dim>
class CStackArray {
public:
	typedef Elem TElemType;

	CStackArray() = default;
	// Construct the array from a range of arguments.
	template<class... TT>
	CStackArray( TT&&... argList );
	// All versions of copy and move constructors need to explicitly declared to prevent the previous constructor from rerplacing them.
	CStackArray( CStackArray<Elem, dim>& other );
	CStackArray( const CStackArray<Elem, dim>& other );
	CStackArray( CStackArray<Elem, dim>&& other ) = default;
	CStackArray& operator=( const CStackArray<Elem, dim>& other ) = default;
	CStackArray& operator=( CStackArray<Elem, dim>&& other ) = default;

	static int Size()
		{ return dim; }

	// Uniform array buffer representation.
	operator CArrayView<Elem>() const
		{ return CArrayView<Elem>( Ptr(), Size() ); }
	operator CArrayBuffer<Elem>()
		{ return CArrayBuffer<Elem>( Ptr(), Size() ); }

	Elem ( &Ptr() )[dim]
		{ return bufferWrapper.Buffer; }
	const Elem ( &Ptr() const )[dim]
		{ return bufferWrapper.Buffer; }

	// Access operators.
	Elem& operator[]( int index );
	const Elem& operator[]( int index ) const;

	// Partition.
	CArrayView<Elem> ( Left ( int count ) const )
		{ return Mid( 0, count ); }
	CArrayBuffer<Elem> Left( int count )
		{ return Mid( 0, count ); }
	CArrayView<Elem> Mid( int first ) const
		{ return Mid( first, Size() - first ); }
	CArrayBuffer<Elem> Mid( int first )
		{ return Mid( first, Size() - first ); }
	CArrayView<Elem> Right( int count ) const
		{ return Mid( Size() - count, count ); }
	CArrayBuffer<Elem> Right( int count )
		{ return Mid( Size() - count, count ); }
		
	CArrayView<Elem> Mid( int first, int count ) const
		{ return CArrayView<Elem>( bufferWrapper.Buffer + first, count ); }
	CArrayBuffer<Elem> Mid( int first, int count )
		{ return CArrayBuffer<Elem>( bufferWrapper.Buffer + first, count ); }

private:
	// An array wrapper is necessary in order to invoke the plain array's copy constructor.
	struct CArrayWrapper {
		Elem Buffer[dim] = {};
	};
	CArrayWrapper bufferWrapper;
};

//////////////////////////////////////////////////////////////////////////

template <class ElemType, int dim>
template<class... TT>
CStackArray<ElemType, dim>::CStackArray( TT&&... argList ) :
	bufferWrapper{ forward<TT>( argList )... }
{
	static_assert( sizeof...( TT ) == dim, "Stack array size mismatch." );
}

template <class Elem, int dim>
CStackArray<Elem, dim>::CStackArray( CStackArray<Elem, dim>& other ) :
	bufferWrapper( other.bufferWrapper )
{
}

template <class Elem, int dim>
CStackArray<Elem, dim>::CStackArray( const CStackArray<Elem, dim>& other ) :
	bufferWrapper( other.bufferWrapper )
{
}

template <class ElemType, int dim>
const ElemType& CStackArray<ElemType, dim>::operator[]( int index ) const
{
	assert( index >= 0 );
	assert( index < Size() );
	return bufferWrapper.Buffer[index];
}

template <class ElemType, int dim>
ElemType& CStackArray<ElemType, dim>::operator[]( int index )
{
	assert( index >= 0 );
	assert( index < Size() );
	return bufferWrapper.Buffer[index];
}

//////////////////////////////////////////////////////////////////////////

// Range-based for loops support.
template<class ElemType, int dim>
ElemType* begin( CStackArray<ElemType, dim>& arr ) 
{
	return arr.Ptr();
}

template<class ElemType, int dim>
ElemType* end( CStackArray<ElemType, dim>& arr ) 
{
	return arr.Ptr() + arr.Size();
}

template<class ElemType, int dim>
const ElemType* begin( const CStackArray<ElemType, dim>& arr ) 
{
	return arr.Ptr();
}

template<class ElemType, int dim>
const ElemType* end( const CStackArray<ElemType, dim>& arr ) 
{
	return arr.Ptr() + arr.Size();
}

//////////////////////////////////////////////////////////////////////////
// Serialization.

template <class ElemType, int dim>
CArchiveWriter& operator<<( CArchiveWriter& archive, const CStackArray<ElemType, dim>& arr )
{
	for( const auto& elem : arr ) {
		archive << elem;
	}
	return archive;
}

template <class ElemType, int dim>
CArchiveReader& operator>>( CArchiveReader& archive, CStackArray<ElemType, dim>& arr )
{
	for( const auto& elem : arr ) {
		archive >> elem;
	}
	return archive;
}

//////////////////////////////////////////////////////////////////////////

// Creation function.
template <class T, class... Args>
auto CreateStackArray( Args&&... args )
{
	return CStackArray<T, sizeof...( args )>( forward<Args>( args )... );
}

}	// namespace Relib.

