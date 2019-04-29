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

namespace Relib {

namespace RelibInternal {

// Class for data serialization.
class REAPI CArchive {
public:
	explicit CArchive( int bufferSize );

protected:
	static const int minArchiveSize = 4096;

	// Read/Write methods for arbitrary data.
	void read( CFileReadView file, void* ptr, int size );
	void write( CFileWriteView file, const void* ptr, int size );
	bool isEndOfReading( CFileReadView file ) const;

	void flushReading( CFileReadView file  );
	void flushWriting( CFileWriteView file  );

	void skipReading( CFileReadView file, int byteCount );
	void skipWriting( CFileWriteView file, int byteCount );

	// Serialization for small values.
	// Integers from 0 to 254 are stored in 1 byte.
	// Other integers are stored in 5 bytes.
	int readSmallValue( CFileReadView file );
	void writeSmallValue( CFileWriteView file, int value );

	// Custom serializable read/write.
	CSharedPtr<ISerializable> readObject( CFileReadView file );
	void writeObject( CFileWriteView file, const ISerializable* object );

	// Enumeration type read/write.
	template<class Type>
	Type readEnum( CFileReadView file );
	template<class Type>
	void writeEnum( CFileWriteView file, Type enumValue );

	template<class Type>
	void readSimpleType( CFileReadView file, Type& var );
	template<class Type>
	void writeSimpleType( CFileWriteView file, Type var );

	// Serializes a version of an archive.
	// If the version in the archive is bigger than currentVersion. The archive is incompatible with the program and an exception is thrown.
	int readVersion( CFileReadView file, int currentVersion );
	int writeVersion( CFileWriteView file, int currentVersion );

	void abort();
	
private:
	// Buffer to be written to the file.
	CArray<BYTE, CRuntimeHeap> buffer;
	const int bufferSize;

	// Buffered parameters for speedy Seek.
	mutable __int64 archiveStartPos;
	mutable __int64 filePos;
	mutable __int64 fileLength;
	mutable bool arePosAndLengthActual;

	// Current buffer position.
	int currentBufferPos;
	// The count of meaningful information that is situated after currentBufferPos.
	// For archive readers this corresponds to the amount of unread data in the buffer.
	// For archive writers this value is non-zero only if we decided to change archive position and rewrite some previously written data.
	int bufferLeftoverCount;

	// Map that returns creationFunctionsBuffer's index for an object's name.
	CMap<CUnicodeString, int, CDefaultHash<CUnicodeString>, CRuntimeHeap> objectNamesDictionary;
	// Buffer for quick access to object's creation functions. Filled only in reading mode.
	CArray<const CBaseObjectCreationFunction*, CRuntimeHeap> creationFunctionsBuffer;

	void readOverBuffer( CFileReadView file, void* ptr, int size );
	void writeOverBuffer( CFileWriteView file, const void* ptr, int size );
	void calcActualFilePos( CFileWriteView file ) const;

	CPtrOwner<ISerializable> readUniqueObject( CFileReadView file );

	void writeExternalName( CFileWriteView file, CUnicodeView name );
	const CBaseObjectCreationFunction* readCreationFunctionPtr( int objectId );
};

//////////////////////////////////////////////////////////////////////////

inline void CArchive::read( CFileReadView file, void* ptr, int size )
{
	assert( size >= 0 );

	if( size == 0 ) {
		return;
	} else if( size <= bufferLeftoverCount ) {
		memcpy( ptr, buffer.Ptr() + currentBufferPos, size );
		currentBufferPos += size;
		bufferLeftoverCount -= size;
	} else {
		// Not enough data left in buffer.
		readOverBuffer( file, ptr, size );
	}
}

inline void CArchive::write( CFileWriteView file, const void* ptr, int size )
{
	assert( size >= 0 );

	if( size == 0 ) {
		return;
	} else if( size + currentBufferPos < bufferSize ) {
		memcpy( buffer.Ptr() + currentBufferPos, ptr, size );
		currentBufferPos += size;
		bufferLeftoverCount = max( bufferLeftoverCount - size, 0 );
	} else {
		// Note enough space left in buffer.
		writeOverBuffer( file, ptr, size );
	}
}

inline bool CArchive::isEndOfReading( CFileReadView file ) const
{
	return bufferLeftoverCount == 0 && file.IsEndOfFile();
}

template<class Type>
Type CArchive::readEnum( CFileReadView file )
{
	staticAssert( Types::IsEnum<Type>::Result );
	return Type( readSmallValue( file ) );
}

template<class Type>
void CArchive::writeEnum( CFileWriteView file, Type enumValue )
{
	staticAssert( Types::IsEnum<Type>::Result );
	writeSmallValue( file, enumValue );
}

template<class Type>
inline void CArchive::readSimpleType( CFileReadView file, Type& var )
{
	read( file, &var, sizeof( Type ) );
}

template<class Type>
inline void CArchive::writeSimpleType( CFileWriteView file, Type var )
{
	write( file, &var, sizeof( Type ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace RelibInternal.

//////////////////////////////////////////////////////////////////////////

// Class that reads and serializes data from a given file into the data structures.
class REAPI CArchiveReader : public RelibInternal::CArchive {
public:
	CArchiveReader( CFileReadView _file, int bufferSize = minArchiveSize ) : RelibInternal::CArchive( bufferSize ), file( _file ) {}

	~CArchiveReader();

	CUnicodeString GetName() const
		{ return file.GetFileName(); }

	void Flush()
		{ flushReading( file ); }
	void Skip( int byteCount )
		{ skipReading( file, byteCount ); }

	bool IsEndOfArchive() const
		{ return isEndOfReading( file ); }
	
	int ReadSmallValue()
		{ return readSmallValue( file ); }
	void Read( void* ptr, int size )
		{ read( file, ptr, size ); }

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
	CFileReadView file;
};

//////////////////////////////////////////////////////////////////////////

inline CArchiveReader& operator>>( CArchiveReader& archive, bool& var )
{
	BYTE byte;
	archive.readSimpleType( archive.file, byte );
	check( byte == 0 || byte == 1, Err_BadArchive, archive.file.GetFileName() );
	var = byte == 1;
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, char& var )
{
	archive.readSimpleType( archive.file, var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, signed char& var )
{
	archive.readSimpleType( archive.file, var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, unsigned char& var )
{
	archive.readSimpleType( archive.file, var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, wchar_t& var )
{
	archive.readSimpleType( archive.file, var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, short& var )
{
	archive.readSimpleType( archive.file, var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, unsigned short& var )
{
	archive.readSimpleType( archive.file, var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, int& var )
{
	archive.readSimpleType( archive.file, var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, unsigned int& var )
{
	archive.readSimpleType( archive.file, var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, long& var )
{
	archive.readSimpleType( archive.file, var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, unsigned long& var )
{
	archive.readSimpleType( archive.file, var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, __int64& var )
{
	archive.readSimpleType( archive.file, var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, unsigned __int64& var )
{
	archive.readSimpleType( archive.file, var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, float& var )
{
	archive.readSimpleType( archive.file, var );
	return archive;
}

inline CArchiveReader& operator>>( CArchiveReader& archive, double& var )
{
	archive.readSimpleType( archive.file, var );
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

// Class that writes data to a given file from the data structures.
class REAPI CArchiveWriter : public RelibInternal::CArchive {
public:
	CArchiveWriter( CFileWriteView _file, int bufferSize = minArchiveSize ) : RelibInternal::CArchive( bufferSize ), file( _file ) {}

	~CArchiveWriter();

	CUnicodeString GetName() const
		{ return file.GetFileName(); }

	void Flush()
		{ flushWriting( file ); }
	void Skip( int byteCount )
		{ skipWriting( file, byteCount ); }

	void WriteSmallValue( int value )
		{ writeSmallValue( file, value ); }
	void Write( const void* ptr, int size )
		{ write( file, ptr, size ); }

	template <class Enum>
	void WriteEnum( Enum value )
		{ writeEnum( file, value ); }

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
	CFileWriteView file;
};

//////////////////////////////////////////////////////////////////////////

inline CArchiveWriter& operator<<( CArchiveWriter& archive, bool var )
{
	BYTE byte = numeric_cast<BYTE>( var );
	// var should be initialized.
	assert( byte == 0 || byte == 1 );
	archive.writeSimpleType( archive.file, byte );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, char var )
{
	archive.writeSimpleType( archive.file, var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, signed char var )
{
	archive.writeSimpleType( archive.file, var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned char var )
{
	archive.writeSimpleType( archive.file, var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, wchar_t var )
{
	archive.writeSimpleType( archive.file, var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, short var )
{
	archive.writeSimpleType( archive.file, var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned short var )
{
	archive.writeSimpleType( archive.file, var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, int var )
{
	archive.writeSimpleType( archive.file, var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned int var )
{
	archive.writeSimpleType( archive.file, var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, long var )
{
	archive.writeSimpleType( archive.file, var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned long var )
{
	archive.writeSimpleType( archive.file, var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, __int64 var )
{
	archive.writeSimpleType( archive.file, var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, unsigned __int64 var )
{
	archive.writeSimpleType( archive.file, var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, float var )
{
	archive.writeSimpleType( archive.file, var );
	return archive;
}

inline CArchiveWriter& operator<<( CArchiveWriter& archive, double var )
{
	archive.writeSimpleType( archive.file, var );
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

extern const REAPI CError Err_BadArchive;
template <class Elem, class Allocator /*= CRuntimeHeap*/, class GrowStrategy /*= CDefaultGrowStrategy<8>*/>
CArchiveReader& operator>>( CArchiveReader& archive, CArray<Elem, Allocator, GrowStrategy>& arr )
{
	arr.Empty();
	int size;
	archive >> size;
	check( size >= 0, Err_BadArchive, archive.GetName() );
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
	check( size >= 0, Err_BadArchive, archive.GetName() );
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
	check( size >= 0, Err_BadArchive, archive.GetName() );
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
		check( false, Err_BadArchive, archive.GetName() );
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
// Bitset serialization functions.
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

}	// namespace Relib.

