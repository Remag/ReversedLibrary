#pragma once
#include <Redefs.h>
#include <Reassert.h>
#include <Remath.h>
#include <Vector.h>
#include <Interval.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Random number generator. Capable of generating random integers and floats within the given bounds.
// Uses XorShift RNGs by George Marsaglia.
class CRandomGenerator {
public:
	CRandomGenerator( unsigned __int64 seed = __rdtsc() );

	void NewSeed( unsigned __int64 newSeed = __rdtsc() );

	// Get the unbound random value.
	unsigned __int64 NextRandomValue();
	
	// Create a randomized seed for another random generator.
	// These method is needed because seeding with NextRandomValue will result in two generators creating the same random sequence.
	unsigned __int64 RandomSeed();

	// Flip a coin.
	bool RandomBool();
	// Boolean with a weighted chance of true.
	bool RandomBool( float trueChance );

	// Random numbers of various types.
	int RandomNumber( int minVal, int maxVal );
	float RandomNumber( float minVal, float maxVal );
	double RandomNumber( double minVal, double maxVal );

	template <class IntervalType>
	IntervalType RandomNumber( CInterval<IntervalType> range );

	template <class EnumType>
	EnumType RandomEnum( EnumType minVal, EnumType maxVal );

	// Random unit direction in 2D space within a given angle range in radians.
	template <class T>
	CVector2<T> RandomDirection( T minAngleRad, T maxAngleRad );

private:
	// Initial value in a random sequence.
	unsigned __int64 seed;

	void updateSeed();
};

//////////////////////////////////////////////////////////////////////////

inline CRandomGenerator::CRandomGenerator( unsigned __int64 _seed /*= __rdtsc() */ ) :
	seed( _seed == 0 ? 1 : _seed )
{
}

inline void CRandomGenerator::NewSeed( unsigned __int64 newSeed /*= __rdtsc() */ )
{
	seed = newSeed;
}

inline unsigned __int64 CRandomGenerator::NextRandomValue()
{
	updateSeed();
	return seed;
}

inline unsigned __int64 CRandomGenerator::RandomSeed()
{
	updateSeed();
	// XOR shift the seed using the different triplet. This way the result seed will produce a different sequence.
	auto result = seed;
	static const unsigned firstShift = 5;
	static const unsigned secondShift = 17;
	static const unsigned thirdShift = 11;
	result ^= result << firstShift;
	result ^= result >> secondShift;
	result ^= result << thirdShift;

	return result;
}

inline bool CRandomGenerator::RandomBool()
{
	updateSeed();
	return seed < _UI64_MAX / 2;
}

inline bool CRandomGenerator::RandomBool( float trueChance )
{
	return RandomNumber( 0.0f, 1.0f ) < trueChance;
}

inline int CRandomGenerator::RandomNumber( int minVal, int maxVal )
{
	assert( maxVal >= minVal );
	updateSeed();
	const unsigned seed32 = seed & 0x7FFFFFFF;
	const unsigned delta = maxVal - minVal;
	// Calculate a random number in the range [0; maxVal - minVal].
	//	randomDelta == ( delta + 1 ) * ( seed32 / seed32_maximum ) ~= ( delta * seed32 + seed32 ) / 2**31
	const unsigned randomDelta = static_cast<unsigned>( ( __emulu( seed32, delta ) + seed32 ) >> 31 );
	return randomDelta + minVal;
}

inline float CRandomGenerator::RandomNumber( float minVal, float maxVal )
{
	assert( maxVal >= minVal );
	updateSeed();
	const float max32BitValueInv = 1.f / _UI32_MAX;
	const unsigned seed32 = seed & 0xFFFFFFFF;
	const float randomMultiplier =  seed32 * max32BitValueInv;
	const float delta = maxVal - minVal;
	return randomMultiplier * delta + minVal;
}

inline double CRandomGenerator::RandomNumber( double minVal, double maxVal )
{
	assert( maxVal >= minVal );
	updateSeed();
	const double max64BitValueInv = 1. / _UI64_MAX;
	const double randomMultiplier =  seed * max64BitValueInv;
	const double delta = maxVal - minVal;
	return randomMultiplier * delta + minVal;
}

template <class IntervalType>
IntervalType CRandomGenerator::RandomNumber( CInterval<IntervalType> range )
{
	return RandomNumber( range.GetLower(), range.GetUpper() );
}

template <class EnumType>
EnumType CRandomGenerator::RandomEnum( EnumType minVal, EnumType maxVal )
{
	return EnumType( RandomNumber( minVal, maxVal ) );
}

template <class T>
inline CVector2<T> CRandomGenerator::RandomDirection( T minAngleRad, T maxAngleRad )
{
	const auto angle = RandomNumber( minAngleRad, maxAngleRad );
	T dirX;
	T dirY;
	relibSinCos( angle, dirX, dirY );
	return CVector2<T>{ dirX, dirY };
}

inline void CRandomGenerator::updateSeed()
{
	// Triplet used in XorShift.
	static const unsigned firstShift = 13;
	static const unsigned secondShift = 7;
	static const unsigned thirdShift = 17;

	seed ^= seed << firstShift;
	seed ^= seed >> secondShift;
	seed ^= seed << thirdShift;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

