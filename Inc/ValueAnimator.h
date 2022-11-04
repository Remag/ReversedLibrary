#pragma once
#include <Redefs.h>
#include <TemplateUtils.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Mechanism doing a smooth change between two values.
// EasingFunction is used for smoothing.
template <class ValueType, class EasingFunction>
class CValueAnimator {
public:
	CValueAnimator( ValueType _leftValue, ValueType _rightValue, int _duration );

	ValueType GetLeftValue() const
		{ return leftValue; }
	ValueType GetRightValue() const
		{ return rightValue; }
	void SetValueRange( ValueType leftValue, ValueType rightValue );

	// Animation duration.
	int GetDuration() const
		{ return duration; }
	void SetDuration( int newValue );

	// Current value.
	ValueType GetCurrentValue() const
		{ return currentValue; }
	// Set the value corresponding to a given animation time.
	void SetCurrentTime( int animationTime );

	// Notify the passing of time.
	void Update( int timePassed );

	// Start the animation in a certain direction.
	void AnimateForward();
	void AnimateBack();
	// Stop the animation abruptly.
	void StopAnimation();
	// Is an animation being performed right now.
	bool IsAnimating() const;
	
private:
	// Animation easing.
	EasingFunction easing;
	// Value range.
	ValueType leftValue;
	ValueType rightValue;
	// Value that is being changed.
	ValueType currentValue;
	// Current animation position.
	int currentTime = 0;
	// Full animation duration;
	int duration = 0;
	// Animation direction. Can be -1 for back animation, 1 for forward animation and 0 for no animation.
	int animationChange = 0;
};

//////////////////////////////////////////////////////////////////////////

template <class ValueType, class EasingFunction>
CValueAnimator<ValueType, EasingFunction>::CValueAnimator( ValueType _leftValue, ValueType _rightValue, int _duration ) :
	leftValue( _leftValue ),
	rightValue( _rightValue ),
	duration( _duration ),
	currentValue( _leftValue ),
	currentTime( 0 )
{
}

template <class ValueType, class EasingFunction>
void CValueAnimator<ValueType, EasingFunction>::SetValueRange( ValueType _leftValue, ValueType _rightValue )
{
	leftValue = _leftValue;
	rightValue = _rightValue;
	currentTime = Clamp( currentValue, _leftValue, _rightValue );
}

template <class ValueType, class EasingFunction>
void CValueAnimator<ValueType, EasingFunction>::SetDuration( int newValue )
{
	assert( newValue >= 0 );
	duration = newValue;
	currentTime = min( currentTime, duration );
}

template <class ValueType, class EasingFunction>
void CValueAnimator<ValueType, EasingFunction>::SetCurrentTime( int animationTime )
{
	assert( animationTime >= 0 && animationTime <= duration );
	StopAnimation();
	currentTime = animationTime;
	currentValue = Ease( currentTime, duration, leftValue, rightValue, easing );
}

template <class ValueType, class EasingFunction>
void CValueAnimator<ValueType, EasingFunction>::Update( int timePassed )
{
	assert( currentTime >= 0 && currentTime <= duration );
	currentTime += timePassed * animationChange;

	if( currentTime <= 0 ) {
		currentValue = leftValue;
		animationChange = 0;
		currentTime = 0;
	} else if( currentTime >= duration ) {
		currentValue = rightValue;
		animationChange = 0;
		currentTime = duration;
	} else {
		currentValue = Ease( currentTime, duration, leftValue, rightValue, easing );
	}
}

template <class ValueType, class EasingFunction>
void CValueAnimator<ValueType, EasingFunction>::AnimateForward()
{
	animationChange = 1;
}

template <class ValueType, class EasingFunction>
void CValueAnimator<ValueType, EasingFunction>::AnimateBack()
{
	animationChange = -1;
}

template <class ValueType, class EasingFunction>
void CValueAnimator<ValueType, EasingFunction>::StopAnimation()
{
	animationChange = 0;
}

template <class ValueType, class EasingFunction>
bool CValueAnimator<ValueType, EasingFunction>::IsAnimating() const
{
	return animationChange != 0;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

