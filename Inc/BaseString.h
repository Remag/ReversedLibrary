#pragma once
#include <StringData.h>
#include <StringOperations.h>
#include <StringAllocator.h>
#include <RawBuffer.h>
#include <SplitEnumerators.h>
#include <StringSearchEnumerator.h>

namespace Relib {

extern const REAPI CError Err_BadArchive;

namespace RelibInternal {

// An opposite character type. Returns char for wchar_t parameters and wchar_t for char parameters.
template <class CharType>
struct COppositeChar {};

template <>
struct COppositeChar<char> {
	typedef wchar_t Result;
};

template <>
struct COppositeChar<wchar_t> {
	typedef char Result;
};

//////////////////////////////////////////////////////////////////////////

// Base class for a string.
// String owns the contained string data.
// String is guaranteed to be null-terminated.
template <class T>
class CBaseString : public CStringData<T> {
public:
	CBaseString() = default;

	// Explicit copy operation.
	CBaseString( const CStringData<T>& other, const CExplicitCopyTag& );

	// Initialize with a null-terminated buffer.
	explicit CBaseString( const T* buffer );
	// Initialize with a string and its length.
	CBaseString( const T* buffer, int bufferLength );
	// Initialize with a string part.
	explicit CBaseString( CBaseStringPart<T> part ) : CBaseString( part.begin(), part.Length() ) {}
	// Initialize with a null-terminated view.
	explicit CBaseString( CBaseStringView<T> view ) : CBaseString( view.Ptr(), view.Length() ) {}

	// Deallocation.
	~CBaseString();

	// Movement.
	CBaseString( CBaseString<T>&& other );
	CBaseString& operator=( CBaseString<T>&& other );

	// Assignment.
	CBaseString& operator=( const CBaseString<T>& other );
	CBaseString& operator=( const T* buffer );
	CBaseString& operator=( CBaseStringView<T> part );
	CBaseString& operator=( CBaseStringPart<T> part );

	operator CBaseStringPart<T>() const
		{ return CBaseStringPart<T>( Ptr(), this->Length() ); }
	operator CBaseStringView<T>() const
		{ return CBaseStringView<T>( Ptr(), this->Length() ); }

	int Capacity() const;

	T operator[]( int pos ) const;
	T& operator[]( int pos );
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
	int FindNoCase( CBaseStringPart<T> substring, int from = 0 ) const
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

	auto FindAll( T symbol ) const
		{ return CStringSearchEnumerator<CBaseStringView<T>, T>( *this, symbol ); }
	auto FindAll( CBaseStringView<T> substring ) const
		{ return CStringSearchEnumerator<CBaseStringView<T>, CBaseStringView<T>>( *this, substring ); }
	auto FindAllNoCase( CBaseStringView<T> substring ) const
		{ return CNoCaseStringSearchEnumerator<CBaseStringView<T>, CBaseStringView<T>>( *this, substring ); }

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

	// Increase buffer size if necessary.
	void ReserveBuffer( int length );

	// Appending.
	CBaseString<T>& operator+=( CBaseStringPart<T> string );

	// Case change.
	void MakeUpper()
		{ CStringOperations<T>::MakeUpper( this->getWritableBuffer(), this->Length() ); }
	void MakeLower()
		{ CStringOperations<T>::MakeLower( this->getWritableBuffer(), this->Length() ); }
	// Reversing.
	void Reverse()
		{ CStringOperations<T>::Reverse( this->getWritableBuffer() ); }

	// Low-level work.
	CRawStringBuffer<T> CreateRawBuffer( int length );
	CRawStringBuffer<T> CreateRawBuffer();

	// Redaction.
	void Empty();
	void DeleteAt( int pos, int count = 1 );
	void DeleteFrom( int pos )
		{ DeleteAt( pos, this->Length() - pos ); }
	void InsertAt( int pos, CBaseStringPart<T> part );
	void ReplaceAt( int pos, CBaseStringPart<T> part, int count = 1 );

	void ReplaceAll( CBaseStringPart<T> source, CBaseStringPart<T> newString );

	// Raw string buffer needs to adjust the strings length on release.
	friend class CRawStringBuffer<T>;

private:
	int allocatedSize = 0;

	CBaseString( CRawBuffer rawBuffer, const T* str, int strLength );

	static int getRequiredSize( int strLength );
	static void copyBuffer( const T* src, T* dest, int srcLength );
	void createBuffer( const T* src, int srcLength, int bufferSize );
	void freeBuffer();
	void markBufferLength( int newLength );

	// Copy construction is prohibited.
	CBaseString( CBaseString& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class T>
int CBaseString<T>::Capacity() const
{
	return allocatedSize / sizeof( T ) - 1;
}

template <class T>
CBaseString<T>::CBaseString( const T* buffer ) :
	CBaseString( buffer, CStringOperations<T>::Length( buffer ) )
{
}

template <class T>
CBaseString<T>::CBaseString( const T* str, int strLength ) :
	CBaseString( GetStringAllocator().AllocateSized( getRequiredSize( strLength ) ), str, strLength )
{
}

template <class T>
int CBaseString<T>::getRequiredSize( int strLength )
{
	return sizeof( T ) * ( strLength + 1 );
}

template <class T>
CBaseString<T>::CBaseString( CRawBuffer rawBuffer, const T* str, int strLength ) :
	CStringData<T>( static_cast<T*>( rawBuffer.Ptr() ), strLength ),
	allocatedSize( rawBuffer.Size() )
{
	copyBuffer( str, this->getWritableBuffer(), strLength );
}

template <class T>
void CBaseString<T>::copyBuffer( const T* src, T* dest, int srcLength )
{
	memcpy( dest, src, srcLength * sizeof( T ) );
	dest[srcLength] = 0;
}

template <class T>
CBaseString<T>::~CBaseString()
{
	freeBuffer(); 
}

template <class T>
void CBaseString<T>::freeBuffer()
{
	if( allocatedSize > 0 ) {
		GetStringAllocator().Free( { this->getWritableBuffer(), allocatedSize } );
	}
}

template <class T>
CBaseString<T>::CBaseString( const CStringData<T>& other, const CExplicitCopyTag& ) :
	CBaseString( other.begin(), other.Length() )
{
}

template <class T>
CBaseString<T>::CBaseString( CBaseString<T>&& other ) :
	CStringData<T>( other.Ptr(), other.Length() ),
	allocatedSize( other.allocatedSize )
{
	other.CStringData<T>::operator=( {} );
	other.allocatedSize = 0;
}

template <class T>
CBaseString<T>& CBaseString<T>::operator=( const CBaseString<T>& other )
{
	return operator=( CBaseStringPart<T>( other.Ptr(), other.Length() ) );
}

template <class T>
CBaseString<T>& CBaseString<T>::operator=( CBaseString<T>&& other )
{
	freeBuffer();

	CStringData<T>::operator=( other );
	allocatedSize = other.allocatedSize;
	other.CStringData<T>::operator=( {} );
	other.allocatedSize = 0;
	return *this;
}

template <class T>
CBaseString<T>& CBaseString<T>::operator=( const T* newBuffer )
{
	return operator=( CBaseStringPart<T>( newBuffer, CStringOperations<T>::Length( newBuffer ) ) );
}

template <class T>
CBaseString<T>& CBaseString<T>::operator=( CBaseStringView<T> part )
{
	return operator=( CBaseStringPart<T>( part.Ptr(), part.Length() ) );
}

template <class T>
CBaseString<T>& CBaseString<T>::operator=( CBaseStringPart<T> part )
{
	const int newLength = part.Length();
	const int newBufferSize = getRequiredSize( newLength );
	const T* partBuffer = part.begin();
	T* oldBuffer = this->getWritableBuffer();
	if( newBufferSize <= allocatedSize ) {
		// New string can fit into the allocated size, no need to reallocate.
		::memmove( oldBuffer, partBuffer, newBufferSize - sizeof( T ) );
		oldBuffer[newLength] = 0;
		this->setBufferLength( newLength );
	} else {
		// Free the old buffer and allocate a new one.
		createBuffer( partBuffer, newLength, newBufferSize );
	}
	return *this;
}

template <class T>
void CBaseString<T>::createBuffer( const T* src, int srcLength, int bufferSize )
{
	CRawBuffer newBuffer = GetStringAllocator().AllocateSized( bufferSize );
	T* bufferPtr = static_cast<T*>( newBuffer.Ptr() );
	copyBuffer( src, bufferPtr, srcLength );
	freeBuffer();
	CStringData<T>::operator=( { bufferPtr, srcLength } );
	allocatedSize = newBuffer.Size();
}

template <class T>
T CBaseString<T>::operator[]( int pos ) const
{
	assert( pos >= 0 );
	assert( pos <= this->Length() );
	return Ptr()[pos];
}

template <class T>
T& CBaseString<T>::operator[]( int pos )
{
	assert( pos >= 0 );
	assert( pos < this->Length() );
	return this->getWritableBuffer()[pos];
}

template <class T>
CBaseStringPart<T> CBaseString<T>::Mid( int first, int count ) const
{
	assert( first >= 0 );
	assert( count >= 0 );
	assert( first + count <= this->Length() );
	return CBaseStringPart<T>( Ptr() + first, count );
}

template <class T>
CBaseStringView<T> CBaseString<T>::Mid( int first ) const
{
	assert( first >= 0 );
	assert( first <= this->Length() );
	return CBaseStringView<T>( Ptr() + first, this->Length() - first );
}

template <class T>
CBaseStringView<T> CBaseString<T>::Right( int count ) const
{
	assert( count >= 0 );
	assert( count <= this->Length() );
	return CBaseStringView<T>( Ptr() + this->Length() - count, count );
}

template <class T>
CBaseStringView<T> CBaseString<T>::TrimLeft() const
{
	const auto trimmedPart = CStringOperations<T>::TrimLeft( *this );
	return CBaseStringView<T>( trimmedPart.begin(), trimmedPart.Length() );
}

template <class T>
void CBaseString<T>::ReserveBuffer( int length )
{
	assert( length >= 0 );
	const int newBufferSize = getRequiredSize( length );
	if( newBufferSize > allocatedSize ) {
		createBuffer( Ptr(), this->Length(), newBufferSize );
	}
}

template <class T>
CBaseString<T>& CBaseString<T>::operator+=( CBaseStringPart<T> string )
{
	const int length = this->Length();
	const int srcLength = string.Length();
	const T* strBuffer = string.begin();
	assert( srcLength >= 0 );

	const int newLength = length + srcLength;
	const int newBufferSize = getRequiredSize( newLength );
	if( allocatedSize < newBufferSize ) {
		// Create a new buffer, copy the data into it and replace the old one.
		CRawBuffer newBuffer = GetStringAllocator().AllocateSized( newBufferSize );
		T* bufferPtr = static_cast<T*>( newBuffer.Ptr() );
		memcpy( bufferPtr, Ptr(), length * sizeof( T ) );
		memcpy( bufferPtr + length, strBuffer, srcLength * sizeof( T ) );
		bufferPtr[newLength] = 0;
		freeBuffer();
		CStringData<T>::operator=( { bufferPtr, newLength } );
		allocatedSize = newBuffer.Size();
	} else {
		// Simply copy the data into existing buffer.
		memcpy( this->getWritableBuffer() + length, strBuffer, srcLength * sizeof( T ) );
		markBufferLength( newLength );
	}

	return *this;
}

template <class T>
CRawStringBuffer<T> CBaseString<T>::CreateRawBuffer()
{
	if( allocatedSize == 0 ) {
		createBuffer( Ptr(), 0, sizeof( T ) );
	}

	return CRawStringBuffer<T>( *this );
}

template <class T>
CRawStringBuffer<T> CBaseString<T>::CreateRawBuffer( int length ) 
{
	ReserveBuffer( length );
	this->getWritableBuffer()[length] = 0;
	return CRawStringBuffer<T>( *this );
}

template <class T>
void CBaseString<T>::markBufferLength( int newLength )
{
	this->getWritableBuffer()[newLength] = 0;
	this->setBufferLength( newLength );
}

template <class T>
void CBaseString<T>::Empty()
{
	markBufferLength( 0 );
}

template <class T>
void CBaseString<T>::DeleteAt( int pos, int count /*= 1 */ )
{
	const int length = this->Length();
	assert( pos >= 0 && count >= 0 );
	assert( pos + count <= length );

	T* targetBuffer = this->getWritableBuffer() + pos;
	const int newLength = length - count;
	::memmove( targetBuffer, targetBuffer + count, ( newLength - pos ) * sizeof( T ) );
	markBufferLength( newLength );
}

template <class T>
void CBaseString<T>::InsertAt( int pos, CBaseStringPart<T> part )
{
	const int length = this->Length();
	const int srcLength = part.Length();
	const T* srcBuffer = part.begin();
	assert( pos >= 0 && pos <= length );
	assert( srcLength >= 0 );

	const int newLength = length + srcLength;
	ReserveBuffer( newLength );
	T* targetBuffer = this->getWritableBuffer() + pos;
	::memmove( targetBuffer + srcLength, targetBuffer, ( length - pos ) * sizeof( T ) );
	memcpy( targetBuffer, srcBuffer, srcLength * sizeof( T ) );
	markBufferLength( newLength );
}

template <class T>
void CBaseString<T>::ReplaceAt( int pos, CBaseStringPart<T> part, int count /*= 1 */ )
{
	const int length = this->Length();
	const int srcLength = part.Length();

	assert( srcLength >= 0 );
	assert( pos >= 0 && pos <= length );
	assert( count >= 0 );
	assert( pos + count <= length );

	const int shift = srcLength - count;
	const int growSize = max( 0, shift );

	const int newBufferLength = length + growSize;
	ReserveBuffer( newBufferLength );
	const auto strBuffer = this->getWritableBuffer();
	strBuffer[newBufferLength] = 0;
	T* newBuffer = strBuffer + pos;

	if( shift != 0 ) {
		::memmove( newBuffer + count + shift, newBuffer + count, ( length - pos - count ) * sizeof( T ) );
	}
	memcpy( newBuffer, part.begin(), srcLength * sizeof( T ) );
	const int shiftedLength = length + shift;
	strBuffer[shiftedLength] = 0;
	this->setBufferLength( shiftedLength );
}

template <class T>
void CBaseString<T>::ReplaceAll( CBaseStringPart<T> source, CBaseStringPart<T> newString )
{
	const int srcLength = source.Length();
	assert( srcLength > 0 );
	const int newStringLength = newString.Length();
	for( int i = 0; i < this->Length() - srcLength + 1; ) {
		const CBaseStringPart<T> subStr = Mid( i, srcLength );
		if( subStr == source ) {
			ReplaceAt( i, newString, srcLength );
			i += newStringLength;
		} else {
			i++;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

// Concatenation operators.
template <class T>
CBaseString<T> operator+( CStringData<T> string, T ch )
{
	assert( ch != 0 );
	return CStringOperations<T>::Concatenate( string.begin(), string.Length(), &ch, 1 );
}

template <class T>
CBaseString<T> operator+( CBaseString<T>&& string, T ch )
{
	assert( ch != 0 );
	string += ch;
	return move( string );
}

template <class T>
CBaseString<T> operator+( T ch, CStringData<T> string )
{
	assert( ch != 0 );
	return CStringOperations<T>::Concatenate( &ch, 1, string.begin(), string.Length() );
}

template <class T>
CBaseString<T> operator+( T ch, CBaseString<T>&& string )
{
	assert( ch != 0 );
	return CStringOperations<T>::Concatenate( &ch, 1, move( string ) );
}

template <class T>
CBaseString<T> operator+( CStringData<T> left, const T* right )
{
	assert( right != nullptr );
	const int rightLen = CStringOperations<T>::Length( right );
	return CStringOperations<T>::Concatenate( left.begin(), left.Length(), right, rightLen );
}

template <class T>
CBaseString<T> operator+( CBaseString<T>&& left, const T* right )
{
	assert( right != nullptr );
	left += right;
	return move( left );
}

template <class T>
CBaseString<T> operator+( const T* left, CStringData<T> right )
{
	assert( left != nullptr );
	const int leftLen = CStringOperations<T>::Length( left );
	return CStringOperations<T>::Concatenate( left, leftLen, right.begin(), right.Length() );
}

template <class T>
CBaseString<T> operator+( const T* left, CBaseString<T>&& right )
{
	assert( left != nullptr );
	const int leftLen = CStringOperations<T>::Length( left );
	return CStringOperations<T>::Concatenate( left, leftLen, move( right ) );
}

template <class T>
CBaseString<T> operator+( CStringData<T> leftStr, CStringData<T> rightStr )
{
	return CStringOperations<T>::Concatenate( leftStr.begin(), leftStr.Length(), rightStr.begin(), rightStr.Length() );
}

template <class T>
CBaseString<T> operator+( CBaseString<T>&& leftStr, CStringData<T> rightStr )
{
	leftStr += rightStr;
	return move( leftStr );
}

template <class T>
CBaseString<T> operator+( CStringData<T> leftStr, CBaseString<T>&& rightStr )
{
	return CStringOperations<T>::Concatenate( leftStr.begin(), leftStr.Length(), move( rightStr ) );
}

template <class T>
CBaseString<T> operator+( CBaseString<T>&& leftStr, const CBaseString<T>& rightStr )
{
	leftStr += rightStr;
	return move( leftStr );
}

template <class T>
CBaseString<T> operator+( CBaseString<T>&& leftStr, CBaseString<T>&& rightStr )
{
	leftStr += rightStr;
	return move( leftStr );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
