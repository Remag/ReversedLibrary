#pragma once
#include <Interval.h>

namespace Relib {

// Type of a hitbox shape.
enum THitboxShapeType {
	HST_Null,	// non-existent hitbox.
	HST_Point,	// point in two dimensions.
	HST_AARect,	// axis-aligned rectangle.
	HST_AngledRect,	// angled rectangle.
	HST_Polygon,	// arbitrary polygon.
	HST_Circle, // circle shape.
	HST_Complex,	// a union of several shapes.
	HST_Bitmap,	// bitset-defined shape.
	HST_EnumCount
};

// An abstract two dimensional shape. Provides methods useful for collision detection.
template<class FloatingPointType>
class CShape {
public:
	typedef FloatingPointType Type;
	typedef CVector2<Type> TVector2;
	typedef CAARect<Type> TAARect;
	typedef CMatrix<Type, 3, 3> TMatrix3;
	typedef CInterval<Type> TInterval;

	virtual ~CShape() {}

	virtual THitboxShapeType GetType() const = 0;

	// Shape bounding rectangle.
	// transformation represents the shape's transformation matrix. It's applied to the base shape before calculating method's return value.
	virtual TAARect GetBoundRect( TMatrix3 transformation ) const = 0;

protected:
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

