#pragma once

#include <cmath>
#include <time.h>
#include <stdlib.h>
#include <Interval.h>
#include <TemplateUtils.h>
#include <MathUtils.h>
#include <Reassert.h>

namespace Relib {

const float Pi = 3.1415926f;
const float HalfPi = Pi / 2;

#pragma warning( push )
#pragma warning( disable : 4389 ) // '==' : signed/unsigned mismatch

// Numbers cast with bounds checking.
template<class Dest, class Src>
inline Dest numeric_cast( Src src )
{
	static_assert( Types::IsNumeric<Src>::Result, "numeric_cast only works with numbers" );
	assert( static_cast<Dest>( src ) == src ); // src shouldn't change value after the cast.
	return static_cast<Dest>( src );
}

#pragma warning( pop )

template <class Type>
inline constexpr Type min( Type a, Type b ) { return a < b ? a : b; }
template <class Type>
inline constexpr Type max( Type a, Type b ) { return a > b ? a : b; }

template <class Type, class... Types>
inline constexpr Type min( Type a, Type b, Types... rest )
{
	return min( min( a, b ), rest... );
}

template <class Type, class... Types>
inline constexpr Type max( Type a, Type b, Types... rest )
{
	return max( max( a, b ), rest... );
}

template <class Type>
inline constexpr Type abs( Type a ) { return a >= 0 ? a : -a; }
template <class T>
inline constexpr int sign( T val ) { return ( T( 0 ) < val ) - ( val < T( 0 ) ); }

template<>
inline float min( float a, float b )
{
	__m128 inA = _mm_load_ss( &a );
	__m128 inB = _mm_load_ss( &b );
	float result;
	_mm_store_ss( &result, _mm_min_ss( inA, inB ) );
	return result;
}

template<>
inline float max( float a, float b )
{
	__m128 inA = _mm_load_ss( &a );
	__m128 inB = _mm_load_ss( &b );
	float result;
	_mm_store_ss( &result, _mm_max_ss( inA, inB ) );
	return result;
}

template<>
inline double min( double a, double b )
{
	__m128d inA = _mm_load_sd( &a );
	__m128d inB = _mm_load_sd( &b );
	double result;
	_mm_store_sd( &result, _mm_min_sd( inA, inB ) );
	return result;
}

template<>
inline double max( double a, double b )
{
	__m128d inA = _mm_load_sd( &a );
	__m128d inB = _mm_load_sd( &b );
	double result;
	_mm_store_sd( &result, _mm_max_sd( inA, inB ) );
	return result;
}

template <class T, class... Types>
inline CInterval<T> minmax( T a, T b, Types... rest )
{
	return a < b ? RelibInternal::calculateMinMax( a, b, rest... ) : RelibInternal::calculateMinMax( b, a, rest... );
}

// General extremum search function.
// Find a result value so that comp( result, other ) is true for all other values in the array.
template <class ValueRange, class Comparator>
inline int FindExtremumPos( const ValueRange& range, const Comparator& comp )
{
	CArrayView<typename Types::ArrayElemType<ValueRange>::Result> values( range );
	assert( !values.IsEmpty() );

	int currentIndex = 0;
	for( int i = 1; i < values.Size(); i++ ) {
		if( comp( values[i], values[currentIndex] ) ) {
			currentIndex = i;
		}
	}
	return currentIndex;
}

template <class ValueRange, class Comparator>
inline typename Types::ArrayElemType<ValueRange>::Result FindExtremum( const ValueRange& range, const Comparator& comp )
{
	return range[FindExtremumPos( range, comp )];
}

//////////////////////////////////////////////////////////////////////////

// Square root.
inline float fastSqrt( float x )
{
	assert( x >= 0 );
	__m128 inX = _mm_load_ss( &x );
	float result;
	_mm_store_ss( &result, _mm_sqrt_ss( inX ) );
	return result;
}

// Fast calculation of an inverse square root.
inline float invSqrt( float x )
{
	assert( x > 0 );
	__m128 inX = _mm_load_ss( &x );
	float result;
	_mm_store_ss( &result, _mm_rsqrt_ss( inX ) );	

	result *= 1.5f - x * result * result / 2;
	return result;
}

inline double invSqrt( double x )
{
	assert( x > 0 );
	return 1 / sqrt( x );
}

inline float relibSqrt( float x )
{
	return fastSqrt( x );
}

inline double relibSqrt( double x ) 
{
	return sqrt( x );
}

// Trigonometry.

// Fast sine function. The angle is assumed to be between -Pi/2 and Pi/2.
// Maximum relative error in this range is < 0.001%.
// On angles close to Pi relative error reaches 35%.
inline float fastSinSmallAngle( float angleRad )
{
	// Function uses polynomial approximation with Remez Exchange Algorithm.
	// Polynomials are presented using Estrin’s Method.
	// http://www.research.scea.com/gdc2003/fast-math-functions_p2.pdf
	const float squareAngle = angleRad * angleRad;
	return angleRad * ( ( 1.0f - 0.16666f * squareAngle ) + squareAngle * squareAngle * ( 0.0083143f - 0.00018542f * squareAngle ) );
}

// Fast cosine function. Uses fastSinSmallAngle.
inline float fastCosSmallAngle( float angleRad )
{
	return fastSinSmallAngle( static_cast<float>( HalfPi ) - abs( angleRad ) );
}

// Sine function for arbitrary angles.
inline float fastSin( float angleRad )
{
	const float halfPif = static_cast<float>( HalfPi );
	const float invHalfPi = 1.0f / halfPif;
	const int halfPiCount = static_cast<int>( angleRad * invHalfPi );
	const float normalizedAngle = angleRad - halfPiCount * halfPif;
	switch( halfPiCount % 4 ) {
	case 0:
		return fastSinSmallAngle( normalizedAngle );
	case -1:
		return -fastSinSmallAngle( halfPif + normalizedAngle );
	case 1:
		return fastSinSmallAngle( halfPif - normalizedAngle );
	case -2:
	case 2:
		return -fastSinSmallAngle( normalizedAngle );
	case -3:
		return fastSinSmallAngle( halfPif + normalizedAngle );
	case 3:
		return -fastSinSmallAngle( halfPif - normalizedAngle );
	default:
		assert( false );
		return 0;
	} 
	return 0;
}

// Cosine function for arbitrary angles.
inline float fastCos( float angleRad )
{
	const float invHalfPi = 1.0f / HalfPi;
	const int halfPiCount = static_cast<int>( angleRad * invHalfPi );
	const float normalizedAngle = angleRad - halfPiCount * HalfPi;
	switch( halfPiCount % 4 ) {
	case 0:
		return fastCosSmallAngle( normalizedAngle );
	case -1:
		return fastSinSmallAngle( normalizedAngle );
	case 1:
		return -fastSinSmallAngle( normalizedAngle );
	case -2:
		return -fastSinSmallAngle( HalfPi + normalizedAngle );
	case 2:
		return -fastSinSmallAngle( HalfPi - normalizedAngle );
	case -3:
		return -fastSinSmallAngle( normalizedAngle );
	case 3:
		return fastSinSmallAngle( normalizedAngle );
	default:
		assert( false );
		return 0;
	} 
	return 0;
}

inline void fastSinCos( float angleRad, float& sinVal, float& cosVal )
{
	const float invHalfPi = 1.0f / HalfPi;
	const int halfPiCount = static_cast<int>( angleRad * invHalfPi );
	const float normalizedAngle = angleRad - halfPiCount * HalfPi;
	switch( halfPiCount % 4 ) {
	case 0:
		sinVal = fastSinSmallAngle( normalizedAngle );
		cosVal = fastCosSmallAngle( normalizedAngle );
		break;
	case -1:
		sinVal = -fastSinSmallAngle( HalfPi + normalizedAngle );
		cosVal = fastSinSmallAngle( normalizedAngle );
		break;
	case 1:
		sinVal = fastSinSmallAngle( HalfPi - normalizedAngle );
		cosVal = -fastSinSmallAngle( normalizedAngle );
		break;
	case -2:
		sinVal = -fastSinSmallAngle( normalizedAngle );
		cosVal = -fastSinSmallAngle( HalfPi + normalizedAngle );
		break;
	case 2:
		sinVal = -fastSinSmallAngle( normalizedAngle );
		cosVal = -fastSinSmallAngle( HalfPi - normalizedAngle );
		break;
	case -3:
		sinVal = fastSinSmallAngle( HalfPi + normalizedAngle );
		cosVal = -fastSinSmallAngle( normalizedAngle );
		break;
	case 3:
		sinVal = -fastSinSmallAngle( HalfPi - normalizedAngle );
		cosVal = fastSinSmallAngle( normalizedAngle );
		break;
	default:
		assert( false );
		break;
	} 
}

// Sine and cosine functions used by the library.
// Normal functions are used for double.
// Fast functions are used for float.
inline float relibSin( float angleRad )
{
	return fastSin( angleRad );
}

inline float relibCos( float angleRad )
{
	return fastCos( angleRad );
}

inline double relibSin( double angleRad )
{
	return sin( angleRad );
}

inline double relibCos( double angleRad )
{
	return cos( angleRad );
}

inline void relibSinCos( float angleRad, float& sinVal, float& cosVal )
{
	fastSinCos( angleRad, sinVal, cosVal );
}

inline void relibSinCos( double angleRad, double& sinVal, double& cosVal )
{
	sinVal = sin( angleRad );
	cosVal = cos( angleRad );
}

//////////////////////////////////////////////////////////////////////////

// Converts a to an rvalue reference.
// See reference collapsing rules to understand what's going on here.
template<class Type>
inline typename Types::RemoveReference<Type>::Result&& move( Type&& rValue )
{
	return static_cast<typename Types::RemoveReference<Type>::Result&&>( rValue );
}

// Converts ref to the exact type that it has.
// Same behavior as std::forward.
template <class Type>
Type&& forward( typename Types::RemoveReference<Type>::Result& ref )
{
	return static_cast<Type&&>( ref );
}

template <class Type>
Type&& forward( typename Types::RemoveReference<Type>::Result&& ref )
{
	return static_cast<Type&&>( ref );
}

template <class Type>
void swap( Type& a, Type& b )
{
	Type t = move( a );
	a = move( b );
	b = move( t );
}

//////////////////////////////////////////////////////////////////////////

inline bool HasFractionalPart( double d )
{
	double dummy;
	return std::modf( d, &dummy ) != 0.0;
}

inline bool HasFractionalPart( float d )
{
	float dummy;
	return std::modf( d, &dummy ) != 0.0f;
}

inline int Round( double d )
{
	const double result = d > 0 ? ( d + 0.5 ) : ( d - 0.5 );
	assert( INT_MIN <= result && result <= INT_MAX );
	return static_cast<int>( result );
}

inline int Round( float d )
{
	const double result = d > 0 ? ( d + 0.5 ) : ( d - 0.5 );
	assert( INT_MIN <= result && result <= INT_MAX );
	return static_cast<int>( result );
}

inline int Floor( double d )
{
	assert( INT_MIN <= floor( d ) && floor( d ) <= INT_MAX );
	return static_cast<int>( d );
}

inline int Floor( float d )
{
	assert( INT_MIN <= d && d <= INT_MAX );
	return static_cast<int>( d );
}

inline int Ceil( double d )
{
	const double result = ceil( d );
	assert( INT_MIN <= result && result <= INT_MAX );
	return static_cast<int>( result );
}

inline int Ceil( float d )
{
	const float result = ceilf( d );
	assert( INT_MIN <= result && result <= INT_MAX );
	return static_cast<int>( result );
}

// Get the integer closest to the result of value / step.
inline constexpr int Round( int value, int step )
{
	return ( value + step / 2 ) / step;
}

inline constexpr int Ceil( int value, int step )
{
	return ( value + step - 1 ) / step;
}

inline constexpr int Floor( int value, int step )
{
	return value / step;
}

// Get the integer closest to value that is divisible by step.
inline constexpr int RoundTo( int value, int step )
{
	return Round( value, step ) * step;
}

inline constexpr int CeilTo( int value, int step )
{
	return Ceil( value, step ) * step;
}

inline constexpr int FloorTo( int value, int step )
{
	return Floor( value, step ) * step;
}

// Get a floating point number closest to value that contains an integer number of steps.
inline float RoundFloatTo( float value, float step )
{
	assert( step > 0 );
	const int valueCount = Round( value / step );
	return step * valueCount;
}

template <class ValueType>
inline ValueType Clamp( ValueType val, ValueType minVal, ValueType maxVal )
{
	assert( maxVal >= minVal );
	return min( maxVal, max( minVal, val ) );
}

// Linear interpolation.
template <class ValueType, class WeightType>
inline ValueType Lerp( ValueType left, ValueType right, WeightType t )
{
	assert( t >= 0 );
	const auto lerpDiff = t * ( right - left );
	return left + static_cast<ValueType>( lerpDiff );
}

//////////////////////////////////////////////////////////////////////////

// Handling bit flags.
#define BIT_FLAG( n ) ( static_cast<DWORD>( 1UL << ( n ) ) )

// Set flags in set.
inline void SetFlags( DWORD& set, DWORD flags ) 
{
	set |= flags;
}

// Clear flags in set.
inline void ClearFlags( DWORD& set, DWORD flags ) 
{
	set &= ~flags;
}

// Swap flags in set.
inline void ToggleFlags( DWORD& set, DWORD flags ) 
{
	set ^= flags;
}

// Check the presence of at least a single flag from sourceFlag in the set.
inline bool HasFlag( DWORD set, DWORD sourceFlag )
{
	return ( set & sourceFlag ) != 0;
}

inline bool HasAllFlags( DWORD set, DWORD subset )
{
	return ( set & subset ) == subset;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib