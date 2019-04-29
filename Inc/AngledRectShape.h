#pragma once
#include <Shape.h>
#include <StackArray.h>

namespace Relib {

// A convex shape represented by an arbitrary rectangle.
template<class T>
class CAngledRectShape : public CShape<T> {
public:
	CAngledRectShape( CAARect<T> _rect );

	CAARect<T> GetBaseRect() const
		{ return baseRect; }
	void SetBaseRect( CAARect<T> newRect ) 
		{ baseRect = newRect; }

	// Fill the given array with position of an angled rect points.
	void GetRectPoints( CMatrix3<T> transformation, CStackArray<CVector2<T>, 4>& result ) const;

	virtual THitboxShapeType GetType() const override final 
		{ return HST_AngledRect; }
	virtual CAARect<T> GetBoundRect( CMatrix3<T> transformation ) const override final;

private:
	// Base rectangle.
	CAARect<T> baseRect;
};

//////////////////////////////////////////////////////////////////////////

template<class T>
CAngledRectShape<T>::CAngledRectShape( CAARect<T> _rect ) :
	baseRect( _rect )
{
}

template<class T>
CAARect<T> CAngledRectShape<T>::GetBoundRect( CMatrix3<T> transformation ) const
{
	CStackArray<CVector2<T>, 4> points;
	GetRectPoints( transformation, points );

	const auto minmaxX = minmax( points[0].X(), points[1].X(), points[2].X(), points[3].X() );
	const auto minmaxY = minmax( points[0].Y(), points[1].Y(), points[2].Y(), points[3].Y() );

	return TAARect( minmaxX.GetLower(), minmaxY.GetUpper(), minmaxX.GetUpper(), minmaxY.GetLower() );
}

template<class T>
void CAngledRectShape<T>::GetRectPoints( CMatrix3<T> transformation, CStackArray<CVector2<T>, 4>& result ) const
{
	const auto widthVec = TVector2( baseRect.Width(), T( 0 ) );
	const auto heightVec = TVector2( T( 0 ), baseRect.Height() );
	
	const auto widthOffset = VecTransform( transformation, widthVec );
	const auto heightOffset = VecTransform( transformation, heightVec );

	result[0] = PointTransform( transformation, baseRect.BottomLeft() );
	result[1] = result[0] + widthOffset;
	result[2] = result[1] + heightOffset;
	result[3] = result[0] + heightOffset;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.