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
	typedef RelibInternal::CHashIndexConstIterator<CMapData<CString, int>, RelibInternal::CMapHashStrategy<CString, int, CCaselessStringHash>, CRuntimeHeap> TMapIterator;

	CIniSectionKeyRange( const CMap<CString, int, CCaselessStringHash>& _valueNameToIds, CArrayView<CString> _values, TMapIterator _mapIterator ) :
		valueNameToIds( _valueNameToIds ), values( _values ), mapIterator( _mapIterator ) {}

	void operator++()
		{ ++mapIterator; } 
	CPair<CStringPart> operator*() const;
	bool operator!=( CIniSectionKeyRange other ) const
		{ return mapIterator != other.mapIterator; }

	CIniSectionKeyRange begin() const
		{ return CIniSectionKeyRange( valueNameToIds, values, valueNameToIds.begin() ); }
	CIniSectionKeyRange end() const
		{ return CIniSectionKeyRange( valueNameToIds, values, valueNameToIds.end() ); }

private:
	const CMap<CString, int, CCaselessStringHash>& valueNameToIds; 
	CArrayView<CString> values;
	TMapIterator mapIterator;
};

//////////////////////////////////////////////////////////////////////////

// Section in an .ini file. Provides access to its keys.
class REAPI CIniFileSection {
public:
	explicit CIniFileSection( CStringPart name ) : sectionName( name ) {}

	CStringPart GetName() const
		{ return sectionName; }
	bool IsEmpty() const;
	// Delete all keys and values. All key ids are invalidated.
	void Empty();

	int GetKeyId( CStringPart keyName ) const;
	int GetOrCreateKeyId( CStringPart keyName );
	int GetOrCreateKeyId( CStringPart keyName, CString defaultValue );

	// Check key presence.
	bool HasKey( CStringPart keyName ) const;
	bool HasKey( int keyId ) const;
	// Conditional lookup.
	const CString* LookupString( CStringPart keyName ) const;
	const CString* LookupString( int keyId ) const;
	void SetString( CStringPart keyName, CStringPart newValue );
	void SetString( int keyId, CStringPart newValue );
	// Delete the given key. Do nothing if no key is present.
	void DeleteKey( CStringPart keyName );
	void DeleteKey( int keyId );

	CIniSectionKeyRange KeyValuePairs() const;
	// Get contents of the section in a single string.
	CString GetKeyValuePairsString() const;

private:
	// Section name.
	CStringPart sectionName;
	// Key-value pairs.
	CMap<CString, int, CCaselessStringHash, CRuntimeHeap> valueNameToId;
	CArray<CString> valueStrings;
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
	explicit CIniFile( CStringPart name );
	CIniFile( CIniFile&& other );
	const CIniFile& operator=( CIniFile&& other );
	~CIniFile();

	// File name.
	CStringView Name() const
		{ return filePath; }
	// Does this file contain unsaved changes?
	bool IsModified() const
		{ return isModified; }

	// Save the changes explicitly.
	void Save();
	// Delete all values. All section ids are invalidated.
	void Empty();
	void EmptySection( CStringPart sectionName );

	// Get the section's numerical id for quick section access. Section must be present in the file.
	int GetSectionId( CStringPart sectionName ) const;
	int GetOrCreateSectionId( CStringPart sectionName );

	// Get all section data with the given name. If the section doesn't exist, nullptr is returned.
	const CIniFileSection* GetSection( CStringPart sectionName ) const;
	const CIniFileSection& GetSection( int sectionId ) const;
	CIniFileSection* GetSection( CStringPart sectionName );
	CIniFileSection& GetSection( int sectionId );

	// Retrieve an id of the given key name for quick access.
	int GetKeyId( int sectionId, CStringPart keyName ) const;

	// Conditional string lookup.
	const CString* LookupString( CStringPart sectionName, CStringPart keyName ) const;
	const CString* LookupString( int sectionId, int keyId ) const;
	// Write the given value.
	void SetString( CStringPart sectionName, CStringPart keyName, CStringPart newValue );
	void SetString( int sectionId, int keyId, CStringPart newValue );
	
	// Delete the given key or do nothing if a key is not present in the file.
	void DeleteKey( CStringPart sectionName, CStringPart keyName );
	void DeleteKey( int sectionId, int keyId );

	// Check section/key presence.
	bool HasSection( CStringPart sectionName ) const;
	bool HasKey( CStringPart sectionName, CStringPart keyName ) const;
	bool HasKey( int sectionId, int keyId ) const;
	
	// Iteration through sections.
	CArrayView<CIniFileSection> Sections() const
		{ return sections; } 

	// Template methods that automatically perform the string-value casts with Value/UnicodeStr functions.
	template <class ValueType>
	int GetOrCreateKeyId( int sectionId, CStringPart keyName, const ValueType& defaultValue );
	template <class ValueType>
	ValueType GetValue( CStringPart sectionName, CStringPart keyName, const ValueType& defaultValue ) const;
	template <class ValueType>
	ValueType GetValue( int sectionId, int keyId, const ValueType& defaultValue ) const;
	template <class ValueType>
	void SetValue( CStringPart sectionName, CStringPart keyName, const ValueType& newValue );
	template <class ValueType>
	void SetValue( int sectionId, int keyId, const ValueType& newValue );
	// Conditional value lookup.
	template <class ValueType>
	bool LookupValue( CStringPart sectionName, CStringPart keyName, ValueType& result ) const;

private:
	// Full name of the .ini file.
	CString filePath;
	// Section name to section map for fast section access.
	CMap<CString, int> sectionNameToSectionId;
	// List of available sections.
	CArray<CIniFileSection> sections;
	// Has the file been modified.
	bool isModified;

	void readFile( CStringPart fileName );
	static bool shouldSkip( CStringPart str );
	static bool parseKeyValuePair( CStringPart str, CString& key, CString& value );
	static bool parseSection( CStringPart str, CString& section );
	CIniFileSection* getSection( CStringPart name );
	const CIniFileSection* getSection( CStringPart name ) const
		{ return const_cast<CIniFile*>( this )->getSection( move( name ) ); }
	CIniFileSection& getOrCreateSection( CStringPart name );
	int getOrCreateKeyId( int sectionId, CStringPart keyName, CString defaultValue );

	// Copying is prohibited.
	CIniFile( CIniFile& ) = delete;
	void operator=( CIniFile& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template<class ValueType>
int CIniFile::GetOrCreateKeyId( int sectionId, CStringPart keyName, const ValueType& defaultValue )
{
	return getOrCreateKeyId( sectionId, keyName, Str( defaultValue ) );
}

template <class ValueType>
ValueType CIniFile::GetValue( CStringPart sectionName, CStringPart keyName, const ValueType& defaultValue ) const
{
	const CUnicodeString* resultStr = LookupString( sectionName, keyName );
	if( resultStr != nullptr ) {
		auto result = Value<ValueType>( *resultStr );
		if( result.IsValid() ) {
			return move( *result );
		}
	}
	return copy( defaultValue );
}

template <class ValueType>
ValueType CIniFile::GetValue( int sectionId, int keyId, const ValueType& defaultValue ) const
{
	const auto resultStr = LookupString( sectionId, keyId );
	if( resultStr != nullptr ) {
		auto result = Value<ValueType>( *resultStr );
		if( result.IsValid() ) {
			return move( *result );
		}
	}
	return copy( defaultValue );
}

template <class ValueType>
void CIniFile::SetValue( CStringPart sectionName, CStringPart keyName, const ValueType& newValue )
{
	SetString( sectionName, keyName, Str( newValue ) );
}

template <class ValueType>
void CIniFile::SetValue( int sectionId, int keyId, const ValueType& newValue )
{
	SetString( sectionId, keyId, Str( newValue ) );
}

template <class ValueType>
bool CIniFile::LookupValue( CStringPart sectionName, CStringPart keyName, ValueType& result ) const
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

