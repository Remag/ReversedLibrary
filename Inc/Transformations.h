#pragma once
#include <Vector.h>
#include <Matrix.h>

namespace Relib {

// Vector and matrix transformation functions.
//////////////////////////////////////////////////////////////////////////

// Transformation utility functions.
template <class T, int dim, TMatrixOrder order = MO_ColumnMajor>
CMatrix<T, dim + 1, dim + 1, order> CreateTransformation( const CVector<T, dim>& offset )
{
	CMatrix<T, dim + 1, dim + 1, order> result( numeric_cast<T>( 1 ) );
	for( int i = 0; i < dim; i++ ) {
		result( dim, i ) = offset[i];
	}
	return result;
}

template <class T, int dim, TMatrixOrder order = MO_ColumnMajor>
CMatrix<T, dim + 1, dim + 1, order> CreateTransformation( const CVector<T, dim>& offset, const CVector<T, dim>& scale )
{
	CMatrix<T, dim + 1, dim + 1, order> result;
	for( int i = 0; i < dim; i++ ) {
		result( i, i ) = scale[i];
		result( dim, i ) = offset[i];
	}
	result( dim, dim ) = numeric_cast<T>( 1 );
	return result;
}

template <class T, int dim, TMatrixOrder order = MO_ColumnMajor>
CMatrix<T, dim + 1, dim + 1, order> CreateTransformation( const CVector<T, dim>& offset, const CMatrix<T, dim, dim, order>& rotScale )
{
	CMatrix<T, dim + 1, dim + 1, order> result = CMatrix<T, dim + 1, dim + 1, order>::CreateRawMatrix();
	for( int x = 0; x < dim; x++ ) {
		for( int y = 0; y < dim; y++ ) {
			result( x, y ) = rotScale( x, y );
		}
		result( x, dim ) = numeric_cast<T>( 0 );
	}
	for( int i = 0; i < dim; i++ ) {
		result( dim, i ) = offset[i];
	}
	result( dim, dim ) = numeric_cast<T>( 1 );
	return result;
}

template <class T, int dim, TMatrixOrder order>
CVector<T, dim - 1> GetOffset( const CMatrix<T, dim, dim, order>& transform )
{
	CVector<T, dim - 1> result = CVector<T, dim - 1>::CreateRawVector();
	for( int i = 0; i < dim - 1; i++ ) {
		result[i] = transform( dim - 1, i );
	}
	return result;
}

template <class T, int dim, TMatrixOrder order>
void SetOffset( CMatrix<T, dim, dim, order>& transform, const CVector<T, dim - 1>& newValue )
{
	for( int i = 0; i < dim - 1; i++ ) {
		transform( dim - 1, i ) = newValue[i];
	}
}

template <class T, int dim, TMatrixOrder order>
CVector<T, dim - 1> GetScale( const CMatrix<T, dim, dim, order>& transform )
{
	CVector<T, dim - 1> result = CVector<T, dim - 1>::CreateRawVector();
	for( int i = 0; i < dim - 1; i++ ) {
		result[i] = transform( i, i );
	}
	return result;
}

template <class T, int dim, TMatrixOrder order>
void SetScale( CMatrix<T, dim, dim, order>& transform, const CVector<T, dim - 1>& newValue )
{
	for( int i = 0; i < dim - 1; i++ ) {
		transform( i, i ) = newValue[i];
	}
}

//////////////////////////////////////////////////////////////////////////

// Linear vector interpolation.
template <class VecType, int dim>
inline CVector<VecType, dim> Lerp( const CVector<VecType, dim>& left, const CVector<VecType, dim>& right, typename CVector<VecType, dim>::FloatingPointVecType t )
{
	assert( t >= 0 );
	return left + t * ( right - left );
}

// Affine transformations. The last coordinate of the vector is assumed to be 1.
// The last column of the matrix is ignored.
template <class Vec, class MatType, int dim>
Vec PointTransform( const CMatrix<MatType, dim, dim>& affineMatrix, const Vec& point )
{
	static_assert( Vec::Size() == dim - 1, "Illegal vector size." );
	const int lastIndex = dim - 1;
	for( int x = 0; x < lastIndex; x++ ) {
		assert( affineMatrix( x, lastIndex ) == MatType( 0 ) );
	}
	assert( affineMatrix( lastIndex, lastIndex ) == MatType( 1 ) );

	Vec result;
	for( int y = 0; y < lastIndex; y++ ) {
		for( int x = 0; x < lastIndex; x++ ) {
			result[y] += affineMatrix( x, y ) * point[x];
		}
		result[y] += affineMatrix( lastIndex, y );
	}
	return result;
}

// Affine transformation version for transformation matrices without the last column.
template <class Vec, class MatType, int dim>
Vec PointTransform( const CMatrix<MatType, dim + 1, dim>& affineMatrix, const Vec& point )
{
	static_assert( Vec::Size() == dim, "Illegal vector size." );

	Vec result;
	for( int y = 0; y < dim; y++ ) {
		for( int x = 0; x < dim; x++ ) {
			result[y] += affineMatrix( x, y ) * point[x];
		}
		result[y] += affineMatrix( dim, y );
	}
	return result;
}

// Affine transformations. The last coordinate of the vector is assumed to be 0.
// The last column of the matrix is ignored.
template <class Vec, class MatType, int dim>
Vec VecTransform( const CMatrix<MatType, dim, dim>& affineMatrix, const Vec& vec )
{
	static_assert( Vec::Size() == dim - 1, "Illegal vector size." );

	const int lastIndex = dim - 1;
	for( int x = 0; x < lastIndex; x++ ) {
		assert( affineMatrix( x, lastIndex ) == MatType( 0 ) );
	}
	assert( affineMatrix( lastIndex, lastIndex ) == MatType( 1 ) );

	Vec result;
	for( int y = 0; y < lastIndex; y++ ) {
		for( int x = 0; x < lastIndex; x++ ) {
			result[y] += affineMatrix( x, y ) * vec[x];
		}
	}
	return result;
}

// Affine transformation version for transformation matrices without the last column.
template <class Vec, class MatType, int dim>
Vec VecTransform( const CMatrix<MatType, dim + 1, dim>& affineMatrix, const Vec& vec )
{
	static_assert( Vec::Size() == dim, "Illegal vector size." );

	Vec result;
	for( int y = 0; y < dim; y++ ) {
		for( int x = 0; x < dim; x++ ) {
			result[y] += affineMatrix( x, y ) * vec[x];
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////

// Transformation of an axis-aligned rect.
// The matrix is assumed to only have its scale and offset factors set.
template <class Rect, class Mat>
Rect AARectTransform( const Mat& affineMatrix, Rect aaRect )
{
	const auto newLeft = aaRect.Left() * affineMatrix( 0, 0 ) + affineMatrix( 2, 0 );
	const auto newTop = aaRect.Top() * affineMatrix( 1, 1 ) + affineMatrix( 2, 1 );
	const auto newRight = aaRect.Right() * affineMatrix( 0, 0 ) + affineMatrix( 2, 0 );
	const auto newBottom = aaRect.Bottom() * affineMatrix( 1, 1 ) + affineMatrix( 2, 1 );

	const auto newVerticals = minmax( newLeft, newRight );
	const auto newHorizontals = minmax( newTop, newBottom );

	return Rect{ newVerticals.GetLower(), newHorizontals.GetUpper(), newVerticals.GetUpper(), newHorizontals.GetLower() };
}

//////////////////////////////////////////////////////////////////////////

// 2D column major transformation creation.
template <class T>
CMatrix<T, 3, 3> CreateTransformation( CVector2<T> offset, typename CVector2<T>::FloatingPointVecType angleSin, 
	typename CVector2<T>::FloatingPointVecType angleCos )
{
	auto result = CMatrix<T, 3, 3>::CreateRawMatrix();
	result( 0, 0 ) = angleCos;
	result( 1, 0 ) = -angleSin;
	result( 0, 1 ) = angleSin;
	result( 1, 1 ) = angleCos;
	result( 2, 0 ) = offset[0];
	result( 2, 1 ) = offset[1];

	result( 0, 2 ) = T( 0 );
	result( 1, 2 ) = T( 0 );
	result( 2, 2 ) = T( 1 );
	return result;
}

template <class T>
CMatrix<T, 3, 3> CreateTransformation( CVector2<T> offset, CVector2<T> scale, typename CVector2<T>::FloatingPointVecType angleSin, 
	typename CVector2<T>::FloatingPointVecType angleCos )
{
	auto result = CMatrix<T, 3, 3>::CreateRawMatrix();
	result( 0, 0 ) = angleCos * scale.X();
	result( 1, 0 ) = -angleSin * scale.Y();
	result( 0, 1 ) = angleSin * scale.X();
	result( 1, 1 ) = angleCos * scale.Y();
	result( 2, 0 ) = offset[0];
	result( 2, 1 ) = offset[1];

	result( 0, 2 ) = T( 0 );
	result( 1, 2 ) = T( 0 );
	result( 2, 2 ) = T( 1 );
	return result;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

