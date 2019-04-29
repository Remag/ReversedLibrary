#pragma once
#include <StringData.h>
#include <StringOperations.h>
#include <SplitEnumerators.h>
#include <Remath.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Base class for a string view.
// String view contains string data which owned by another entity.
// String view is guaranteed to be null-terminated.
template <class T>
class CBaseStringView : public CStringData<T> {
public:
	CBaseStringView() = default;
	// Initialize with a null-terminated buffer.
	CBaseStringView( const T* buffer ) : CBaseStringView( buffer, CStringOperations<T>::Length( buffer ) ) {}
	// Initialize with a string and its length.
	CBaseStringView( const T* buffer, int bufferLength ) : CStringData<T>( buffer, bufferLength ) { assert( buffer[bufferLength] == 0 ); }

	operator CBaseStringPart<T>() const
		{ return CBaseStringPart<T>( Ptr(), this->Length() ); }

	T operator[]( int pos ) const;
	const T* Ptr() const
		{ return this->begin(); }

	// Partition.
	CBaseStringPart<T> Mid( int first, int count ) const;
	CBaseStringPart<T> Left( int count ) const
		{ return Mid( 0, count ); }
	// Some partitions are guaranteed to be null-terminated.
	CBaseStringView<T> Mid( int first ) const;
	CBaseStringView<T> Right( int count ) const;

	// Search.
	int Find( T symbol, int from = 0 ) const
		{ return CStringOperations<T>::Find( *this, symbol, from ); }
	int Find( CBaseStringView<T> substring, int from = 0 ) const
		{ return CStringOperations<T>::Find( *this, substring, from ); }
	int FindNoCase( CBaseStringView<T> substring, int from = 0 ) const
		{ return CStringOperations<T>::FindNoCase( *this, substring, from ); }
	int FindOneOf( CBaseStringView<T> charSet, int from = 0 ) const
		{ return CStringOperations<T>::FindOneOf( *this, charSet, from ); }
	int ReverseFind( T symbol, int from ) const
		{ return CStringOperations<T>::ReverseFind( *this, symbol, from ); }
	int ReverseFindOneOf( CBaseStringView<T> charSet, int from ) const
		{ return CStringOperations<T>::ReverseFindOneOf( *this, charSet, from ); }
	int ReverseFind( T symbol ) const
		{ return ReverseFind( symbol, this->Length() ); }
	int ReverseFindOneOf( CBaseStringView<T> charSet ) const
		{ return ReverseFindOneOf( charSet, this->Length() ); }

	// Left trimming.
	CBaseStringView<T> TrimLeft() const;

	// Iterate through substrings.
	CSplitEnumerator<CBaseStringView<T>, T> Split( T delimiter ) const
		{ return CSplitEnumerator<CBaseStringView<T>, T>( *this, delimiter ); }
	CSplitEnumerator<CBaseStringView<T>, CBaseStringView<T>> Split( CBaseStringView<T> delimiter ) const
		{ return CSplitEnumerator<CBaseStringView<T>, CBaseStringView<T>>( *this, delimiter ); }
	// Conditional split. 
	// SkipOperation takes a string view and returns a number of characters that are part of a delimiter starting from the first character of the substring.
	// If the first character is not a delimiter, 0 is returned.
	template <class SkipOperation>
	CActionSplitEnumerator<CBaseStringView<T>, SkipOperation> SplitByAction( const SkipOperation& skipAction ) const
		{ return CActionSplitEnumerator<CBaseStringView<T>, SkipOperation>( *this, skipAction ); }

	// Substitution.
	// Parameters are defined as "%n" in the template string, where n is an integer.
	template<class... ParamList>
	CBaseString<T> SubstParam( ParamList&&... params ) const
		{ return CStringOperations<T>::SubstParam( *this, forward<ParamList>( params )... ); }
};

//////////////////////////////////////////////////////////////////////////

template <class T>
T CBaseStringView<T>::operator[]( int pos ) const
{
	assert( pos >= 0 );
	assert( pos <= this->Length() );
	return Ptr()[pos];
}

template <class T>
CBaseStringPart<T> CBaseStringView<T>::Mid( int first, int count ) const
{
	assert( first >= 0 );
	assert( count >= 0 );
	assert( first + count <= this->Length() );
	return CBaseStringPart<T>( Ptr() + first, count );
}

template <class T>
CBaseStringView<T> CBaseStringView<T>::Mid( int first ) const
{
	assert( first >= 0 );
	assert( first <= this->Length() );
	return CBaseStringView<T>( Ptr() + first, this->Length() - first );
}

template <class T>
CBaseStringView<T> CBaseStringView<T>::Right( int count ) const
{
	assert( count >= 0 );
	assert( count <= this->Length() );
	return CBaseStringView<T>( Ptr() + this->Length() - count, count );
}

template <class T>
CBaseStringView<T> CBaseStringView<T>::TrimLeft() const
{
	const auto trimmedPart = CStringOperations<T>::TrimLeft( *this );
	return CBaseStringView<T>( trimmedPart.begin(), trimmedPart.Length() );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.


//////////////////////////////////////////////////////////////////////////

// Common split actions.
inline int SplitLinesAction( RelibInternal::CBaseStringView<char> substr )
{
	switch( substr.First() ) {
		case '\r':
			return substr[1] == '\n' ? 2 : 1;
		case '\n':
		case '\v':
		case '\f':
			return 1;
		default:
			return 0;
	}
}

inline int SplitUnicodeLinesAction( RelibInternal::CBaseStringView<wchar_t> substr )
{
	switch( substr[0] ) {
		case L'\r':
			return substr[1] == L'\n' ? 2 : 1;
		case L'\n':
		case L'\v':
		case L'\f':
		case 0x2028:
		case 0x2029:
			return 1;
		default:
			return 0;
	}
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
