#pragma once

#include <Reassert.h>
#include <Vector.h>

namespace Relib {

// Axis aligned rectangle. Bottom is assumed to be less than top.
template <class Type>
class CAARect {
public:
	// Floating point type used as return value for some methods.
	// Double is used for integral types. Type is used for floating point types.
	typedef typename Types::Conditional<Types::IsFloatingPoint<Type>::Result, Type, float>::Result FloatingPointType;

	CAARect();
	CAARect( Type left, Type top, Type right, Type bottom );
	CAARect( CVector2<Type> bottomLeft, CVector2<Type> topRight );
	CAARect( CVector2<Type> bottomLeft, Type width, Type height );
	static CAARect CreateRawRect();

	// Conversion to floating point.
	explicit operator CAARect<FloatingPointType>() const;

	Type Left() const
		{ return left; }
	Type& Left()
		{ return left; }
	Type Top() const
		{ return top; }
	Type& Top()
		{ return top; }
	Type Right() const
		{ return right; }
	Type& Right()
		{ return right; }		
	Type Bottom() const
		{ return bottom; }
	Type& Bottom()
		{ return bottom; }

	CVector2<Type> Size() const;
	Type Width() const;
	Type Height() const;
	Type Area() const;

	CVector2<Type> TopLeft() const;
	CVector2<Type> BottomRight() const;
	CVector2<Type> TopRight() const;
	CVector2<Type> BottomLeft() const;
	CVector2<Type> CenterPoint() const;

	bool IsNull() const;
	bool IsEmpty() const;
	bool IsValid() const;
	void SetRect( Type left, Type top, Type right, Type bottom );
	void SetRect( CVector2<Type> bottomLeft, CVector2<Type> topRight );
	void SetRect( CVector2<Type> bottomLeft, Type width, Type height );
	void Empty();
	void OffsetRect( Type x, Type y );
	void OffsetRect( CVector2<Type> offset );

	// Weak containment and intersection checks. operator<= is used for edge cases.
	bool Has( CVector2<Type> pos ) const;
	bool Has( CAARect<Type> other ) const;
	bool Intersects( CAARect other ) const;

	// Strict containment and intersection checks. operator< is used for edge cases.
	bool StrictHas( CVector2<Type> pos ) const;
	bool StrictHas( CAARect<Type> other ) const;
	bool StrictIntersects( CAARect<Type> other ) const;

	bool operator==( CAARect<Type> rect ) const;
	bool operator!=( CAARect<Type> rect ) const;

private:
	Type left;
	Type top;
	Type right;
	Type bottom;

	class CRawCreationTag {};

	CAARect( CRawCreationTag ) {}
};

template<class Type>
inline CAARect<Type>::CAARect() :
	left( 0 ),
	top( 0 ),
	right( 0 ),
	bottom( 0 )
{
}

template<class Type>
inline CAARect<Type>::CAARect( Type _left, Type _top, Type _right, Type _bottom ) :
	left( _left ),
	top( _top ),
	right( _right ),
	bottom( _bottom )
{
}

template<class Type>
inline CAARect<Type>::CAARect( CVector2<Type> bottomLeft, CVector2<Type> topRight ) :
	left( bottomLeft.X() ),
	top( topRight.Y() ),
	right( topRight.X() ),
	bottom( bottomLeft.Y() )
{
}

template<class Type>
inline CAARect<Type>::CAARect( CVector2<Type> bottomLeft, Type width, Type height ) :
	left( bottomLeft.X() ),
	top( bottomLeft.Y() + height ),
	right( bottomLeft.X() + width ),
	bottom( bottomLeft.Y() )
{
}

template<class Type>
CAARect<Type> CAARect<Type>::CreateRawRect()
{
	CRawCreationTag tag;
	return CAARect<Type>( tag );
}

template<class Type>
CAARect<Type>::operator CAARect<typename CAARect<Type>::FloatingPointType>() const
{
	return CAARect<FloatingPointType>{ 
		static_cast<FloatingPointType>( left ),
		static_cast<FloatingPointType>( top ),
		static_cast<FloatingPointType>( right ),
		static_cast<FloatingPointType>( bottom )
		};
}

template<class Type>
CVector2<Type> Relib::CAARect<Type>::Size() const
{
	return CVector2<Type>{ Width(), Height() };
}

template<class Type>
inline Type CAARect<Type>::Width() const
{
	return right - left;
}

template<class Type>
inline Type CAARect<Type>::Height() const
{
	return top - bottom;
}

template<class Type>
inline Type CAARect<Type>::Area() const
{
	return Width() * Height();
}

template<class Type>
inline CVector2<Type> CAARect<Type>::TopLeft() const
{
	return CVector2<Type>( left, top );
}

template<class Type>
inline CVector2<Type> CAARect<Type>::TopRight() const
{
	return CVector2<Type>( right, top );
}

template<class Type>
inline CVector2<Type> CAARect<Type>::BottomRight() const
{
	return CVector2<Type>( right, bottom );
}

template<class Type>
inline CVector2<Type> CAARect<Type>::BottomLeft() const
{
	return CVector2<Type>( left, bottom );
}

template<class Type>
inline CVector2<Type> CAARect<Type>::CenterPoint() const
{
	return CVector2<Type>( ( left + right ) / 2, ( top + bottom ) / 2 );
}

template<class Type>
inline bool CAARect<Type>::IsNull() const
{
	return left == 0 && right == 0 && top == 0 && bottom == 0;
}

template<class Type>
inline bool CAARect<Type>::IsEmpty() const
{
	return left >= right || top <= bottom;
}

template<class Type>
bool CAARect<Type>::IsValid() const
{
	return left < right && top > bottom;
}

template<class Type>
inline void CAARect<Type>::SetRect( Type _left, Type _top, Type _right, Type _bottom )
{
	left = _left;
	top = _top;
	right = _right;
	bottom = _bottom;
}

template<class Type>
inline void CAARect<Type>::SetRect( CVector2<Type> bottomLeft, CVector2<Type> topRight )
{
	SetRect( bottomLeft.X(), topRight.Y(), topRight.X(), bottomLeft.Y() );
}

template<class Type>
inline void CAARect<Type>::SetRect( CVector2<Type> bottomLeft, Type width, Type height )
{
	SetRect( bottomLeft.X(), bottomLeft.Y() + height, bottomLeft.X() + width, bottomLeft.Y() );
}

template<class Type>
inline void CAARect<Type>::Empty()
{
	left = right = top = bottom = 0;
}

template<class Type>
inline void CAARect<Type>::OffsetRect( Type x, Type y )
{
	left += x;
	right += x;
	top += y;
	bottom += y;
}

template<class Type>
inline void CAARect<Type>::OffsetRect( CVector2<Type> offset )
{
	OffsetRect( offset.X(), offset.Y() );
}

template<class Type>
inline bool CAARect<Type>::operator==( CAARect<Type> rect ) const
{
	return left == rect.left && right == rect.right && top == rect.top && bottom == rect.bottom;
}

template<class Type>
inline bool CAARect<Type>::operator!=( CAARect<Type> rect ) const
{
	return left != rect.left || right != rect.right || top != rect.top || bottom != rect.bottom;
}

template <class Type>
bool CAARect<Type>::Has( CVector2<Type> pos ) const
{
	return left <= pos.X() && pos.X() <= right && pos.Y() <= top && bottom <= pos.Y();
}

template <class Type>
bool CAARect<Type>::Has( CAARect<Type> other ) const
{
	return left <= other.left && other.right <= right && other.top <= top && bottom <= other.bottom;
}

template<class Type>
bool CAARect<Type>::Intersects( CAARect<Type> other ) const
{
	return !( other.left > right
		|| other.right < left
		|| other.top < bottom
		|| other.bottom > top );
}

template<class Type>
bool CAARect<Type>::StrictHas( CVector2<Type> pos ) const
{
	return left < pos.X() && pos.X() < right && pos.Y() < top && bottom < pos.Y();
}

template<class Type>
bool CAARect<Type>::StrictHas( CAARect<Type> other ) const
{
	return left < other.left && other.right < right && other.top < top && bottom < other.bottom;
}

template<class Type>
bool CAARect<Type>::StrictIntersects( CAARect<Type> other ) const
{
	return !( other.left >= right
		|| other.right <= left
		|| other.top <= bottom
		|| other.bottom >= top );
}

//////////////////////////////////////////////////////////////////////////

template<class Type>
inline CAARect<Type> operator*( typename CAARect<Type>::FloatingPointType mul, CAARect<Type> rect )
{
	return rect * mul;
}

template<class Type>
inline CAARect<Type> operator/( typename CAARect<Type>::FloatingPointType div, CAARect<Type> rect )
{
	return rect / div;
}

//////////////////////////////////////////////////////////////////////////

// Rectangle utility functions.

// Create an origin centered rectangle with the given size.
template <class T>
CAARect<T> CreateCenterRect( CVector2<T> size )
{
	const CVector2<T> halfSize = size / 2;
	return CAARect<T>( -halfSize, size.X(), size.Y() );
}

// Get the intersection of two rectangles.
template <class RectType>
RectType GetIntersection( RectType leftRect, RectType rightRect )
{
	const auto left = max( leftRect.Left(), rightRect.Left() );
	const auto right = min( leftRect.Right(), rightRect.Right() );
	if( left > right ) {
		return RectType();
	}

	const auto top = min( leftRect.Top(), rightRect.Top() );
	const auto bottom = max( leftRect.Bottom(), rightRect.Bottom() );
	if( top < bottom ) {
		return RectType();
	}

	return RectType( left, top, right, bottom );
}

// Get a union of two rectangles.
template <class RectType>
RectType GetRectUnion( RectType leftRect, RectType rightRect )
{
	if( leftRect.IsEmpty() ) {
		return rightRect;
	}
	if( rightRect.IsEmpty() ) {
		return leftRect;
	}

	const auto left = min( leftRect.Left(), rightRect.Left() );
	const auto right = max( leftRect.Right(), rightRect.Right() );
	const auto top = max( leftRect.Top(), rightRect.Top() );
	const auto bottom = min( leftRect.Bottom(), rightRect.Bottom() );

	return RectType( left, top, right, bottom );
}

// Get a union of a rectangle and a point.
template <class RectType, class PointType>
RectType GetPointUnion( RectType leftRect, PointType rightPoint )
{
	if( leftRect.IsEmpty() ) {
		return leftRect;
	}

	const auto left = min( leftRect.Left(), rightPoint.X() );
	const auto right = max( leftRect.Right(), rightPoint.X() );
	const auto top = max( leftRect.Top(), rightPoint.Y() );
	const auto bottom = min( leftRect.Bottom(), rightPoint.Y() );

	return RectType( left, top, right, bottom );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.