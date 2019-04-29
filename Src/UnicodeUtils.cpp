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

bool TryConvertWideToInt( wchar_t ch, int& result )
{
	staticAssert( sizeof( wchar_t ) == 2 );
	// Part of a surrogate pair cannot be translated.
	if( IsSurrogate( ch ) ) {
		return false;
	}

	result = static_cast<int>( ch );
	return true;
}

static const int utf16HalfShift  = 10;
static const int utf16HalfBase = 0x0010000UL;
bool TryConvertWideToInt( wchar_t hiCh, wchar_t loCh, int& result )
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

const int utf8OneByteMask = 0x80;
const int utf8TwoBytesMask = 0xE0;
const int utf8ThreeBytesMask = 0xF0;
const int utf8FourBytesMask = 0xF8;

const int utf8TwoBytesMarker = 0xC0;
const int utf8ThreeBytesMarker = 0xE0;
const int utf8FourBytesMarker = 0xF0;
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


const int utf8ContinuationInvMask = ~0xC0;
int TryConvertUtf8ToInt( const char* str, int length, int& result )
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
			result = ( str[0] & ~utf8TwoBytesMask ) << 12 
				| ( str[1] & utf8ContinuationInvMask ) << 6
				| ( str[2] & utf8ContinuationInvMask );
			return 3;
		case 4:
			result = ( str[0] & ~utf8TwoBytesMask ) << 18
				| ( str[1] & utf8ContinuationInvMask ) << 12
				| ( str[2] & utf8ContinuationInvMask ) << 6
				| ( str[3] & utf8ContinuationInvMask );
			return 4;
		default:
			result = ( str[0] & ~utf8TwoBytesMask ) << 6 
				| ( str[1] & utf8ContinuationInvMask );
			return 0;
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Unicode.

}	// namespace Relib.
