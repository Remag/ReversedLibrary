#pragma once
#include <Redefs.h>
#include <Remath.h>
#include <BaseStringView.h>
#include <BaseStringPart.h>

// Hashing functions and algorithms.
namespace Relib {

//////////////////////////////////////////////////////////////////////////

// String hash functions that are used for hash containers (djb2a)
int REAPI GetUnicodeHash( CUnicodePart string );
int REAPI GetUnicodeHash( const wchar_t* string );
int REAPI GetCaselessUnicodeHash( CUnicodePart string );
int REAPI GetStringHash( CStringPart string );
int REAPI GetStringHash( const char* string );
int REAPI GetCaselessStringHash( CStringPart string );
int REAPI GetMemoryBlockHash( const void* ptr, int size );

// Add additionalValue to the total hash. Uses djb2a algorithm for hash combination.
inline int CombineHashKey( int mainValue, int additionalValue )
{
	const unsigned mValue = mainValue;
	return ( ( mValue << 5 ) + mValue ) ^ additionalValue; 
}

// Stronger string hash functions with less collisions (FNV-1a).
int REAPI GetStrongUnicodeHash( const wchar_t* string );
int REAPI GetStrongUnicodeHash( CUnicodePart string );
int REAPI GetStrongStringHash( const char* string );
int REAPI GetStrongStringHash( CStringPart string );

// Hash combination that uses FNV-1a.
inline int StrongCombineHashKey( int mainValue, int additionalValue )
{
	const unsigned xorValue = mainValue ^ additionalValue;
	return xorValue * 16777619; 
}

// Return the suitable prime size for the hash table, that must contain at least hashTableSize items.
int REAPI GetPrimeHashTableSize( int hashTableSize );
// Return an upper power of 2 for the given size.
int REAPI GetPow2HashTableSize( int hashTableSize );

//////////////////////////////////////////////////////////////////////////

// Hash strategy. Defines static methods HashKey and IsEqual that are used to hash classes in CHashTable.
// Default implementation uses operator== and HashKey method from the class it tries to hash.
template <class Key>
class CDefaultHash {
public:
	static int HashKey( const Key& key )
		{ return key.HashKey(); }
	static bool IsEqual( const Key& leftKey, const Key& rightKey )
		{ return leftKey == rightKey; }
};

//////////////////////////////////////////////////////////////////////////

// Template partial specialization for hashing pointers.
template<class Key>
class CDefaultHash<Key*> {
public:
	static int HashKey( const Key* key )
		{ return static_cast<int>( reinterpret_cast<size_t>( key ) ); }
	static bool IsEqual( const Key* leftKey, const Key* rightKey )
		{ return leftKey == rightKey; }
};

//////////////////////////////////////////////////////////////////////////

// Hash strategy specialization for strings.
template<>
class CDefaultHash<CStringPart> {
public:
	static int HashKey( CStringPart string )
		{ return GetStringHash( string ); }
	static bool IsEqual( CStringPart leftStr, CStringPart rightStr )
		{ return leftStr == rightStr; }
};

template<>
class CDefaultHash<CUnicodePart> {
public:
	static int HashKey( CUnicodePart string )
		{ return GetUnicodeHash( string ); }
	static bool IsEqual( CUnicodePart leftStr, CUnicodePart rightStr )
		{ return leftStr == rightStr; }
};

template<>
class CDefaultHash<const char*> : public CDefaultHash<CStringPart> {
public:
	using CDefaultHash<CStringPart>::HashKey;
	using CDefaultHash<CStringPart>::IsEqual;

	static int HashKey( const char* string )
		{ return GetStringHash( string ); }
	static bool IsEqual( const char* leftStr, const char* rightStr )
		{ return ::strcmp( leftStr, rightStr ) == 0; }
};

template<>
class CDefaultHash<const wchar_t*> : public CDefaultHash<CUnicodePart> {
public:
	using CDefaultHash<CUnicodePart>::HashKey;
	using CDefaultHash<CUnicodePart>::IsEqual;

	static int HashKey( const wchar_t* string )
		{ return GetUnicodeHash( string ); }
	static bool IsEqual( const wchar_t* leftStr, const wchar_t* rightStr )
		{ return ::wcscmp( leftStr, rightStr ) == 0; }
};

template<>
class CDefaultHash<CStringView> : public CDefaultHash<CStringPart> {
};

template<>
class CDefaultHash<CUnicodeView> : public CDefaultHash<CUnicodePart> {
};

template<>
class CDefaultHash<CString> : public CDefaultHash<CStringView> {
};

template<>
class CDefaultHash<CUnicodeString> : public CDefaultHash<CUnicodeView> {
};

template<>
class CDefaultHash<const type_info*> {
public:
	static int HashKey( const type_info* info )
		{ return static_cast<int>( info->hash_code() ); }
	static bool IsEqual( const type_info* leftInfo, const type_info* rightInfo )
		{ return *leftInfo == *rightInfo; }
};

//////////////////////////////////////////////////////////////////////////

// HashKey specializations for hashing default types.
template<>
inline int CDefaultHash<char>::HashKey( const char& key )
{
	return static_cast<int>( key );
}

template<>
inline int CDefaultHash<unsigned char>::HashKey( const unsigned char& key )
{
	return static_cast<int>( key );
}

template<>
inline int CDefaultHash<wchar_t>::HashKey( const wchar_t& key )
{
	return static_cast<int>( key );
}

template<>
inline int CDefaultHash<short>::HashKey( const short& key )
{
	return static_cast<int>( key );
}

template<>
inline int CDefaultHash<unsigned short>::HashKey( const unsigned short& key )
{
	return static_cast<int>( key );
}

template<> 
inline int CDefaultHash<int>::HashKey( const int& key )
{
	return key;
}

template<> 
inline int CDefaultHash<unsigned int>::HashKey( const unsigned int& key )
{
	return static_cast<int>( key );
}

template<>
inline int CDefaultHash<long>::HashKey( const long& key )
{
	return static_cast<int>( key );
}

template<>
inline int CDefaultHash<unsigned long>::HashKey( const unsigned long& key )
{
	return static_cast<int>( key );
}

template<>
inline int CDefaultHash<__int64>::HashKey( const __int64& key )
{	
	const int lowWord = numeric_cast<int>( key & 0xFFFFFFFF );
	const int highWord = numeric_cast<int>( ( key & 0xFFFFFFFF00000000 ) >> 32 );
	return static_cast<int>( lowWord ^ highWord );
}

template<>
inline int CDefaultHash<unsigned __int64>::HashKey( const unsigned __int64& key )
{
	const int lowWord = numeric_cast<unsigned>( key & 0xFFFFFFFF );
	const int highWord = numeric_cast<unsigned>( ( key & 0xFFFFFFFF00000000 ) >> 32 );
	return static_cast<int>( lowWord ^ highWord );
}

//////////////////////////////////////////////////////////////////////////

// Hash strategy that uses the specified class member as the hash source.
template <class T, class ReturnType, ReturnType T::*Member>
class CMemberHash : public CDefaultHash<ReturnType> {
	using CDefaultHash<ReturnType>::HashKey;
	using CDefaultHash<ReturnType>::IsEqual;
	static int HashKey( const T& key )
		{ return CDefaultHash::HashKey( key.*Member ); }
	static bool IsEqual( const T& leftKey, const T& rightKey )
		{ return CDefaultHash::IsEqual( leftKey.*Member, rightKey.*Member ); }
	static bool IsEqual( const ReturnType& leftKey, const T& rightKey )
		{ return CDefaultHash::IsEqual( leftKey, rightKey.*Member ); }
	static bool IsEqual( const T& leftKey, const ReturnType& rightKey )
		{ return CDefaultHash::IsEqual( leftKey.*Member, rightKey ); }
};

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

