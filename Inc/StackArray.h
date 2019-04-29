#pragma once

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A simple array wrapper that performs bound checks in debug mode.
template <class Elem, int dim>
class CStackArray {
public:
	typedef Elem TElemType;

	// Default constructor. 
	// Copying and movement are defaulted.
	CStackArray() = default;
	// Construct the array from a range of arguments.
	// This method of construction provides compile-time size checking.
	template <class FirstType, class... Types>
	CStackArray( FirstType&& first, Types&&... rest );

	static int Size()
		{ return dim; }

	// Uniform array buffer representation.
	operator CArrayView<Elem>() const
		{ return CArrayView<Elem>( Ptr(), Size() ); }
	operator CArrayBuffer<Elem>()
		{ return CArrayBuffer<Elem>( Ptr(), Size() ); }

	Elem ( &Ptr() )[dim]
		{ return buffer; }
	const Elem ( &Ptr() const )[dim]
		{ return buffer; }

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
		{ return CArrayView<Elem>( buffer + first, count ); }
	CArrayBuffer<Elem> Mid( int first, int count )
		{ return CArrayBuffer<Elem>( buffer + first, count ); }

private:
	// Internal buffer.
	Elem buffer[dim] = {};
};

//////////////////////////////////////////////////////////////////////////

template <class ElemType, int dim>
template <class FirstType, class... Types>
CStackArray<ElemType, dim>::CStackArray( FirstType&& first, Types&&... rest ) :
	buffer{ forward<FirstType>( first ), forward<Types>( rest )... }
{
	static_assert( sizeof...( Types ) + 1 == dim, "Stack array size mismatch." );
}

template <class ElemType, int dim>
const ElemType& CStackArray<ElemType, dim>::operator[]( int index ) const
{
	assert( index >= 0 );
	assert( index < Size() );
	return buffer[index];
}

template <class ElemType, int dim>
ElemType& CStackArray<ElemType, dim>::operator[]( int index )
{
	assert( index >= 0 );
	assert( index < Size() );
	return buffer[index];
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

