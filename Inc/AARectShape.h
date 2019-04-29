#pragma once
#include <Shape.h>

namespace Relib {

// A convex shape represented by an axis aligned rectangle.
template<class T>
class CAARectShape : public CShape<T> {
public:
	CAARectShape() = default;
	CAARectShape( CAARect<T> rect );

	// Get/Set rectangle base size.
	CAARect<T>& GetBaseRect()
		{ return baseRect; }
	CAARect<T> GetBaseRect() const
		{ return baseRect; }
	void SetBaseRect( CAARect<T> newSize )
		{ baseRect = newSize; }

	// Calculate rectangle's position in global coordinates.
	CAARect<T> GetGlobalRect( CMatrix3<T> transformation ) const;

	virtual THitboxShapeType GetType() const override final 
		{ return HST_AARect; }
	virtual CAARect<T> GetBoundRect( CMatrix3<T> transformation ) const override final
		{ return GetGlobalRect( transformation ); }

private:
	// Base rectangle.
	CAARect<T> baseRect;
};

//////////////////////////////////////////////////////////////////////////

template<class T>
CAARectShape<T>::CAARectShape( CAARect<T> _rect ) :
	baseRect( _rect )
{
}

template<class T>
CAARect<T> CAARectShape<T>::GetGlobalRect( CMatrix3<T> transformation ) const
{
	const auto offset = GetOffset( transformation );
	const auto scale = GetScale( transformation );
	const auto globalLeft = baseRect.Left() * scale.X() + offset.X();
	const auto globalTop = baseRect.Top() * scale.Y() + offset.Y();
	const auto globalRight = baseRect.Right() * scale.X() + offset.X();
	const auto globalBottom = baseRect.Bottom() * scale.Y() + offset.Y();
	return TAARect( min( globalLeft, globalRight ), max( globalTop, globalBottom ), max( globalLeft, globalRight ), min( globalTop, globalBottom ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.