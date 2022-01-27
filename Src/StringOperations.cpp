#include <StringOperations.h>
#include <StringData.h>
#include <Remath.h>
#include <string.h>
#include <BitSet.h>
#include <UnicodeSet.h>
#include <BaseString.h>
#include <StrConversions.h>
#include <mbstring.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

int CStringOperations<char>::Length( const char* str )
{
	return static_cast<int>( ::strlen( str ) );
}

int CStringOperations<char>::CompareNoCase( CStringData<char> left, CStringData<char> right )
{
	const int leftLen = left.Length();
	const unsigned char* leftBuf = reinterpret_cast<const unsigned char*>( left.begin() );
	const int rightLen = right.Length();
	const unsigned char* rightBuf = reinterpret_cast<const unsigned char*>( right.begin() );

	if( leftLen < rightLen ) {
		const int compareResult = ::_mbsnicmp( leftBuf, rightBuf, leftLen );
		return compareResult == 0 ? -1 : compareResult;
	} else if( leftLen > rightLen ) {
		const int compareResult = ::_mbsnicmp( leftBuf, rightBuf, rightLen );
		return compareResult == 0 ? 1 : compareResult;
	} else {
		return ::_mbsnicmp( leftBuf, rightBuf, rightLen );
	}
}

int CStringOperations<char>::Find( CStringData<char> data, CStringData<char> substring, int from )
{
	const char* buffer = data.begin();
	assert( from >= 0 && from <= data.Length() );

	const char* findResult = ::strstr( buffer + from, substring.begin() );
	return findResult != nullptr ? numeric_cast<int>( findResult - buffer ) : NotFound;
}

void CStringOperations<char>::MakeUpper( char* buffer, int length )
{
	::_mbsupr_s( reinterpret_cast<unsigned char*>( buffer ), length + 1 );
}

void CStringOperations<char>::MakeLower( char* buffer, int length )
{
	::_mbslwr_s( reinterpret_cast<unsigned char*>( buffer ), length + 1 );
}

void CStringOperations<char>::Reverse( char* buffer )
{
	::_strrev( buffer );
}

void CStringOperations<char>::ConvertStr( CStringData<wchar_t> convertData, unsigned codePage, CBaseString<char>& result )
{
	doConvertStr( convertData, codePage, nullptr, result );
}

void CStringOperations<char>::ConvertStr( CStringData<wchar_t> convertData, unsigned codePage, char defaultChar, CBaseString<char>& result )
{
	doConvertStr( convertData, codePage, &defaultChar, result );
}

void CStringOperations<char>::doConvertStr( CStringData<wchar_t> convertData, unsigned codePage, char* defaultChar, CBaseString<char>& result )
{
	assert( result.IsEmpty() );
	const int length = convertData.Length();
	const wchar_t* buffer = convertData.begin();

	if( codePage == CP_ACP ) {
		codePage = ::GetACP();
	} else if( codePage == CP_OEMCP ) {
		codePage = ::GetOEMCP();
	}

	const DWORD conversionFlags = convertCodePageToFlags( codePage );
	const int requiredLength = ::WideCharToMultiByte( codePage, conversionFlags, buffer, length, 0, 0, defaultChar, nullptr );

	if( requiredLength >= 0 ) {
		auto resultBuffer = result.CreateRawBuffer( requiredLength );
		::WideCharToMultiByte( codePage, conversionFlags, buffer, length, resultBuffer, requiredLength, defaultChar, nullptr );
		resultBuffer.Release( requiredLength );
	} 
}

DWORD CStringOperations<char>::convertCodePageToFlags( unsigned codePage )
{
	if( checkSpecialCodePage( codePage ) ) {
		return 0;
	}

	return WC_COMPOSITECHECK | WC_SEPCHARS;
}

//////////////////////////////////////////////////////////////////////////

int CStringOperations<wchar_t>::Length( const wchar_t* str )
{
	return static_cast<int>( ::wcslen( str ) );
}

int CStringOperations<wchar_t>::CompareNoCase( CStringData<wchar_t> left, CStringData<wchar_t> right )
{
	const int leftLen = left.Length();
	const wchar_t* leftBuf = left.begin();
	const int rightLen = right.Length();
	const wchar_t* rightBuf = right.begin();

	if( leftLen < rightLen ) {
		const int compareResult = ::_wcsnicmp( leftBuf, rightBuf, leftLen );
		return compareResult == 0 ? -1 : compareResult;
	} else if( leftLen > rightLen ) {
		const int compareResult = ::_wcsnicmp( leftBuf, rightBuf, rightLen );
		return compareResult == 0 ? 1 : compareResult;
	} else {
		return ::_wcsnicmp( leftBuf, rightBuf, rightLen );
	}
}

int CStringOperations<wchar_t>::Find( CStringData<wchar_t> data, CStringData<wchar_t> substring, int from ) 
{
	const wchar_t* buffer = data.begin();
	assert( from >= 0 && from <= data.Length() );

	const wchar_t* ptr = ::wcsstr( buffer + from, substring.begin() );
	return ptr == nullptr ? NotFound : numeric_cast<int>( ptr - buffer );
}

void CStringOperations<wchar_t>::MakeUpper( wchar_t* buffer, int length )
{
	::_wcsupr_s( buffer, length + 1 ); 
}

void CStringOperations<wchar_t>::MakeLower( wchar_t* buffer, int length )
{
	::_wcslwr_s( buffer, length + 1 );
}

void CStringOperations<wchar_t>::Reverse( wchar_t* buffer )
{
	::_wcsrev( buffer );
}

void CStringOperations<wchar_t>::ConvertStr( CStringData<char> convertData, unsigned codePage, CBaseString<wchar_t>& result )
{
	const int length = convertData.Length();
	const char* buffer = convertData.begin();
	assert( length >= 0 );

	const DWORD flags = convertCodePageToFlags( codePage );
	const int requiredLength = ::MultiByteToWideChar( codePage, flags, buffer, length, 0, 0 );
	if( requiredLength >= 0 ) {
		auto resultBuffer = result.CreateRawBuffer( requiredLength );
		::MultiByteToWideChar( codePage, flags, buffer, length, resultBuffer, requiredLength );
		resultBuffer.Release( requiredLength );
	}
}

DWORD CStringOperations<wchar_t>::convertCodePageToFlags( unsigned codePage )
{
	if( checkSpecialCodePage( codePage ) ) {
		return 0;
	}

	return MB_PRECOMPOSED | MB_ERR_INVALID_CHARS;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

