#pragma once

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A quaternion wrapper. Supports common operation with objects' orientation.
template <class Type>
class CQuaternion {
public:
	typedef typename Types::Conditional<Types::IsFloatingPoint<Type>::Result, Type, double>::Result FloatingPointType;

	// Create a zero quaternion.
	CQuaternion() = default;
	// Create a quaternion from the given vector.
	explicit CQuaternion( const CVector4<Type>& vec );
	// Create a quaternion with the given direction and angle.
	CQuaternion( CVector3<Type> dir, FloatingPointType angleRad );
	// Construct a quaternion that transforms startDir into endDir.
	CQuaternion( CVector3<Type> startDir, CVector3<Type> endDir );
	CQuaternion( const CQuaternion& other ) = default;

	CVector4<Type> VectorForm() const
		{ return baseVec; }
	CMatrix<Type, 4, 4> MatrixForm() const;

	bool IsNull() const
		{ return baseVec.IsNull(); }

	// Normalization.
	CQuaternion<Type> Normalize() const;

	// Get the opposite angle rotation.
	CQuaternion<Type> Inverse() const;

	// Quaternion combination.
	CQuaternion<Type> operator*( const CQuaternion& other ) const;
	CQuaternion<Type>& operator*=( const CQuaternion& other );

	// Vector transformation.
	CVector4<Type> GetTransform( const CVector4<Type>& vec ) const;
	CVector3<Type> GetTransform( const CVector3<Type>& vec ) const;

	// Matrix transformation.
	CMatrix<Type, 4, 4> GetLeftTransform( const CMatrix<Type, 4, 4>& mat ) const;
	CMatrix<Type, 4, 4> GetRightTransform( const CMatrix<Type, 4, 4>& mat ) const;

private:
	// Base vector with the necessary data.
	CVector4<Type> baseVec;
};

//////////////////////////////////////////////////////////////////////////

template <class Type>
CQuaternion<Type>::CQuaternion( const CVector4<Type>& vec ) :
	baseVec( vec )
{
}

template <class Type>
CQuaternion<Type>::CQuaternion( CVector3<Type> dir, FloatingPointType angleRad )
{
	FloatingPointType halfAngleSin;
	FloatingPointType halfAngleCos;
	relibSinCos( angleRad / 2, halfAngleSin, halfAngleCos );
	baseVec[0] = Type( dir.X() * halfAngleSin );
	baseVec[1] = Type( dir.Y() * halfAngleSin );
	baseVec[2] = Type( dir.Z() * halfAngleSin );
	baseVec[3] = Type( halfAngleCos );
}

template <class Type>
CQuaternion<Type>::CQuaternion( CVector3<Type> startDir, CVector3<Type> endDir )
{
	FloatingPointType realPart = FloatingPointType( 1 ) + startDir * endDir;
	CVector3<Type> rotAxis;
	static const FloatingPointType epsilon = FloatingPointType( 0.00001f );
	if( realPart < epsilon ) {
		// startDir and endDir are directly opposite of one another.
		realPart = FloatingPointType( 0 );
		rotAxis = abs( startDir.X() ) > abs( startDir.Z() )
			? CVector3<Type>( -startDir.Y(), startDir.X(), Type( 0 ) )
			: CVector3<Type>( Type( 0 ), -startDir.Z(), startDir.Y() );
	} else {
		rotAxis = Cross( startDir, endDir );
	}

	baseVec = CVector4<Type>( rotAxis, realPart ).Normalize();
}

template <class Type>
CMatrix<Type, 4, 4> CQuaternion<Type>::MatrixForm() const
{
	CMatrix<Type, 4, 4> result = CMatrix<Type, 4, 4>::CreateRawMatrix();

	const FloatingPointType qxx = baseVec.X() * baseVec.X();
	const FloatingPointType qxy = 2 * baseVec.X() * baseVec.Y();
	const FloatingPointType qxz = 2 * baseVec.X() * baseVec.Z();
	const FloatingPointType qxw = 2 * baseVec.X() * baseVec[3];
	const FloatingPointType qyy = baseVec.Y() * baseVec.Y();
	const FloatingPointType qyz = 2 * baseVec.Y() * baseVec.Z();
	const FloatingPointType qyw = 2 * baseVec.Y() * baseVec[3];
	const FloatingPointType qzz = baseVec.Z() * baseVec.Z();
	const FloatingPointType qzw = 2 * baseVec.Z() * baseVec[3];
	const FloatingPointType qww = baseVec[3] * baseVec[3];

	result( 0, 0 ) = qww + qxx - qyy - qzz;
	result( 0, 1 ) = qxy + qzw;
	result( 0, 2 ) = qxz - qyw;
	result( 0, 3 ) = 0;
	result( 1, 0 ) = qxy - qzw;
	result( 1, 1 ) = qww + qyy - qxx - qzz;
	result( 1, 2 ) = qyz + qxw;
	result( 1, 3 ) = 0;
	result( 2, 0 ) = qxz + qyw;
	result( 2, 1 ) = qyz - qxw;
	result( 2, 2 ) = qww + qzz - qxx - qyy;
	result( 2, 3 ) = 0;
	result( 3, 0 ) = 0;
	result( 3, 1 ) = 0;
	result( 3, 2 ) = 0;
	result( 3, 3 ) = 1;

	return result;
}

template <class Type>
CQuaternion<Type> CQuaternion<Type>::Normalize() const
{
	return CQuaternion<Type>( baseVec.Normalize() );
}

template <class Type>
CQuaternion<Type> CQuaternion<Type>::Inverse() const
{
	return CQuaternion<Type>( CVector4<Type>( -baseVec.X(), -baseVec.Y(), -baseVec.Z(), baseVec.W() ) );
}

template <class Type>
CQuaternion<Type> CQuaternion<Type>::operator*( const CQuaternion<Type>& other ) const
{
	CQuaternion result;
	const CVector4<Type>& left = baseVec;
	const CVector4<Type>& right = other.baseVec;
	result.baseVec[0] = left[3] * right[0] + left[0] * right[3] + left[1] * right[2] - left[2] * right[1];
	result.baseVec[1] = left[3] * right[1] + left[1] * right[3] + left[2] * right[0] - left[0] * right[2];
	result.baseVec[2] = left[3] * right[2] + left[2] * right[3] + left[0] * right[1] - left[1] * right[0];
	result.baseVec[3] = left[3] * right[3] - left[0] * right[0] - left[1] * right[1] - left[2] * right[2];
	return result;
}

template <class Type>
CQuaternion<Type>& CQuaternion<Type>::operator*=( const CQuaternion<Type>& other )
{
	CQuaternion result = *this * other;
	baseVec = result.baseVec;
	return *this;
}

template <class Type>
CVector4<Type> CQuaternion<Type>::GetTransform( const CVector4<Type>& vec ) const
{
	return MatrixForm() * vec;
}

template <class Type>
CVector3<Type> CQuaternion<Type>::GetTransform( const CVector3<Type>& vec ) const
{
	return vec.GetLeftTransform( MatrixForm() );
}

template <class Type>
CMatrix<Type, 4, 4> CQuaternion<Type>::GetLeftTransform( const CMatrix<Type, 4, 4>& mat ) const
{
	return MatrixForm() * mat;
}

template <class Type>
CMatrix<Type, 4, 4> CQuaternion<Type>::GetRightTransform( const CMatrix<Type, 4, 4>& mat ) const
{
	return mat * MatrixForm();
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

