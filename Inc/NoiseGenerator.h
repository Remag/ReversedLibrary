#pragma once
#include <RandomGenerator.h>
#include <Vector.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Mechanism for creating smooth arrays with random values.
template <class ValueType>
class CGradientNoise {
public:
	explicit CGradientNoise( unsigned __int64 seed = __rdtsc() );
	void NewSeed( unsigned __int64 seed = __rdtsc() );

	// Noise functions.
	// Value range of the functions lies within (-1, 1)[smaller, actually].

	// A two dimensional gradient noise function.
	ValueType Noise( CVector2<ValueType> pos ) const;
	// A three dimensional gradient noise function.
	ValueType Noise( CVector3<ValueType> pos ) const;
	
private:
	// Grid with random values.
	static const int tableSize = 256;
	BYTE permTable[tableSize + 1];

	void populateGrid( unsigned __int64 seed );

	static ValueType fade( ValueType value );
	static ValueType gradValue( BYTE seed, ValueType x, ValueType y, ValueType z );

	// Copying is prohibited.
	CGradientNoise( CGradientNoise& ) = delete;
	void operator=( CGradientNoise& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class ValueType>
CGradientNoise<ValueType>::CGradientNoise( unsigned __int64 seed /*= __rdtsc() */ )
{
	populateGrid( seed );
}

template <class ValueType>
void CGradientNoise<ValueType>::NewSeed( unsigned __int64 seed /*= __rdtsc() */ )
{
	populateGrid( seed );
}

template <class ValueType>
void CGradientNoise<ValueType>::populateGrid( unsigned __int64 seed )
{
	CRandomGenerator rng( seed );
	for( int i = 0; i < tableSize + 1; i++ ) {
		permTable[i] = numeric_cast<BYTE>( rng.RandomNumber( 0, tableSize - 1 ) );
	}
}

template <class ValueType>
ValueType CGradientNoise<ValueType>::Noise( CVector2<ValueType> pos ) const
{
	const ValueType x = pos.X();
	const ValueType y = pos.Y();

	const int maxTableValue = tableSize - 1;
	const int xFloored = x >= 0 ? Floor( x ) : Floor( x ) - 1;
	const int yFloored = y >= 0 ? Floor( y ) : Floor( y ) - 1;

	// Grid values.
	const int xGrid = x >= 0 ? xFloored & maxTableValue : tableSize - ( -xFloored & maxTableValue );
	const int yGrid = y >= 0 ? yFloored & maxTableValue : tableSize - ( -yFloored & maxTableValue );

	// Create a random value for every corner in the grid.
	// T - top; B - bottom.
	// L - left; R - right.

	const BYTE valueL = permTable[xGrid];
	const int valueTLIndex = valueL + yGrid;
	const int valueBLIndex = valueTLIndex + 1;

	const BYTE valueTL = permTable[valueTLIndex & maxTableValue];
	const BYTE valueBL = permTable[valueBLIndex & maxTableValue];

	const BYTE valueR = permTable[xGrid + 1];
	const int valueTRIndex = valueR + yGrid;
	const int valueBRIndex = valueTRIndex + 1;

	const BYTE valueTR = permTable[valueTRIndex & maxTableValue];
	const BYTE valueBR = permTable[valueBRIndex & maxTableValue];

	// XYZ values relative to the grid.
	const ValueType xRel = x - xFloored;
	const ValueType yRel = y - yFloored;

	// Weighted gradient values.
	const ValueType gradValueTL = gradValue( valueTL, xRel, yRel, 0 );
	const ValueType gradValueTR = gradValue( valueTR, xRel - 1, yRel, 0 );
	const ValueType gradValueBL = gradValue( valueBL, xRel, yRel - 1, 0 );
	const ValueType gradValueBR = gradValue( valueBR, xRel - 1, yRel - 1, 0 );

	// Fade curves for XYZ.
	const ValueType xFaded = fade( xRel );
	const ValueType yFaded = fade( yRel );
	
	// Interpolated values.
	const ValueType lerpT = Lerp( gradValueTL, gradValueTR, xFaded );
	const ValueType lerpB = Lerp( gradValueBL, gradValueBR, xFaded );

	const ValueType resultLerp = Lerp( lerpT, lerpB, yFaded );
	return resultLerp;
}

template <class ValueType>
ValueType CGradientNoise<ValueType>::Noise( CVector3<ValueType> pos ) const
{
	const ValueType x = pos.X();
	const ValueType y = pos.Y();
	const ValueType z = pos.Z();

	const int maxTableValue = tableSize - 1;
	const int xFloored = x >= 0 ? Floor( x ) : Floor( x ) - 1;
	const int yFloored = y >= 0 ? Floor( y ) : Floor( y ) - 1;
	const int zFloored = z >= 0 ? Floor( z ) : Floor( z ) - 1;

	// Grid values.
	const int xGrid = x >= 0 ? xFloored & maxTableValue : tableSize - ( -xFloored & maxTableValue );
	const int yGrid = y >= 0 ? yFloored & maxTableValue : tableSize - ( -yFloored & maxTableValue );
	const int zGrid = z >= 0 ? zFloored & maxTableValue : tableSize - ( -zFloored & maxTableValue );

	// Create a random value for every corner in the grid.
	// C - close; F - far.
	// T - top; B - bottom.
	// L - left; R - right.

	const BYTE valueL = permTable[xGrid];
	const int valueTLIndex = valueL + yGrid;
	const int valueBLIndex = valueTLIndex + 1;

	const BYTE valueTL = permTable[valueTLIndex & maxTableValue];
	const BYTE valueBL = permTable[valueBLIndex & maxTableValue];

	const int valueCTLIndex = valueTL + zGrid;
	const int valueFTLIndex = valueCTLIndex + 1;
	const int valueCBLIndex = valueBL + zGrid;
	const int valueFBLIndex = valueCBLIndex + 1;

	const BYTE valueR = permTable[xGrid + 1];
	const int valueTRIndex = valueR + yGrid;
	const int valueBRIndex = valueTRIndex + 1;

	const BYTE valueTR = permTable[valueTRIndex & maxTableValue];
	const BYTE valueBR = permTable[valueBRIndex & maxTableValue];

	const int valueCTRIndex = valueTR + zGrid;
	const int valueFTRIndex = valueCTRIndex + 1;
	const int valueCBRIndex = valueBR + zGrid;
	const int valueFBRIndex = valueCBRIndex + 1;

	const BYTE valueCTL = permTable[valueCTLIndex & maxTableValue];
	const BYTE valueCTR = permTable[valueCTRIndex & maxTableValue];
	const BYTE valueCBL = permTable[valueCBLIndex & maxTableValue];
	const BYTE valueCBR = permTable[valueCBRIndex & maxTableValue];

	const BYTE valueFTL = permTable[valueFTLIndex & maxTableValue];
	const BYTE valueFTR = permTable[valueFTRIndex & maxTableValue];
	const BYTE valueFBL = permTable[valueFBLIndex & maxTableValue];
	const BYTE valueFBR = permTable[valueFBRIndex & maxTableValue];

	// XYZ values relative to the grid.
	const ValueType xRel = x - xFloored;
	const ValueType yRel = y - yFloored;
	const ValueType zRel = z - zFloored;

	// Weighted gradient values.
	const ValueType gradValueCTL = gradValue( valueCTL, xRel, yRel, zRel );
	const ValueType gradValueCTR = gradValue( valueCTR, xRel - 1, yRel, zRel );
	const ValueType gradValueCBL = gradValue( valueCBL, xRel, yRel - 1, zRel );
	const ValueType gradValueCBR = gradValue( valueCBR, xRel - 1, yRel - 1, zRel );

	const ValueType gradValueFTL = gradValue( valueFTL, xRel, yRel, zRel - 1 );
	const ValueType gradValueFTR = gradValue( valueFTR, xRel - 1, yRel, zRel - 1 );
	const ValueType gradValueFBL = gradValue( valueFBL, xRel, yRel - 1, zRel - 1 );
	const ValueType gradValueFBR = gradValue( valueFBR, xRel - 1, yRel - 1, zRel - 1 );

	// Fade curves for XYZ.
	const ValueType xFaded = fade( xRel );
	const ValueType yFaded = fade( yRel );
	const ValueType zFaded = fade( zRel );
	
	// Interpolated values.
	const ValueType lerpCT = Lerp( gradValueCTL, gradValueCTR, xFaded );
	const ValueType lerpCB = Lerp( gradValueCBL, gradValueCBR, xFaded );
	const ValueType lerpFT = Lerp( gradValueFTL, gradValueFTR, xFaded );
	const ValueType lerpFB = Lerp( gradValueFBL, gradValueFBR, xFaded );

	const ValueType lerpC = Lerp( lerpCT, lerpCB, yFaded );
	const ValueType lerpF = Lerp( lerpFT, lerpFB, yFaded );

	const ValueType resultLerp = Lerp( lerpC, lerpF, zFaded );
	return resultLerp;
}

template <class ValueType>
ValueType CGradientNoise<ValueType>::fade( ValueType value )
{
	// Value of the faded curve is taken from Ken Perlin's paper "Improving Noise".
	// http://mrl.nyu.edu/~perlin/paper445.pdf

	// 6 * value^5 - 15 * value^4 + 10 * value^3.
	return value * value * value * ( value * ( value * 6 - 15 ) + 10 );
}

// Calculate a dot product with one of the gradient vectors described by seed.
// Possible gradient vectors can equal one of the 12 possible values described in Perlin's paper.
// Gradient function optimization is used: http://riven8192.blogspot.com/2010/08/calculate-perlinnoise-twice-as-fast.html
template <class ValueType>
ValueType CGradientNoise<ValueType>::gradValue( BYTE seed, ValueType x, ValueType y, ValueType z )
{
	switch( seed & 15 ) {
		case 0:
			return x + y;
		case 1:
			return -x + y;
		case 2:
			return x - y;
		case 3:
			return -x - y;
		case 4:
			return x + z;
		case 5:
			return -x + z;
		case 6:
			return x - z;
		case 7:
			return -x - z;
		case 8:
			return y + z;
		case 9:
			return -y + z;
		case 10:
			return y - z;
		case 11:
			return -y - z;
		case 12:
			return y + x;
		case 13:
			return -y + z;
		case 14:
			return y - x;
		case 15:
			return -y - z;
		default:
			assert( false );
			return 0;
    }
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.


















