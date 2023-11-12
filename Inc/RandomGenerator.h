#pragma once
#include <Interval.h>
#include <Reassert.h>
#include <Redefs.h>
#include <Remath.h>
#include <Vector.h>

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

	// Random numbers of various types. Both edge values can be returned.
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

	// Randomly shuffle elements in the given buffer.
	template <class T>
	void Shuffle( CArrayBuffer<T> elements );

	// Uniformly choose between a given range of options.
	template <class T>
	T Choose( std::initializer_list<T> variants );

private:
	// Initial value in a random sequence.
	unsigned __int64 seed;

	void updateSeed();
};

//////////////////////////////////////////////////////////////////////////

inline CRandomGenerator::CRandomGenerator( unsigned __int64 _seed /*= __rdtsc() */ )
	: seed( _seed == 0 ? 1 : _seed )
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
	static const unsigned firstShift = 17;
	static const unsigned secondShift = 23;
	static const unsigned thirdShift = 29;
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

inline double CRandomGenerator::RandomNumber( double minVal, double maxVal )
{
	assert( maxVal >= minVal );
	updateSeed();

	// "A standard double (64-bit) floating-point number in IEEE floating point format has 52 bits of significand, plus an implicit bit at the left of the significand. 
	// Thus, the representation can actually store numbers with 53 significant binary digits."
	// See https://prng.di.unimi.it/
	const auto randomMultiplier = ( seed >> 11 ) * 0x1.0p-53;
	const auto delta = maxVal - minVal;
	return randomMultiplier * delta + minVal;
}

inline float CRandomGenerator::RandomNumber( float minVal, float maxVal )
{
	assert( maxVal >= minVal );
	updateSeed();

	// Same as double, but with 24 bits of precision for floats.
	const auto randomMultiplier = ( seed >> 40 ) * 0x1.0p-24f;

	const float delta = maxVal - minVal;
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

template <class T>
inline void CRandomGenerator::Shuffle( CArrayBuffer<T> elements )
{
	// Fisher-Yates shuffle.
	for( int i = elements.Size() - 1; i >= 1; i-- ) {
		const auto swapIndex = RandomNumber( 0, i );
		swap( elements[swapIndex], elements[i] );
	}
}

template <class T>
inline T CRandomGenerator::Choose( std::initializer_list<T> variants )
{
	const auto index = RandomNumber( 0, variants.size() - 1 );
	return variants.begin()[index];
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

}	 // namespace Relib.
