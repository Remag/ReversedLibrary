#pragma once
#include <Shape.h>

namespace Relib {

// A convex shape which is an arbitrary convex polygon.
template<class T>
class CPolygonShape : public CShape<T> {
public:
	typedef T Type;

	explicit CPolygonShape( CArrayView<CVector2<T>> vertices );
	explicit CPolygonShape( CArray<CVector2<T>>&& vertices );

	// CConvexShape.
	virtual THitboxShapeType GetType() const override final
		{ return HST_Polygon; }
	virtual CAARect<T> GetBoundRect( CMatrix3<T> transformation ) const;

	// Get polygon vertex winding order.
	bool IsClockwise() const
		{ return isClockwise; }
	CArrayView<CVector2<T>> BaseVertices() const
		{ return vertices; }
	
private:
	// Polygon vertices.
	CArray<CVector2<T>> vertices;
	// Polygon winding order.
	bool isClockwise;

	static bool findWindingOrder( CArrayView<CVector2<T>> vertices );
};

//////////////////////////////////////////////////////////////////////////

template<class T>
CPolygonShape<T>::CPolygonShape( CArrayView<CVector2<T>> _vertices ) :
	isClockwise( findWindingOrder( _vertices ) )
{
	vertices.CopyFrom( _vertices );
}

template<class T>
CPolygonShape<T>::CPolygonShape( CArray<CVector2<T>>&& _vertices ) :
	vertices( move( _vertices ) ),
	isClockwise( findWindingOrder( vertices ) )
{
}

template<class T>
bool CPolygonShape<T>::findWindingOrder( CArrayView<CVector2<T>> vertices )
{
	assert( vertices.Size() > 2 );
	for( int i = 2; i < vertices.Size(); i++ ) {
		const auto edge1 = vertices[i - 1] - vertices[i - 2];
		const auto edge2 = vertices[i] - vertices[i - 1];
		const auto crossProductZ = edge1.X() * edge2.Y() - edge2.X() * edge1.Y();
		if( crossProductZ > 0 ) {
			return false;
		} else if( crossProductZ < 0 ) {
			return true;
		}
	}
	// Trivial case: a line. Any winding order is correct.
	return false;
}

template<class T>
CAARect<T> CPolygonShape<T>::GetBoundRect( CMatrix3<T> transformation ) const
{
	assert( !vertices.IsEmpty() );

	const auto firstVertex = PointTransform( transformation, vertices[0] );
	CAARect<T> boundRect( firstVertex, firstVertex );
	for( int i = 1; i < vertices.Size(); i++ ) {
		const auto vertex = PointTransform( transformation, vertices[i] );
		boundRect.Left() = min( boundRect.Left(), vertex.X() );
		boundRect.Top() = min( boundRect.Top(), vertex.Y() );
		boundRect.Right() = max( boundRect.Right(), vertex.X() );
		boundRect.Bottom() = max( boundRect.Bottom(), vertex.Y() );
	}
	return boundRect;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.