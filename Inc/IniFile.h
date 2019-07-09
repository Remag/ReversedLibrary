#pragma once
#include <Redefs.h>
#include <BaseString.h>
#include <Map.h>
#include <ArrayBuffer.h>
#include <StrConversions.h>

namespace Relib {

class CIniFileData;
//////////////////////////////////////////////////////////////////////////

// Class for range-based for loop section values iteration.
class REAPI CIniSectionKeyRange {
public:
	typedef RelibInternal::CHashIndexConstIterator<CMapData<CUnicodeString, int>, RelibInternal::CMapHashStrategy<CUnicodeString, int, CCaselessUnicodeHash>, CRuntimeHeap> TMapIterator;

	CIniSectionKeyRange( const CMap<CUnicodeString, int, CCaselessUnicodeHash, CRuntimeHeap>& _valueNameToIds, CArrayView<CUnicodeString> _values, TMapIterator _mapIterator ) :
		valueNameToIds( _valueNameToIds ), values( _values ), mapIterator( _mapIterator ) {}

	void operator++()
		{ ++mapIterator; } 
	CPair<CUnicodeView> operator*() const;
	bool operator!=( CIniSectionKeyRange other ) const
		{ return mapIterator != other.mapIterator; }

	CIniSectionKeyRange begin() const
		{ return CIniSectionKeyRange( valueNameToIds, values, valueNameToIds.begin() ); }
	CIniSectionKeyRange end() const
		{ return CIniSectionKeyRange( valueNameToIds, values, valueNameToIds.end() ); }

private:
	const CMap<CUnicodeString, int, CCaselessUnicodeHash, CRuntimeHeap>& valueNameToIds; 
	CArrayView<CUnicodeString> values;
	TMapIterator mapIterator;
};

//////////////////////////////////////////////////////////////////////////

// Section in an .ini file. Provides access to its keys.
class REAPI CIniFileSection {
public:
	explicit CIniFileSection( CUnicodeView name ) : sectionName( name ) {}

	CUnicodeView Name() const
		{ return sectionName; }
	bool IsEmpty() const;
	// Delete all keys and values. All key ids are invalidated.
	void Empty();

	int GetKeyId( CUnicodePart keyName ) const;
	int GetOrCreateKeyId( CUnicodePart keyName );

	// Check key presence.
	bool HasKey( CUnicodePart keyName ) const;
	bool HasKey( int keyId ) const;
	// Conditional lookup.
	const CUnicodeString* LookupString( CUnicodePart keyName ) const;
	const CUnicodeString* LookupString( int keyId ) const;
	void SetString( CUnicodePart keyName, CUnicodePart newValue );
	void SetString( int keyId, CUnicodePart newValue );
	// Delete the given key. Do nothing if no key is present.
	void DeleteKey( CUnicodePart keyName );
	void DeleteKey( int keyId );

	CIniSectionKeyRange KeyValuePairs() const;
	// Get contents of the section in a single string.
	CUnicodeString GetKeyValuePairsString() const;

private:
	// Section name.
	CUnicodeView sectionName;
	// Key-value pairs.
	CMap<CUnicodeString, int, CCaselessUnicodeHash, CRuntimeHeap> valueNameToId;
	CArray<CUnicodeString> valueStrings;
};

//////////////////////////////////////////////////////////////////////////

// Initialization file. File contents are separated into sections headed with "[SectionName]"
// Each section contains a variety of key-value string pairs "KeyName=ValueName".
// Sections with the same name are merged together.
// If two keys within a section have the same name a random one is deleted.
// Key-value pairs headed with no section are put in the default section.
// File is saved in the destructor if relevant changes are detected.
// Save format - UTF16LE with a BOM.
class REAPI CIniFile {
public:
	explicit CIniFile( CUnicodePart name );
	CIniFile( CIniFile&& other );
	const CIniFile& operator=( CIniFile&& other );
	~CIniFile();

	// File name.
	CUnicodeView Name() const
		{ return filePath; }
	// Does this file contain unsaved changes?
	bool IsModified() const
		{ return isModified; }

	// Save the changes explicitly.
	void Save();
	// Delete all values. All section ids are invalidated.
	void Empty();
	void EmptySection( CUnicodePart sectionName );

	// Get the section's numerical id for quick section access. Section must be present in the file.
	int GetSectionId( CUnicodePart sectionName ) const;
	int GetOrCreateSectionId( CUnicodePart sectionName );

	// Get all section data with the given name. If the section doesn't exist, nullptr is returned.
	const CIniFileSection* GetSection( CUnicodePart sectionName ) const;
	const CIniFileSection& GetSection( int sectionId ) const;
	CIniFileSection* GetSection( CUnicodePart sectionName );
	CIniFileSection& GetSection( int sectionId );

	// Retrieve an id of the given key name for quick access.
	int GetKeyId( int sectionId, CUnicodePart keyName ) const;
	int GetOrCreateKeyId( int sectionId, CUnicodePart keyName );

	// Conditional string lookup.
	const CUnicodeString* LookupString( CUnicodePart sectionName, CUnicodePart keyName ) const;
	const CUnicodeString* LookupString( int sectionId, int keyId ) const;
	// Write the given value.
	void SetString( CUnicodePart sectionName, CUnicodePart keyName, CUnicodeView newValue );
	void SetString( int sectionId, int keyId, CUnicodeView newValue );
	
	// Delete the given key or do nothing if a key is not present in the file.
	void DeleteKey( CUnicodePart sectionName, CUnicodePart keyName );
	void DeleteKey( int sectionId, int keyId );

	// Check section/key presence.
	bool HasSection( CUnicodePart sectionName ) const;
	bool HasKey( CUnicodePart sectionName, CUnicodePart keyName ) const;
	bool HasKey( int sectionId, int keyId ) const;
	
	// Iteration through sections.
	CArrayView<CIniFileSection> Sections() const
		{ return sections; } 

	// Template methods that automatically perform the string-value casts with Value/UnicodeStr functions.
	template <class ValueType>
	ValueType GetValue( CUnicodePart sectionName, CUnicodePart keyName, ValueType defaultValue ) const;
	template <class ValueType>
	ValueType GetValue( int sectionId, int keyId, ValueType defaultValue ) const;
	template <class ValueType>
	void SetValue( CUnicodeView sectionName, CUnicodeView keyName, ValueType newValue );
	template <class ValueType>
	void SetValue( int sectionId, int keyId, ValueType newValue );
	// Conditional value lookup.
	template <class ValueType>
	bool LookupValue( CUnicodePart sectionName, CUnicodePart keyName, ValueType& result ) const;

private:
	// Full name of the .ini file.
	CUnicodeString filePath;
	// Section name to section map for fast section access.
	CMap<CUnicodeString, int> sectionNameToSectionId;
	// List of available sections.
	CArray<CIniFileSection> sections;
	// Has the file been modified.
	bool isModified;

	void readFile( CUnicodePart fileName );
	static bool shouldSkip( CUnicodePart str );
	static bool parseKeyValuePair( CUnicodePart str, CUnicodeString& key, CUnicodeString& value );
	static bool parseSection( CUnicodePart str, CUnicodeString& section );
	CIniFileSection* getSection( CUnicodePart name );
	const CIniFileSection* getSection( CUnicodePart name ) const
		{ return const_cast<CIniFile*>( this )->getSection( move( name ) ); }
	CIniFileSection& getOrCreateSection( CUnicodePart name );

	// Copying is prohibited.
	CIniFile( CIniFile& ) = delete;
	void operator=( CIniFile& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class ValueType>
ValueType CIniFile::GetValue( CUnicodePart sectionName, CUnicodePart keyName, ValueType defaultValue ) const
{
	const CUnicodeString* resultStr = LookupString( sectionName, keyName );
	if( resultStr != nullptr ) {
		const auto result = Value<ValueType>( *resultStr );
		if( result.IsValid() ) {
			return *result;
		}
	}
	return defaultValue;
}

template <class ValueType>
ValueType CIniFile::GetValue( int sectionId, int keyId, ValueType defaultValue ) const
{
	const auto resultStr = LookupString( sectionId, keyId );
	if( resultStr != nullptr ) {
		const auto result = Value<ValueType>( *resultStr );
		if( result.IsValid() ) {
			return *result;
		}
	}
	return defaultValue;
}

template <class ValueType>
void CIniFile::SetValue( CUnicodeView sectionName, CUnicodeView keyName, ValueType newValue )
{
	SetString( sectionName, keyName, UnicodeStr( newValue ) );
}

template <class ValueType>
void CIniFile::SetValue( int sectionId, int keyId, ValueType newValue )
{
	SetString( sectionId, keyId, UnicodeStr( newValue ) );
}

template <class ValueType>
bool CIniFile::LookupValue( CUnicodePart sectionName, CUnicodePart keyName, ValueType& result ) const
{
	const CUnicodeString* resultStr = LookupString( sectionName, keyName );
	if( resultStr == nullptr ) {
		return false;
	}
	const auto resultValue = Value<ValueType>( *resultStr );
	if( resultValue.IsValid() ) {
		result = *resultValue;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

