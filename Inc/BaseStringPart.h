#pragma once
#include <StringData.h>
#include <StringOperations.h>
#include <SplitEnumerators.h>
#include <StringSearchEnumerator.h>

namespace Relib {

namespace RelibInternal {

//////////////////////////////////////////////////////////////////////////

// Base class for string parts.
// String part contains string data which owned by another entity.
// String part may not be null-terminated.
template <class T>
class CBaseStringPart : public CStringData<T> {
public:
	CBaseStringPart() = default;
	// Initialize with raw data.
	CBaseStringPart( CStringData<T> data ) : CBaseStringPart( data.begin(), data.Length() ) {}
	// Initialize with a symbol.
	CBaseStringPart( const T& symbol ) : CBaseStringPart( &symbol, 1 ) {}
	// Initialize with a null-terminated buffer.
	CBaseStringPart( const T* buffer ) : CBaseStringPart( buffer, CStringOperations<T>::Length( buffer ) ) {}
	// Initialize with a string and its length.
	CBaseStringPart( const T* buffer, int bufferLength ) : CStringData<T>{ buffer, bufferLength } {}

	T operator[]( int pos ) const;

	// Partition.
	CBaseStringPart<T> Mid( int first, int count ) const;
	CBaseStringPart<T> Mid( int first ) const
		{ return Mid( first, this->Length() - first ); }
	CBaseStringPart<T> Left( int count ) const
		{ return Mid( 0, count ); }
	CBaseStringPart<T> Right( int count ) const
		{ return Mid( this->Length() - count, count ); }

	// Symbol search.
	int Find( T symbol, int from = 0 ) const
		{ return CStringOperations<T>::Find( *this, symbol, from ); }
	int Find( CBaseStringPart<T> substring, int from = 0 ) const
		{ return CStringOperations<T>::FindCommon( *this, substring, from ); }
	int FindNoCase( CBaseStringPart<T> substring, int from = 0 ) const
		{ return CStringOperations<T>::FindNoCase( *this, substring, from ); }
	int ReverseFind( T symbol, int from ) const
		{ return CStringOperations<T>::ReverseFind( *this, symbol, from ); }
	int ReverseFind( T symbol ) const
		{ return ReverseFind( symbol, this->Length() ); }

	int FindOneOf( CBaseStringView<T> symbols, int from = 0 ) const
		{ return CStringOperations<T>::FindOneOf( *this, symbols, from ); }
	int ReverseFindOneOf( CBaseStringView<T> symbols, int from ) const
		{ return CStringOperations<T>::ReverseFindOneOf( *this, symbols, from ); }
	int ReverseFindOneOf( CBaseStringView<T> symbols ) const
		{ return ReverseFindOneOf( symbols, this->Length() ); }
	
	auto FindAll( T symbol ) const
		{ return CStringSearchEnumerator<CBaseStringView<T>, T>( *this, symbol ); }

	// Left trimming.
	CBaseStringPart<T> TrimLeft() const
		{ return CStringOperations<T>::TrimLeft( *this ); }

	// Iterate through substrings.
	CSplitEnumerator<CBaseStringPart<T>, T> Split( T delimiter ) const
		{ return CSplitEnumerator<CBaseStringPart<T>, T>( *this, delimiter ); }
	// Conditional split. 
	// SkipOperation takes a string view and returns a number of characters that are part of a delimiter starting from the first character of the substring.
	// If the first character is not a delimiter, 0 is returned.
	template <class SkipOperation>
	CActionSplitEnumerator<CBaseStringPart<T>, SkipOperation> SplitByAction( const SkipOperation& skipAction ) const
		{ return CActionSplitEnumerator<CBaseStringPart<T>, SkipOperation>( *this, skipAction ); }
};

//////////////////////////////////////////////////////////////////////////

template <class T>
T CBaseStringPart<T>::operator[]( int pos ) const
{
	assert( pos >= 0 );
	assert( pos < this->Length() );
	return this->begin()[pos];
}

template <class T>
CBaseStringPart<T> CBaseStringPart<T>::Mid( int first, int count ) const
{
	assert( first >= 0 );
	assert( count >= 0 );
	assert( first + count <= this->Length() );
	return CBaseStringPart<T>( this->begin() + first, count );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
