#pragma once
#include <RawStringBuffer.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

template <class T>
int CCommonStringOperations<T>::Compare( CStringData<T> left, CStringData<T> right )
{
	const T* leftBuffer = left.begin();
	const T* rightBuffer = right.begin();
	const int leftLen = left.Length() * sizeof( T );
	const int rightLen = right.Length() * sizeof( T );

	if( leftLen < rightLen ) {
		const int compareResult = ::memcmp( leftBuffer, rightBuffer, leftLen );
		return compareResult == 0 ? -1 : compareResult;
	} else if( leftLen > rightLen ) {
		const int compareResult = ::memcmp( leftBuffer, rightBuffer, rightLen );
		return compareResult == 0 ? 1 : compareResult;
	} else {
		return ::memcmp( leftBuffer, rightBuffer, leftLen );
	}
}

template <class T>
bool CCommonStringOperations<T>::Equals( CStringData<T> left, CStringData<T> right )
{
	const int leftLen = left.Length();
	const int rightLen = right.Length();
	return leftLen == rightLen && ::memcmp( left.begin(), right.begin(), leftLen * sizeof( T ) ) == 0;
}

template <class T>
int CCommonStringOperations<T>::Find( CStringData<T> data, T symbol, int from )
{
	const int length = data.Length();
	assert( from >= 0 && from <= length );
	const auto dataPtr = data.begin();
	for( int i = from; i < length; i++ ) {
		if( dataPtr[i] == symbol ) {
			return i;
		}
	}
	return NotFound;
}

template <class T>
int CCommonStringOperations<T>::FindCommon( CStringData<T> data, CStringData<T> substr, int from )
{
	const auto dataPtr = data.begin();
	const auto substrPtr = substr.begin();
	const auto dataLen = data.Length();
	const auto substrLen = substr.Length();
	if( substrLen == 0 ) {
		return from < dataLen ? from : NotFound;
	}
	int dataPos = from;
	int substrPos = 0;
	int currentResultPos = NotFound;
	while( substrPos < substrLen ) {
		if( dataPos >= dataLen ) {
			return NotFound;
		}
		if( dataPtr[dataPos] == substrPtr[substrPos] ) {
			if( currentResultPos == NotFound ) {
				currentResultPos = dataPos;
			}
			substrPos++;
			dataPos++;
		} else {
			substrPos = 0;
			if( currentResultPos != NotFound ) {
				dataPos = currentResultPos + 1;
				currentResultPos = NotFound;
			} else {
				dataPos++;
			}
		}
	}
	return currentResultPos;
}

template <class T>
int CCommonStringOperations<T>::FindNoCase( CStringData<T> data, CStringData<T> substr, int from )
{
	const auto dataPtr = data.begin();
	const auto substrPtr = substr.begin();
	const auto dataLen = data.Length();
	const auto substrLen = substr.Length();
	if( substrLen == 0 ) {
		return from < dataLen ? from : NotFound;
	}
	int dataPos = from;
	int substrPos = 0;
	int currentResultPos = NotFound;
	while( substrPos < substrLen ) {
		if( dataPos >= dataLen ) {
			return NotFound;
		}
		if( ::tolower( static_cast<unsigned char>( dataPtr[dataPos] ) ) == ::tolower( static_cast<unsigned char>( substrPtr[substrPos] ) ) ) {
			if( currentResultPos == NotFound ) {
				currentResultPos = dataPos;
			}
			substrPos++;
			dataPos++;
		} else {
			substrPos = 0;
			if( currentResultPos != NotFound ) {
				dataPos = currentResultPos + 1;
				currentResultPos = NotFound;
			} else {
				dataPos++;
			}
		}
	}
	return currentResultPos;
}

template <class T>
int CCommonStringOperations<T>::ReverseFind( CStringData<T> data, T symbol, int from )
{
	assert( from >= 0 && from <= data.Length() );
	const auto dataPtr = data.begin();
	for( int i = from - 1; i >= 0; i-- ) {
		if( dataPtr[i] == symbol ) {
			return i;
		}
	}
	return NotFound;
}

template <class T>
int CCommonStringOperations<T>::FindOneOf( CStringData<T> data, CStringData<T> charSet, int from )
{
	const int length = data.Length();
	const auto dataPtr = data.begin();
	assert( from >= 0 && from <= length );
	for( int i = from; i < length; i++ ) {
		if( CCommonStringOperations<T>::Find( charSet, dataPtr[i], 0 ) != NotFound ) {
			return i;
		}
	}
	return NotFound;
}

template <class T>
int CCommonStringOperations<T>::ReverseFindOneOf( CStringData<T> data, CStringData<T> charSet, int from )
{
	assert( from >= 0 && from <= data.Length() );
	const auto dataPtr = data.begin();
	for( int i = from - 1; i >= 0; i-- ) {
		if( CCommonStringOperations<T>::Find( charSet, dataPtr[i], 0 ) != NotFound ) {
			return i;
		}
	}
	return NotFound;
}

template <class T>
bool CCommonStringOperations<T>::HasPrefix( CBaseStringPart<T> str, CBaseStringPart<T> prefix )
{
	const auto prefixLength = prefix.Length();
	return str.Length() >= prefixLength && str.Left( prefixLength ) == prefix;
}

template <class T>
bool CCommonStringOperations<T>::HasPrefixNoCase( CBaseStringPart<T> str, CBaseStringPart<T> prefix )
{
	const auto prefixLength = prefix.Length();
	return str.Length() >= prefixLength && str.Left( prefixLength ).EqualsNoCase( prefix );
}

template <class T>
bool CCommonStringOperations<T>::HasSuffix( CBaseStringPart<T> str, CBaseStringPart<T> suffix )
{
	const auto suffixLength = suffix.Length();
	return str.Length() >= suffixLength && str.Right( suffixLength ) == suffix;
}

template <class T>
bool CCommonStringOperations<T>::HasSuffixNoCase( CBaseStringPart<T> str, CBaseStringPart<T> suffix )
{
	const auto suffixLength = suffix.Length();
	return str.Length() >= suffixLength && str.Right( suffixLength ).EqualsNoCase( suffix );
}

template <class T>
CBaseString<T> CCommonStringOperations<T>::Concatenate( const T* leftStr, int leftLen, CBaseString<T>&& rightStr )
{
	assert( leftLen >= 0 );
	const auto rightLen = rightStr.Length();
	const auto resultLength = leftLen + rightLen;
	auto buffer = rightStr.CreateRawBuffer( resultLength );
	memmove( buffer + leftLen, buffer, rightLen * sizeof( T ) );
	memcpy( buffer, leftStr, leftLen * sizeof( T ) );
	buffer.Release( resultLength );
	return move( rightStr );	
}

template <class T>
CBaseString<T> CCommonStringOperations<T>::Concatenate( const T* leftStr, int leftLen, const T* rightStr, int rightLen )
{
	assert( leftLen >= 0 && rightLen >= 0 );
	CBaseString<T> result;

	const int resultLength = leftLen + rightLen;
	auto buffer = result.CreateRawBuffer( resultLength );
	memcpy( buffer, leftStr, leftLen * sizeof( T ) );
	memcpy( buffer + leftLen, rightStr, rightLen * sizeof( T ) );
	buffer.Release( resultLength );
	return result;
}

template <class T>
CBaseStringPart<T> CCommonStringOperations<T>::TrimRight( CStringData<T> data )
{
	const T* buffer = data.begin();
	const int length = data.Length();

	int rightPos;
	for( rightPos = length - 1; rightPos >= 0; rightPos-- ) {
		if( !data.IsCharWhiteSpace( buffer[rightPos] ) ) {
			break;
		}
	}

	return CBaseStringPart<T>( buffer, rightPos + 1 );
}

template <class T>
CBaseStringPart<T> CCommonStringOperations<T>::TrimLeft( CStringData<T> data )
{
	const T* buffer = data.begin();
	const int length = data.Length();

	int leftCount;
	for( leftCount = 0; leftCount < length; leftCount++ ) {
		if( !data.IsCharWhiteSpace( buffer[leftCount] ) ) {
			break;
		}
	}

	return CBaseStringPart<T>( buffer + leftCount, length - leftCount );
}

template <class T>
CBaseStringPart<T> CCommonStringOperations<T>::TrimSpaces( CStringData<T> data )
{
	return TrimLeft( TrimRight( data ) );
}

template <class T>
template <class... ParamList>
CBaseString<T> CCommonStringOperations<T>::SubstParam( CBaseStringView<T> data, ParamList&&... params ) 
{
	return doSubstParams( data, createStrFromValue( forward<ParamList>( params ), Types::IsString<typename Types::PureType<ParamList>::Result, T>() )... );
}

template <class T>
template <class FirstParam, class... StrParams>
CBaseString<T> CCommonStringOperations<T>::doSubstParams( CBaseStringView<T> data, FirstParam&& firstParam, StrParams&&... params ) 
{
	const int paramCount = 1 + sizeof...( params );

	CBaseStringPart<T> paramList[paramCount]{ firstParam, params... };
	return substParam( data, paramList, paramCount );	
}

template <class T>
CBaseString<T> CCommonStringOperations<T>::doSubstParams( CBaseStringView<T> data ) 
{
	return substParam( data, nullptr, 0 );	
}

template <class T>
template <class ValueType>
CBaseStringPart<T> CCommonStringOperations<T>::createStrFromValue( ValueType&& val, Types::TrueType )
{
	return val;
}

template <class T>
template <class ValueType>
CBaseString<T> CCommonStringOperations<T>::createStrFromValue( ValueType&& val, Types::FalseType )
{
	return CStrConversionFunctions<T>::ToString( forward<ValueType>( val ) );
}

template <>
inline char CCommonStringOperations<char>::getMessageParamPrefix()
{
	return '%';
}

template <>
inline wchar_t CCommonStringOperations<wchar_t>::getMessageParamPrefix()
{
	return L'%';
}

template <class T>
bool CCommonStringOperations<T>::checkSpecialCodePage( unsigned codePage )
{
	// "For the code pages listed below, dwFlags must be set to 0. Otherwise, the function fails with ERROR_INVALID_FLAGS." MSDN, 2013
	return ( 57002 <= codePage && codePage <= 57011 ) || codePage == 50220 || codePage == 50221 || codePage ==  50222
		|| codePage == 50225 || codePage == 50227 || codePage == 50229 || codePage == 52936 || codePage == 54936
		|| codePage == CP_UTF8 || codePage == CP_UTF7 || codePage == CP_SYMBOL;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.


