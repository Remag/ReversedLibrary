#pragma once
#include <Reassert.h>
#include <Remath.h>
#include <StringData.h>
#include <TemplateUtils.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

namespace Types {

// String template traits.
template <class T, class CharType>
using IsRelibString = IsDerivedFrom<T, RelibInternal::CStringData<CharType>>;

template <class T>
using IsAnyRelibString = BoolType<IsRelibString<T, char>::Result || IsRelibString<T, wchar_t>::Result>;

template <class T, class CharType>
struct IsString : public BoolType<IsRelibString<T, CharType>::Result> {};

template <int dim>
struct IsString<char[dim], char> : public TrueType {};

template <>
struct IsString<const char*, char> : public TrueType {};

template<>
struct IsString<char*, char> : public TrueType {};

template <int dim>
struct IsString<wchar_t[dim], wchar_t> : public TrueType {};

template<>
struct IsString<const wchar_t*, wchar_t> : public TrueType {};

template<>
struct IsString<wchar_t*, wchar_t> : public TrueType {};

}	// namespace Types.

namespace RelibInternal {

template <class T>
struct CStrConversionFunctions;
//////////////////////////////////////////////////////////////////////////

// Operations that can be uniformly described for all string types.
template <class T>
class CCommonStringOperations {
public:
	static int Compare( CStringData<T> left, CStringData<T> right );
	static bool Equals( CStringData<T> left, CStringData<T> right );
	static int Find( CStringData<T> data, T symbol, int from );
	static int FindCommon( CStringData<T> data, CStringData<T> substr, int from );
	static int FindNoCase( CStringData<T> data, CStringData<T> substr, int from );
	static int ReverseFind( CStringData<T> data, T symbol, int from );
	static int FindOneOf( CStringData<T> data, CStringData<T> charSet, int from );
	static int ReverseFindOneOf( CStringData<T> data, CStringData<T> charSet, int from );

	static bool HasPrefix( CBaseStringPart<T> str, CBaseStringPart<T> prefix );
	static bool HasSuffix( CBaseStringPart<T> str, CBaseStringPart<T> suffix );
	static CBaseString<T> Concatenate( const T* leftStr, int leftLen, CBaseString<T>&& rightStr );
	static CBaseString<T> Concatenate( const T* leftStr, int leftLen, const T* rightStr, int rightLen );
	static CBaseStringPart<T> TrimRight( CStringData<T> data );
	static CBaseStringPart<T> TrimLeft( CStringData<T> data );
	static CBaseStringPart<T> TrimSpaces( CStringData<T> data );

	template <class... ParamList>
	static CBaseString<T> SubstParam( CBaseStringView<T> data, ParamList&&... params );

protected:
	static bool checkSpecialCodePage( unsigned codePage );

private:
	template <class FirstParam, class... StrParams>
	static CBaseString<T> doSubstParams( CBaseStringView<T> data, FirstParam&& firstParam, StrParams&&... params );
	static CBaseString<T> doSubstParams( CBaseStringView<T> data );
	template <class ValueType>
	static CBaseStringPart<T> createStrFromValue( ValueType&& val, Types::TrueType strMarker );
	template <class ValueType>
	static CBaseString<T> createStrFromValue( ValueType&& val, Types::FalseType otherMarker );

	// Defined in StrConversions.h
	static CBaseString<T> substParam( CBaseStringView<T> data, CBaseStringPart<T> params[], int size );

	static T getMessageParamPrefix();
};

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

}	// namespace Relib.


