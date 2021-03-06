#pragma once
#include <Redefs.h>
#include <reassert.h>
// Header for unicode utility constants and functions.

namespace Relib {

namespace Unicode {

//////////////////////////////////////////////////////////////////////////

// Integer codes of starts and ends of UTF16 surrogate pair components.
// Both start and end codes belong to the surrogate pair.
extern const int Utf16SurrogateHiFirst;
extern const int Utf16SurrogateHiLast;
extern const int Utf16SurrogateLoFirst;
extern const int Utf16SurrogateLoLast;

//////////////////////////////////////////////////////////////////////////

// Check if a symbol belongs to a surrogate pair.
inline bool IsSurrogateHi( wchar_t ch )
{
	const int charCode = static_cast<int>( ch );
	return charCode <= Utf16SurrogateHiLast && charCode >= Utf16SurrogateHiFirst;
}

inline bool IsSurrogateLo( wchar_t ch )
{
	const int charCode = static_cast<int>( ch );
	return charCode <= Utf16SurrogateLoLast && charCode >= Utf16SurrogateLoFirst;
}

inline bool IsSurrogate( wchar_t ch )
{
	return IsSurrogateHi( ch ) || IsSurrogateLo( ch );
}

// UTF16 to UTF32 conversion.
bool REAPI CanConvertToUTF32( wchar_t ch );
bool REAPI TryConvertUtf16ToUtf32( wchar_t ch, unsigned& result );
bool REAPI TryConvertUtf16ToUtf32( wchar_t hiCh, wchar_t loCh, unsigned& result );

// Convert the first char of a given sequence to UTF32.
// Return the number of symbols used for conversion or 0 in case of an invalid sequence.
int REAPI TryConvertUtf8ToUtf32( const char* str, int length, unsigned& result );
// Return the number of symbols used for conversion or 0 in case of an invalid sequence.
// Result must have at least 4 bytes allocated.
int REAPI TryConvertUtf32ToUtf8( int utf32, char* result );

//////////////////////////////////////////////////////////////////////////

}	// namespace Unicode.

}	// namespace Relib.

