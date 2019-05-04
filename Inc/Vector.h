#pragma once
#include <Remath.h>
#include <Matrix.h>
#include <HashUtils.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Vector of primitive types. VecType must be fundamental.
template <class VecType, int dim>
class CVector {
public:
	staticAssert( Types::IsFundamental<VecType>::Result );
	// Floating point type used as return value for some methods.
	// Float is used for integral types. VecType is used for floating point types.
	typedef typename Types::Conditional<Types::IsFloatingPoint<VecType>::Result, VecType, float>::Result FloatingPointVecType;
	typedef VecType ElemVecType;

	// Default constructor. Sets all the data to zero.
	CVector();
	// Fill with the same value.
	explicit CVector( VecType fillValue );
	// Construct from several values.
	// Each argument must have the the type VecType or CVector<VecType>.
	template <class FirstType, class SecondType, class... RestTypes>
	CVector( FirstType first, SecondType second, RestTypes... rest );

	// Create a vector without initializing its members.
	static CVector<VecType, dim> CreateRawVector();

	static constexpr int Size()
		{ return dim; }

	// Element access.
	VecType& operator[]( int pos )
		{ assert( pos < dim ); return vectorData[pos]; }
	VecType operator[]( int pos ) const
		{ assert( pos < dim ); return vectorData[pos]; }
	
	VecType& X()
		{ staticAssert( dim >= 2 ); return operator[]( 0 ); }
	VecType X() const
		{ staticAssert( dim >= 2 ); return operator[]( 0 ); }
	VecType& Y()
		{ staticAssert( dim >= 2 ); return operator[]( 1 ); }
	VecType Y() const
		{ staticAssert( dim >= 2 ); return operator[]( 1 ); }
	VecType& Z()
		{ staticAssert( dim >= 3 ); return operator[]( 2 ); }
	VecType Z() const
		{ staticAssert( dim >= 3 ); return operator[]( 2 ); }
	VecType& W()
		{ staticAssert( dim >= 4 ); return operator[]( 3 ); }
	VecType W() const
		{ staticAssert( dim >= 4 ); return operator[]( 3 ); }

	// Simple swizzling.
	CVector<VecType, 2> XX() const;
	CVector<VecType, 2> XY() const;
	CVector<VecType, 2> XZ() const;
	CVector<VecType, 2> XW() const;
	CVector<VecType, 2> YX() const;
	CVector<VecType, 2> YY() const;
	CVector<VecType, 2> YZ() const;
	CVector<VecType, 2> YW() const;
	CVector<VecType, 2> ZX() const;
	CVector<VecType, 2> ZY() const;
	CVector<VecType, 2> ZZ() const;
	CVector<VecType, 2> ZW() const;
	CVector<VecType, 2> WX() const;
	CVector<VecType, 2> WY() const;
	CVector<VecType, 2> WZ() const;
	CVector<VecType, 2> WW() const;

	CVector<VecType, 3> XYZ() const
		{ staticAssert( dim >= 3 ); return CVector<VecType, 3>( vectorData[0], vectorData[1], vectorData[2] ); }

	// Direct buffer access.
	VecType* Ptr()
		{ return vectorData; }
	const VecType* Ptr() const
		{ return vectorData; }

	// Floating point conversion.
	template<class T>
	explicit operator CVector<T, dim>() const;

	// Comparison.
	bool operator==( const CVector& other ) const;
	bool operator!=( const CVector& other ) const
		{ return !operator==( other ); }
	// Comparison with a zero vector.
	bool IsNull() const;
	// Is the vector zero within a given epsilon.
	bool IsAlmostNull( FloatingPointVecType epsilon ) const;

	// Addition and subtraction.
	CVector& operator+=( const CVector& other );
	CVector& operator-=( const CVector& other );

	// Scalar multiplication and division.
	CVector& operator*=( FloatingPointVecType mul );
	CVector& operator*=( int mul );
	CVector& operator/=( FloatingPointVecType mul );

	// Unary minus.
	CVector operator-() const;

	// Vector's length methods.
	FloatingPointVecType SquareLength() const;
	FloatingPointVecType Length() const
		{ return relibSqrt( SquareLength() ); }
	// Normalize the vector.
	CVector<VecType, dim> Normalize() const;

	int HashKey() const;

private:
	VecType vectorData[dim];

	// Constructor for raw vectors.
	class CRawCreationTag {};
	explicit CVector( CRawCreationTag ) {}

	template <int i, class... Types>
	void initVector( VecType first, Types... rest );
	template <int i, int argDim, class... Types>
	void initVector( CVector<VecType, argDim> first, Types... rest );
	template <int i, class... Types>
	void initVector( Types... nothing );
};

//////////////////////////////////////////////////////////////////////////

template <class VecType, int dim>
CVector<VecType, dim>::CVector()
{
	memset( vectorData, 0, dim * sizeof( VecType ) );
}

template <class VecType, int dim>
CVector<VecType, dim>::CVector( VecType fillValue )
{
	for( auto& target : vectorData ) {
		target = fillValue;
	}
}

template <class VecType, int dim>
template <class FirstType, class SecondType, class... RestTypes>
CVector<VecType, dim>::CVector( FirstType first, SecondType second, RestTypes... rest )
{
	initVector<0>( first, second, rest... );
}

template <class VecType, int dim>
template <int i, class... Types>
void CVector<VecType, dim>::initVector( VecType first, Types... rest )
{
	staticAssert( i < dim );
	vectorData[i] = first;
	initVector<i + 1>( rest... );
}

template <class VecType, int dim>
template <int i, int argDim, class... Types>
void CVector<VecType, dim>::initVector( CVector<VecType, argDim> first, Types... rest )
{
	staticAssert( i + argDim <= dim );
	memcpy( vectorData + i, first.Ptr(), argDim * sizeof( VecType ) );
	initVector<i + argDim>( rest... );
}

template <class VecType, int dim>
template <int i, class... Types>
void CVector<VecType, dim>::initVector( Types... nothing )
{
	// This function is called at the end or if one of the types is not VecType or CVector<VecType>.
	static_assert( sizeof...( nothing ) == 0, "Wrong type has been specified on vector initialization." );
	// All values must be initialized.
	static_assert( i == dim, "Invalid number of arguments." );
}

template < class VecType, int dim>
inline CVector<VecType, dim> CVector<VecType, dim>::CreateRawVector()
{
	CRawCreationTag tag;
	CVector<VecType, dim> result( tag );
	return result;
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::XX() const
{
	return CVector<VecType, 2>( vectorData[0], vectorData[0] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::XY() const
{
	return CVector<VecType, 2>( vectorData[0], vectorData[1] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::XZ() const
{
	return CVector<VecType, 2>( vectorData[0], vectorData[2] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::XW() const
{
	return CVector<VecType, 2>( vectorData[0], vectorData[3] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::YX() const
{
	return CVector<VecType, 2>( vectorData[1], vectorData[0] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::YY() const
{
	return CVector<VecType, 2>( vectorData[1], vectorData[1] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::YZ() const
{
	return CVector<VecType, 2>( vectorData[1], vectorData[2] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::YW() const
{
	return CVector<VecType, 2>( vectorData[1], vectorData[3] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::ZX() const
{
	return CVector<VecType, 2>( vectorData[2], vectorData[0] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::ZY() const
{
	return CVector<VecType, 2>( vectorData[2], vectorData[1] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::ZZ() const
{
	return CVector<VecType, 2>( vectorData[2], vectorData[2] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::ZW() const
{
	return CVector<VecType, 2>( vectorData[2], vectorData[3] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::WX() const
{
	return CVector<VecType, 2>( vectorData[3], vectorData[0] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::WY() const
{
	return CVector<VecType, 2>( vectorData[3], vectorData[1] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::WZ() const
{
	return CVector<VecType, 2>( vectorData[3], vectorData[2] );
}

template <class VecType, int dim>
CVector<VecType, 2> Relib::CVector<VecType, dim>::WW() const
{
	return CVector<VecType, 2>( vectorData[3], vectorData[3] );
}

template <class VecType, int dim>
template<class T>
CVector<VecType, dim>::operator CVector<T, dim>() const
{
	CVector<T, dim> result;
	for( int i = 0; i < dim; i++ ) {
		result[i] = static_cast<T>( vectorData[i] );
	}
	return result;
}

template <class VecType, int dim>
inline bool CVector<VecType, dim>::operator==( const CVector<VecType, dim>& other ) const
{
	for( int i = 0; i < dim; i++ ) {
		if( vectorData[i] != other.vectorData[i] ) {
			return false;
		}
	}
	return true;
}

template <class VecType, int dim>
inline CVector<VecType, dim>& CVector<VecType, dim>::operator+=( const CVector<VecType, dim>& other )
{
	for( int i = 0; i < dim; i++ ) {
		vectorData[i] += other.vectorData[i];
	}
	return *this;
}

template <class VecType, int dim>
inline CVector<VecType, dim>& CVector<VecType, dim>::operator-=( const CVector<VecType, dim>& other )
{
	for( int i = 0; i < dim; i++ ) {
		vectorData[i] -= other.vectorData[i];
	}
	return *this;
}

template <class VecType, int dim>
CVector<VecType, dim>& CVector<VecType, dim>::operator*=( FloatingPointVecType mul )
{
	for( auto& elem : vectorData ) {
		elem = static_cast<VecType>( elem * mul );
	}
	return *this;
}

template <class VecType, int dim>
CVector<VecType, dim>& CVector<VecType, dim>::operator*=( int mul )
{
	return *this *= static_cast<FloatingPointVecType>( mul );
}

template <class VecType, int dim>
inline CVector<VecType, dim>& CVector<VecType, dim>::operator/=( FloatingPointVecType mul )
{
	for( auto& elem : vectorData ) {
		elem = static_cast<VecType>( elem / mul );
	}
	return *this;
}

template < class VecType, int dim>
inline CVector<VecType, dim> CVector<VecType, dim>::operator-() const
{
	CVector<VecType, dim> result = CreateRawVector();
	for( int i = 0; i < dim; i++ ) {
		result[i] = -vectorData[i];
	}
	return result;
}

template < class VecType, int dim>
inline bool CVector<VecType, dim>::IsNull() const
{
	for( auto elem : vectorData ) {
		if( elem != 0 ) {
			return false;
		}
	}
	return true;
}

template <class VecType, int dim>
inline bool CVector<VecType, dim>::IsAlmostNull( FloatingPointVecType epsilon ) const
{
	assert( epsilon >= 0 );
	for( auto elem : vectorData ) {
		if( elem > epsilon || elem < -epsilon ) {
			return false;
		}
	}
	return true;
}

template < class VecType, int dim>
inline typename CVector<VecType, dim>::FloatingPointVecType CVector<VecType, dim>::SquareLength() const
{
	FloatingPointVecType result = 0;
	for( auto elem : vectorData ) {
		result += elem * elem;
	}
	return result;
}

template <class VecType, int dim>
CVector<VecType, dim> CVector<VecType, dim>::Normalize() const
{
	return invSqrt( static_cast<FloatingPointVecType>( SquareLength() ) ) * ( *this );
}

template <class VecType, int dim>
inline int CVector<VecType, dim>::HashKey() const
{
	int resultHash = 0;
	for( const auto& elem : vectorData ) {
		resultHash = CombineHashKey( resultHash, CDefaultHash<VecType>::HashKey( elem ) );
	}
	return resultHash;
}

//////////////////////////////////////////////////////////////////////////

// Vector math functions.

template <class VecType, int dim>
inline CVector<VecType, dim> operator+( CVector<VecType, dim> left, const CVector<VecType, dim>& right )
{
	left += right;
	return left;
}

template <class VecType, int dim>
inline CVector<VecType, dim> operator-( CVector<VecType, dim> left, const CVector<VecType, dim>& right )
{
	left -= right;
	return left;
}

template <class VecType, int dim>
inline CVector<VecType, dim> operator*( CVector<VecType, dim> left, typename CVector<VecType, dim>::FloatingPointVecType mul )
{
	left *= mul;
	return left;
}

template <class VecType, int dim>
inline CVector<VecType, dim> operator*( CVector<VecType, dim> left, int mul )
{
	left *= mul;
	return left;
}

template <class VecType, int dim>
inline CVector<VecType, dim> operator*( typename CVector<VecType, dim>::FloatingPointVecType mul, CVector<VecType, dim> right )
{
	right *= mul;
	return right;
}

template <class VecType, int dim>
inline CVector<VecType, dim> operator*( int mul, CVector<VecType, dim> right )
{
	right *= mul;
	return right;
}

template <class VecType, int dim>
inline CVector<VecType, dim> operator/( CVector<VecType, dim> left, typename CVector<VecType, dim>::FloatingPointVecType mul )
{
	left /= mul;
	return left;
}

template <class VecType, int dim>
inline typename CVector<VecType, dim>::FloatingPointVecType Dot( const CVector<VecType, dim>& left, const CVector<VecType, dim>& right )
{
	typename CVector<VecType, dim>::FloatingPointVecType result = 0;
	for( int i = 0; i < dim; i++ ) {
		result += left[i] * right[i];
	}
	return result;
}

template <class VecType, int dim>
inline typename CVector<VecType, dim>::FloatingPointVecType operator*( const CVector<VecType, dim>& left, const CVector<VecType, dim>& right )
{
	return Dot( left, right );
}

//////////////////////////////////////////////////////////////////////////

// Serialization.

template <class VecType, int dim>
inline CArchiveWriter& operator<<( CArchiveWriter& archive, const CVector<VecType, dim>& vec )
{
	for( const auto& elem : vec ) {
		archive << elem;
	}
	return archive;
}

template <class VecType, int dim>
inline CArchiveReader& operator>>( CArchiveReader& archive, CVector<VecType, dim>& vec )
{
	for( auto& elem : vec ) {
		archive >> elem;
	}
	return archive;
}

// Range-based for loops support.

template <class VecType, int dim>
VecType* begin( CVector<VecType, dim>& vec )
{
	return vec.Ptr();
}

template <class VecType, int dim>
VecType* end( CVector<VecType, dim>& vec )
{
	return vec.Ptr() + dim;
}

template <class VecType, int dim>
const VecType* begin( const CVector<VecType, dim>& vec )
{
	return vec.Ptr();
}

template <class VecType, int dim>
const VecType* end( const CVector<VecType, dim>& vec )
{
	return vec.Ptr() + dim;
}

//////////////////////////////////////////////////////////////////////////

// Special functions for two dimensional vectors.
template <class VecType>
using CVector2 = CVector<VecType, 2>;

// Length of vector's projection.
template <class VecType>
typename CVector2<VecType>::FloatingPointVecType ProjectionLength( const CVector2<VecType>& vec, const CVector2<VecType>& projectVec )
{
	return ( vec * projectVec ) / projectVec.Length();
}

// Rotation in two dimensions with known cosine and sine.
template <class VecType>
CVector2<VecType> Rotation( const CVector2<VecType>& vec, typename CVector2<VecType>::FloatingPointVecType angleSin, typename CVector2<VecType>::FloatingPointVecType angleCos )
{
	return CVector2<VecType>( vec.X() * angleCos - vec.Y() * angleSin, vec.Y() * angleCos + vec.X() * angleSin );
}

// Rotation in two dimensions.
// Angle is given in radians.
template <class VecType>
CVector2<VecType> Rotation( const CVector2<VecType>& vec, typename CVector2<VecType>::FloatingPointVecType angle )
{
	typename CVector2<VecType>::FloatingPointVecType angleSin;
	typename CVector2<VecType>::FloatingPointVecType angleCos;
	relibSinCos( angle, angleSin, angleCos );
	return Rotation( vec, angleSin, angleCos );
}

//////////////////////////////////////////////////////////////////////////

// Special functions for three dimensional vectors.
template <class VecType>
using CVector3 = CVector<VecType, 3>;

template <class VecType>
CVector3<VecType> Cross( const CVector3<VecType>& left, const CVector3<VecType>& right )
{
	const VecType resultX = left.Y() * right.Z() - left.Z() * right.Y();
	const VecType resultY = left.Z() * right.X() - left.X() * right.Z();
	const VecType resultZ = left.X() * right.Y() - left.Y() * right.X();
	return CVector3<VecType>( resultX, resultY, resultZ );
}

//////////////////////////////////////////////////////////////////////////

template <class VecType>
using CVector4 = CVector<VecType, 4>;

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

