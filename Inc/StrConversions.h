#pragma once
#include <ArrayBuffer.h>
#include <BaseString.h>
#include <BaseStringPart.h>
#include <BaseStringView.h>
#include <Color.h>
#include <Optional.h>
#include <DateTime.h>
#include <RawStringBuffer.h>
#include <Relimits.h>
#include <StackArray.h>
#include <TemplateUtils.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Conversion functions that can be expressed by a common template.
template <class T>
class CCommonConversionFunctions {
public:
	static COptional<bool> ParseBool( CBaseStringPart<T> str, CArrayView<CBaseStringView<T>> trueStrs, CArrayView<CBaseStringView<T>> falseStrs );
	template <class IntType>
	static COptional<IntType> ParseInteger( CBaseStringPart<T> str );
	template <class FltType>
	static COptional<FltType> ParseFloat( CBaseStringPart<T> str );
	static COptional<CColor> ParseColor( CBaseStringPart<T> str );
	static COptional<unsigned> ParseHexValue( CBaseStringPart<T> str );

	template <class FltType>
	static CBaseString<T> FloatToString( FltType value, int digitCount );

private:
	static const int base10 = 10;
	static const int base16 = 16;

	template <class IntType>
	static int parseFloatPart( CBaseStringPart<T> str, int strPos, IntType& result, bool& isMinus );
	template <class FltType>
	static int parseMantissa( CBaseStringPart<T> str, int strPos, FltType& result );
	template <class FltType, class IntType>
	static FltType mergeBaseWithMantissa( IntType base, FltType mantissa, bool isMinus );
	template <class FltType, class IntType>
	static FltType mergeFloatWithExponent( FltType flt, IntType exp, bool isMinus );

	template <class IntType>
	static COptional<IntType> parsePositiveInteger( CBaseStringPart<T> str );
	static COptional<unsigned> parseHexValue( CBaseStringPart<T> str );
	static bool tryGetHexDigit( T ch, int& result );

	static T getMinusSign();
	static T getZeroSymbol();
	static T getDecimalPoint();
	static T getSmallExponentLetter();
	static T getLargeExponentLetter();
	static T getCapitalA();
	static T getCapitalF();
	static T getA();
	static T getF();
	static T getX();
	static T getNumberSign();
};

//////////////////////////////////////////////////////////////////////////

template <class T>
COptional<bool> CCommonConversionFunctions<T>::ParseBool( CBaseStringPart<T> str, CArrayView<CBaseStringView<T>> trueStrs, CArrayView<CBaseStringView<T>> falseStrs )
{
	for( auto trueStr : trueStrs ) {
		if( str.EqualsNoCase( trueStr ) ) {
			return COptional<bool>( true );
		}
	}
	for( auto falseStr : falseStrs ) {
		if( str.EqualsNoCase( falseStr ) ) {
			return COptional<bool>( false );
		}
	}
	return COptional<bool>();
}

template <class T>
template <class IntType>
COptional<IntType> CCommonConversionFunctions<T>::ParseInteger( CBaseStringPart<T> str )
{
	const IntType minValue = CLimits<IntType>::Min;
	const int maxDigits = CLimits<IntType>::MaxDigitsBase10;
	const T minus = getMinusSign();
	const T zero = getZeroSymbol();

	const int length = str.Length();

	if( length == 0 ) {
		return COptional<IntType>();
	}

	const T firstChar = str[0];
	if( firstChar == minus ) {
		IntType result = 0;
		const int digitCount = length - 1;
		if( digitCount > maxDigits || digitCount == 0 ) {
			return COptional<IntType>();
		}
		if( digitCount < maxDigits ) {
			for( int i = 1; i < length; i++ ) {
				const T currentChar = str[i];
				if( !str.IsCharDigit( currentChar ) ) {
					return COptional<IntType>();
				}
				const int currentDigit = currentChar - zero;
				result = base10 * result - currentDigit;
			}
		} else {
			for( int i = 1; i < length - 1; i++ ) {
				const T currentChar = str[i];
				if( !str.IsCharDigit( currentChar ) ) {
					return COptional<IntType>();
				}
				const int currentDigit = currentChar - zero;
				result = base10 * result - currentDigit;
			}
			const T lastChar = str[length - 1];
			if( !str.IsCharDigit( lastChar ) ) {
				return COptional<IntType>();
			}
			const int lastDigit = lastChar - zero;
			const IntType minResult = ( minValue + lastDigit ) / base10;
			if( result < minResult ) {
				return COptional<IntType>();
			} else {
				result = base10 * result - lastDigit;
			}
		}

		return COptional<IntType>( result );

	} else {
		return parsePositiveInteger<IntType>( str );
	}
}

template <class T>
template <class IntType>
COptional<IntType> CCommonConversionFunctions<T>::parsePositiveInteger( CBaseStringPart<T> str )
{
	const IntType maxValue = CLimits<IntType>::Max;
	const int maxDigits = CLimits<IntType>::MaxDigitsBase10;
	const T zero = getZeroSymbol();

	IntType result = 0;
	const int digitCount = str.Length();

	if( digitCount > maxDigits ) {
		return COptional<IntType>();
	}
	if( digitCount < maxDigits ) {
		for( int i = 0; i < digitCount; i++ ) {
			const T currentChar = str[i];
			if( !str.IsCharDigit( currentChar ) ) {
				return COptional<IntType>();
			}
			const int currentDigit = currentChar - zero;
			result = base10 * result + currentDigit;
		}
	} else {
		for( int i = 0; i < digitCount - 1; i++ ) {
			const T currentChar = str[i];
			if( !str.IsCharDigit( currentChar ) ) {
				return COptional<IntType>();
			}
			const int currentDigit = currentChar - zero;
			result = base10 * result + currentDigit;
		}
		const T lastChar = str[digitCount - 1];
		if( !str.IsCharDigit( lastChar ) ) {
			return COptional<IntType>();
		}
		const int lastDigit = lastChar - zero;
		const IntType minResult = ( maxValue - lastDigit ) / base10;
		if( result > minResult ) {
			return COptional<IntType>();
		} else {
			result = base10 * result + lastDigit;
		}
	}

	return COptional<IntType>( result );
}

inline char CCommonConversionFunctions<char>::getMinusSign()
{
	return '-';
}

inline wchar_t CCommonConversionFunctions<wchar_t>::getMinusSign()
{
	return L'-';
}

inline char CCommonConversionFunctions<char>::getZeroSymbol()
{
	return '0';
}

inline wchar_t CCommonConversionFunctions<wchar_t>::getZeroSymbol()
{
	return L'0';
}

template <class T>
COptional<unsigned> CCommonConversionFunctions<T>::parseHexValue( CBaseStringPart<T> str )
{
	const unsigned maxValue = CLimits<unsigned>::Max;
	const int maxDigits = CLimits<unsigned>::MaxDigitsBase16;

	unsigned result = 0;
	const int digitCount = str.Length();

	if( digitCount > maxDigits || digitCount == 0 ) {
		return COptional<unsigned>();
	}

	for( int i = 0; i < digitCount; i++ ) {
		const T currentChar = str[i];
		int currentDigit;
		if( !tryGetHexDigit( currentChar, currentDigit ) ) {
			return COptional<unsigned>();
		}
		result = ( result << 4 ) + currentDigit;
	}

	return CreateOptional( result );
}

template <class T>
bool CCommonConversionFunctions<T>::tryGetHexDigit( T ch, int& result )
{
	if( CBaseStringPart<T>::IsCharDigit( ch ) ) {
		result = ch - getZeroSymbol();
		return true;
	}

	const auto capitalA = getCapitalA();
	const auto capitalF = getCapitalF();
	if( ch >= capitalA && ch <= capitalF ) {
		result = ( ch - capitalA ) + base10;
		return true;
	}

	const auto a = getA();
	const auto f = getF();
	if( ch >= a && ch <= f ) {
		result = ( ch - a ) + base10;
		return true;
	}

	return false;
}

inline char CCommonConversionFunctions<char>::getCapitalA()
{
	return 'A';
}

inline wchar_t CCommonConversionFunctions<wchar_t>::getCapitalA()
{
	return L'A';
}

inline char CCommonConversionFunctions<char>::getCapitalF()
{
	return 'F';
}

inline wchar_t CCommonConversionFunctions<wchar_t>::getCapitalF()
{
	return L'F';
}

inline char CCommonConversionFunctions<char>::getA()
{
	return 'a';
}

inline wchar_t CCommonConversionFunctions<wchar_t>::getA()
{
	return L'a';
}

inline char CCommonConversionFunctions<char>::getF()
{
	return 'f';
}

inline wchar_t CCommonConversionFunctions<wchar_t>::getF()
{
	return L'f';
}

inline char CCommonConversionFunctions<char>::getX()
{
	return 'x';
}

inline wchar_t CCommonConversionFunctions<wchar_t>::getX()
{
	return L'x';
}

template <class T>
template <class FltType>
COptional<FltType> CCommonConversionFunctions<T>::ParseFloat( CBaseStringPart<T> str )
{
	typedef typename CLimits<FltType>::SameSizedIntType TIntType;
	const FltType maxValue = CLimits<FltType>::Max;
	const FltType minValue = CLimits<FltType>::Min;
	const auto length = str.Length();

	int parsePos = 0;
	TIntType floatBase;
	bool isMinus;
	parsePos = parseFloatPart( str, parsePos, floatBase, isMinus );
	if( parsePos == NotFound || floatBase > maxValue || floatBase < minValue ) {
		return COptional<FltType>();
	}
	FltType floatResult;
	if( parsePos < length && str[parsePos] == getDecimalPoint() ) {
		FltType mantissa;
		parsePos = parseMantissa( str, parsePos + 1, mantissa );
		if( parsePos == NotFound ) {
			return COptional<FltType>();
		}
		floatResult = mergeBaseWithMantissa( floatBase, mantissa, isMinus );
	} else {
		floatResult = static_cast<FltType>( floatBase );
	}
	if( parsePos < length && ( str[parsePos] == getSmallExponentLetter() || str[parsePos] == getLargeExponentLetter() ) ) {
		TIntType exponent;
		bool isExpMinus;
		parsePos = parseFloatPart( str, parsePos + 1, exponent, isExpMinus );
		if( parsePos == NotFound ) {
			return COptional<FltType>();
		}
		floatResult = mergeFloatWithExponent( floatResult, exponent, isExpMinus );
	}

	return parsePos == length ? COptional<FltType>( floatResult ) : COptional<FltType>();
}

template <class T>
template <class IntType>
int CCommonConversionFunctions<T>::parseFloatPart( CBaseStringPart<T> str, int strPos, IntType& result, bool& isMinus )
{
	const IntType maxValue = CLimits<IntType>::Max;
	const int maxDigits = CLimits<IntType>::MaxDigitsBase10;
	const T zero = getZeroSymbol();
	const auto length = str.Length();

	if( strPos >= length ) {
		return NotFound;
	}
	isMinus = str[strPos] == getMinusSign();

	IntType currentResult = 0;
	const auto startPos = strPos + isMinus;
	for( int i = startPos; i < length; i++ ) {
		const T currentChar = str[i];
		if( !str.IsCharDigit( currentChar ) ) {
			result = currentResult;
			return i;
		}
		const int currentDigit = currentChar - zero;
		if( i - startPos == maxDigits ) {
			if( result > ( maxValue - currentDigit ) / base10 ) {
				return NotFound;
			}
			currentResult = currentResult * base10 + currentDigit;
			result = currentResult;
			return i + 1;
		}
		currentResult = currentResult * base10 + currentDigit;
	}

	result = currentResult;
	return length;
}

template <class T>
template <class FltType>
int CCommonConversionFunctions<T>::parseMantissa( CBaseStringPart<T> str, int strPos, FltType& result )
{
	const T zero = getZeroSymbol();

	const auto length = str.Length();
	auto currentResult = FltType( 0 );
	auto currentBaseModifier = FltType( 0.1 );
	for( int i = strPos; i < length; i++ ) {
		const T ch = str[i];
		if( !str.IsCharDigit( ch ) ) {
			result = currentResult;
			return i;
		}
		const int currentDigit = ch - zero;
		currentResult += currentDigit * currentBaseModifier;
		currentBaseModifier *= FltType( 0.1 );
	}

	result = currentResult;
	return length;
}

template <class T>
template <class FltType, class IntType>
FltType CCommonConversionFunctions<T>::mergeBaseWithMantissa( IntType base, FltType mantissa, bool isMinus )
{
	const FltType fBase = static_cast<FltType>( base );
	return isMinus ? -fBase - mantissa : fBase + mantissa;
}

template <class T>
template <class FltType, class IntType>
FltType CCommonConversionFunctions<T>::mergeFloatWithExponent( FltType flt, IntType exp, bool isMinus )
{
	if( !isMinus ) {
		while( exp > 9 ) {
			flt *= 1000000000;
			exp -= 9;
		}
		while( exp > 4 ) {
			flt *= 10000;
			exp -= 4;
		}
		while( exp > 0 ) {
			flt *= 10;
			exp--;
		}
	} else {
		while( exp > 9 ) {
			flt /= 1000000000;
			exp -= 9;
		}
		while( exp > 4 ) {
			flt /= 10000;
			exp -= 4;
		}
		while( exp > 0 ) {
			flt /= 10;
			exp--;
		}
	}
	return flt;
}

inline char CCommonConversionFunctions<char>::getDecimalPoint()
{
	return '.';
}

inline wchar_t CCommonConversionFunctions<wchar_t>::getDecimalPoint()
{
	return L'.';
}

inline char CCommonConversionFunctions<char>::getSmallExponentLetter()
{
	return 'e';
}

inline wchar_t CCommonConversionFunctions<wchar_t>::getSmallExponentLetter()
{
	return L'e';
}

inline char CCommonConversionFunctions<char>::getLargeExponentLetter()
{
	return 'E';
}

inline wchar_t CCommonConversionFunctions<wchar_t>::getLargeExponentLetter()
{
	return L'E';
}

template <class T>
COptional<CColor> CCommonConversionFunctions<T>::ParseColor( CBaseStringPart<T> str )
{
	const auto fullLength = str.Length();
	if( fullLength > 0 && str[0] == getNumberSign() ) {
		str = str.Mid( 1 );
	}

	if( fullLength > 1 && str[1] == getX() ) {
		str = str.Mid( 2 );
	}

	const auto digitCount = str.Length();
	const auto hexValueOpt = parseHexValue( str );
	if( !hexValueOpt.IsValid() ) {
		return COptional<CColor>();
	}

	const auto hexValue = *hexValueOpt;
	CColor result;
	result.B = static_cast<BYTE>( hexValue & 0x000000FF );
	result.G = static_cast<BYTE>( ( hexValue & 0x0000FF00 ) >> 8 );
	result.R = static_cast<BYTE>( ( hexValue & 0x00FF0000 ) >> 16 );

	if( digitCount > 6 ) {
		result.A = static_cast<BYTE>( ( hexValue & 0xFF000000 ) >> 24 );
	}
	return CreateOptional( result );
}

template <class T>
COptional<unsigned> CCommonConversionFunctions<T>::ParseHexValue( CBaseStringPart<T> str )
{
	const auto fullLength = str.Length();
	if( fullLength > 1 && str[1] == getX() ) {
		str = str.Mid( 2 );
	}

	return parseHexValue( str );
}

inline char CCommonConversionFunctions<char>::getNumberSign()
{
	return '#';
}

inline wchar_t CCommonConversionFunctions<wchar_t>::getNumberSign()
{
	return L'#';
}

template <class T>
template <class FltType>
CBaseString<T> CCommonConversionFunctions<T>::FloatToString( FltType value, int digitCount )
{
	assert( digitCount >= 0 );
	FltType currentFractPart;
	CBaseString<T> result;
	if( value >= 0 ) {
		const auto intPart = static_cast<unsigned>( value );
		result = CStrConversionFunctions<T>::ToString( intPart );
		currentFractPart = value - intPart;
	} else {
		const auto intPart = static_cast<unsigned>( -value );
		result = getMinusSign() + CStrConversionFunctions<T>::ToString( intPart );
		currentFractPart = -value - intPart;
	}

	const int intLength = result.Length();
	const int resultLength = intLength + digitCount + 1;
	auto resultBuffer = result.CreateRawBuffer( resultLength );
	resultBuffer[intLength] = getDecimalPoint();

	for( int i = intLength + 1; i < resultLength; i++ ) {
		currentFractPart *= 10;
		const int currentDigit = static_cast<int>( currentFractPart );
		resultBuffer[i] = numeric_cast<T>( getZeroSymbol() + currentDigit );

		currentFractPart -= currentDigit;
	}

	resultBuffer.Release( resultLength );
	return move( result );
}

//////////////////////////////////////////////////////////////////////////

// String conversion functions.
template <class StrType>
struct CStrConversionFunctions {
	staticAssert( ( Types::IsSame<StrType, char>::Result || Types::IsSame<StrType, wchar_t>::Result ) );
};

template <>
class CStrConversionFunctions<char> : public CCommonConversionFunctions<char> {
public:
	template <class T>
	static COptional<T> GetValue( CStringPart str, Types::Type<T> );
	static COptional<bool> GetValue( CStringPart str, Types::Type<bool> );
	static COptional<int> GetValue( CStringPart str, Types::Type<int> );
	static COptional<unsigned> GetValue( CStringPart str, Types::Type<unsigned> );
	static COptional<__int64> GetValue( CStringPart str, Types::Type<__int64> );
	static COptional<double> GetValue( CStringPart str, Types::Type<double> );
	static COptional<float> GetValue( CStringPart str, Types::Type<float> );
	static COptional<CStringView> GetValue( CStringView str, Types::Type<CStringView> );
	static COptional<CColor> GetValue( CStringPart str, Types::Type<CColor> );
	template <class VecType, int dim>
	static COptional<CVector<VecType, dim>> GetValue( CStringPart str, Types::Type<CVector<VecType, dim>>, char delim );
	template <class VecType, int dim>
	static COptional<CVector<VecType, dim>> GetValue( CStringPart str, Types::Type<CVector<VecType, dim>> );

	static CString ToString( bool value );
	static CString ToString( int value, int base = 10 );
	static CString ToString( unsigned value, int base = 10 );
	static CString ToString( __int64 value, int base = 10 );
	static CString ToString( double value, int digitCount = 3 );
	static CString ToString( CStringPart value );
	static CString ToString( const char* value );
	static CString ToString( CUnicodePart value, unsigned codePage = CP_ACP );
	static CString ToString( CUnicodePart value, char defaultChar, unsigned codePage );
	static CString ToString( CColor value );
	template <class VecType, int dim>
	static CString ToString( CVector<VecType, dim> vec, char delim = ';' );
	// Date format:
	// YYYY - full year.
	// YY - short year.
	// MM - month.
	// DD - day.
	// H - hour.
	// M - minute.
	// S - second.
	static CString ToString( CDateTime date, CStringPart format	);

private:
	// String values of bool type.
	static const REAPI CStringView trueStrings[2];
	static const REAPI CStringView falseStrings[2];
};

//////////////////////////////////////////////////////////////////////////

template <class T>
inline COptional<T> CStrConversionFunctions<char>::GetValue( CStringPart str, Types::Type<T> )
{
	return COptional<T>( T( str ) );
}

inline COptional<bool> CStrConversionFunctions<char>::GetValue( CStringPart str, Types::Type<bool> )
{
	return ParseBool( str, trueStrings, falseStrings );
}

inline COptional<int> CStrConversionFunctions<char>::GetValue( CStringPart str, Types::Type<int> )
{
	return ParseInteger<int>( str );
}

inline COptional<unsigned> CStrConversionFunctions<char>::GetValue( CStringPart str, Types::Type<unsigned> )
{
	return ParseHexValue( str );
}

inline COptional<__int64> CStrConversionFunctions<char>::GetValue( CStringPart str, Types::Type<__int64> )
{
	return ParseInteger<__int64>( str );
}

inline COptional<double> CStrConversionFunctions<char>::GetValue( CStringPart str, Types::Type<double> )
{
	return ParseFloat<double>( str );
}

inline COptional<float> CStrConversionFunctions<char>::GetValue( CStringPart str, Types::Type<float> )
{
	return ParseFloat<float>( str );
}

inline COptional<CStringView> CStrConversionFunctions<char>::GetValue( CStringView str, Types::Type<CStringView> )
{
	return CreateOptional( str );
}

inline COptional<CColor> CStrConversionFunctions<char>::GetValue( CStringPart str, Types::Type<CColor> )
{
	return ParseColor( str );
}

template <class VecType, int dim>
inline COptional<CVector<VecType, dim>> CStrConversionFunctions<char>::GetValue( CStringPart str, Types::Type<CVector<VecType, dim>>, char delimiter )
{
	CVector<VecType, dim> result;
	int index = 0;
	for( auto part : str.Split( delimiter ) ) {
		if( index >= dim ) {
			return COptional<CVector<VecType, dim>>();
		}

		const auto indexValue = GetValue( part, Types::Type<VecType>() );
		if( !indexValue.IsValid() ) {
			return COptional<CVector<VecType, dim>>();
		}

		result[index] = *indexValue;
		index++;
	}

	if( index < dim - 1 ) {
		return COptional<CVector<VecType, dim>>();
	}

	return COptional<CVector<VecType, dim>>( result );
}

template <class VecType, int dim>
inline COptional<CVector<VecType, dim>> CStrConversionFunctions<char>::GetValue( CStringPart str, Types::Type<CVector<VecType, dim>> type )
{
	return GetValue( str, type, ';' );
}

inline CString CStrConversionFunctions<char>::ToString( bool value )
{
	staticAssert( _countof( trueStrings ) > 0 && _countof( falseStrings ) > 0 );
	return value ? ToString( trueStrings[0] ) : ToString( falseStrings[0] );
}

inline CString CStrConversionFunctions<char>::ToString( int value, int base )
{
	const int maxIntDigits = CLimits<int>::MaxDigitsBase10 + 1;
	CString result;
	_itoa_s( value, result.CreateRawBuffer( maxIntDigits ), maxIntDigits + 1, base );
	return result;
}

inline CString CStrConversionFunctions<char>::ToString( unsigned value, int base )
{
	const int maxIntDigits = CLimits<unsigned>::MaxDigitsBase10 + 1;
	CString result;
	_ultoa_s( value, result.CreateRawBuffer( maxIntDigits ), maxIntDigits + 1, base );
	return result;
}

inline CString CStrConversionFunctions<char>::ToString( __int64 value, int base )
{
	const int maxIntDigits = CLimits<__int64>::MaxDigitsBase10;
	CString result;
	_i64toa_s( value, result.CreateRawBuffer( maxIntDigits ), maxIntDigits + 1, base );
	return result;
}

inline CString CStrConversionFunctions<char>::ToString( double value, int digitCount )
{
	return FloatToString( value, digitCount );
}

inline CString CStrConversionFunctions<char>::ToString( CStringPart value )
{
	return CString( value.begin(), value.Length() );
}

inline CString CStrConversionFunctions<char>::ToString( const char* value )
{
	return CString( value );
}

inline CString CStrConversionFunctions<char>::ToString( CUnicodePart value, unsigned codePage )
{
	CString result;
	CStringOperations<char>::ConvertStr( value, codePage, result );
	return result;
}

inline CString CStrConversionFunctions<char>::ToString( CUnicodePart value, char defaultChar, unsigned codePage )
{
	CString result;
	CStringOperations<char>::ConvertStr( value, codePage, defaultChar, result );
	return result;
}

inline CString CStrConversionFunctions<char>::ToString( CColor value )
{
	const unsigned rValue = value.R;
	const unsigned gValue = value.G;
	const unsigned bValue = value.B;
	const unsigned aValue = value.A;

	const auto colorValue = bValue + ( gValue << 8 ) + ( rValue << 16 ) + ( aValue << 24 );
	CString result;
	::_ultoa_s( colorValue, result.CreateRawBuffer( 10 ), 9, 16 );
	const auto leadingZeroCount = 8 - result.Length();
	const auto zeroStr = "0x00000000";
	const CStringPart zeroStrPart( zeroStr, leadingZeroCount + 2 );
	result.InsertAt( 0, zeroStrPart );
	return result;
}

template<class VecType, int dim>
CString CStrConversionFunctions<char>::ToString( CVector<VecType,dim> vec, char delim )
{
	CString result;
	for( int i = 0; i < dim - 1; i++ ) {
		result += ToString( vec[i] );
		result += delim;
	}
	result += ToString( vec[dim - 1] );
	return result;
}

inline CString CStrConversionFunctions<char>::ToString( CDateTime date, CStringPart format )
{
	CString result( format );
	const auto fullYear = ToString( date.GetYear() );
	const auto shortYearCount = max( 2, fullYear.Length() );
	const auto shortYear = fullYear.Right( shortYearCount );

	CStackArray<CString, 5> dateStr {
		ToString( date.GetMonth() ),
		ToString( date.GetDay() ),
		ToString( date.GetHour() ),
		ToString( date.GetMinute() ),
		ToString( date.GetSecond() )
	};

	for( auto& dateNum : dateStr ) {
		if( dateNum.Length() == 1 ) {
			dateNum.InsertAt( 0, '0' );
		}
	}

	result.ReplaceAll( "YYYY", fullYear );
	result.ReplaceAll( "YY", shortYear );
	result.ReplaceAll( "MM", dateStr[0] );
	result.ReplaceAll( "DD", dateStr[1] );
	result.ReplaceAll( "H", dateStr[2] );
	result.ReplaceAll( "M", dateStr[3] );
	result.ReplaceAll( "S", dateStr[4] );
	return result;
}

//////////////////////////////////////////////////////////////////////////

template <>
class CStrConversionFunctions<wchar_t> : public CCommonConversionFunctions<wchar_t> {
public:
	template <class T>
	static COptional<T> GetValue( CUnicodePart str, Types::Type<T> );
	static COptional<bool> GetValue( CUnicodePart str, Types::Type<bool> );
	static COptional<int> GetValue( CUnicodePart str, Types::Type<int> );
	static COptional<unsigned> GetValue( CUnicodePart str, Types::Type<unsigned> );
	static COptional<__int64> GetValue( CUnicodePart str, Types::Type<__int64> );
	static COptional<double> GetValue( CUnicodePart str, Types::Type<double> );
	static COptional<float> GetValue( CUnicodePart str, Types::Type<float> );
	static COptional<CUnicodeView> GetValue( CUnicodeView str, Types::Type<CUnicodeView> );
	static COptional<CColor> GetValue( CUnicodePart str, Types::Type<CColor> );
	static COptional<CString> GetValue( CUnicodePart str, Types::Type<CString> );
	template <class VecType, int dim>
	static COptional<CVector<VecType, dim>> GetValue( CUnicodePart str, Types::Type<CVector<VecType, dim>>, wchar_t delim );
	template <class VecType, int dim>
	static COptional<CVector<VecType, dim>> GetValue( CUnicodePart str, Types::Type<CVector<VecType, dim>> );

	static CUnicodeString ToString( bool value );
	static CUnicodeString ToString( int value, int base = 10 );
	static CUnicodeString ToString( unsigned value, int base = 10 );
	static CUnicodeString ToString( __int64 value, int base = 10 );
	static CUnicodeString ToString( double value, int digitCount = 3 );
	static CUnicodeString ToString( CUnicodePart value );
	static CUnicodeString ToString( const wchar_t* value );
	static CUnicodeString ToString( const char* value, unsigned codePage = CP_ACP );
	static CUnicodeString ToString( CStringPart value, unsigned codePage = CP_ACP );
	static CUnicodeString ToString( CColor value );
	template <class VecType, int dim>
	static CUnicodeString ToString( CVector<VecType, dim> vec, wchar_t delim = L';' );
	// Date format:
	// YYYY - full year.
	// YY - short year.
	// MM - month.
	// DD - day.
	// H - hour.
	// M - minute.
	// S - second.
	static CUnicodeString ToString( CDateTime date, CUnicodePart format	);

private:
	// String values of bool type.
	static const REAPI CUnicodeView trueUnicodeStrings[2];
	static const REAPI CUnicodeView falseUnicodeStrings[2];
};

//////////////////////////////////////////////////////////////////////////

template <class T>
inline COptional<T> CStrConversionFunctions<wchar_t>::GetValue( CUnicodePart str, Types::Type<T> )
{
	return COptional<T>( T( str ) );
}

inline COptional<bool> CStrConversionFunctions<wchar_t>::GetValue( CUnicodePart str, Types::Type<bool> )
{
	return ParseBool( str, trueUnicodeStrings, falseUnicodeStrings );
}

inline COptional<int> CStrConversionFunctions<wchar_t>::GetValue( CUnicodePart str, Types::Type<int> )
{
	return ParseInteger<int>( str );
}

inline COptional<unsigned> CStrConversionFunctions<wchar_t>::GetValue( CUnicodePart str, Types::Type<unsigned> )
{
	return ParseHexValue( str );
}

inline COptional<__int64> CStrConversionFunctions<wchar_t>::GetValue( CUnicodePart str, Types::Type<__int64> )
{
	return ParseInteger<__int64>( str );
}

inline COptional<double> CStrConversionFunctions<wchar_t>::GetValue( CUnicodePart str, Types::Type<double> )
{
	return ParseFloat<double>( str );
}

inline COptional<float> CStrConversionFunctions<wchar_t>::GetValue( CUnicodePart str, Types::Type<float> )
{
	return ParseFloat<float>( str );
}

inline COptional<CUnicodeView> CStrConversionFunctions<wchar_t>::GetValue( CUnicodeView str, Types::Type<CUnicodeView> )
{
	return CreateOptional( str );
}

inline COptional<CColor> CStrConversionFunctions<wchar_t>::GetValue( CUnicodePart str, Types::Type<CColor> )
{
	return ParseColor( str );
}

template <class VecType, int dim>
inline COptional<CVector<VecType, dim>> CStrConversionFunctions<wchar_t>::GetValue( CUnicodePart str, Types::Type<CVector<VecType, dim>>, wchar_t delimiter )
{
	CVector<VecType, dim> result;
	int index = 0;
	for( auto part : str.Split( delimiter ) ) {
		if( index >= dim ) {
			return COptional<CVector<VecType, dim>>();
		}

		const auto indexValue = GetValue( part, Types::Type<VecType>() );
		if( !indexValue.IsValid() ) {
			return COptional<CVector<VecType, dim>>();
		}

		result[index] = *indexValue;
		index++;
	}

	if( index < dim - 1 ) {
		return COptional<CVector<VecType, dim>>();
	}

	return COptional<CVector<VecType, dim>>( result );
}

template <class VecType, int dim>
inline COptional<CVector<VecType, dim>> CStrConversionFunctions<wchar_t>::GetValue( CUnicodePart str, Types::Type<CVector<VecType, dim>> type )
{
	return GetValue( str, type, L';' );
}

inline COptional<CString> CStrConversionFunctions<wchar_t>::GetValue( CUnicodePart str, Types::Type<CString> )
{
	return CreateOptional( CStrConversionFunctions<char>::ToString( str ) );
}

inline CUnicodeString CStrConversionFunctions<wchar_t>::ToString( int value, int base )
{
	const int maxIntDigits = CLimits<int>::MaxDigitsBase10 + 1;
	CUnicodeString result;
	::_itow_s( value, result.CreateRawBuffer( maxIntDigits ), maxIntDigits + 1, base );
	return result;
}

inline CUnicodeString CStrConversionFunctions<wchar_t>::ToString( unsigned value, int base )
{
	const int maxIntDigits = CLimits<unsigned>::MaxDigitsBase10 + 1;
	CUnicodeString result;
	::_ultow_s( value, result.CreateRawBuffer( maxIntDigits ), maxIntDigits + 1, base );
	return result;
}

inline CUnicodeString CStrConversionFunctions<wchar_t>::ToString( bool value )
{
	staticAssert( _countof( trueUnicodeStrings ) > 0 && _countof( falseUnicodeStrings ) > 0 );
	return value ? ToString( trueUnicodeStrings[0] ) : ToString( falseUnicodeStrings[0] );
}

inline CUnicodeString CStrConversionFunctions<wchar_t>::ToString( __int64 value, int base )
{
	const int maxIntDigits = CLimits<__int64>::MaxDigitsBase10;
	CUnicodeString result;
	_i64tow_s( value, result.CreateRawBuffer( maxIntDigits ), maxIntDigits + 1, base );
	return result;
}

inline CUnicodeString CStrConversionFunctions<wchar_t>::ToString( double value, int digitCount )
{
	return FloatToString( value, digitCount );
}

inline CUnicodeString CStrConversionFunctions<wchar_t>::ToString( CUnicodePart value )
{
	return CUnicodeString( value.begin(), value.Length() );
}

inline CUnicodeString CStrConversionFunctions<wchar_t>::ToString( const wchar_t* value )
{
	return CUnicodeString( value );
}

inline CUnicodeString CStrConversionFunctions<wchar_t>::ToString( const char* value, unsigned codePage /*= CP_ACP */ )
{
	CUnicodeString result;
	const CStringView valueStr = value;
	CStringOperations<wchar_t>::ConvertStr( valueStr, codePage, result );
	return result;
}

inline CUnicodeString CStrConversionFunctions<wchar_t>::ToString( CStringPart value, unsigned codePage /*= CP_ACP */ )
{
	CUnicodeString result;
	CStringOperations<wchar_t>::ConvertStr( value, codePage, result );
	return result;
}

inline CUnicodeString CStrConversionFunctions<wchar_t>::ToString( CColor value )
{
	const unsigned rValue = value.R;
	const unsigned gValue = value.G;
	const unsigned bValue = value.B;
	const unsigned aValue = value.A;

	const auto colorValue = bValue + ( gValue << 8 ) + ( rValue << 16 ) + ( aValue << 24 );
	CUnicodeString result;
	::_ultow_s( colorValue, result.CreateRawBuffer( 10 ), 9, 16 );
	const auto leadingZeroCount = 8 - result.Length();
	const auto zeroStr = L"0x00000000";
	const CUnicodePart zeroStrPart( zeroStr, leadingZeroCount + 2 );
	result.InsertAt( 0, zeroStrPart );

	return result;
}

template<class VecType, int dim>
CUnicodeString CStrConversionFunctions<wchar_t>::ToString( CVector<VecType,dim> vec, wchar_t delim )
{
	CUnicodeString result;
	for( int i = 0; i < dim - 1; i++ ) {
		result += ToString( vec[i] );
		result += delim;
	}
	result += ToString( vec[dim -1] );
	return result;
}

inline CUnicodeString CStrConversionFunctions<wchar_t>::ToString( CDateTime date, CUnicodePart format )
{
	CUnicodeString result( format );
	const auto fullYear = ToString( date.GetYear() );
	const auto shortYearCount = max( 2, fullYear.Length() );
	const auto shortYear = fullYear.Right( shortYearCount );

	CStackArray<CUnicodeString, 5> dateStr {
		ToString( date.GetMonth() ),
		ToString( date.GetDay() ),
		ToString( date.GetHour() ),
		ToString( date.GetMinute() ),
		ToString( date.GetSecond() )
	};

	for( auto& dateNum : dateStr ) {
		if( dateNum.Length() == 1 ) {
			dateNum.InsertAt( 0, L'0' );
		}
	}

	result.ReplaceAll( L"YYYY", fullYear );
	result.ReplaceAll( L"YY", shortYear );
	result.ReplaceAll( L"MM", dateStr[0] );
	result.ReplaceAll( L"DD", dateStr[1] );
	result.ReplaceAll( L"H", dateStr[2] );
	result.ReplaceAll( L"M", dateStr[3] );
	result.ReplaceAll( L"S", dateStr[4] );
	return result;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

// Conversion operators.

// Conversion from a string to a templated value.
// Returns an optional that contains a value if the given string was successfully parsed.
// Specialization for types that are not a string view.
template <class T, class StrType>
typename Types::EnableIf<!Types::IsSame<T, RelibInternal::CBaseStringView<StrType>>::Result, COptional<T>>::Result Value( RelibInternal::CStringData<StrType> str )
{
	return RelibInternal::CStrConversionFunctions<StrType>::GetValue( str, Types::Type<T>() );
}

// Specialization for string views. Only accept a string view or an owning string as input as string parts cannot be converted to string views.
// The conversion in this case is obviously trivial, it exists so that templates that use Value would work in the general case.
template <class T, class StrType>
typename Types::EnableIf<Types::IsSame<T, RelibInternal::CBaseStringView<StrType>>::Result, COptional<T>>::Result Value( RelibInternal::CBaseStringView<StrType> str )
{
	return CreateOptional( str );
}

template <class T, class StrType>
typename Types::EnableIf<Types::IsSame<T, RelibInternal::CBaseStringView<StrType>>::Result, COptional<T>>::Result Value( const RelibInternal::CBaseString<StrType>& str )
{
	return CreateOptional( static_cast<RelibInternal::CBaseStringView<StrType>>( str ) );
}

// Vector conversion.
template <class VecType, int dim, class StrType, class DelimType>
COptional<CVector<VecType, dim>> Value( RelibInternal::CStringData<StrType> str, const DelimType& delimiter )
{
	return RelibInternal::CStrConversionFunctions<StrType>::GetValue( str, Types::Type<CVector<VecType, dim>>(), delimiter );
}

//////////////////////////////////////////////////////////////////////////

template <class T, class... Args>
CString Str( const T& val, Args&&... args )
{
	return RelibInternal::CStrConversionFunctions<char>::ToString( val, args... );
}

template <class T, class... Args>
CUnicodeString UnicodeStr( const T& val, Args&&... args )
{
	return RelibInternal::CStrConversionFunctions<wchar_t>::ToString( val, args... );
}

//////////////////////////////////////////////////////////////////////////

// Conversion between strings with the external buffer.
// The provided buffer is asserted to be empty.
inline void ConvertString( CStringPart val, CUnicodeString& result, unsigned codePage = CP_ACP )
{
	RelibInternal::CStringOperations<wchar_t>::ConvertStr( val, codePage, result );
}

inline void ConvertString( CUnicodePart val, CString& result, unsigned codePage = CP_ACP )
{
	RelibInternal::CStringOperations<char>::ConvertStr( val, codePage, result );
}

inline void ConvertString( CUnicodePart val, CString& result, char defaultChar, unsigned codePage = CP_ACP )
{
	RelibInternal::CStringOperations<char>::ConvertStr( val, codePage, defaultChar, result );
}

//////////////////////////////////////////////////////////////////////////

namespace RelibInternal {

// Common string method definitions that depend on string conversions.
template <class T>
CBaseString<T> CCommonStringOperations<T>::substParam( CBaseStringView<T> data, CBaseStringPart<T> params[], int size )
{
	CBaseString<T> result;
	const int length = data.Length();
	const T* buffer = data.begin();
	const T messageParamPrefix = getMessageParamPrefix();
	result.ReserveBuffer( length );
	int pos = 0;
	// Start parsing.
	while( pos < length ) {
		// Find the parameter prefix.
		const int prevPosition = pos;
		pos = data.Find( messageParamPrefix, prevPosition );
		if( pos == NotFound || pos + 1 >= length ) {
			// No parameters were found.
			result += data.Mid( prevPosition );
			break;
		}
		result += data.Mid( prevPosition, pos - prevPosition );
		// Try and parse the digit after the prefix.
		// First, find the digit's length.
		int digitPos = pos + 1;
		for( ; digitPos < length && data.IsCharDigit( buffer[digitPos] ); digitPos++ ) {
		}
		// Now, parse the digit.
		if( digitPos > pos + 1 ) {
			const auto indexValue = Value<int>( data.Mid( pos + 1, digitPos - pos - 1 ) );
			if( indexValue.IsValid() && 0 <= *indexValue && *indexValue < size ) {
				// The digit can be substituted with an entry from the provided array.
				result += params[*indexValue];
			}
		} else {
			// No digits were found after the prefix.
			result += data.Mid( pos, digitPos - pos );
		}
		pos = digitPos;
	}
	return result;
}

}	// namespace RelibInternal

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
