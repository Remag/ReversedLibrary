#pragma once
#include <Redefs.h>
#include <Errors.h>
#include <FileViews.h>
#include <Map.h>
#include <HashTable.h>
#include <PagedBitSet.h>
#include <SafeCounters.h>
#include <ObjectCreation.h>
#include <Serializable.h>
#include <PtrOwner.h>
#include <TemplateUtils.h>
#include <Color.h>

namespace Relib {

extern const REAPI CError Err_SmallArchive;

namespace RelibInternal {

// Class for data serialization.
class REAPI CArchive {
public:
	CArchive();

protected:
	// Read/Write methods for arbitrary data.
	void read( void* ptr, int size );
	void write( const void* ptr, int size );
	bool isEndOfReading() const;

	CArray<BYTE>& getBuffer()
		{ return buffer; }
	int getBufferSize() const
		{ return buffer.Size(); }

	void increaseBuffer( int bufferSize );
	void attachBuffer( CArray<BYTE> newBuffer );
	CArray<BYTE> detachBuffer();

	void skip( int byteCount );

	// Serialization for small values.
	// Integers from 0 to 254 are stored in 1 byte.
	// Other integers are stored in 5 bytes.
	int readSmallValue();
	void writeSmallValue( int value );

	// Custom serializable read/write.
	CSharedPtr<ISerializable> readObject();
	void writeObject( const ISerializable* object );

	// Enumeration type read/write.
	template<class Type>
	Type readEnum();
	template<class Type>
	void writeEnum( Type enumValue );

	template<class Type>
	void readSimpleType( Type& var );
	template<class Type>
	void writeSimpleType( Type var );

	// Serializes a version of an archive.
	// If the version in the archive is bigger than currentVersion. The archive is incompatible with the program and an exception is thrown.
	int readVersion( int currentVersion );
	int writeVersion( int currentVersion );
	
private:
	// Buffer to be written to the file.
	CArray<BYTE> buffer;
	// Current buffer position.
	int currentBufferPos;

	// Map that returns creationFunctionsBuffer's index for an object's name.
	CMap<CUnicodeString, int> objectNamesDictionary;
	// Buffer for quick access to object's creation functions. Filled only in reading mode.
	CArray<const CBaseObjectCreationFunction*> creationFunctionsBuffer;

	CPtrOwner<ISerializable> readUniqueObject();

	void writeExternalName( CUnicodeView name );
	const CBaseObjectCreationFunction* readCreationFunctionPtr( int objectId );
};

//////////////////////////////////////////////////////////////////////////

inline void CArchive::read( void* ptr, int size )
{
	assert( size >= 0 );
	check( size < buffer.Size(), Err_SmallArchive );
	if( size > 0 ) {
		::memcpy( ptr, buffer.Ptr() + currentBufferPos, size );
		currentBufferPos += size;
	}
}

inline void CArchive::write( const void* ptr, int size )
{
	assert( size >= 0 );
	if( size + currentBufferPos >= buffer.Size() ) {
		buffer.IncreaseSizeNoInitialize( size + currentBufferPos + 1 );
	}
	::memcpy( buffer.Ptr() + currentBufferPos, ptr, size );
	currentBufferPos += size;
}

inline bool CArchive::isEndOfReading() const
{
	return currentBufferPos == buffer.Size();
}

template<class Type>
Type CArchive::readEnum()
{
	staticAssert( Types::IsEnum<Type>::Result );
	return Type( readSmallValue() );
}

template<class Type>
void CArchive::writeEnum( Type enumValue )
{
	staticAssert( Types::IsEnum<Type>::Result );
	writeSmallValue( enumValue );
}

template<class Type>
inline void CArchive::readSimpleType( Type& var )
{
	read( &var, sizeof( Type ) );
}

template<class Type>
inline void CArchive::writeSimpleType( Type var )
{
	write( &var, sizeof( Type ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// Class that reads and serializes data from a given file into the data structures.
class REAPI CArchiveReader : public RelibInternal::CArchive {
public:
	explicit CArchiveReader( CStringPart fileName );
	explicit CArchiveReader( CFileReadView _file );
	explicit CArchiveReader( CArray<BYTE> _fileData );

	void Skip( int byteCount )
		{ skip( byteCount ); }

	bool IsEndOfArchive() const
		{ return isEndOfReading(); }
	
	int ReadSmallValue()
		{ return readSmallValue(); }
	int ReadVersion( int currentVersion )
		{ return readVersion( currentVersion ); }
	void Read( void* ptr, int size )
		{ read( ptr, size ); }

	template <class Enum>
	Enum ReadEnum()
		{ return readEnum<Enum>(); }

	friend CArchiveReader& operator>>( CArchiveReader& archive, bool& var );
	friend CArchiveReader& operator>>( CArchiveReader& archive, char& var );
	friend CArchiveReader& operator>>( CArchiveReader& archive, signed char& var );
	friend CArchiveReader& operator>>( CArchiveReader& archive, unsigned char& var );
	friend CArchiveReader& operator>>( CArchiveReader& archive, wchar_t& var );
	friend CArchiveReader& operator>>( CArchiveReader& archive, unsigned short& var );
	friend CArchiveReader& operator>>( CArchiveReader& archive, short& var );
	friend CArchiveReader& operator>>( CArchiveReader& archive, int& var );
	friend CArchiveReader& operator>>( CArchiveReader& archive, unsigned int& var );
	friend CArchiveReader& operator>>( CArchiveReader& archive, long& var );
	friend CArchiveReader& operator>>( CArchiveReader& archive, unsigned long& var );
	friend CArchiveReader& operator>>( CArchiveReader& archive, __int64& var );
	friend CArchiveReader& operator>>( CArchiveReader& archive, unsigned __int64& var );
	friend CArchiveReader& operator>>( CArchiveReader& archive, float& var );
	friend CArchiveReader& operator>>( CArchiveReader& archive, double& var );

	template<class ObjectType>
	friend CArchiveReader& operator>>( CArchiveReader& archive, CSharedPtr<ObjectType>& object );
	template<class ObjectType>
	friend CArchiveReader& operator>>( CArchiveReader& archive, CPtrOwner<ObjectType>& object );

private:
	void handleArchiveFlags();
};

//////////////////////////////////////////////////////////////////////////

extern const REAPI CError Err_BadArchive;
inline CArchiveReader& operator>>( CArchiveReader& archive, bool& var )
{
	BYTE byte;
	archive.readSimpleType( byte );
	check( byte == 0 || byte == 1, Err_BadArchive );
	var = byte == 1;
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, char& var )
{
	archive.readSimpleType( var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, signed char& var )
{
	archive.readSimpleType( var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, unsigned char& var )
{
	archive.readSimpleType( var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, wchar_t& var )
{
	archive.readSimpleType( var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, short& var )
{
	archive.readSimpleType( var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, unsigned short& var )
{
	archive.readSimpleType( var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, int& var )
{
	archive.readSimpleType( var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, unsigned int& var )
{
	archive.readSimpleType( var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, long& var )
{
	archive.readSimpleType( var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, unsigned long& var )
{
	archive.readSimpleType( var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, __int64& var )
{
	archive.readSimpleType( var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, unsigned __int64& var )
{
	archive.readSimpleType( var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, float& var )
{
	archive.readSimpleType( var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, double& var )
{
	archive.readSimpleType( var );
	return archive;
}

template <class ObjectType>
CArchiveReader& operator>>( CArchiveReader& archive, CSharedPtr<ObjectType>& object )
{
	object = ptr_dynamic_cast<ObjectType>( archive.readObject() );
	return archive;
}

template<class ObjectType>
inline CArchiveReader& operator>>( CArchiveReader& archive, CPtrOwner<ObjectType>& object )
{
	object = ptr_dynamic_cast<ObjectType>( archive.readUniqueObject() );
	return archive;
}

//////////////////////////////////////////////////////////////////////////

// Class that binarizes the data. It must be flushed to a valid source before it's destroyed. 
class REAPI CArchiveWriter : public RelibInternal::CArchive {
public:
	explicit CArchiveWriter( int bufferSize = 0 );

	~CArchiveWriter();

	void FlushToFile( CStringPart fileName );
	void FlushToCompressedFile( CStringPart fileName );
	CArray<BYTE> FlushToByteString();

	void Skip( int byteCount )
		{ skip( byteCount ); }

	void WriteSmallValue( int value )
		{ writeSmallValue( value ); }
	int WriteVersion( int currentVersion )
		{ return writeVersion( currentVersion ); }
	void Write( const void* ptr, int size )
		{ write( ptr, size ); }

	template <class Enum>
	void WriteEnum( Enum value )
		{ writeEnum( value ); }

	friend CArchiveWriter& operator<<( CArchiveWriter& archive, bool var );
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, char var );
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, signed char var );
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned char var );
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, wchar_t var );
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, short var );
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned short var );
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, int var );
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned int var );
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, long var );
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned long var );
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, __int64 var );
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned __int64 var );
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, float var );
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, double var );

	template<class ObjectType>
	friend CArchiveWriter& operator<<( CArchiveWriter& archive, const CPtrOwner<ObjectType>& object );

private:
	void writeArchiveFlag( BYTE flagValue, CArray<BYTE>& dest ) const;
};

//////////////////////////////////////////////////////////////////////////

inline CArchiveWriter& operator<<( CArchiveWriter& archive, bool var )
{
	BYTE byte = numeric_cast<BYTE>( var );
	// var should be initialized.
	assert( byte == 0 || byte == 1 );
	archive.writeSimpleType( byte );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, char var )
{
	archive.writeSimpleType( var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, signed char var )
{
	archive.writeSimpleType( var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned char var )
{
	archive.writeSimpleType( var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, wchar_t var )
{
	archive.writeSimpleType( var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, short var )
{
	archive.writeSimpleType( var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned short var )
{
	archive.writeSimpleType( var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, int var )
{
	archive.writeSimpleType( var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned int var )
{
	archive.writeSimpleType( var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, long var )
{
	archive.writeSimpleType( var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned long var )
{
	archive.writeSimpleType( var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, __int64 var )
{
	archive.writeSimpleType( var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned __int64 var )
{
	archive.writeSimpleType( var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, float var )
{
	archive.writeSimpleType( var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, double var )
{
	archive.writeSimpleType( var );
	return archive;
}

template <class ObjectType>
CArchiveWriter& operator<<( CArchiveWriter& archive, const ObjectType* object )
{
	staticAssert( ( Types::IsDerivedFrom<ObjectType, ISerializable>::Result ) );

	archive.writeObject( object );
	return archive;
}

// Template operators for objects.
template <class ObjectType>
CArchiveWriter& operator<<( CArchiveWriter& archive, const CSharedPtr<ObjectType>& object )
{
	archive.writeObject( object );
	return archive;
}

template<class ObjectType>
inline CArchiveWriter& operator<<( CArchiveWriter& archive, const CPtrOwner<ObjectType>& object )
{
	staticAssert( ( Types::IsDerivedFrom<ObjectType, ISerializable>::Result ) );

	archive.writeObject( object );
	return archive;
}

//////////////////////////////////////////////////////////////////////////

// Array serialization.

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
CArchiveWriter& operator<<( CArchiveWriter& archive, const CArray<Elem, Allocator, GrowStrategy>& arr )
{
	archive << arr.Size();
	for( const auto& elem : arr ) {
		archive << elem;
	}
	return archive;
}

template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
CArchiveReader& operator>>( CArchiveReader& archive, CArray<Elem, Allocator, GrowStrategy>& arr )
{
	arr.Empty();
	int size;
	archive >> size;
	check( size >= 0, Err_BadArchive );
	arr.IncreaseSize( size );
	for( int i = 0; i < size; i++ ) {
		archive >> arr[i];
	}
	return archive;
}

//////////////////////////////////////////////////////////////////////////
// Static array serialization.

template <class Elem, class Allocator>
CArchiveWriter& operator<<( CArchiveWriter& archive, const CStaticArray<Elem, Allocator>& arr )
{
	archive << arr.Size();
	for( const auto& elem : arr ) {
		archive << elem;
	}
	return archive;
}

template <class Elem, class Allocator>
CArchiveReader& operator>>( CArchiveReader& archive, CStaticArray<Elem, Allocator>& arr )
{
	arr.Empty();
	int size;
	archive >> size;
	check( size >= 0, Err_BadArchive );
	arr.ResetSize( size );
	for( int i = 0; i < size; i++ ) {
		archive >> arr[i];
	}
	return archive;
}

//////////////////////////////////////////////////////////////////////////
// Persistent storage serialization.

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
CArchiveWriter& operator<<( CArchiveWriter& archive, const CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>& arr )
{
	archive << arr.Size();
	for( const auto& elem : arr ) {
		archive << elem;
	}
	return archive;
}

template <class Elem, int groupSize, class GeneralAllocator /*= CRuntimeHeap*/, class GroupAllocator /*= CRuntimeHeap*/>
CArchiveReader& operator>>( CArchiveReader& archive, CPersistentStorage<Elem, groupSize, GeneralAllocator, GroupAllocator>& arr )
{
	arr.Empty();
	int size;
	archive >> size;
	check( size >= 0, Err_BadArchive );
	arr.IncreaseSize( size );
	for( int i = 0; i < size; i++ ) {
		archive >> arr[i];
	}
	return archive;
}

//////////////////////////////////////////////////////////////////////////

// String serialization.
template <class T>
CArchiveWriter& operator<<( CArchiveWriter& archive, RelibInternal::CStringData<T> str )
{
	const int length = str.Length();
	const T* buffer = str.begin();
	archive.WriteSmallValue( length );
	archive.Write( buffer, length * sizeof( T ) );
	return archive;	
}

template <class T>
CArchiveReader& operator>>( CArchiveReader& archive, RelibInternal::CBaseString<T>& str )
{
	str.Empty();
	const int length = archive.ReadSmallValue();
	if( length < 0 ) {
		check( false, Err_BadArchive );
	}
	if( length == 0 ) {
		return archive;
	}
	auto buffer = str.CreateRawBuffer( length );
	archive.Read( buffer, length * sizeof( T ) );
	buffer.Release( length );
	return archive;
}

//////////////////////////////////////////////////////////////////////////
// Hash table serialization.

template<class Elem, class HashStrategy, class Allocator>
CArchiveWriter& operator<<( CArchiveWriter& archive, const CHashTable<Elem, HashStrategy, Allocator>& dict )
{
	const int length = dict.Size();
	archive << length;
	for( const auto& entry : dict ) {
		archive << entry;
	}
	return archive;
}

template<class Elem, class HashStrategy, class Allocator>
CArchiveReader& operator>>( CArchiveReader& archive, CHashTable<Elem, HashStrategy, Allocator>& dict )
{
	dict.Empty();
	int length;
	archive >> length;
	dict.ReserveBuffer( length );
	for( int i = 0; i < length; i++ ) {
		Elem val;
		archive >> val;
		dict.Set( move( val ) );
	}
	return archive;
}

//////////////////////////////////////////////////////////////////////////
// Map serialization.

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
CArchiveWriter& operator<<( CArchiveWriter& left, const CMap<KeyType, ValueType, HashStrategy, Allocator>& right )
{
	const int length = right.Size();
	left << length;
	for( auto& entry : right ) {
		left << entry.Key() << entry.Value();
	}

	return left;
}

template<class KeyType, class ValueType, class HashStrategy, class Allocator>
CArchiveReader& operator>>( CArchiveReader& left, CMap<KeyType, ValueType, HashStrategy, Allocator>& right )
{
	right.Empty();
	int length;
	left >> length;
	right.ReserveBuffer( length );
	for( int i = 0; i < length; i++ ) {
		KeyType key;
		left >> key;
		auto& entry = right.Add( move( key ) );
		left >> entry.Value();
	}

	return left;
}

//////////////////////////////////////////////////////////////////////////
// Bitset serialization.

template <int bitSetSize, int pageSize, class Allocator>
CArchiveReader& operator>>( CArchiveReader& archive, RelibInternal::CPagedStorage<bitSetSize, pageSize, Allocator>& set )
{
	set.Empty();
	CBitSet<set.PagesCount> nonEmptyPages;
	archive >> nonEmptyPages;
	for( int i = nonEmptyPages.First(); i != NotFound; i = nonEmptyPages.Next( i ) ) {
		set.Pages()[i] = CreateOwner<RelibInternal::CPagedStorage<bitSetSize, pageSize, Allocator>::TPage, Allocator>( set );
		archive >> *set.Pages()[i];
	}
	return archive;
}

template <int bitSetSize, int pageSize, class Allocator>
CArchiveWriter& operator<<( CArchiveWriter& archive, const RelibInternal::CPagedStorage<bitSetSize, pageSize, Allocator>& set )
{
	CBitSet<set.PagesCount> nonEmptyPages;
	for( int i = 0; i < set.PagesCount; i++ ) {
		if( set.Pages()[i] != 0 && !set.Pages()[i]->IsEmpty() ) {
			nonEmptyPages |= i;
		}
	}
	archive << nonEmptyPages;

	for( int i = 0; i < set.PagesCount; i++ ) {
		if( set.Pages()[i] != 0 && !set.Pages()[i]->IsEmpty() ) {
			archive << *set.Pages()[i];
		}
	}
	return archive;
}

//////////////////////////////////////////////////////////////////////////
// Color serialization.

inline CArchiveReader& operator>>( CArchiveReader& reader, CColor& color )
{
	unsigned hexValue;
	reader >> hexValue;
	const auto alphaValue = hexValue >> 24;
	color = CColor( hexValue, alphaValue );
	return reader;
}

inline CArchiveWriter& operator<<( CArchiveWriter& writer, CColor color )
{
	writer << color.GetHexRgbaValue();
	return writer;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

