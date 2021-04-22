#pragma once
#include <Reassert.h>

namespace Relib {

namespace RelibInternal {

template <class T>
class CStringOperations;
//////////////////////////////////////////////////////////////////////////

// Common string data.
template <class T>
class CStringData {
public:
	typedef T TSymbolType;

	CStringData() : buffer( &emptyBufferStr ), bufferLength( 0 ) {}
	// Initialize with a null-terminated buffer.
	CStringData( const T* _buffer ) : CStringData( _buffer, CStringOperations<T>::Length( _buffer ) ) {}
	CStringData( const T* _buffer, int _bufferLength ) : buffer( _buffer ), bufferLength( _bufferLength ) { assert( bufferLength >= 0 ); }

	int Length() const
		{ return bufferLength; }
	bool IsEmpty() const
		{ return bufferLength == 0; }

	// Create a string part from this data.
	CBaseStringPart<T> GetPart() const
		{ return CBaseStringPart<T>( buffer, bufferLength ); }

	T First() const
		{ assert( !IsEmpty() ); return buffer[0]; }
	T Last() const
		{ assert( !IsEmpty() ); return buffer[bufferLength - 1]; }

	// Comparison.
	int CompareNoCase( CBaseStringPart<T> string ) const
		{ return CStringOperations<T>::CompareNoCase( *this, string ); }
	bool EqualsNoCase( CBaseStringPart<T> string ) const
		{ return CStringOperations<T>::CompareNoCase( *this, string ) == 0; }
	bool HasPrefix( CBaseStringPart<T> prefix ) const
		{ return CStringOperations<T>::HasPrefix( *this, prefix ); }
	bool HasPrefixNoCase( CBaseStringPart<T> prefix ) const
		{ return CStringOperations<T>::HasPrefixNoCase( *this, prefix ); }
	bool HasSuffix( CBaseStringPart<T> suffix ) const
		{ return CStringOperations<T>::HasSuffix( *this, suffix ); }
	bool HasSuffixNoCase( CBaseStringPart<T> suffix ) const
		{ return CStringOperations<T>::HasSuffixNoCase( *this, suffix ); }

	// Get a part without spaces on the edges.
	CBaseStringPart<T> TrimRight() const
		{ return CStringOperations<T>::TrimRight( *this ); }
	CBaseStringPart<T> TrimSpaces() const
		{ return CStringOperations<T>::TrimSpaces( *this ); }

	// Static utility functions.
	static bool IsCharWhiteSpace( T testChar )
		{ return CStringOperations<T>::IsCharWhiteSpace( testChar ); }
	static bool IsCharDigit( T testChar )
		{ return CStringOperations<T>::IsCharDigit( testChar ); }

	// Range-based for loops support.
	const T* begin() const
		{ return buffer; }
	const T* end() const
		{ return buffer + bufferLength; }

protected:
	// Editable string buffer gets access to non-const buffer.
	T* getWritableBuffer()
		{ return const_cast<T*>( begin() ); }

	void setBufferLength( int newValue )
		{ bufferLength = newValue; }

private:
	// String buffer.
	const T* buffer = nullptr;
	// Buffer size in symbols.
	int bufferLength = 0;

	// Empty strings point to this buffer.
	static T emptyBufferStr;
};

template <class T>
T CStringData<T>::emptyBufferStr;

//////////////////////////////////////////////////////////////////////////
// Comparison.

template <class T>
bool operator==( RelibInternal::CStringData<T> left, RelibInternal::CStringData<T> right )
{
	return CStringOperations<T>::Equals( left, right );
}

template <class T>
bool operator==( RelibInternal::CStringData<T> left, const T* right )
{
	return left == RelibInternal::CStringData<T>( right );
}

template <class T>
bool operator==( const T* left, RelibInternal::CStringData<T> right )
{
	return right == RelibInternal::CStringData<T>( left );
}

template <class T>
bool operator!=( RelibInternal::CStringData<T> left, RelibInternal::CStringData<T> right )
{
	return !CStringOperations<T>::Equals( left, right );
}

template <class T>
bool operator!=( RelibInternal::CStringData<T> left, const T* right )
{
	return left != RelibInternal::CStringData<T>( right );
}

template <class T>
bool operator!=( const T* left, RelibInternal::CStringData<T> right )
{
	return right != RelibInternal::CStringData<T>( left );
}

template <class T>
bool operator<( RelibInternal::CStringData<T> left, RelibInternal::CStringData<T> right )
{
	return CStringOperations<T>::Compare( left, right ) < 0;
}

template <class T>
bool operator<( RelibInternal::CStringData<T> left, const T* right )
{
	return left < RelibInternal::CStringData<T>( right );
}

template <class T>
bool operator<( const T* left, RelibInternal::CStringData<T> right )
{
	return RelibInternal::CStringData<T>( left ) < right;
}

template <class T>
bool operator<=( RelibInternal::CStringData<T> left, RelibInternal::CStringData<T> right )
{
	return CStringOperations<T>::Compare( left, right ) <= 0;
}

template <class T>
bool operator<=( RelibInternal::CStringData<T> left, const T* right )
{
	return left <= RelibInternal::CStringData<T>( right );
}

template <class T>
bool operator<=( const T* left, RelibInternal::CStringData<T> right )
{
	return RelibInternal::CStringData<T>( left ) <= right;
}

template <class T>
bool operator>( RelibInternal::CStringData<T> left, RelibInternal::CStringData<T> right )
{
	return CStringOperations<T>::Compare( left, right ) > 0;
}

template <class T>
bool operator>( RelibInternal::CStringData<T> left, const T* right )
{
	return left > RelibInternal::CStringData<T>( right );
}

template <class T>
bool operator>( const T* left, RelibInternal::CStringData<T> right )
{
	return RelibInternal::CStringData<T>( left ) > right;
}

template <class T>
bool operator>=( RelibInternal::CStringData<T> left, RelibInternal::CStringData<T> right )
{
	return CStringOperations<T>::Compare( left, right ) >= 0;
}

template <class T>
bool operator>=( RelibInternal::CStringData<T> left, const T* right )
{
	return left >= RelibInternal::CStringData<T>( right );
}

template <class T>
bool operator>=( const T* left, RelibInternal::CStringData<T> right )
{
	return RelibInternal::CStringData<T>( left ) >= right;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

