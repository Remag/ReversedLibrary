#pragma once
#include <Vector.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A 32-bit color in BGRA format ( Windows native ).
struct CColor {
	// Color components.
	BYTE B = 0;
	BYTE G = 0;
	BYTE R = 0;
	BYTE A = 255;

	static BYTE MaxValue()
		{ return UCHAR_MAX; }

	// Color in vector form.
	CVector4<float> GetVector() const
		{ return CVector4<float>{ GetRed(), GetGreen(), GetBlue(), GetAlpha() }; }
	// Color in a single integer form.
	unsigned GetHexRgbaValue() const;
	// Floating point color values in range [0,1].
	float GetRed() const
		{ return toFloat( R ); }
	float GetGreen() const
		{ return toFloat( G ); }
	float GetBlue() const
		{ return toFloat( B ); }
	float GetAlpha() const
		{ return toFloat( A ); }

	void SetRed( float newValue )
		{ R = toByte( newValue ); }
	void SetGreen( float newValue )
		{ G = toByte( newValue ); }
	void SetBlue( float newValue )
		{ B = toByte( newValue ); }
	void SetAlpha( float newValue )
		{ A = toByte( newValue ); }

	// Default construction. Creates a black color.
	CColor() = default;

	// Construct a color from a single value in RGB format.
	explicit CColor( unsigned colorValue, int alpha = 0 );
	CColor( CVector3<int> rgb ) : CColor( rgb.X(), rgb.Y(), rgb.Z() ) {}
	CColor( CVector4<int> rgba ) : CColor( rgba.X(), rgba.Y(), rgba.Z(), rgba.W() ) {}
	CColor( CVector3<float> rgb ) : CColor( rgb.X(), rgb.Y(), rgb.Z() ) {}
	CColor( CVector4<float> rgba ) : CColor( rgba.X(), rgba.Y(), rgba.Z(), rgba.W() ) {}

	// Construct from RGB(A) components.
	CColor( int r, int g, int b, int a = 255 );
	// Construct from floating point components.
	// All of them must be in range [0, 1].
	CColor( float r, float g, float b, float a = 1.0f );

	// Conversion to proper vectors.
	operator CVector3<int>() const
		{ return CVector3<int>( int( R ), int( G ), int( B ) ); }
	operator CVector4<int>() const
		{ return CVector4<int>( int( R ), int( G ), int( B ), int( A ) ); }
	operator CVector3<float>() const
		{ return CVector3<float>( GetRed(), GetGreen(), GetBlue() ); }
	operator CVector4<float>() const
		{ return CVector4<float>( GetRed(), GetGreen(), GetBlue(), GetAlpha() ); }
	
	// Color comparison.
	bool operator==( const CColor& other ) const
		{ return B == other.B && G == other.G && R == other.R && A == other.A; }
	bool operator!=( const CColor& other ) const
		{ return !( *this == other ); }

private:
	static float toFloat( BYTE color );
	static BYTE toByte( float color );
};

//////////////////////////////////////////////////////////////////////////

inline CColor::CColor( unsigned colorValue, int alpha ) :
	R( ( colorValue >> 16 ) & 0xFF ),
	G( ( colorValue >> 8 ) & 0xFF ),
	B( colorValue & 0xFF ),
	A( static_cast<BYTE>( alpha ) )
{
	assert( alpha >= 0 && alpha <= UCHAR_MAX );
}

inline CColor::CColor( int r, int g, int b, int a /*= 255 */ ) :
	R( static_cast<BYTE>( r ) ),
	G( static_cast<BYTE>( g ) ),
	B( static_cast<BYTE>( b ) ),
	A( static_cast<BYTE>( a ) )
{
	assert( r <= UCHAR_MAX );
	assert( g <= UCHAR_MAX );
	assert( b <= UCHAR_MAX );
	assert( a <= UCHAR_MAX );
}

inline CColor::CColor( float r, float g, float b, float a /*= 1.0f */ ) :
	B( static_cast<BYTE>( b * 255 ) ),
	G( static_cast<BYTE>( g * 255 ) ),
	R( static_cast<BYTE>( r * 255 ) ),
	A( static_cast<BYTE>( a * 255 ) )
{
	assert( r >= 0 && r <= 1 );
	assert( g >= 0 && r <= 1 );
	assert( b >= 0 && r <= 1 );
	assert( a >= 0 && r <= 1 );
}

inline unsigned CColor::GetHexRgbaValue() const
{
	return ( static_cast<unsigned>( A ) << 24 ) 
		+ ( static_cast<unsigned>( R ) << 16 ) 
		+ ( static_cast<unsigned>( G ) << 8 ) 
		+ ( static_cast<unsigned>( B ) ); 
}

inline float CColor::toFloat( BYTE color )
{
	return static_cast<float>( color * 1.0f / MaxValue() );
}

inline BYTE CColor::toByte( float color )
{
	assert( color >= 0 && color <= 1 );
	return static_cast<BYTE>( color * MaxValue() + 0.5f );
}

//////////////////////////////////////////////////////////////////////////

// Conversion between color spaces.

inline CVector4<float> SRGBToLinear( CVector4<float> src )
{
	const float sRGBGammaValue = 2.2f;
	return CVector4<float>{ powf( src.X(), sRGBGammaValue ), powf( src.Y(), sRGBGammaValue ), powf( src.Z(), sRGBGammaValue ), src.W() };
}

inline CVector3<float> SRGBToLinear( CVector3<float> src )
{
	return SRGBToLinear( CVector4<float>{ src, 1.0f } ).XYZ();
}

inline CColor SRGBToLinear( CColor src )
{
	return CColor( SRGBToLinear( CVector4<float>{ src.GetRed(), src.GetGreen(), src.GetBlue(), src.GetAlpha() } ) );
}

inline CVector4<float> LinearToSRGB( CVector4<float> src )
{
	const float sRGBGammaValueInv = 1.0f / 2.2f;
	return CColor( powf( src.X(), sRGBGammaValueInv ), powf( src.Y(), sRGBGammaValueInv ), powf( src.Z(), sRGBGammaValueInv ), src.W() );
}

inline CVector3<float> LinearToSRGB( CVector3<float> src )
{
	return LinearToSRGB( CVector4<float>{ src, 1.0f } ).XYZ();
}

inline CColor LinearToSRGB( CColor src )
{
	return CColor( LinearToSRGB( CVector4<float>{ src.GetRed(), src.GetGreen(), src.GetBlue(), src.GetAlpha() } ) );
}

// Linear color interpolation.
template <class WeightType>
inline CColor Lerp( const CColor& left, const CColor& right, WeightType t )
{
	CColor result;
	assert( t >= 0 && t <= 1 );
	result.B = static_cast<BYTE>( Round( Lerp( static_cast<float>( left.B ), static_cast<float>( right.B ), t ) ) );
	result.G = static_cast<BYTE>( Round( Lerp( static_cast<float>( left.G ), static_cast<float>( right.G ), t ) ) );
	result.R = static_cast<BYTE>( Round( Lerp( static_cast<float>( left.R ), static_cast<float>( right.R ), t ) ) );
	result.A = static_cast<BYTE>( Round( Lerp( static_cast<float>( left.A ), static_cast<float>( right.A ), t ) ) );
	return result;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

