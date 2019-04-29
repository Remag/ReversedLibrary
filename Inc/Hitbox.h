#pragma once
#include <Shape.h>
#include <PointShape.h>
#include <AARectShape.h>
#include <AngledRectShape.h>
#include <PolygonShape.h>
#include <CircleShape.h>
#include <BitmapShape.h>
#include <ComplexShape.h>
#include <RawBuffer.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A shape with a concrete transformation applied. 
// Necessary data is allocated on the given stack allocator and is assumed to be forgotten afterwards.
template <class FloatType>
class CHitbox {
public:
	CHitbox( CRawBuffer buffer, THitboxShapeType type ) : hitboxData( buffer ), hitboxType( type ) {}
	template <class Arena>
	CHitbox( const CShape<FloatType>& shape, Arena& arena );
	template <class Arena>
	CHitbox( const CShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena );

	THitboxShapeType GetType() const
		{ return hitboxType; }
	const CRawBuffer& HitboxData() const
		{ return hitboxData; }

	void OffsetPosition( CVector2<FloatType> delta );

private:
	// Arbitrary data associated with the hitbox. Depends on hitboxType.
	CRawBuffer hitboxData;
	THitboxShapeType hitboxType;

	void initNullData();
	template <class Arena>
	void initPointData( const CPointShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena );
	template <class Arena>
	void initAARectData( const CAARectShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena );
	template <class Arena>
	void initAngledRectData( const CAngledRectShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena );
	template <class Arena>
	void initPolygonData( const CPolygonShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena );
	template <class Arena>
	void initCircleData( const CCircleShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena );
	template <class Arena>
	void initBitmapData( const CBitmapShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena );
	template <class Arena>
	void initComplexData( const CComplexShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena );

	template <class Arena>
	void initPointData( const CPointShape<FloatType>& shape, Arena& arena );
	template <class Arena>
	void initAARectData( const CAARectShape<FloatType>& shape, Arena& arena );
	template <class Arena>
	void initAngledRectData( const CAngledRectShape<FloatType>& shape, Arena& arena );
	template <class Arena>
	void initPolygonData( const CPolygonShape<FloatType>& shape, Arena& arena );
	template <class Arena>
	void initCircleData( const CCircleShape<FloatType>& shape, Arena& arena );
	template <class Arena>
	void initBitmapData( const CBitmapShape<FloatType>& shape, Arena& arena );
	template <class Arena>
	void initComplexData( const CComplexShape<FloatType>& shape, Arena& arena );

	void offsetPointData( CVector2<FloatType> offset );
	void offsetAARectData( CVector2<FloatType> offset );
	void offsetAngledRectData( CVector2<FloatType> offset );
	void offsetPolygonData( CVector2<FloatType> offset );
	void offsetCircleData( CVector2<FloatType> offset );
	void offsetBitmapData( CVector2<FloatType> offset );
	void offsetComplexData( CVector2<FloatType> offset );
};

//////////////////////////////////////////////////////////////////////////

template <class FloatType>
template <class Arena>
CHitbox<FloatType>::CHitbox( const CShape<FloatType>& shape, Arena& arena ) :
	hitboxType( shape.GetType() )
{
	staticAssert( HST_EnumCount == 8 );
	switch( hitboxType ) {
		case HST_Null:
			initNullData();
			break;
		case HST_Point:
			initPointData( static_cast< const CPointShape<FloatType>& >( shape ), arena );
			break;
		case HST_AARect:
			initAARectData( static_cast< const CAARectShape<FloatType>& >( shape ), arena );
			break;
		case HST_AngledRect:
			initAngledRectData( static_cast< const CAngledRectShape<FloatType>& >( shape ), arena );
			break;
		case HST_Polygon:
			initPolygonData( static_cast< const CPolygonShape<FloatType>& >( shape ), arena );
			break;
		case HST_Circle:
			initCircleData( static_cast< const CCircleShape<FloatType>& >( shape ), arena );
			break;
		case HST_Bitmap:
			initBitmapData( static_cast< const CBitmapShape<FloatType>& >( shape ), arena );
			break;
		case HST_Complex:
			initComplexData( static_cast< const CComplexShape<FloatType>& >( shape ), arena );
			break;
		default:
			assert( false );
			return;
	}
}

template <class FloatType>
template <class Arena>
CHitbox<FloatType>::CHitbox( const CShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena ) :
	hitboxType( shape.GetType() )
{
	staticAssert( HST_EnumCount == 8 );
	switch( hitboxType ) {
		case HST_Null:
			initNullData();
			break;
		case HST_Point:
			initPointData( static_cast<const CPointShape<FloatType>&>( shape ), transform, arena );
			break;
		case HST_AARect:
			initAARectData( static_cast<const CAARectShape<FloatType>&>( shape ), transform, arena );
			break;
		case HST_AngledRect:
			initAngledRectData( static_cast<const CAngledRectShape<FloatType>&>( shape ), transform, arena );
			break;
		case HST_Polygon:
			initPolygonData( static_cast<const CPolygonShape<FloatType>&>( shape ), transform, arena );
			break;
		case HST_Circle:
			initCircleData( static_cast<const CCircleShape<FloatType>&>( shape ), transform, arena );
			break;
		case HST_Bitmap:
			initBitmapData( static_cast<const CBitmapShape<FloatType>&>( shape ), transform, arena );
			break;
		case HST_Complex:
			initComplexData( static_cast<const CComplexShape<FloatType>&>( shape ), transform, arena );
			break;
		default:
			assert( false );
			return;
	}
}

template <class FloatType>
void CHitbox<FloatType>::initNullData()
{
}

template <class FloatType>
template <class Arena>
void CHitbox<FloatType>::initPointData( const CPointShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena )
{
	auto& point = arena.Copy( shape.GetGlobalPoint( transform ) );
	hitboxData.SetBuffer( &point );
}

template <class FloatType>
template <class Arena>
void CHitbox<FloatType>::initAARectData( const CAARectShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena )
{
	auto& rect = arena.Copy( shape.GetGlobalRect( transform ) );
	hitboxData.SetBuffer( &rect );
}

template <class FloatType>
template <class Arena>
void CHitbox<FloatType>::initAngledRectData( const CAngledRectShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena )
{
	auto& points = arena.Create<CStackArray<CVector2<FloatType>, 4>>();
	shape.GetRectPoints( transform, points );
	hitboxData.SetBuffer( &points );
}

template <class FloatType>
template <class Arena>
void CHitbox<FloatType>::initPolygonData( const CPolygonShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena )
{
	const short vertexCount = numeric_cast<short>( shape.BaseVertices().Size() );

	const short baseWindingOrder = shape.IsClockwise() ? -1 : 1;
	const short windingOrder = static_cast<short>( baseWindingOrder * sign( transform( 0, 0 ) ) * sign( transform( 1, 1 ) ) );
	assert( windingOrder != 0 );

	const int vectorSize = sizeof( CVector2<FloatType> );
	const int dataSize = 2 * sizeof( short ) + vectorSize * vertexCount;
	const int dataAlign = alignof( int );
	hitboxData = arena.Create( dataSize, dataAlign );
	hitboxData.Set( vertexCount, 0 );
	hitboxData.Set( windingOrder, sizeof( short ) );
	for( int i = 0; i < vertexCount; i++ ) {
		const auto newVector = PointTransform( transform, shape.BaseVertices()[i] );
		hitboxData.Set( newVector, i * vectorSize + 2 * sizeof( short ) );
	}
}

template <class FloatType>
template <class Arena>
void CHitbox<FloatType>::initCircleData( const CCircleShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena )
{
	const int vectorSize = sizeof( CVector2<FloatType> );
	const int dataSize = vectorSize + sizeof( FloatType );
	const int dataAlign = alignof( int );
	hitboxData = arena.Create( dataSize, dataAlign );
	assert( transform( 0, 0 ) == transform( 1, 1 ) );
	const auto globalCenter = PointTransform( transform, shape.GetBaseCenter() );
	const auto globalRadius = shape.GetBaseRadius() * transform( 0, 0 );
	hitboxData.Set( globalCenter, 0 );
	hitboxData.Set( globalRadius, vectorSize );
}

template <class FloatType>
template <class Arena>
void CHitbox<FloatType>::initBitmapData( const CBitmapShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena )
{
	const auto boundRect = shape.GetBoundRect( transform );
	const auto cellCount = shape.GetCellCount();

	const auto angleSin = transform( 0, 1 );
	const auto angleCos = relibSqrt( FloatType( 1 ) - angleSin * angleSin );
	const CVector2<FloatType> cellSize{ transform( 0, 0 ) / angleCos, transform( 1, 1 ) / angleCos };
	const CVector2<FloatType> origin( transform( 2, 0 ), transform( 2, 1 ) );

	const CVector2<FloatType> invertOrigin{ -origin.X() * angleCos - origin.Y() * angleSin, origin.X() * angleSin - origin.Y() * angleCos };

	const auto& bitset = shape.GetBitmap();

	const auto dataSize = sizeof( boundRect ) + sizeof( cellCount ) + sizeof( cellSize ) + sizeof( origin ) + sizeof( angleSin ) + sizeof( angleCos ) + sizeof( void* );
	const auto dataAlign = alignof( FloatType );
	hitboxData = arena.Create( dataSize, dataAlign );
	hitboxData.Set( boundRect, 0 );
	const auto cellCountOffset = sizeof( boundRect );
	hitboxData.Set( cellCount, cellCountOffset );
	const auto cellSizeOffset = sizeof( cellCount ) + cellCountOffset;
	hitboxData.Set( cellSize, cellSizeOffset );
	const auto originOffset = sizeof( cellSize ) + cellSizeOffset;
	hitboxData.Set( invertOrigin, originOffset );
	const auto angleSinOffset = sizeof( invertOrigin ) + originOffset;
	hitboxData.Set( -angleSin, angleSinOffset );
	const auto angleCosOffset = sizeof( angleSin ) + angleSinOffset;
	hitboxData.Set( angleCos, angleCosOffset );
	const auto bitsetOffset = sizeof( angleCos ) + angleCosOffset;
	hitboxData.Set( &bitset, bitsetOffset );
}

template <class FloatType>
template <class Arena>
void CHitbox<FloatType>::initComplexData( const CComplexShape<FloatType>& shape, const CMatrix3<FloatType>& transform, Arena& arena )
{
	const int shapeCount = shape.Shapes().Size();
	const int hitboxSize = sizeof( CHitbox<FloatType> );
	const int sizeOffset = 2 * sizeof( int );
	const int dataSize = hitboxSize * shapeCount + sizeOffset;
	const int dataAlign = alignof( int );
	hitboxData = arena.Create( dataSize, dataAlign );
	hitboxData.Set( shapeCount, 0 );
	for( int i = 0; i < shapeCount; i++ ) {
		hitboxData.Set( CHitbox<FloatType>( *shape.Shapes()[i], transform, arena ), sizeOffset + i * hitboxSize );
	}
}

template <class FloatType>
template <class Arena>
void CHitbox<FloatType>::initPointData( const CPointShape<FloatType>& shape, Arena& arena )
{
	auto& point = arena.Copy( shape.GetBasePoint() );
	hitboxData.SetBuffer( &point );
}

template <class FloatType>
template <class Arena>
void CHitbox<FloatType>::initAARectData( const CAARectShape<FloatType>& shape, Arena& arena )
{
	auto& rect = arena.Copy( shape.GetBaseRect() );
	hitboxData.SetBuffer( &rect );
}

template <class FloatType>
template <class Arena>
void CHitbox<FloatType>::initAngledRectData( const CAngledRectShape<FloatType>& shape, Arena& arena )
{
	auto& points = arena.Create<CStackArray<CVector2<FloatType>, 4>>();
	const auto rect = shape.GetBaseRect();
	points[0] = rect.BottomLeft();
	points[1] = rect.BottomRight();
	points[2] = rect.TopRight();
	points[3] = rect.TopLeft();
	hitboxData.SetBuffer( &points );
}

template <class FloatType>
template <class Arena>
void CHitbox<FloatType>::initPolygonData( const CPolygonShape<FloatType>& shape, Arena& arena )
{
	const short vertexCount = numeric_cast< short >( shape.BaseVertices().Size() );

	const short windingOrder = shape.IsClockwise() ? -1 : 1;

	const int vectorSize = sizeof( CVector2<FloatType> );
	const int dataSize = 2 * sizeof( short ) + vectorSize * vertexCount;
	const int dataAlign = alignof( int );
	hitboxData = arena.Create( dataSize, dataAlign );
	hitboxData.Set( vertexCount, 0 );
	hitboxData.Set( windingOrder, sizeof( short ) );
	for( int i = 0; i < vertexCount; i++ ) {
		const auto newVector = shape.BaseVertices()[i];
		hitboxData.Set( newVector, i * vectorSize + 2 * sizeof( short ) );
	}
}

template <class FloatType>
template <class Arena>
void CHitbox<FloatType>::initCircleData( const CCircleShape<FloatType>& shape, Arena& arena )
{
	const int vectorSize = sizeof( CVector2<FloatType> );
	const int dataSize = vectorSize + sizeof( FloatType );
	const int dataAlign = alignof( int );
	hitboxData = arena.Create( dataSize, dataAlign );
	const auto globalCenter = shape.GetBaseCenter();
	const auto globalRadius = shape.GetBaseRadius(); 
	hitboxData.Set( globalCenter, 0 );
	hitboxData.Set( globalRadius, vectorSize );
}

template <class FloatType>
template <class Arena>
void CHitbox<FloatType>::initBitmapData( const CBitmapShape<FloatType>& shape, Arena& arena )
{
	const auto boundRect = shape.GetBaseBoundRect();
	const auto cellCount = shape.GetCellCount();

	const auto angleSin = FloatType( 0 );
	const auto angleCos = FloatType( 1 );
	const CVector2<FloatType> cellSize{ FloatType( 1 ), FloatType( 1 ) };
	const CVector2<FloatType> origin{};
	const CVector2<FloatType> invertOrigin{};

	const auto& bitset = shape.GetBitmap();

	const auto bitsetSize = FindBitSetSize( cellCount.X() * cellCount.Y() );
	const auto dataSize = sizeof( boundRect ) + sizeof( cellCount ) + sizeof( cellSize ) + sizeof( origin ) + sizeof( angleSin ) + sizeof( angleCos ) + sizeof( int ) * bitsetSize;
	const auto dataAlign = alignof( FloatType );
	hitboxData = arena.Create( dataSize, dataAlign );
	hitboxData.Set( boundRect, 0 );
	const auto cellCountOffset = sizeof( boundRect );
	hitboxData.Set( cellCount, cellCountOffset );
	const auto cellSizeOffset = sizeof( cellCount ) + cellCountOffset;
	hitboxData.Set( cellSize, cellSizeOffset );
	const auto originOffset = sizeof( cellSize ) + cellSizeOffset;
	hitboxData.Set( invertOrigin, originOffset );
	const auto angleSinOffset = sizeof( invertOrigin ) + originOffset;
	hitboxData.Set( -angleSin, angleSinOffset );
	const auto angleCosOffset = sizeof( angleSin ) + angleSinOffset;
	hitboxData.Set( angleCos, angleCosOffset );
	const auto bitsetOffset = sizeof( angleCos ) + angleCosOffset;
	for( int i = 0; i < bitsetSize; i++ ) {
		auto bitsetWord = bitset.GetStorage()[i];
		hitboxData.Set( bitsetWord, bitsetOffset + i * sizeof( bitsetWord ) );
	}
}

template <class FloatType>
template <class Arena>
void CHitbox<FloatType>::initComplexData( const CComplexShape<FloatType>& shape, Arena& arena )
{
	const int shapeCount = shape.Shapes().Size();
	const int hitboxSize = sizeof( CHitbox<FloatType> );
	const int sizeOffset = 2 * sizeof( int );
	const int dataSize = hitboxSize * shapeCount + sizeOffset;
	const int dataAlign = alignof( int );
	hitboxData = arena.Create( dataSize, dataAlign );
	hitboxData.Set( shapeCount, 0 );
	for( int i = 0; i < shapeCount; i++ ) {
		hitboxData.Set( CHitbox<FloatType>( *shape.Shapes()[i], arena ), sizeOffset + i * hitboxSize );
	}
}

template <class FloatType>
void CHitbox<FloatType>::OffsetPosition( CVector2<FloatType> delta )
{
	staticAssert( HST_EnumCount == 8 );
	switch( hitboxType ) {
		case HST_Null:
			break;
		case HST_Point:
			offsetPointData( delta );
			break;
		case HST_AARect:
			offsetAARectData( delta );
			break;
		case HST_AngledRect:
			offsetAngledRectData( delta );
			break;
		case HST_Polygon:
			offsetPolygonData( delta );
			break;
		case HST_Circle:
			offsetCircleData( delta );
			break;
		case HST_Bitmap:
			offsetBitmapData( delta );
			break;
		case HST_Complex:
			offsetComplexData( delta );
			break;
		default:
			assert( false );
			return;
	}
}

template <class FloatType>
void CHitbox<FloatType>::offsetPointData( CVector2<FloatType> offset )
{
	auto& point = hitboxData.As<CVector2<float>>();
	point += offset;
}

template <class FloatType>
void CHitbox<FloatType>::offsetAARectData( CVector2<FloatType> offset )
{
	auto& rect = hitboxData.As<CAARect<float>>();
	rect.OffsetRect( offset );
}

template <class FloatType>
void CHitbox<FloatType>::offsetAngledRectData( CVector2<FloatType> offset )
{
	auto& points = hitboxData.As<CStackArray<CVector2<FloatType>, 4>>();
	for( auto& point : points ) {
		point += offset;
	}
}

template <class FloatType>
void CHitbox<FloatType>::offsetPolygonData( CVector2<FloatType> offset )
{
	const auto vertexCount = hitboxData.Get<short>( 0 );

	auto dataOffset = 2 * sizeof( short );
	for( int i = 0; i < vertexCount; i++ ) {
		auto& point = hitboxData.Get<CVector2<FloatType>>( dataOffset );
		point += offset;
		dataOffset += sizeof( CVector2<FloatType> );
	}
}

template <class FloatType>
void CHitbox<FloatType>::offsetCircleData( CVector2<FloatType> offset )
{
	auto& center = hitboxData.Get<CVector2<float>>( 0 );
	center += offset;
}

template <class FloatType>
void CHitbox<FloatType>::offsetBitmapData( CVector2<FloatType> offset )
{
	auto& boundRect = hitboxData.Get<CAARect<FloatType>>( 0 );
	boundRect.OffsetRect( offset );

	const auto originOffset = sizeof( CAARect<FloatType> ) + sizeof( CVector2<int> ) + sizeof( CVector2<float> );
	const auto sincosOffset = sizeof( CVector2<FloatType> ) + originOffset;
	const auto sinCos = hitboxData.Get<CVector2<FloatType>>( sincosOffset );

	const CVector2<FloatType> invertOffset{ -offset.X() * sinCos.Y() + offset.Y() * sinCos.X(), -offset.X() * sinCos.X() - offset.Y() * sinCos.Y() };
	auto& invertOrigin = hitboxData.Get<CVector2<FloatType>>( originOffset );
	invertOrigin += invertOffset;
}

template <class FloatType>
void CHitbox<FloatType>::offsetComplexData( CVector2<FloatType> offset )
{
	const auto shapeCount = hitboxData.Get<int>( 0 );
	int hitboxOffset = sizeof( int );
	for( int i = 0; i < shapeCount; i++ ) {
		auto& hitbox = hitboxData.Get<CHitbox<FloatType>>( hitboxOffset );
		hitbox.OffsetPosition( offset );
		hitboxOffset += sizeof( CHitbox<FloatType> );
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

