#pragma once

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A circular shape for collision detection.
template <class T>
class CCircleShape : public CShape<T> {
public:
	CCircleShape( CVector2<T> _centerPoint, T _radius ) : centerPoint( _centerPoint ), radius( _radius ) {}

	// Circle parameters in model space.
	CVector2<T> GetBaseCenter() const
		{ return centerPoint; }
	T GetBaseRadius() const
		{ return radius; }

	virtual THitboxShapeType GetType() const override final 
		{ return HST_Circle; }
	virtual CAARect<T> GetBoundRect( CMatrix3<T> transformation ) const override final;

private:
	CVector2<T> centerPoint;
	T radius;
};

//////////////////////////////////////////////////////////////////////////

template <class T>
CAARect<T> CCircleShape<T>::GetBoundRect( CMatrix3<T> transformation ) const
{
	assert( transformation( 0, 0 ) == transformation( 1, 1 ) );
	const auto globalCenter = PointTransform( transformation, centerPoint );
	const auto globalRadius = radius * transformation( 0, 0 );
	return TAARect( globalCenter.X() - globalRadius, globalCenter.Y() + globalRadius, globalCenter.X() + globalRadius, globalCenter.Y() - globalRadius );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

