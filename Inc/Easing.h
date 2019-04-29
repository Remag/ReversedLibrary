#pragma once
#include <Remath.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// A tweening function. 
class IEasingFunction {
public:
	virtual ~IEasingFunction() {}

	// Get the corrected proportion between currentTime and duration.
	// The proportion is calculated using one of the few possible easing functions.
	virtual float GetTimeRatio( float currentTime, float duration ) const = 0;
};

//////////////////////////////////////////////////////////////////////////
// Linear easing function.

class CLinearEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final
		{ return currentTime / duration; }
};

//////////////////////////////////////////////////////////////////////////
// Quadratic easing functions.

class CQuadInEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = currentTime / duration;
		return timeRatio * timeRatio;
	}
};

class CQuadOutEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = currentTime / duration; 
		return -timeRatio * ( timeRatio - 2 );
	}
};

class CQuadInOutEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = currentTime * 2.0f / duration; 
		return timeRatio < 1
			? 0.5f * timeRatio * timeRatio
			: -0.5f * ( ( timeRatio - 1 ) * ( timeRatio - 3 ) - 1 );
	}
};

//////////////////////////////////////////////////////////////////////////
// Cubic easing functions.

class CCubeInEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = currentTime / duration;
		return timeRatio * timeRatio * timeRatio;
	}
};

class CCubeOutEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = ( currentTime / duration ) - 1;
		return timeRatio * timeRatio * timeRatio + 1; 
	}
};

class CCubeInOutEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = currentTime * 2.0f / duration;
		return timeRatio < 1 
			? 0.5f * timeRatio * timeRatio * timeRatio
			: 0.5f * ( ( timeRatio - 2 ) * ( timeRatio - 2 ) * ( timeRatio - 2 ) + 2 );
	}
};

//////////////////////////////////////////////////////////////////////////
// Quartic easing functions.

class CQuarticInEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = currentTime / duration;
		return timeRatio * timeRatio * timeRatio * timeRatio;
	}
};

class CQuarticOutEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = ( currentTime / duration ) - 1;
		return -( timeRatio * timeRatio * timeRatio * timeRatio - 1 );
	}
};

class CQuarticInOutEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = currentTime * 2.0f / duration;
		return timeRatio < 1 
			? 0.5f * timeRatio * timeRatio * timeRatio * timeRatio
			: -0.5f * ( ( timeRatio - 2 ) * ( timeRatio - 2 ) * ( timeRatio - 2 ) * ( timeRatio - 2 ) - 2 );
	}
};

//////////////////////////////////////////////////////////////////////////
// Quintic easing functions.

class CQuintInEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = currentTime / duration;
		return timeRatio * timeRatio * timeRatio * timeRatio * timeRatio;
	}
};

class CQuintOutEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = ( currentTime / duration ) - 1;
		return timeRatio * timeRatio * timeRatio * timeRatio * timeRatio + 1;
	}
};

class CQuintInOutEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = currentTime * 2.0f / duration;
		return timeRatio < 1 
			? 0.5f * timeRatio * timeRatio * timeRatio * timeRatio * timeRatio
			: 0.5f * ( ( timeRatio - 2 ) * ( timeRatio - 2 ) * ( timeRatio - 2 ) * ( timeRatio - 2 ) * ( timeRatio - 2 ) + 2 );
	}
};

//////////////////////////////////////////////////////////////////////////
// Sine easing functions.

class CSineInEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = currentTime * 0.5f / duration;
		return 1.0f - fastCosSmallAngle( timeRatio * Pi );
	}
};

class CSineOutEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = currentTime * 0.5f / duration;
		return fastSinSmallAngle( timeRatio * Pi );
	}
};

class CSineInOutEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = currentTime / duration;
		return -0.5f * ( fastCos( timeRatio * Pi ) - 1 );
	}
};

//////////////////////////////////////////////////////////////////////////
// Exponential easing functions.

class CExpInEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = ( currentTime * 1.0f / duration ) - 1;
		return powf( 2, 10 * timeRatio );
	}
};

class CExpOutEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = ( currentTime * 1.0f / duration );
		return 1 - powf( 2, -10 * timeRatio );
	}
};

class CExpInOutEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = ( currentTime * 2.0f / duration ) - 1;
		return timeRatio < 0 
			? 0.5f * powf( 2, 10 * timeRatio )
			: 0.5f * ( 2 - powf( 2, -10 * timeRatio ) );
	}
};

//////////////////////////////////////////////////////////////////////////
// Circular easing functions.

class CCircularInEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = ( currentTime * 1.0f / duration );
		return 1 - fastSqrt( 1 - timeRatio * timeRatio );
	}
};

class CCircularOutEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = ( currentTime * 1.0f / duration ) - 1;
		return fastSqrt( 1 - timeRatio * timeRatio );
	}
};

class CCircularInOutEasingFunction : public IEasingFunction {
public:
	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = ( currentTime * 2.0f / duration );
		return timeRatio < 1
			? 0.5f * ( 1 - fastSqrt( 1 - timeRatio * timeRatio ) )
			: 0.5f * ( fastSqrt( 1 - ( timeRatio - 2 ) * ( timeRatio - 2 ) ) + 1 );
	}
};

//////////////////////////////////////////////////////////////////////////
// Back easing functions.

class CBackInEasingFunction : public IEasingFunction {
public:
	explicit CBackInEasingFunction( float _curveFactor = 1.70158f ) : curveFactor( _curveFactor ) {}

	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = ( currentTime * 1.0f / duration );
		return timeRatio * timeRatio * ( ( curveFactor + 1 ) * timeRatio - curveFactor );
	}

private:
	float curveFactor;
};

class CBackOutEasingFunction : public IEasingFunction {
public:
	explicit CBackOutEasingFunction( float _curveFactor = 1.70158f ) : curveFactor( _curveFactor ) {}

	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = ( currentTime * 1.0f / duration ) - 1;
		return ( timeRatio * timeRatio * ( ( curveFactor + 1 ) * timeRatio + curveFactor ) + 1 );
	}

private:
	float curveFactor;
};

class CBackInOutEasingFunction : public IEasingFunction {
public:
	explicit CBackInOutEasingFunction( float _curveFactor = 1.70158f ) : adjustedCurveFactor( _curveFactor * 1.525f ) {}

	virtual float GetTimeRatio( float currentTime, float duration ) const override final {
		const float timeRatio = ( currentTime * 2.0f / duration );
		return timeRatio < 1
			? 0.5f * ( timeRatio * timeRatio * ( ( adjustedCurveFactor + 1 ) * timeRatio - adjustedCurveFactor ) )
			: 0.5f * ( ( timeRatio - 2 ) * ( timeRatio - 2 ) * ( ( adjustedCurveFactor + 1 ) * ( timeRatio - 2 ) + adjustedCurveFactor ) + 2 );
	}

private:
	float adjustedCurveFactor;
};

//////////////////////////////////////////////////////////////////////////

// An ease operation with the given function.
// Type T must have a Lerp function or use the default one.
// The default Lerp function uses operator*( T, float ) and operator+( T, T ).
template <class T, class EaseFunc>
T Ease( float currentTime, float duration, T start, T end, const EaseFunc& easing )
{	
	assert( currentTime >= 0 && currentTime <= duration );
	return Lerp( start, end, easing.GetTimeRatio( currentTime, duration ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

