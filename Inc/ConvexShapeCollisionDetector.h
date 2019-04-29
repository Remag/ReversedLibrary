#pragma once
 #include <Hitbox.h>
#include <RawBuffer.h>
#include <StackArray.h>
#include <BitSet.h>
#include <AngledRectShape.h>

namespace Relib {

// Detector for collisions between a pair of convex shapes.
// Convex shapes are checked using the separating axis theorem.
template<class FloatingPointType>
class CConvexShapeCollisionDetector {
public:
	typedef FloatingPointType Type;

	typedef CHitbox<Type> THitbox;
	typedef CInterval<Type> TInterval;
	typedef CVector2<Type> TVector2;
	typedef CAARect<Type> TAARect;
	typedef CMatrix3<Type> TMatrix3;

	// Check if two shapes intersect.
	bool DetectCollision( const THitbox& left, const THitbox& right ) const;

private:
	static const int externalBitsetOffset = sizeof( TAARect ) + sizeof( CVector2<int> ) + sizeof( TVector2 ) + sizeof( TVector2 ) + sizeof( float ) + sizeof( float );

	// Partially specialized functions.
	bool detectPointCollision( const THitbox& point, const THitbox& right ) const;
	bool detectAARectCollision( const THitbox& rect, const THitbox& right ) const;
	bool detectAngledRectCollision( const THitbox& rect, const THitbox& right ) const;
	bool detectPolygonCollision( const THitbox& polygon, const THitbox& right ) const;
	bool detectCircleCollision( const THitbox& circle, const THitbox& right ) const;
	bool detectComplexCollision( const THitbox& complexShape, const THitbox& right ) const;
	bool detectBitmapCollision( const THitbox& bitmap, const THitbox& right ) const;

	// Fully specialized functions.
	bool detectPointRectCollision( const THitbox& point, const THitbox& rect ) const;
	bool detectPointAngledRectCollision( const THitbox& point, const THitbox& rect ) const;
	bool detectPointPolygonCollision( const THitbox& point, const THitbox& polygon ) const;
	bool detectPointCircleCollision( const THitbox& point, const THitbox& circle ) const;
	bool detectPointBitmapCollision( const THitbox& point, const THitbox& bitmap ) const;
	bool detectPointComplexCollision( const THitbox& point, const THitbox& complexShape ) const;

	bool detectRectRectCollision( const THitbox& left, const THitbox& right ) const;
	bool detectRectAngledRectCollision( const THitbox& rect, const THitbox& angledRect ) const;
	bool detectRectAngledRectCollision( TAARect rect, CArrayView<TVector2> rectPoints ) const;
	bool detectRectPolygonCollision( const THitbox& rect, const THitbox& polygon ) const;
	bool detectRectPolygonCollision( TAARect rect, const TVector2* vertices, short vertexCount, short windingOrder ) const;
	bool detectRectCircleCollision( const THitbox& rect, const THitbox& circle ) const;
	bool detectRectCircleCollision( TAARect rect, TVector2 center, Type radius ) const;
	bool detectRectBitmapCollision( const THitbox& rect, const THitbox& bitmap ) const;
	bool detectRectBitmapCollision( TAARect rect, CVector2<int> rightCellCount, TVector2 rightCellSize, TMatrix3 rightTransform, const CDynamicBitSet<>& rightBitSet ) const;
	bool detectRectComplexCollision( const THitbox& rect, const THitbox& complexShape ) const;

	bool detectAngledRectAngledRectCollision( const THitbox& left, const THitbox& right ) const;
	bool detectAngledRectPolygonCollision( const THitbox& rect, const THitbox& polygon ) const;
	bool detectAngledRectCircleCollision( const THitbox& rect, const THitbox& circle ) const;
	bool detectAngledRectBitmapCollision( const THitbox& rect, const THitbox& bitmap ) const;
	bool detectAngledRectComplexCollision( const THitbox& rect, const THitbox& complexShape ) const;

	bool detectPolygonPolygonCollision( const THitbox& left, const THitbox& right ) const;
	bool detectPolygonCircleCollision( const THitbox& polygon, const THitbox& circle ) const;
	bool detectPolygonBitmapCollision( const THitbox& polygon, const THitbox& bitmap ) const;
	bool detectPolygonComplexCollision( const THitbox& polygon, const THitbox& complexShape ) const;

	bool detectCircleCircleCollision( const THitbox& left, const THitbox& right ) const;
	bool detectCircleBitmapCollision( const THitbox& circle, const THitbox& bitmap ) const;
	bool detectCircleComplexCollision( const THitbox& circle, const THitbox& complexShape ) const;

	bool detectBitmapBitmapCollision( const THitbox& left, const THitbox& right ) const;
	bool detectBitmapComplexCollision( const THitbox& bitmap, const THitbox& complexShape ) const;

	bool checkPointCircleDistance( TVector2 point, TVector2 circleCenter, Type radius ) const;
	TInterval getAngledRectSelfProjection( TVector2 angledEdge, CArrayView<TVector2> points ) const;
	TInterval getRectProjection( const TAARect& rect, TVector2 vec ) const;
	TInterval getAngledRectProjection( const CStackArray<TVector2, 4>& points, TVector2 vec ) const;
	TInterval getPolygonProjection( const TVector2* vertices, int vertexCount, TVector2 vec ) const;
	TInterval getCircleProjection( TVector2 center, Type radius, TVector2 vec ) const;
	TVector2 findClosestVertex( TVector2 circleCetner, const TVector2* vertices, int size ) const; 
	bool checkEdgeSeparation( TAARect rect, TVector2 edge, TVector2 baseVertex, short windingOrder ) const;
	bool checkEdgeSeparation( const CStackArray<TVector2, 4>& points, TVector2 edge, TVector2 baseVertex, short windingOrder ) const;
	bool checkEdgeSeparation( const TVector2* leftVertices, short leftVertexCount, TVector2 edge, TVector2 baseVertex, short windingOrder ) const;
	bool checkEdgeSeparation( TVector2 circleCenter, Type circleRadius, TVector2 edge, TVector2 baseVertex, short windingOrder ) const;
	bool checkAngledRectSeparation( const CStackArray<TVector2, 4>& edgeSrc, const CStackArray<TVector2, 4>& rectPoints ) const;

	TMatrix3 extractBitmapMatrix( const THitbox& bitmap ) const;
	bool findBitmapRectPointsCollision( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, CArrayView<TVector2> rectPoints ) const;
	TAARect getPointsBoundRect( CArrayView<TVector2> points ) const; 
	CAARect<int> findCellRect( TAARect rect, CVector2<int> cellCount, TVector2 cellSize ) const;

	bool findBitmapPolygonPointsCollision( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, CArrayView<TVector2> vertices, short windingOrder ) const;
	TAARect getPolygonBoundRect( CArrayView<TVector2> points ) const; 

	bool findBitmapCircleCollision( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, TVector2 center, Type size ) const;
	TAARect getCircleBoundRect( TVector2 center, Type size ) const; 

	TMatrix3 combineBitsetTransforms( TMatrix3 leftTransform, TMatrix3 rightTransform ) const;
	TMatrix3 invertBitsetTransform( TMatrix3 transform ) const;
	bool findBitmapBitmapCollision( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, TAARect rightBoundRect,
	CVector2<int> rightCellCount, TVector2 rightCellSize, TMatrix3 leftTransform, TMatrix3 rightTransform, const CDynamicBitSet<>& rightBitSet ) const;

	bool checkBitSectorPointsCollisions( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, CAARect<int> cellRect, CArrayView<TVector2> rectPoints ) const;
	bool checkBitSectorPolygonCollisions( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, CAARect<int> cellRect, CArrayView<TVector2> polygonPoints, short windingOrder ) const;
	bool checkBitSectorCircleCollisions( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, CAARect<int> cellRect, TVector2 center, Type radius ) const;
	bool checkBitSectorBitmapCollisions( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, CAARect<int> cellRect, TAARect rightBoundRect,
		CVector2<int> rightCellCount, TVector2 rightCellSize, TMatrix3 rightTransform, const CDynamicBitSet<>& rightBitSet ) const;

	CAARect<int> findFlaggedRect( int startX, int startY, int rowOffset, const CDynamicBitSet<>& bitset, CAARect<int> cellRect ) const;
	int findFlaggedRectEndY( int startX, int endX, int startY, int rowOffset, const CDynamicBitSet<>& bitset, int limitY ) const;
	TAARect findRealCellRect( CAARect<int> cellRect, TVector2 cellSize ) const;
};

//////////////////////////////////////////////////////////////////////////

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::DetectCollision( const THitbox& left, const THitbox& right ) const
{
	// Double dispatch is implemented with the easiest way: a huge switch statement.
	// Unfortunately, all the alternatives are either slower or require all shapes to know about each other.
	staticAssert( HST_EnumCount == 8 );
	switch( left.GetType() ) {
		case HST_Null:
			return false;
		case HST_Point:
			return detectPointCollision( left, right );
		case HST_AARect:
			return detectAARectCollision( left, right );
		case HST_AngledRect:
			return detectAngledRectCollision( left, right );
		case HST_Polygon:
			return detectPolygonCollision( left, right );
		case HST_Circle:
			return detectCircleCollision( left, right );
		case HST_Bitmap:
			return detectBitmapCollision( left, right );
		case HST_Complex:
			return detectComplexCollision( left, right );
		default:
			assert( false );
			return false;
	}
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectPointCollision( const THitbox& point, const THitbox& right ) const
{
	staticAssert( HST_EnumCount == 8 );
	assert( point.GetType() == HST_Point );
	switch( right.GetType() ) {
		case HST_Null:
		case HST_Point:
			return false;
		case HST_AARect:
			return detectPointRectCollision( point, right );
		case HST_AngledRect:
			return detectPointAngledRectCollision( point, right );
		case HST_Polygon:
			return detectPointPolygonCollision( point, right );
		case HST_Circle:
			return detectPointCircleCollision( point, right );
		case HST_Bitmap:
			return detectPointBitmapCollision( point, right );
		case HST_Complex:
			return detectPointComplexCollision( point, right );
		default:
			assert( false );
			return false;
	}
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectAARectCollision( const THitbox& rect, const THitbox& right ) const
{
	staticAssert( HST_EnumCount == 8 );
	assert( rect.GetType() == HST_AARect );
	switch( right.GetType() ) {
		case HST_Null:
			return false;
		case HST_Point:
			return detectPointRectCollision( right, rect );
		case HST_AARect:
			return detectRectRectCollision( rect, right );
		case HST_AngledRect:
			return detectRectAngledRectCollision( rect, right );
		case HST_Polygon:
			return detectRectPolygonCollision( rect, right );
		case HST_Circle:
			return detectRectCircleCollision( rect, right );
		case HST_Bitmap:
			return detectRectBitmapCollision( rect, right );
		case HST_Complex:
			return detectRectComplexCollision( rect, right );
		default:
			assert( false );
			return false;
	}
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectAngledRectCollision( const THitbox& rect, const THitbox& right ) const
{
	staticAssert( HST_EnumCount == 8 );
	assert( rect.GetType() == HST_AngledRect );
	switch( right.GetType() ) {
		case HST_Null:
			return false;
		case HST_Point:
			return detectPointAngledRectCollision( right, rect );
		case HST_AARect:
			return detectRectAngledRectCollision( right, rect );
		case HST_AngledRect:
			return detectAngledRectAngledRectCollision( rect, right );
		case HST_Polygon:
			return detectAngledRectPolygonCollision( rect, right );
		case HST_Circle:
			return detectAngledRectCircleCollision( rect, right );
		case HST_Bitmap:
			return detectAngledRectBitmapCollision( rect, right );
		case HST_Complex:
			return detectAngledRectComplexCollision( rect, right );
		default:
			assert( false );
			return false;
	}
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectPolygonCollision( const THitbox& polygon, const THitbox& right ) const
{
	staticAssert( HST_EnumCount == 8 );
	assert( polygon.GetType() == HST_Polygon );
	switch( right.GetType() ) {
		case HST_Null:
			return false;
		case HST_Point:
			return detectPointPolygonCollision( right, polygon );
		case HST_AARect:
			return detectRectPolygonCollision( right, polygon );
		case HST_AngledRect:
			return detectAngledRectPolygonCollision( right, polygon );
		case HST_Polygon:
			return detectPolygonPolygonCollision( polygon, right );
		case HST_Circle:
			return detectPolygonCircleCollision( polygon, right );
		case HST_Bitmap:
			return detectPolygonBitmapCollision( polygon, right );
		case HST_Complex:
			return detectPolygonComplexCollision( polygon, right );
		default:
			assert( false );
			return false;
	}
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectCircleCollision( const THitbox& circle, const THitbox& right ) const
{
	staticAssert( HST_EnumCount == 8 );
	assert( circle.GetType() == HST_Circle );
	switch( right.GetType() ) {
		case HST_Null:
			return false;
		case HST_Point:
			return detectPointCircleCollision( right, circle );
		case HST_AARect:
			return detectRectCircleCollision( right, circle );
		case HST_AngledRect:
			return detectAngledRectCircleCollision( right, circle );
		case HST_Polygon:
			return detectPolygonCircleCollision( right, circle );
		case HST_Circle:
			return detectCircleCircleCollision( circle, right );
		case HST_Bitmap:
			return detectCircleBitmapCollision( circle, right );
		case HST_Complex:
			return detectCircleComplexCollision( circle, right );
		default:
			assert( false );
			return false;
	}
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectBitmapCollision( const THitbox& bitmap, const THitbox& right ) const
{
	staticAssert( HST_EnumCount == 8 );
	assert( bitmap.GetType() == HST_Bitmap );
	switch( right.GetType() ) {
		case HST_Null:
			return false;
		case HST_Point:
			return detectPointBitmapCollision( right, bitmap );
		case HST_AARect:
			return detectRectBitmapCollision( right, bitmap );
		case HST_AngledRect:
			return detectAngledRectBitmapCollision( right, bitmap );
		case HST_Polygon:
			return detectPolygonBitmapCollision( right, bitmap );
		case HST_Circle:
			return detectCircleBitmapCollision( right, right );
		case HST_Bitmap:
			return detectBitmapBitmapCollision( bitmap, right );
		case HST_Complex:
			return detectBitmapComplexCollision( bitmap, right );
		default:
			assert( false );
			return false;
	}
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectComplexCollision( const THitbox& complexShape, const THitbox& right ) const
{
	assert( complexShape.GetType() == HST_Complex );
	const auto& hitboxData = complexShape.HitboxData();
	const int shapeCount = hitboxData.Get<int>( 0 );
	const int countOffset = 2 * sizeof( int );
	for( int i = 0; i < shapeCount; i++ ) {
		const auto& subShape = hitboxData.Get<THitbox>( countOffset + i * sizeof( THitbox ) );
		if( DetectCollision( subShape, right ) ) {
			return true;
		}
	}
	return false;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectPointRectCollision( const THitbox& point, const THitbox& rect ) const
{
	const TVector2 globalPoint = point.HitboxData().As<TVector2>();
	const TAARect& globalRect = rect.HitboxData().As<TAARect>();
	return globalRect.StrictHas( globalPoint );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectPointAngledRectCollision( const THitbox& point, const THitbox& rect ) const
{
	const auto& rectPoints = rect.HitboxData().As<CStackArray<TVector2, 4>>();
	const TVector2 globalPoint = point.HitboxData().As<TVector2>();

	const TVector2 edge1 = rectPoints[1] - rectPoints[0];
	const TVector2 edge2 = rectPoints[3] - rectPoints[0];

	const TVector2 pointVec = globalPoint - rectPoints[0];
	const Type dotProduct1 = pointVec * edge1;
	if( dotProduct1 < 0 || dotProduct1 > edge1 * edge1 ) {
		return false;
	}
	const Type dotProduct2 = pointVec * edge2;
	if( dotProduct2 < 0 || dotProduct2 > edge2 * edge2 ) {
		return false;
	}

	return true;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectPointPolygonCollision( const THitbox& point, const THitbox& polygon ) const
{
	const short vertexCount = polygon.HitboxData().Get<short>( 0 );
	const short windingOrder = polygon.HitboxData().Get<short>( sizeof( short ) );
	const TVector2* vertices = &polygon.HitboxData().Get<TVector2>( 2 * sizeof( short ) );
	const TVector2 globalPoint = point.HitboxData().As<TVector2>();

	const TVector2 firstVertex = vertices[0];
	TVector2 prevVertex = firstVertex;
	for( int i = 1; i < vertexCount; i++ ) {
		const TVector2 vertex = vertices[i];
		const TVector2 edge = vertex - prevVertex;
		const TVector2 pointVector = globalPoint - prevVertex;
		const Type crossProductZ = edge.X() * pointVector.Y() - pointVector.X() * edge.Y();
		if( windingOrder * crossProductZ < 0 ) {
			return false;
		}
		prevVertex = vertex;
	}
	const TVector2 lastEdge = firstVertex - prevVertex;
	const TVector2 pointVector = globalPoint - prevVertex;
	const Type crossProductZ = lastEdge.X() * pointVector.Y() - pointVector.X() * lastEdge.Y();
	if( windingOrder * crossProductZ < 0 ) {
		return false;
	}
	return true;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectPointCircleCollision( const THitbox& point, const THitbox& circle ) const
{
	const TVector2 globalPoint = point.HitboxData().As<TVector2>();
	const TVector2 circleCenter = circle.HitboxData().Get<TVector2>( 0 );
	const Type circleRadius = circle.HitboxData().Get<Type>( sizeof( circleCenter ) );

	return checkPointCircleDistance( globalPoint, circleCenter, circleRadius );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::checkPointCircleDistance( TVector2 point, TVector2 circleCenter, Type radius ) const
{
	return ( point - circleCenter ).SquareLength() <= radius * radius;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectPointComplexCollision( const THitbox& point, const THitbox& complexShape ) const
{
	assert( complexShape.GetType() == HST_Complex );
	const auto& hitboxData = complexShape.HitboxData();
	const int shapeCount = hitboxData.Get<int>( 0 );
	const int countOffset = 2 * sizeof( int );
	for( int i = 0; i < shapeCount; i++ ) {
		const auto& subShape = hitboxData.Get<THitbox>( countOffset + i * sizeof( THitbox ) );
		if( detectPointCollision( point, subShape ) ) {
			return true;
		}
	}
	return false;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectPointBitmapCollision( const THitbox& point, const THitbox& bitmap ) const
{
	const auto globalPoint = point.HitboxData().As<TVector2>();
	const auto& bitmapData = bitmap.HitboxData();
	const auto boundRect = bitmapData.Get<TAARect>( 0 );
	if( !boundRect.StrictHas( globalPoint ) ) {
		return false;
	}

	const auto cellCountOffset = sizeof( TAARect );
	const auto sizeOffset = sizeof( CVector2<int> ) + cellCountOffset;
	const auto cellCount = bitmapData.Get<CVector2<int>>( cellCountOffset );
	const auto cellSize = bitmapData.Get<TVector2>( sizeOffset );
	auto bitmapTransform = extractBitmapMatrix( bitmap );
	const auto& bitset = *bitmapData.Get<const CDynamicBitSet<>*>( externalBitsetOffset );

	const auto transformedPoint = PointTransform( bitmapTransform, globalPoint );
	const auto pointX = static_cast<int>( transformedPoint.X() );
	const auto pointY = static_cast<int>( transformedPoint.Y() );
	if( pointX < 0 || pointX >= cellCount.X() || pointY < 0 || pointY >= cellCount.Y() ) {
		return false;
	}

	return bitset.Has( pointY * cellCount.X() + pointX );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectRectRectCollision( const THitbox& left, const THitbox& right ) const
{
	const TAARect leftRect = left.HitboxData().As<TAARect>();
	const TAARect rightRect = right.HitboxData().As<TAARect>();

	return leftRect.StrictIntersects( rightRect );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectRectAngledRectCollision( const THitbox& left, const THitbox& right ) const
{
	const TAARect leftRect = left.HitboxData().As<TAARect>();
	const auto& rightPoints = right.HitboxData().As<CStackArray<TVector2, 4>>();
	return detectRectAngledRectCollision( leftRect, rightPoints );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectRectAngledRectCollision( TAARect leftRect, CArrayView<TVector2> rightPoints ) const
{
	// Projections on AA rect edges.
	const TInterval leftFirstSelfProjection( leftRect.Left(), leftRect.Right() );
	const TInterval rightFirstProjection = minmax( rightPoints[0].X(), rightPoints[1].X(), rightPoints[2].X(), rightPoints[3].X() );
	if( !leftFirstSelfProjection.StrictIntersects( rightFirstProjection ) ) {
		return false;
	}
	const TInterval leftSecondSelfProjection( leftRect.Bottom(), leftRect.Top() );
	const TInterval rightSecondProjection = minmax( rightPoints[0].Y(), rightPoints[1].Y(), rightPoints[2].Y(), rightPoints[3].Y() );
	if( !leftSecondSelfProjection.StrictIntersects( rightSecondProjection ) ) {
		return false;
	}

	// Projections on angled rect edges.
	const TVector2 rightFirstEdge = ( rightPoints[2] - rightPoints[1] ).Normalize();
	const TInterval rightFirstSelfProjection = getAngledRectSelfProjection( rightFirstEdge, rightPoints );
	const TInterval rectFirstProjection = getRectProjection( leftRect, rightFirstEdge );
	if( !rightFirstSelfProjection.StrictIntersects( rectFirstProjection ) ) {
		return false;
	}

	const TVector2 rightSecondEdge = ( rightPoints[1] - rightPoints[0] ).Normalize();
	const TInterval rightSecondSelfProjection = getAngledRectSelfProjection( rightSecondEdge, rightPoints );
	const TInterval rectSecondProjection = getRectProjection( leftRect, rightSecondEdge );
	return rightSecondSelfProjection.StrictIntersects( rectSecondProjection );
}

template<class FloatingPointType>
CInterval<FloatingPointType> CConvexShapeCollisionDetector<FloatingPointType>::getAngledRectSelfProjection( TVector2 angledEdge, CArrayView<TVector2> points ) const
{
	const Type right = points[2] * angledEdge;
	const Type left = points[0] * angledEdge;	
	return TInterval( left, right );
}

// Project the given rectangle onto vec. vec is assumed to be unit.
template<class FloatingPointType>
CInterval<FloatingPointType> CConvexShapeCollisionDetector<FloatingPointType>::getRectProjection( const TAARect& rect, TVector2 vec ) const
{
	if( vec.Y() >= 0 ) {
		if( vec.X() >= 0 ) {
			// I quadrant.
			return TInterval( rect.BottomLeft() * vec, rect.TopRight() * vec );
		} else {
			// II quadrant
			return TInterval( rect.BottomRight() * vec, rect.TopLeft() * vec );
		}
	} else {
		if( vec.X() < 0 ) {
			// III quadrant.
			return TInterval( rect.TopRight() * vec, rect.BottomLeft() * vec );
		} else {
			// IV quadrant.
			return TInterval( rect.TopLeft() * vec, rect.BottomRight() * vec );
		}
	}
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectRectPolygonCollision( const THitbox& rect, const THitbox& polygon ) const
{
	const TAARect globalRect = rect.HitboxData().As<TAARect>();
	const short vertexCount = polygon.HitboxData().Get<short>( 0 );
	assert( vertexCount > 0 );
	const short windingOrder = polygon.HitboxData().Get<short>( sizeof( short ) );
	const TVector2* vertices = &polygon.HitboxData().Get<TVector2>( 2 * sizeof( short ) );

	return detectRectPolygonCollision( globalRect, vertices, vertexCount, windingOrder );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectRectPolygonCollision( TAARect globalRect, const TVector2* vertices, short vertexCount, short windingOrder ) const
{
	// Projections on AA rect edges.
	const TInterval leftFirstSelfProjection( globalRect.Left(), globalRect.Right() );
	TInterval rightFirstProjection( vertices[0].X(), vertices[0].X() );
	for( int i = 1; i < vertexCount; i++ ) {
		rightFirstProjection.Add( vertices[i].X() );
	}
	if( !leftFirstSelfProjection.StrictIntersects( rightFirstProjection ) ) {
		return false;
	}
	const TInterval leftSecondSelfProjection( globalRect.Bottom(), globalRect.Top() );
	TInterval rightSecondProjection( vertices[0].Y(), vertices[0].Y() );
	for( int i = 1; i < vertexCount; i++ ) {
		rightSecondProjection.Add( vertices[i].Y() );
	}
	if( !leftSecondSelfProjection.StrictIntersects( rightSecondProjection ) ) {
		return false;
	}

	// Projections on polygon edges.
	for( int i = 0; i < vertexCount - 1; i++ ) {
		const TVector2 edgeDir = vertices[i + 1] - vertices[i];
		if( checkEdgeSeparation( globalRect, edgeDir, vertices[i], windingOrder ) ) {
			return false;
		}
	}
	return true;
}

template <class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::checkEdgeSeparation( TAARect rect, TVector2 edge, TVector2 baseVertex, short windingOrder ) const
{
	const TVector2 normEdge = edge.Normalize();
	const TVector2 edgeNormal( normEdge.Y(), -normEdge.X() );
	const TInterval rectProjection = getRectProjection( rect, edgeNormal );
	const Type polygonPoint = baseVertex * edgeNormal;
	return ( windingOrder < 0 && polygonPoint > rectProjection.GetUpper() ) || ( windingOrder > 0 && polygonPoint < rectProjection.GetLower() );
}

template<class FloatingPointType>
CInterval<FloatingPointType> CConvexShapeCollisionDetector<FloatingPointType>::getPolygonProjection( const TVector2* vertices, int vertexCount, TVector2 dir ) const
{
	assert( vertexCount > 0 );

	const TVector2 firstVertex = vertices[0];
	const Type firstPoint = firstVertex * dir;
	TInterval projection( firstPoint, firstPoint );
	for( int i = 1; i < vertexCount; i++ ) {
		const Type vertexProjection = vertices[i] * dir;
		projection.Add( vertexProjection );
	}
	return projection;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectRectCircleCollision( const THitbox& rect, const THitbox& circle ) const
{
	const TAARect globalRect = rect.HitboxData().As<TAARect>();
	const TVector2 circleCenter = circle.HitboxData().Get<TVector2>( 0 );
	const Type circleRadius = circle.HitboxData().Get<Type>( sizeof( circleCenter ) );
	return detectRectCircleCollision( globalRect, circleCenter, circleRadius );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectRectCircleCollision( TAARect globalRect, TVector2 circleCenter, Type circleRadius ) const
{
	// Center to the left of the rect.
	if( circleCenter.X() < globalRect.Left() ) {
		if( circleCenter.Y() > globalRect.Top() ) {
			return checkPointCircleDistance( globalRect.TopLeft(), circleCenter, circleRadius );
		}	
		if( circleCenter.Y() < globalRect.Bottom() ) {
			return checkPointCircleDistance( globalRect.BottomLeft(), circleCenter, circleRadius );
		}
		return circleCenter.X() + circleRadius >= globalRect.Left();
	}

	// Center to the right of the rect.
	if( circleCenter.X() > globalRect.Right() ) {
		if( circleCenter.Y() > globalRect.Top() ) {
			return checkPointCircleDistance( globalRect.TopRight(), circleCenter, circleRadius );
		}	
		if( circleCenter.Y() < globalRect.Bottom() ) {
			return checkPointCircleDistance( globalRect.BottomRight(), circleCenter, circleRadius );
		}
		return circleCenter.X() - circleRadius <= globalRect.Right();
	}

	// Center on top.
	if( circleCenter.Y() > globalRect.Top() ) {
		return circleCenter.Y() - circleRadius <= globalRect.Top();
	}	
	// Center at the bottom.
	if( circleCenter.Y() < globalRect.Bottom() ) {
		return circleCenter.Y() + circleRadius >= globalRect.Bottom();
	}
	// Center inside the rect.
	return true;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectRectBitmapCollision( const THitbox& rect, const THitbox& bitmap ) const
{
	const TAARect globalRect = rect.HitboxData().As<TAARect>();
	const auto& bitmapData = bitmap.HitboxData();
	const auto boundRect = bitmapData.Get<TAARect>( 0 );
	if( !boundRect.StrictIntersects( globalRect ) ) {
		return false;
	}

	const auto cellCountOffset = sizeof( TAARect );
	const auto sizeOffset = sizeof( CVector2<int> ) + cellCountOffset;
	const auto cellCount = bitmapData.Get<CVector2<int>>( cellCountOffset );
	const auto cellSize = bitmapData.Get<TVector2>( sizeOffset );
	const auto bitmapTransform = extractBitmapMatrix( bitmap );
	const auto& bitset = *bitmapData.Get<const CDynamicBitSet<>*>( externalBitsetOffset );

	return detectRectBitmapCollision( globalRect, cellCount, cellSize, bitmapTransform, bitset );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectRectBitmapCollision( TAARect globalRect, CVector2<int> cellCount,
	TVector2 cellSize, TMatrix3 bitmapTransform, const CDynamicBitSet<>& bitset ) const
{
	// Check for a simple case of a non-rotated bitmap.
	if( bitmapTransform( 0, 1 ) == 0 ) {
		SetScale( bitmapTransform, TVector2( bitmapTransform( 0, 0 ) * cellSize.X(), bitmapTransform( 1, 1 ) * cellSize.Y() ) );
		const auto transformedRect = AARectTransform( bitmapTransform, globalRect );
		const auto cellRect = findCellRect( transformedRect, cellCount, cellSize );
		for( int y = cellRect.Bottom(); y < cellRect.Top(); y++ ) {
			for( int x = cellRect.Left(); x < cellRect.Right(); x++ ) {
				if( bitset.Has( y * cellCount.X() + x ) ) {
					return true;
				}
			}
		}

		return false;
	}

	// The bitmap is rotated, treat the AA rect as an angled rect.
	CAngledRectShape<Type> angledRect( globalRect );
	CStackArray<TVector2, 4> rectPoints;
	angledRect.GetRectPoints( bitmapTransform, rectPoints );
	return findBitmapRectPointsCollision( bitset, cellCount, cellSize, rectPoints );
}

template<class FloatingPointType>
CAARect<int> CConvexShapeCollisionDetector<FloatingPointType>::findCellRect( TAARect rect, CVector2<int> cellCount, TVector2 cellSize ) const
{
	const auto cellLeft = max( static_cast<int>( rect.Left() / cellSize.X() ), 0 );
	const auto cellTop = min( Ceil( rect.Top() / cellSize.Y() ), cellCount.Y() );
	const auto cellRight = min( Ceil( rect.Right() / cellSize.X() ), cellCount.X() );
	const auto cellBottom = max( static_cast<int>( rect.Bottom() / cellSize.Y() ), 0 );
	return CAARect<int>( cellLeft, cellTop, cellRight, cellBottom );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectRectComplexCollision( const THitbox& rect, const THitbox& complexShape ) const
{
	assert( complexShape.GetType() == HST_Complex );
	const auto& hitboxData = complexShape.HitboxData();
	const int shapeCount = hitboxData.Get<int>( 0 );
	const int countOffset = 2 * sizeof( int );
	for( int i = 0; i < shapeCount; i++ ) {
		const auto& subShape = hitboxData.Get<THitbox>( countOffset + i * sizeof( THitbox ) );
		if( detectAARectCollision( rect, subShape ) ) {
			return true;
		}
	}
	return false;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectAngledRectAngledRectCollision( const THitbox& left, const THitbox& right ) const
{
	const auto& leftPoints = left.HitboxData().As<CStackArray<TVector2, 4>>();
	const auto& rightPoints = right.HitboxData().As<CStackArray<TVector2, 4>>();

	return !checkAngledRectSeparation( leftPoints, rightPoints ) && !checkAngledRectSeparation( rightPoints, leftPoints );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::checkAngledRectSeparation( const CStackArray<TVector2, 4>& edgeSrc, const CStackArray<TVector2, 4>& rectPoints ) const
{
	const TVector2 srcFirstEdge = ( edgeSrc[2] - edgeSrc[1] ).Normalize();
	const TInterval srcFirstSelfProjection = getAngledRectSelfProjection( srcFirstEdge, edgeSrc );
	const TInterval rectFirstProjection = getAngledRectProjection( rectPoints, srcFirstEdge );
	if( !srcFirstSelfProjection.StrictIntersects( rectFirstProjection ) ) {
		return true;
	}

	const TVector2 srcSecondEdge = ( edgeSrc[1] - edgeSrc[0] ).Normalize();
	const TInterval srcSecondSelfProjection = getAngledRectSelfProjection( srcSecondEdge, edgeSrc );
	const TInterval rectSecondProjection = getAngledRectProjection( rectPoints, srcSecondEdge );
	return !srcSecondSelfProjection.StrictIntersects( rectSecondProjection );
}

template<class FloatingPointType>
CInterval<FloatingPointType> CConvexShapeCollisionDetector<FloatingPointType>::getAngledRectProjection( const CStackArray<TVector2, 4>& points, TVector2 vec ) const
{
	const Type topLeftProj = points[0] * vec;
	const Type topRightProj = points[1] * vec;
	const Type bottomRightProj = points[2] * vec;
	const Type bottomLeftProj = points[3] * vec;

	return minmax( topLeftProj, topRightProj, bottomRightProj, bottomLeftProj );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectAngledRectPolygonCollision( const THitbox& rect, const THitbox& polygon ) const
{
	const auto& rectPoints = rect.HitboxData().As<CStackArray<TVector2, 4>>();
	const short vertexCount = polygon.HitboxData().Get<short>( 0 );
	assert( vertexCount > 0 );
	const short windingOrder = polygon.HitboxData().Get<short>( sizeof( short ) );
	const TVector2* vertices = &polygon.HitboxData().Get<TVector2>( 2 * sizeof( short ) );

	// Projections on angled rect edges.
	const TVector2 rectFirstEdge = ( rectPoints[2] - rectPoints[1] ).Normalize();
	const TInterval rectFirstSelfProjection = getAngledRectSelfProjection( rectFirstEdge, rectPoints );
	const TInterval polygonFirstProjection = getPolygonProjection( vertices, vertexCount, rectFirstEdge );
	if( !rectFirstSelfProjection.StrictIntersects( polygonFirstProjection ) ) {
		return false;
	}

	const TVector2 rectSecondEdge = ( rectPoints[1] - rectPoints[0] ).Normalize();
	const TInterval rectSecondSelfProjection = getAngledRectSelfProjection( rectSecondEdge, rectPoints );
	const TInterval polygonSecondProjection = getPolygonProjection( vertices, vertexCount, rectSecondEdge );
	if( !rectSecondSelfProjection.StrictIntersects( polygonSecondProjection ) ) {
		return false;
	}

	// Projections on polygon edges.
	for( int i = 0; i < vertexCount - 1; i++ ) {
		const TVector2 edgeDir = vertices[i + 1] - vertices[i];
		if( checkEdgeSeparation( rectPoints, edgeDir, vertices[i], windingOrder ) ) {
			return false;
		}
	}
	return true;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::checkEdgeSeparation( const CStackArray<TVector2, 4>& points, TVector2 edge, TVector2 baseVertex, short windingOrder ) const
{
	const TVector2 normEdge = edge.Normalize();
	const TVector2 edgeNormal( normEdge.Y(), -normEdge.X() );
	const TInterval rectProjection = getAngledRectProjection( points, edgeNormal );
	const Type polygonPoint = baseVertex * edgeNormal;
	return ( windingOrder < 0 && polygonPoint > rectProjection.GetUpper() ) || ( windingOrder > 0 && polygonPoint < rectProjection.GetLower() );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectAngledRectCircleCollision( const THitbox& rect, const THitbox& circle ) const
{
	const auto& rectPoints = rect.HitboxData().As<CStackArray<TVector2, 4>>();
	const TVector2 circleCenter = circle.HitboxData().Get<TVector2>( 0 );
	const Type circleRadius = circle.HitboxData().Get<Type>( sizeof( circleCenter ) );

	// Projections on angled rect edges.
	const TVector2 rectFirstEdge = ( rectPoints[2] - rectPoints[1] ).Normalize();
	const TInterval rectFirstSelfProjection = getAngledRectSelfProjection( rectFirstEdge, rectPoints );
	const TInterval circleFirstProjection = getCircleProjection( circleCenter, circleRadius, rectFirstEdge );
	if( !rectFirstSelfProjection.StrictIntersects( circleFirstProjection ) ) {
		return false;
	}

	const TVector2 rectSecondEdge = ( rectPoints[1] - rectPoints[0] ).Normalize();
	const TInterval rectSecondSelfProjection = getAngledRectSelfProjection( rectSecondEdge, rectPoints );
	const TInterval circleSecondProjection = getCircleProjection( circleCenter, circleRadius, rectSecondEdge );
	if( !rectSecondSelfProjection.StrictIntersects( circleSecondProjection ) ) {
		return false;
	}

	// Get projection on the circle separation edge.
	const TVector2 closestVertex = findClosestVertex( circleCenter, rectPoints.Ptr(), rectPoints.Size() );
	const TVector2 circleEdge = circleCenter - closestVertex;
	const TVector2 normal( circleEdge.Y(), -circleEdge.X() );
	const TInterval rectProjection = getAngledRectProjection( rectPoints, normal );
	const TInterval circleProjection = getCircleProjection( circleCenter, circleRadius, normal );
	return circleProjection.StrictIntersects( rectProjection );
}

template<class FloatingPointType>
CInterval<FloatingPointType> CConvexShapeCollisionDetector<FloatingPointType>::getCircleProjection( TVector2 center, Type radius, TVector2 vec ) const
{
	const Type projection = center * vec;
	return TInterval( projection - radius, projection + radius );
}

template<class FloatingPointType>
CVector2<FloatingPointType> CConvexShapeCollisionDetector<FloatingPointType>::findClosestVertex( TVector2 circleCetner, const TVector2* vertices, int size ) const
{
	assert( size > 0 );
	TVector2 result = vertices[0];
	Type resultDistance = ( result - circleCetner ).SquareLength();
	for( int i = 1; i < size; i++ ) {
		const Type newDistance = ( vertices[i] - circleCetner ).SquareLength();
		if( newDistance < resultDistance ) {
			resultDistance = newDistance;
			result = vertices[i];
		}
	}
	return result;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectAngledRectBitmapCollision( const THitbox& rect, const THitbox& bitmap ) const
{
	const auto& rectPoints = rect.HitboxData().As<CStackArray<TVector2, 4>>();
	const auto& bitmapData = bitmap.HitboxData();

	const auto angledBoundRect = getPointsBoundRect( rectPoints );
	const auto boundRect = bitmapData.Get<TAARect>( 0 );

	if( !boundRect.StrictIntersects( angledBoundRect ) ) {
		return false;
	}

	const auto cellCountOffset = sizeof( TAARect );
	const auto sizeOffset = sizeof( CVector2<int> ) + cellCountOffset;
	const auto cellCount = bitmapData.Get<CVector2<int>>( cellCountOffset );
	const auto cellSize = bitmapData.Get<TVector2>( sizeOffset );
	auto bitmapTransform = extractBitmapMatrix( bitmap );
	const auto& bitset = *bitmapData.Get<const CDynamicBitSet<>*>( externalBitsetOffset );

	const auto widthVec = rectPoints[1] - rectPoints[0];
	const auto heightVec = rectPoints[3] - rectPoints[0];
	const TVector2 widthOffset = VecTransform( bitmapTransform, widthVec );
	const TVector2 heightOffset = VecTransform( bitmapTransform, heightVec );

	CStackArray<TVector2, 4> transformedPoints;
	transformedPoints[0] = PointTransform( bitmapTransform, rectPoints[0] );
	transformedPoints[1] = transformedPoints[0] + widthOffset;
	transformedPoints[2] = transformedPoints[1] + heightOffset;
	transformedPoints[3] = transformedPoints[0] + heightOffset;

	return findBitmapRectPointsCollision( bitset, cellCount, cellSize, transformedPoints );
}

template<class FloatingPointType>
CAARect<FloatingPointType> CConvexShapeCollisionDetector<FloatingPointType>::getPointsBoundRect( CArrayView<TVector2> points ) const
{
	const CInterval<Type> minmaxX = minmax( points[0].X(), points[1].X(), points[2].X(), points[3].X() );
	const CInterval<Type> minmaxY = minmax( points[0].Y(), points[1].Y(), points[2].Y(), points[3].Y() );
	return TAARect( minmaxX.GetLower(), minmaxY.GetUpper(), minmaxX.GetUpper(), minmaxY.GetLower() );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::findBitmapRectPointsCollision( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, CArrayView<TVector2> rectPoints ) const
{
	const auto pointsRect = getPointsBoundRect( rectPoints );
	const auto cellRect = findCellRect( pointsRect, cellCount, cellSize );

	return checkBitSectorPointsCollisions( bitset, cellCount, cellSize, cellRect, rectPoints );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectAngledRectComplexCollision( const THitbox& rect, const THitbox& complexShape ) const
{
	assert( complexShape.GetType() == HST_Complex );
	const auto& hitboxData = complexShape.HitboxData();
	const int shapeCount = hitboxData.Get<int>( 0 );
	const int countOffset = 2 * sizeof( int );
	for( int i = 0; i < shapeCount; i++ ) {
		const auto& subShape = hitboxData.Get<THitbox>( countOffset + i * sizeof( THitbox ) );
		if( detectAngledRectCollision( rect, subShape ) ) {
			return true;
		}
	}
	return false;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectPolygonPolygonCollision( const THitbox& left, const THitbox& right ) const
{
	const short leftVertexCount = left.HitboxData().Get<short>( 0 );
	const short leftWindingOrder = left.HitboxData().Get<short>( sizeof( short ) );
	assert( leftVertexCount > 0 );
	const TVector2* leftVertices = &left.HitboxData().Get<TVector2>( 2 * sizeof( short ) );

	const short rightVertexCount = right.HitboxData().Get<short>( 0 );
	const short rightWindingOrder = right.HitboxData().Get<short>( sizeof( short ) );
	assert( rightVertexCount > 0 );
	const TVector2* rightVertices = &right.HitboxData().Get<TVector2>( 2 * sizeof( short ) );

	// Projections on left edges.
	for( int i = 0; i < leftVertexCount - 1; i++ ) {
		const TVector2 edgeDir = leftVertices[i + 1] - leftVertices[i];
		if( checkEdgeSeparation( rightVertices, rightVertexCount, edgeDir, leftVertices[i], leftWindingOrder ) ) {
			return false;
		}
	}

	// Projections on right edges.
	for( int i = 0; i < rightVertexCount - 1; i++ ) {
		const TVector2 edgeDir = rightVertices[i + 1] - rightVertices[i];
		if( checkEdgeSeparation( leftVertices, leftVertexCount, edgeDir, rightVertices[i], rightWindingOrder ) ) {
			return false;
		}
	}
	return true;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::checkEdgeSeparation( const TVector2* leftVertices, short leftVertexCount, 
	TVector2 edge, TVector2 baseVertex, short windingOrder ) const
{
	const TVector2 normEdge = edge.Normalize();
	const TVector2 edgeNormal( normEdge.Y(), -normEdge.X() );
	const TInterval polygonProjection = getPolygonProjection( leftVertices, leftVertexCount, edgeNormal );

	const Type polygonPoint = baseVertex * edgeNormal;
	return ( windingOrder < 0 && polygonPoint > polygonProjection.GetUpper() ) || ( windingOrder > 0 && polygonPoint < polygonProjection.GetLower() );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectPolygonCircleCollision( const THitbox& polygon, const THitbox& circle ) const
{
	const short vertexCount = polygon.HitboxData().Get<short>( 0 );
	assert( vertexCount > 0 );
	const short windingOrder = polygon.HitboxData().Get<short>( sizeof( short ) );
	const TVector2* vertices = &polygon.HitboxData().Get<TVector2>( 2 * sizeof( short ) );
	const TVector2 circleCenter = circle.HitboxData().Get<TVector2>( 0 );
	const Type circleRadius = circle.HitboxData().Get<Type>( sizeof( circleCenter ) );

	// Projections on polygon edges.
	for( int i = 0; i < vertexCount - 1; i++ ) {
		const TVector2 edgeDir = vertices[i + 1] - vertices[i];
		if( checkEdgeSeparation( circleCenter, circleRadius, edgeDir, vertices[i], windingOrder ) ) {
			return false;
		}
	}

	// Get projection on the circle separation edge.
	const TVector2 closestVertex = findClosestVertex( circleCenter, vertices, vertexCount );
	const TVector2 circleEdge = circleCenter - closestVertex;
	const TVector2 normal( circleEdge.Y(), -circleEdge.X() );
	const TInterval rectProjection = getPolygonProjection( vertices, vertexCount, normal );
	const TInterval circleProjection = getCircleProjection( circleCenter, circleRadius, normal );
	return circleProjection.StrictIntersects( rectProjection );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectPolygonBitmapCollision( const THitbox& polygon, const THitbox& bitmap ) const
{
	const auto& polygonData = polygon.HitboxData();
	const short vertexCount = polygonData.Get<short>( 0 );
	assert( vertexCount > 0 );
	const short windingOrder = polygonData.Get<short>( sizeof( short ) );
	const TVector2* vertices = &polygonData.Get<TVector2>( 2 * sizeof( short ) );

	const auto& bitmapData = bitmap.HitboxData();

	const auto polygonBoundRect = getPolygonBoundRect( CArrayView<TVector2>( vertices, vertexCount ) );
	const auto boundRect = bitmapData.Get<TAARect>( 0 );

	if( !boundRect.StrictIntersects( polygonBoundRect ) ) {
		return false;
	}

	const auto cellCountOffset = sizeof( TAARect );
	const auto sizeOffset = sizeof( CVector2<int> ) + cellCountOffset;
	const auto cellCount = bitmapData.Get<CVector2<int>>( cellCountOffset );
	const auto cellSize = bitmapData.Get<TVector2>( sizeOffset );
	auto bitmapTransform = extractBitmapMatrix( bitmap );

	const auto& bitset = *bitmapData.Get<const CDynamicBitSet<>*>( externalBitsetOffset );

	// TODO: arena memory allocation.
	CFlexibleArray<TVector2, 16> transformedPoints;
	transformedPoints.IncreaseSize( vertexCount );
	for( int i = 0; i < vertexCount; i++ ) {
		transformedPoints[i] = PointTransform( bitmapTransform, vertices[i] );
	}

	return findBitmapPolygonPointsCollision( bitset, cellCount, cellSize, transformedPoints, windingOrder );
}

template<class FloatingPointType>
CAARect<FloatingPointType> CConvexShapeCollisionDetector<FloatingPointType>::getPolygonBoundRect( CArrayView<TVector2> points ) const
{
	const TVector2 firstVertex = points[0];
	TAARect boundRect( firstVertex, firstVertex );
	const auto pointCount = points.Size();
	for( int i = 1; i < pointCount; i++ ) {
		const TVector2 vertex = points[i];
		boundRect.Left() = min( boundRect.Left(), vertex.X() );
		boundRect.Top() = min( boundRect.Top(), vertex.Y() );
		boundRect.Right() = max( boundRect.Right(), vertex.X() );
		boundRect.Bottom() = max( boundRect.Bottom(), vertex.Y() );
	}
	return boundRect;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::findBitmapPolygonPointsCollision( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, 
	TVector2 cellSize, CArrayView<TVector2> vertices, short windingOrder ) const
{
	const auto vertexRect = getPolygonBoundRect( vertices );
	const auto cellRect = findCellRect( vertexRect, cellCount, cellSize );
	return checkBitSectorPolygonCollisions( bitset, cellCount, cellSize, cellRect, vertices, windingOrder );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::checkEdgeSeparation( TVector2 circleCenter, Type circleRadius, TVector2 edge, TVector2 baseVertex, short windingOrder ) const
{
	const TVector2 normEdge = edge.Normalize();
	const TVector2 edgeNormal( normEdge.Y(), -normEdge.X() );
	const TInterval circleProjection = getCircleProjection( circleCenter, circleRadius, edgeNormal );
	
	const Type polygonPoint = baseVertex * edgeNormal;
	return ( windingOrder < 0 && polygonPoint > circleProjection.GetUpper() ) || ( windingOrder > 0 && polygonPoint < circleProjection.GetLower() );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectPolygonComplexCollision( const THitbox& polygon, const THitbox& complexShape ) const
{
	assert( complexShape.GetType() == HST_Complex );
	const auto& hitboxData = complexShape.HitboxData();
	const int shapeCount = hitboxData.Get<int>( 0 );
	const int countOffset = 2 * sizeof( int );
	for( int i = 0; i < shapeCount; i++ ) {
		const auto& subShape = hitboxData.Get<THitbox>( countOffset + i * sizeof( THitbox ) );
		if( detectPolygonCollision( polygon, subShape ) ) {
			return true;
		}
	}
	return false;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectCircleCircleCollision( const THitbox& left, const THitbox& right ) const
{
	const TVector2 leftCenter = left.HitboxData().Get<TVector2>( 0 );
	const Type leftRadius = left.HitboxData().Get<Type>( sizeof( leftCenter ) );
	const TVector2 rightCenter = right.HitboxData().Get<TVector2>( 0 );
	const Type rightRadius = right.HitboxData().Get<Type>( sizeof( rightCenter ) );

	return checkPointCircleDistance( leftCenter, rightCenter, leftRadius + rightRadius );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectCircleBitmapCollision( const THitbox& circle, const THitbox& bitmap ) const
{
	const TVector2 center = circle.HitboxData().Get<TVector2>( 0 );
	const auto radius = circle.HitboxData().Get<Type>( sizeof( center ) );

	const auto& bitmapData = bitmap.HitboxData();
	const auto circleBoundRect = getCircleBoundRect( center, radius );
	const auto boundRect = bitmapData.Get<TAARect>( 0 );

	if( !boundRect.StrictIntersects( circleBoundRect ) ) {
		return false;
	}

	const auto cellCountOffset = sizeof( TAARect );
	const auto sizeOffset = sizeof( CVector2<int> ) + cellCountOffset;
	const auto cellCount = bitmapData.Get<CVector2<int>>( cellCountOffset );
	const auto cellSize = bitmapData.Get<TVector2>( sizeOffset );
	auto bitmapTransform = extractBitmapMatrix( bitmap );

	const auto& bitset = *bitmapData.Get<const CDynamicBitSet<>*>( externalBitsetOffset );

	const auto newCenter = PointTransform( bitmapTransform, center );
	return findBitmapCircleCollision( bitset, cellCount, cellSize, newCenter, radius );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::findBitmapCircleCollision( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, TVector2 center, Type radius ) const
{
	const auto circleRect = getCircleBoundRect( center, radius );
	const auto cellRect = findCellRect( circleRect, cellCount, cellSize );
	return checkBitSectorCircleCollisions( bitset, cellCount, cellSize, cellRect, center, radius );
}

template<class FloatingPointType>
CAARect<FloatingPointType> CConvexShapeCollisionDetector<FloatingPointType>::getCircleBoundRect( TVector2 center, Type radius ) const
{
	return TAARect{ center.X() - radius, center.Y() + radius, center.X() + radius, center.Y() - radius };
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectCircleComplexCollision( const THitbox& circle, const THitbox& complexShape ) const
{
	assert( complexShape.GetType() == HST_Complex );
	const auto& hitboxData = complexShape.HitboxData();
	const int shapeCount = hitboxData.Get<int>( 0 );
	const int countOffset = 2 * sizeof( int );
	for( int i = 0; i < shapeCount; i++ ) {
		const auto& subShape = hitboxData.Get<THitbox>( countOffset + i * sizeof( THitbox ) );
		if( detectCircleCollision( circle, subShape ) ) {
			return true;
		}
	}
	return false;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectBitmapBitmapCollision( const THitbox& left, const THitbox& right ) const
{
	const auto& leftData = left.HitboxData();
	const auto& rightData = right.HitboxData();

	const auto leftBoundRect = leftData.Get<TAARect>( 0 );
	const auto rightBoundRect = rightData.Get<TAARect>( 0 );

	if( !leftBoundRect.StrictIntersects( rightBoundRect ) ) {
		return false;
	}

	const auto cellCountOffset = sizeof( TAARect );
	const auto sizeOffset = sizeof( CVector2<int> ) + cellCountOffset;

	const auto leftCellCount = leftData.Get<CVector2<int>>( cellCountOffset );
	const auto leftCellSize = leftData.Get<TVector2>( sizeOffset );
	const auto leftTransform = extractBitmapMatrix( left );
	const auto& leftBitset = *leftData.Get<const CDynamicBitSet<>*>( externalBitsetOffset );

	const auto rightCellCount = rightData.Get<CVector2<int>>( cellCountOffset );
	const auto rightCellSize = rightData.Get<TVector2>( sizeOffset );
	const auto rightTransform = extractBitmapMatrix( right );
	const auto& rightBitset = *rightData.Get<const CDynamicBitSet<>*>( externalBitsetOffset );

	return findBitmapBitmapCollision( leftBitset, leftCellCount, leftCellSize, rightBoundRect, rightCellCount, rightCellSize, leftTransform, rightTransform, rightBitset );
}

template<class FloatingPointType>
CMatrix3<FloatingPointType> CConvexShapeCollisionDetector<FloatingPointType>::combineBitsetTransforms( TMatrix3 leftTransform, TMatrix3 rightTransform ) const
{
	const auto invertedRight = invertBitsetTransform( rightTransform );
	const auto combinedTransform = leftTransform * invertedRight;
	return invertBitsetTransform( combinedTransform );
}

template<class FloatingPointType>
CMatrix3<FloatingPointType> CConvexShapeCollisionDetector<FloatingPointType>::invertBitsetTransform( TMatrix3 transform ) const
{
	TMatrix3 result = TMatrix3::CreateRawMatrix();

	result( 0, 0 ) = transform( 0, 0 );
	result( 0, 1 ) = -transform( 0, 1 );
	result( 0, 2 ) = -transform( 0, 2 );
	result( 1, 0 ) = -transform( 1, 0 );
	result( 1, 1 ) = transform( 1, 1 );
	result( 1, 2 ) = -transform( 1, 2 );
	result( 2, 0 ) = Type( 0 );
	result( 2, 1 ) = Type( 0 );
	result( 2, 2 ) = Type( 1 );

	return result;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::findBitmapBitmapCollision( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, TAARect rightBoundRect,
	CVector2<int> rightCellCount, TVector2 rightCellSize, TMatrix3 leftTransform, TMatrix3 rightTransform, const CDynamicBitSet<>& rightBitSet ) const
{
	CAngledRectShape<Type> angledRect( rightBoundRect );
	const auto boundRect = angledRect.GetBoundRect( leftTransform );

	const auto cellRect = findCellRect( boundRect, cellCount, cellSize );
	const auto totalRightTransform = combineBitsetTransforms( leftTransform, rightTransform );
	return checkBitSectorBitmapCollisions( bitset, cellCount, cellSize, cellRect, rightBoundRect, rightCellCount, rightCellSize, totalRightTransform, rightBitSet );
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::detectBitmapComplexCollision( const THitbox& bitmap, const THitbox& complexShape ) const
{
	assert( complexShape.GetType() == HST_Complex );
	const auto& hitboxData = complexShape.HitboxData();
	const int shapeCount = hitboxData.Get<int>( 0 );
	const int countOffset = 2 * sizeof( int );
	for( int i = 0; i < shapeCount; i++ ) {
		const auto& subShape = hitboxData.Get<THitbox>( countOffset + i * sizeof( THitbox ) );
		if( detectBitmapCollision( bitmap, subShape ) ) {
			return true;
		}
	}
	return false;
}

template<class FloatingPointType>
CMatrix3<FloatingPointType> CConvexShapeCollisionDetector<FloatingPointType>::extractBitmapMatrix( const THitbox& bitmap ) const
{
	TMatrix3 result = TMatrix3::CreateRawMatrix();

	const auto originOffset = sizeof( TAARect ) + sizeof( CVector2<int> ) + sizeof( TVector2 );
	const auto sinOffset = sizeof( TVector2 ) + originOffset;
	const auto cosOffset = sizeof( float ) + sinOffset;
	const auto& bitmapData = bitmap.HitboxData();
	const auto origin = bitmapData.Get<TVector2>( originOffset );
	const auto rotSin = bitmapData.Get<float>( sinOffset );
	const auto rotCos = bitmapData.Get<float>( cosOffset );

	result( 0, 0 ) = rotCos;
	result( 1, 0 ) = -rotSin;
	result( 2, 0 ) = origin.X();
	result( 0, 1 ) = rotSin;
	result( 1, 1 ) = rotCos;
	result( 2, 1 ) = origin.Y();
	result( 0, 2 ) = 0;
	result( 1, 2 ) = 0;
	result( 2, 2 ) = 1;
	return result; 
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::checkBitSectorPointsCollisions( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, CAARect<int> cellRect,
	CArrayView<TVector2> rectPoints ) const
{
	if( cellRect.IsEmpty() ) {
		return false;
	}

	for( int y = cellRect.Bottom(); y < cellRect.Top(); ) {
		const int rowOffset = cellCount.X() * y;
		for( int x = cellRect.Left(); x < cellRect.Right(); x++ ) {
			if( bitset.Has( rowOffset + x ) ) {
				const auto flaggedCellRect = findFlaggedRect( x, y, rowOffset, bitset, cellRect );
				const auto realCellRect = findRealCellRect( flaggedCellRect, cellSize );
				const CAARect<int> leftRect( cellRect.Left(), flaggedCellRect.Top(), x, y );
				const CAARect<int> rightRect( flaggedCellRect.Right(), flaggedCellRect.Top(), cellRect.Right(), y );
				if( detectRectAngledRectCollision( realCellRect, rectPoints )
					|| checkBitSectorPointsCollisions( bitset, cellCount, cellSize, leftRect, rectPoints )
					|| checkBitSectorPointsCollisions( bitset, cellCount, cellSize, rightRect, rectPoints ) ) 
				{
					return true;
				}
				y = flaggedCellRect.Top();
				break;
			}
		}
		y++;
	}

	return false;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::checkBitSectorPolygonCollisions( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, CAARect<int> cellRect,
	CArrayView<TVector2> polygonPoints, short windingOrder ) const
{
	const auto pointsPtr = polygonPoints.Ptr();
	const short vertexCount = numeric_cast<short>( polygonPoints.Size() );
	if( cellRect.IsEmpty() ) {
		return false;
	}

	for( int y = cellRect.Bottom(); y < cellRect.Top(); ) {
		const int rowOffset = cellCount.X() * y;
		for( int x = cellRect.Left(); x < cellRect.Right(); x++ ) {
			if( bitset.Has( rowOffset + x ) ) {
				const auto flaggedCellRect = findFlaggedRect( x, y, rowOffset, bitset, cellRect );
				const auto realCellRect = findRealCellRect( flaggedCellRect, cellSize );
				const CAARect<int> leftRect( cellRect.Left(), flaggedCellRect.Top(), x, y );
				const CAARect<int> rightRect( flaggedCellRect.Right(), flaggedCellRect.Top(), cellRect.Right(), y );
				if( detectRectPolygonCollision( realCellRect, pointsPtr, vertexCount, windingOrder )
					|| checkBitSectorPolygonCollisions( bitset, cellCount, cellSize, leftRect, polygonPoints, windingOrder )
					|| checkBitSectorPolygonCollisions( bitset, cellCount, cellSize, rightRect, polygonPoints, windingOrder ) ) 
				{
					return true;
				}
				y = flaggedCellRect.Top();
				break;
			}
		}
		y++;
	}

	return false;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::checkBitSectorCircleCollisions( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, 
	CAARect<int> cellRect, TVector2 center, Type radius ) const
{
	if( cellRect.IsEmpty() ) {
		return false;
	}

	for( int y = cellRect.Bottom(); y < cellRect.Top(); ) {
		const int rowOffset = cellCount.X() * y;
		for( int x = cellRect.Left(); x < cellRect.Right(); x++ ) {
			if( bitset.Has( rowOffset + x ) ) {
				const auto flaggedCellRect = findFlaggedRect( x, y, rowOffset, bitset, cellRect );
				const auto realCellRect = findRealCellRect( flaggedCellRect, cellSize );
				const CAARect<int> leftRect( cellRect.Left(), flaggedCellRect.Top(), x, y );
				const CAARect<int> rightRect( flaggedCellRect.Right(), flaggedCellRect.Top(), cellRect.Right(), y );
				if( detectRectCircleCollision( realCellRect, center, radius )
					|| checkBitSectorCircleCollisions( bitset, cellCount, cellSize, leftRect, center, radius )
					|| checkBitSectorCircleCollisions( bitset, cellCount, cellSize, rightRect, center, radius ) ) 
				{
					return true;
				}
				y = flaggedCellRect.Top();
				break;
			}
		}
		y++;
	}

	return false;
}

template<class FloatingPointType>
bool CConvexShapeCollisionDetector<FloatingPointType>::checkBitSectorBitmapCollisions( const CDynamicBitSet<>& bitset, CVector2<int> cellCount, TVector2 cellSize, 
	CAARect<int> cellRect, TAARect rightBoundRect, CVector2<int> rightCellCount, TVector2 rightCellSize, TMatrix3 rightTransform, const CDynamicBitSet<>& rightBitSet ) const
{
	if( cellRect.IsEmpty() ) {
		return false;
	}

	for( int y = cellRect.Bottom(); y < cellRect.Top(); ) {
		const int rowOffset = cellCount.X() * y;
		for( int x = cellRect.Left(); x < cellRect.Right(); x++ ) {
			if( bitset.Has( rowOffset + x ) ) {
				const auto flaggedCellRect = findFlaggedRect( x, y, rowOffset, bitset, cellRect );
				const auto realCellRect = findRealCellRect( flaggedCellRect, cellSize );
				const CAARect<int> leftRect( cellRect.Left(), flaggedCellRect.Top(), x, y );
				const CAARect<int> rightRect( flaggedCellRect.Right(), flaggedCellRect.Top(), cellRect.Right(), y );
				if( detectRectBitmapCollision( realCellRect, rightCellCount, rightCellSize, rightTransform, rightBitSet )
					|| checkBitSectorBitmapCollisions( bitset, cellCount, cellSize, leftRect, rightBoundRect, rightCellCount, rightCellSize, rightTransform, rightBitSet )
					|| checkBitSectorBitmapCollisions( bitset, cellCount, cellSize, rightRect, rightBoundRect, rightCellCount, rightCellSize, rightTransform, rightBitSet ) ) 
				{
					return true;
				}
				y = flaggedCellRect.Top();
				break;
			}
		}
		y++;
	}

	return false;
}

template<class FloatingPointType>
CAARect<int> CConvexShapeCollisionDetector<FloatingPointType>::findFlaggedRect( int startX, int startY, int rowOffset, const CDynamicBitSet<>& bitset, CAARect<int> cellRect ) const
{
	int endX = startX + 1;
	for( ; endX < cellRect.Right(); endX++ ) {
		if( !bitset.Has( rowOffset + endX ) ) {
			break;
		}
	}

	const auto endY = findFlaggedRectEndY( startX, endX, startY, rowOffset, bitset, cellRect.Top() );
	return CAARect<int>{ startX, endY, endX, startY };
}

template<class FloatingPointType>
int CConvexShapeCollisionDetector<FloatingPointType>::findFlaggedRectEndY( int startX, int endX, int startY, int rowOffset, const CDynamicBitSet<>& bitset, int limitY ) const
{
	for( int endY = startY + 1 ; endY < limitY; endY++ ) {
		for( int x = startX; x < endX; x++ ) {
			if( !bitset.Has( rowOffset + x ) ) {
				return endY;
			}
		}
	}

	return limitY;
}

template<class FloatingPointType>
CAARect<FloatingPointType> CConvexShapeCollisionDetector<FloatingPointType>::findRealCellRect( CAARect<int> cellRect, TVector2 cellSize ) const
{
	const auto left = cellRect.Left() * cellSize.X();
	const auto top = cellRect.Top() * cellSize.Y();
	const auto right = cellRect.Right() * cellSize.X();
	const auto bottom = cellRect.Bottom() * cellSize.Y();

	return TAARect{ left, top, right, bottom };
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.