#include <UnicodeUtils.h>

namespace Relib {

namespace Unicode {

//////////////////////////////////////////////////////////////////////////

extern const int Utf16SurrogateHiFirst = 0xD800;
extern const int Utf16SurrogateHiLast = 0xDBFF;
extern const int Utf16SurrogateLoFirst = 0xDC00;
extern const int Utf16SurrogateLoLast = 0xDFFF;

bool CanConvertToUTF32( wchar_t ch )
{
	return !IsSurrogate( ch );
}

bool TryConvertUtf16ToUtf32( wchar_t ch, int& result )
{
	staticAssert( sizeof( wchar_t ) == 2 );
	// Part of a surrogate pair cannot be translated.
	if( IsSurrogate( ch ) ) {
		return false;
	}

	result = static_cast<int>( ch );
	return true;
}

static const int utf16HalfShift = 10;
static const int utf16HalfBase = 0x0010000UL;
bool TryConvertUtf16ToUtf32( wchar_t hiCh, wchar_t loCh, int& result )
{
	staticAssert( sizeof( wchar_t ) == 2 );
	// Only parts of a surrogate pair can be translated.
	if( !IsSurrogateHi( hiCh ) || !IsSurrogateLo( loCh ) ) {
		return false;
	}

	const int hiResultCode = static_cast<int>( hiCh );
	const int loResultCode = static_cast<int>( loCh );
	result = ( ( hiResultCode - Unicode::Utf16SurrogateHiFirst ) << utf16HalfShift )
		+ ( loResultCode - Unicode::Utf16SurrogateLoFirst ) + utf16HalfBase;
	return true;
}

//////////////////////////////////////////////////////////////////////////

const unsigned char utf8OneByteMask = 0x80;
const unsigned char utf8TwoBytesMask = 0xE0;
const unsigned char utf8ThreeBytesMask = 0xF0;
const unsigned char utf8FourBytesMask = 0xF8;

const unsigned char utf8TwoBytesMarker = 0xC0;
const unsigned char utf8ThreeBytesMarker = 0xE0;
const unsigned char utf8FourBytesMarker = 0xF0;
static int getUtf8ByteCount( const char firstByte, int length )
{
	if( ( firstByte & utf8OneByteMask ) == 0 ) {
		return 1;
	} else if( ( firstByte & utf8TwoBytesMask ) == utf8TwoBytesMarker ) {
		return length >= 2 ? 2 : 0;
	} else if( ( firstByte & utf8ThreeBytesMask ) == utf8ThreeBytesMarker ) {
		return length >= 3 ? 3 : 0;
	} else if( ( firstByte & utf8FourBytesMask ) == utf8FourBytesMarker ) {
		return length >= 4 ? 4 : 0;
	} else {
		return 0;
	}
}


const int utf8ContinuationInvMask = 0x3f;
int TryConvertUtf8ToUtf32( const char* str, int length, int& result )
{
	const auto byteCount = getUtf8ByteCount( str[0], length );

	switch( byteCount ) {
		case 1:
			result = str[0];
			return 1;
		case 2:
			result = ( str[0] & ~utf8TwoBytesMask ) << 6 
				| ( str[1] & utf8ContinuationInvMask );
			return 2;
		case 3:
			result = ( str[0] & ~utf8ThreeBytesMask ) << 12 
				| ( str[1] & utf8ContinuationInvMask ) << 6
				| ( str[2] & utf8ContinuationInvMask );
			return 3;
		case 4:
			result = ( static_cast<unsigned char>( str[0] ) & ~utf8FourBytesMask ) << 18
				| ( static_cast<unsigned char>( str[1] ) & utf8ContinuationInvMask ) << 12
				| ( static_cast<unsigned char>( str[2] ) & utf8ContinuationInvMask ) << 6
				| ( static_cast<unsigned char>( str[3] ) & utf8ContinuationInvMask );
			return 4;
		default:
			result = 0;
			return 0;
	}
}

const int utf32OneByteValueSize = 0x80;
const int utf32TwoBytesValueSize = 0x800;
const int utf32ThreeBytesValueSize = 0x10000;
int TryConvertUtf32ToUtf8( int utf32, char* result )
{
	if( utf32 < utf32OneByteValueSize ) {
		result[0] = static_cast<char>( utf32 );
		return 1;
	} else if( utf32 < utf32TwoBytesValueSize ) {
		result[0] = static_cast<char>( ( utf32 >> 6 ) | utf8TwoBytesMarker );
		result[1] = static_cast<char>( ( utf32 & utf8ContinuationInvMask ) | utf8OneByteMask );
		return 2;
	} else if( utf32 < utf32ThreeBytesValueSize ) {
		result[0] = static_cast<char>( ( utf32 >> 12 ) | utf8ThreeBytesMarker );
		result[1] = static_cast<char>( ( ( utf32 >> 6 ) & utf8ContinuationInvMask ) | utf8OneByteMask );
		result[2] = static_cast<char>( ( utf32 & utf8ContinuationInvMask ) | utf8OneByteMask );
		return 3;
	} else {
		result[0] = static_cast<char>( ( utf32 >> 18 ) | utf8FourBytesMarker );
		result[1] = static_cast<char>( ( ( utf32 >> 12 ) & utf8ContinuationInvMask ) | utf8OneByteMask );
		result[2] = static_cast<char>( ( ( utf32 >> 6 ) & utf8ContinuationInvMask ) | utf8OneByteMask );
		result[3] = static_cast<char>( ( utf32 & utf8ContinuationInvMask ) | utf8OneByteMask );
		return 4;
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Unicode.

}	// namespace Relib.
