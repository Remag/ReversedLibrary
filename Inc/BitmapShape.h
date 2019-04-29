#pragma once
#include <DynamicBitset.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Shape that is defined by a bitset, where each set bit represents a 1x1 unit of flagged hitbox space.
template <class T>
class CBitmapShape : public CShape<T> {
public:
	CBitmapShape() = default;
	CBitmapShape( CDynamicBitSet<> _bitmap, CVector2<int> _cellCount ) :
		bitmap( move( _bitmap ) ), cellCount( _cellCount ) {}

	// CShape.
	virtual THitboxShapeType GetType() const override final
		{ return HST_Bitmap; }

	CAARect<T> GetBaseBoundRect() const;
	virtual CAARect<T> GetBoundRect( CMatrix3<T> transformation ) const override final;

	void SetBitmap( CDynamicBitSet<> newValue, CVector2<int> newCellCount ) 
		{ bitmap = move( newValue ); cellCount = newCellCount; }

	const CDynamicBitSet<>& GetBitmap() const
		{ return bitmap; }
	CVector2<int> GetCellCount() const
		{ return cellCount; }

private:
	CVector2<int> cellCount;
	CDynamicBitSet<> bitmap;
};

//////////////////////////////////////////////////////////////////////////

template <class T>
CAARect<T> CBitmapShape<T>::GetBaseBoundRect() const
{
	return CAARect<T>{ CVector2<T>{}, T( cellCount.X() ), T( cellCount.Y() ) };
}

template <class T>
CAARect<T> CBitmapShape<T>::GetBoundRect( CMatrix3<T> transformation ) const
{
	const auto boundRect = GetBaseBoundRect();
	// Find transformed bound rectangle points.
	const CVector2<T> widthVec = TVector2( boundRect.Width(), T( 0 ) );
	const CVector2<T> heightVec = TVector2( T( 0 ), boundRect.Height() );
	
	const CVector2<T> widthOffset = VecTransform( transformation, widthVec );
	const CVector2<T> heightOffset = VecTransform( transformation, heightVec );

	CStackArray<CVector2<T>, 4> points;
	points[0] = PointTransform( transformation, boundRect.BottomLeft() );
	points[1] = points[0] + widthOffset;
	points[2] = points[1] + heightOffset;
	points[3] = points[0] + heightOffset;

	// Get the bounding rectangle of the transformed points.
	const auto minmaxX = minmax( points[0].X(), points[1].X(), points[2].X(), points[3].X() );
	const auto minmaxY = minmax( points[0].Y(), points[1].Y(), points[2].Y(), points[3].Y() );

	return TAARect( minmaxX.GetLower(), minmaxY.GetUpper(), minmaxX.GetUpper(), minmaxY.GetLower() );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

