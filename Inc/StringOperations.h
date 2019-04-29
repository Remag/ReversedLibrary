#pragma once
#include <Redefs.h>
#include <CommonStringOperations.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Operations on a string data.
template <class T>
class CStringOperations{};

//////////////////////////////////////////////////////////////////////////

// Specialization for non-unicode strings.
template<>
class REAPI CStringOperations<char> : public CCommonStringOperations<char> {
public:
	static int Length( const char* str );
	static int CompareNoCase( CStringData<char> left, CStringData<char> right );

	using CCommonStringOperations::Find;
	static int Find( CStringData<char> data, CStringData<char> substring, int from );

	static void MakeUpper( char* buffer, int length );
	static void MakeLower( char* buffer, int length );
	static void Reverse( char* buffer );

	static void ConvertStr( CStringData<wchar_t> convertData, unsigned codePage, CBaseString<char>& result );
	static void ConvertStr( CStringData<wchar_t> convertData, unsigned codePage, char defaultChar, CBaseString<char>& result );

	static bool IsCharWhiteSpace( char ch );
	static bool IsCharDigit( char ch );

private:
	static void doConvertStr( CStringData<wchar_t> convertData, unsigned codePage, char* defaultChar, CBaseString<char>& result );
	static DWORD convertCodePageToFlags( unsigned codePage );
};

//////////////////////////////////////////////////////////////////////////

// Specialization for unicode strings.
template<>
class REAPI CStringOperations<wchar_t> : public CCommonStringOperations<wchar_t> {
public:
	static int Length( const wchar_t* str );
	static int CompareNoCase( CStringData<wchar_t> left, CStringData<wchar_t> right );

	using CCommonStringOperations::Find;
	static int Find( CStringData<wchar_t> data, CStringData<wchar_t> substring, int from );

	static void MakeUpper( wchar_t* buffer, int length );
	static void MakeLower( wchar_t* buffer, int length );
	static void Reverse( wchar_t* buffer );

	static void ConvertStr( CStringData<char> convertData, unsigned codePage, CBaseString<wchar_t>& result );

	static bool IsCharWhiteSpace( wchar_t ch );
	static bool IsCharDigit( wchar_t ch );

private:

	static DWORD convertCodePageToFlags( unsigned codePage );
};

//////////////////////////////////////////////////////////////////////////

inline bool CStringOperations<char>::IsCharWhiteSpace( char ch )
{
	return ::isspace( static_cast<unsigned char>( ch ) ) != 0;
}

inline bool CStringOperations<char>::IsCharDigit( char ch )
{
	return ::isdigit( static_cast<unsigned char>( ch ) ) != 0;
}

//////////////////////////////////////////////////////////////////////////

inline bool CStringOperations<wchar_t>::IsCharWhiteSpace( wchar_t ch )
{
	return ::iswspace( ch ) != 0;
}

inline bool CStringOperations<wchar_t>::IsCharDigit( wchar_t ch )
{
	return ::iswdigit( ch ) != 0;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.

