#pragma once
#include <Shape.h>

namespace Relib {

// A two dimensional point.
template<class T>
class CPointShape : public CShape<T> {
public:
	typedef T Type;

	explicit CPointShape( CVector2<T> basePoint );

	// CShape.
	virtual THitboxShapeType GetType() const override final
		{ return HST_Point; }
	virtual CAARect<T> GetBoundRect( CMatrix3<T> transformation ) const override final;

	CVector2<T> GetBasePoint() const
		{ return basePoint; }
	// Get point's position in global coordinates.
	CVector2<T> GetGlobalPoint( CMatrix3<T> transformation ) const
		{ return basePoint + GetOffset( transformation ); }

private:
	CVector2<T> basePoint;
};

//////////////////////////////////////////////////////////////////////////

template<class T>
CPointShape<T>::CPointShape( CVector2<T> _basePoint ) :
	basePoint( _basePoint )
{
}

template<class T>
CAARect<T> CPointShape<T>::GetBoundRect( CMatrix3<T> transformation ) const
{
	const auto globalPoint = GetGlobalPoint( transformation );
	return TAARect( globalPoint, globalPoint );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.