#pragma once
#include <Remath.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

typedef float ( *TEasingFunction )( float, float );

namespace Easing {

//////////////////////////////////////////////////////////////////////////

inline float Linear( float currentTime, float duration )
{
	return currentTime / duration;
}

inline float QuadIn( float currentTime, float duration )
{
	const float timeRatio = currentTime / duration;
	return timeRatio * timeRatio;
}

inline float QuadOut( float currentTime, float duration )
{
	const float timeRatio = currentTime / duration; 
	return -timeRatio * ( timeRatio - 2 );
}

inline float QuadInOut( float currentTime, float duration )
{
	const float timeRatio = currentTime * 2.0f / duration; 
	return timeRatio < 1
		? 0.5f * timeRatio * timeRatio
		: -0.5f * ( ( timeRatio - 1 ) * ( timeRatio - 3 ) - 1 );
}

inline float CubeIn( float currentTime, float duration )
{
	const float timeRatio = currentTime / duration;
	return timeRatio * timeRatio * timeRatio;
}

inline float CubeOut( float currentTime, float duration )
{
	const float timeRatio = ( currentTime / duration ) - 1;
	return timeRatio * timeRatio * timeRatio + 1; 
}

inline float CubeInOut( float currentTime, float duration )
{
	const float timeRatio = currentTime * 2.0f / duration;
	return timeRatio < 1 
		? 0.5f * timeRatio * timeRatio * timeRatio
		: 0.5f * ( ( timeRatio - 2 ) * ( timeRatio - 2 ) * ( timeRatio - 2 ) + 2 );
}

inline float QuartIn( float currentTime, float duration )
{
	const float timeRatio = currentTime / duration;
	return timeRatio * timeRatio * timeRatio * timeRatio;
}

inline float QuartOut( float currentTime, float duration )
{
	const float timeRatio = ( currentTime / duration ) - 1;
	return -( timeRatio * timeRatio * timeRatio * timeRatio - 1 );
}

inline float QuartInOut( float currentTime, float duration )
{
	const float timeRatio = currentTime * 2.0f / duration;
	return timeRatio < 1 
		? 0.5f * timeRatio * timeRatio * timeRatio * timeRatio
		: -0.5f * ( ( timeRatio - 2 ) * ( timeRatio - 2 ) * ( timeRatio - 2 ) * ( timeRatio - 2 ) - 2 );
}

inline float QuintIn( float currentTime, float duration )
{
	const float timeRatio = currentTime / duration;
	return timeRatio * timeRatio * timeRatio * timeRatio * timeRatio;
}

inline float QuintOut( float currentTime, float duration )
{
	const float timeRatio = ( currentTime / duration ) - 1;
	return timeRatio * timeRatio * timeRatio * timeRatio * timeRatio + 1;
}

inline float QuintInOut( float currentTime, float duration )
{
	const float timeRatio = currentTime * 2.0f / duration;
	return timeRatio < 1 
		? 0.5f * timeRatio * timeRatio * timeRatio * timeRatio * timeRatio
		: 0.5f * ( ( timeRatio - 2 ) * ( timeRatio - 2 ) * ( timeRatio - 2 ) * ( timeRatio - 2 ) * ( timeRatio - 2 ) + 2 );
}

inline float SineIn( float currentTime, float duration )
{
	const float timeRatio = currentTime * 0.5f / duration;
	return 1.0f - fastCosSmallAngle( timeRatio * Pi );
}

inline float SineOut( float currentTime, float duration )
{
	const float timeRatio = currentTime * 0.5f / duration;
	return fastSinSmallAngle( timeRatio * Pi );
}

inline float SineInOut( float currentTime, float duration )
{
	const float timeRatio = currentTime / duration;
	return -0.5f * ( fastCos( timeRatio * Pi ) - 1 );
}

inline float ExpIn( float currentTime, float duration )
{
	const float timeRatio = ( currentTime * 1.0f / duration ) - 1;
	return powf( 2, 10 * timeRatio );
}

inline float ExpOut( float currentTime, float duration )
{
	const float timeRatio = ( currentTime * 1.0f / duration );
	return 1 - powf( 2, -10 * timeRatio );
}

inline float ExpInOut( float currentTime, float duration )
{
	const float timeRatio = ( currentTime * 2.0f / duration ) - 1;
	return timeRatio < 0 
		? 0.5f * powf( 2, 10 * timeRatio )
		: 0.5f * ( 2 - powf( 2, -10 * timeRatio ) );
}

inline float CircularIn( float currentTime, float duration )
{
	const float timeRatio = ( currentTime * 1.0f / duration );
	return 1 - fastSqrt( 1 - timeRatio * timeRatio );
}

inline float CircularOut( float currentTime, float duration )
{
	const float timeRatio = ( currentTime * 1.0f / duration ) - 1;
	return fastSqrt( 1 - timeRatio * timeRatio );
}

inline float CircularInOut( float currentTime, float duration )
{
	const float timeRatio = ( currentTime * 2.0f / duration );
	return timeRatio < 1
		? 0.5f * ( 1 - fastSqrt( 1 - timeRatio * timeRatio ) )
		: 0.5f * ( fastSqrt( 1 - ( timeRatio - 2 ) * ( timeRatio - 2 ) ) + 1 );
}

inline float BackIn( float currentTime, float duration, float curveFactor = 1.70158f )
{
	const float timeRatio = ( currentTime * 1.0f / duration );
	return timeRatio * timeRatio * ( ( curveFactor + 1 ) * timeRatio - curveFactor );
}

inline float BackOut( float currentTime, float duration, float curveFactor = 1.70158f )
{
	const float timeRatio = ( currentTime * 1.0f / duration ) - 1;
	return ( timeRatio * timeRatio * ( ( curveFactor + 1 ) * timeRatio + curveFactor ) + 1 );
}

inline float BackInOut( float currentTime, float duration, float curveFactor = 1.70158f )
{
	const auto adjustedCurveFactor = curveFactor * 1.525f;
	const float timeRatio = ( currentTime * 2.0f / duration );
	return timeRatio < 1
		? 0.5f * ( timeRatio * timeRatio * ( ( adjustedCurveFactor + 1 ) * timeRatio - adjustedCurveFactor ) )
		: 0.5f * ( ( timeRatio - 2 ) * ( timeRatio - 2 ) * ( ( adjustedCurveFactor + 1 ) * ( timeRatio - 2 ) + adjustedCurveFactor ) + 2 );
}

//////////////////////////////////////////////////////////////////////////

} // namespace Easing.

//////////////////////////////////////////////////////////////////////////

// Easing function that mirrors the given function movement in the second half of its duration.
template <TEasingFunction srcFunction>
class CBackForthEasing {
public:
	static float Ease( float currentTime, float duration )
	{
		const auto hDuration = duration / 2;
		if( currentTime <= hDuration ) {
			return srcFunction( currentTime, hDuration );
		} else {
			return srcFunction( duration - currentTime, hDuration );
		}
	}

	operator TEasingFunction() const
		{ return Ease; }
	float operator()( float currentTime, float duration ) const
		{ return Ease( currentTime, duration ); }
};

 //////////////////////////////////////////////////////////////////////////

  // An ease operation with the given function.
  // Type T must have a Lerp function or use the default one.
  // The default Lerp function uses operator*( T, float ) and operator+( T, T ).
template <class T, class EaseFunc>
T Ease( float currentTime, float duration, T start, T end, const EaseFunc& easing )
{	
	assert( currentTime >= 0 && currentTime <= duration );
	return Lerp( start, end, easing( currentTime, duration ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

