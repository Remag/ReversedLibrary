#include <HashUtils.h>
#include <Reassert.h>
#include <Remath.h>
#include <BaseStringView.h>
#include <BaseString.h>
#include <StrConversions.h>

namespace Relib {

int GetUnicodeHash( CUnicodePart string )
{
	const int strLength = string.Length();
	if( strLength == 0 ) {
		return 0;
	}

	int result = string[0];
	for( int i = 1; i < strLength; i++ ) {
		result = CombineHashKey( result, numeric_cast<int>( string[i] ) );		
	}

	return result;
}

int GetUnicodeHash( const wchar_t* string )
{
	if( *string == 0 ) {
		return 0;
	}

	int result = *string;
	for( int i = 1; string[i] != 0; i++ ) {
		result = CombineHashKey( result, numeric_cast<int>( string[i] ) );		
	}

	return result;
}

int GetCaselessUnicodeHash( CUnicodePart key )
{
	const int length = key.Length();

	if( length == 0 ) {
		return 0;
	}

	const auto firstCh = key[0];
	int result = ::towupper( firstCh );

	for( int i = 1; i < length; i++ ) {
		const auto ch = key[i];
		result = CombineHashKey( result, static_cast<int>( ::towupper( ch ) ) );
	}

	return result;
}

int GetStringHash( CStringPart string )
{
	const int strLength = string.Length();
	if( strLength == 0 ) {
		return 0;
	}

	int result = string[0];
	for( int i = 1; i < strLength; i++ ) {
		result = CombineHashKey( result, numeric_cast<int>( string[i] ) );		
	}

	return result;
}

int GetStringHash( const char* string )
{
	if( *string == 0 ) {
		return 0;
	}

	int result = *string;
	for( int i = 1; string[i] != 0; i++ ) {
		result = CombineHashKey( result, numeric_cast<int>( string[i] ) );		
	}

	return result;
}

int GetCaselessStringHash( CStringPart key )
{
	const int length = key.Length();

	if( length == 0 ) {
		return 0;
	}

	const auto firstCh = key[0];
	int result = ::toupper( firstCh );

	for( int i = 1; i < length; i++ ) {
		const auto ch = key[i];
		result = CombineHashKey( result, static_cast<int>( ::toupper( ch ) ) );
	}

	return result;
}

int GetMemoryBlockHash( const void* ptr, int size )
{
	int hashKey = 0;
	const DWORD* dwordPtr = reinterpret_cast<const DWORD*>( ptr );
	int i = 0;
	for( ; i < numeric_cast<int>( size / sizeof( DWORD ) ); i++ ) {
		hashKey = CombineHashKey( hashKey, dwordPtr[i] );
	}
	// Add a few last bytes at the end of ptr.
	const BYTE* bytePtr = reinterpret_cast<const BYTE*>( dwordPtr + i );
	for( i = 0; i < numeric_cast<int>( size % sizeof( DWORD ) ); i++ ) {
		hashKey = CombineHashKey( hashKey, bytePtr[i] );
	}
	return hashKey;
}

const unsigned fnvInitialValue = 2166136261;
const unsigned fnvMultiplier = 16777619;
int GetStrongUnicodeHash( const wchar_t* string )
{
	unsigned result = fnvInitialValue;
	for( int i = 0; string[i] != 0; i++ ) {
		result ^= string[i];
		result *= fnvMultiplier;
	}
	return result;
}

int GetStrongUnicodeHash( CUnicodePart string )
{
	unsigned result = fnvInitialValue;
	for( auto ch : string ) {
		result ^= ch;
		result *= fnvMultiplier;
	}
	return result;
}

int GetStrongStringHash( const char* string )
{
	unsigned result = fnvInitialValue;
	for( int i = 0; string[i] != 0; i++ ) {
		result ^= string[i];
		result *= fnvMultiplier;
	}
	return result;
}

int GetStrongStringHash( CStringPart string )
{
	unsigned result = fnvInitialValue;
	for( auto ch : string ) {
		result ^= ch;
		result *= fnvMultiplier;
	}
	return result;
}

// Prime hash table size helps to avoid collisions.
// Values are prime numbers with values roughly doubling.
static const int possibleHashTableSizes[] =
{
	29, 59, 127, 233, 397, 769,
	1549, 3079, 6211, 12097, 24571,
	47629, 99371, 193939, 391939, 800011,
	1629013, 3202411, 6444847, 12835409, 25165843,
	49979693, 104395303, 217645199, 413158523, 817504253, 1600000009
};

int GetPrimeHashTableSize( int hashTableSize )
{
	for( int i = 0; i < _countof( possibleHashTableSizes ); i++ ) {
		if( possibleHashTableSizes[i] >= hashTableSize ) {
			return possibleHashTableSizes[i];
		}
	}
	assert( false );
	return INT_MAX;
}

int GetPow2HashTableSize( int hashTableSize )
{
	int result = hashTableSize - 1;
	result |= result >> 1;
	result |= result >> 2;
	result |= result >> 4;
	result |= result >> 8;
	result |= result >> 16;
	return max( result + 1, 32 );
}

//////////////////////////////////////////////////////////////////////////

} // namespace Relib.