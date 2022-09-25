#pragma once
#include <Redefs.h>
#include <BaseStringView.h>
#include <TemplateUtils.h>
#include <StrConversions.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

// Type of a root registry key.
enum TRegistryRootKey {
	RRK_ClassesRoot,
	RRK_CurrentUser,
	RRK_LocalMachine,
	RRK_Users,
	RRK_CurrentConfig,
	RRK_EnumCount
};

// Type of registry access.
enum TRegistryAccessType {
	RAT_Read = KEY_READ,
	RAT_Write = KEY_WRITE,
	RAT_ReadWrite = ( KEY_READ | KEY_WRITE )
};

//////////////////////////////////////////////////////////////////////////

// Enumerator with registry key value range-based for loop support.
class REAPI CRegistryKeyValueEnumerator {
public:
	CRegistryKeyValueEnumerator( HKEY handle, DWORD pos, DWORD count, DWORD maxLen ) : keyHandle( handle ), enumPosition( pos ), enumCount( count ), maxLength( maxLen ) {}

	CString operator*() const;
	void operator++()
		{ enumPosition++; }
	bool operator!=( CRegistryKeyValueEnumerator other ) const
		{ return enumPosition != other.enumPosition; }

	CRegistryKeyValueEnumerator begin() const
		{ return CRegistryKeyValueEnumerator( keyHandle, 0, enumCount, maxLength ); }
	CRegistryKeyValueEnumerator end() const
		{ return CRegistryKeyValueEnumerator( keyHandle, enumCount, enumCount, maxLength ); }

private:
	HKEY keyHandle;
	DWORD enumPosition = 0;
	DWORD enumCount = 0;
	DWORD maxLength = 0;
};

//////////////////////////////////////////////////////////////////////////

// Mechanism for writing data to the Windows registry.
// The class owns an open key handle. The handle is freed on deletion.
class REAPI CRegistryKey {
public:
	// Create a key with a given name at a given registry path.
	CRegistryKey( TRegistryRootKey rootKey, CStringView keyName, TRegistryAccessType accessType );
	// Create a subkey of a given key.
	CRegistryKey( const CRegistryKey& parentKey, CStringView keyName, TRegistryAccessType accessType );
	~CRegistryKey();

	// Enumerate all values of this key.
	CRegistryKeyValueEnumerator ValueNames() const;

	// Check if a given value exists.
	bool HasValue( CStringView valueName ) const;

	// Value creation functions.

	// Read the value with the given name. If the value does not exist or the type does not match the requested one, the default value is returned.
	template <class T>
	auto GetValue( CStringView valueName, const T& defaultValue ) const;
	// Read the value with the given name. If the value does not exist or the type does not match the requested one, the new value is created.
	template <class T>
	T GetOrCreateValue( CStringView valueName, const T& defaultValue );
	template <class T>
	void SetValue( CStringView valueName, const T& newValue );

private:
	HKEY keyHandle;

	typedef const char* TPrimitiveString;
	
	void initializeRegistryKey( HKEY parent, CStringView name, TRegistryAccessType accessType );

	template <class T>
	T doReadValue( CStringView name, const T& defaultValue ) const;
	CString doReadValue( CStringView name, const CString& defaultValue ) const;
	CString doReadValue( CStringView name, const CStringView& defaultValue ) const;
	CString doReadValue( CStringView name, const CStringPart& defaultValue ) const;
	CString doReadValue( CStringView name, const TPrimitiveString& defaultValue ) const;
	CString readStringValue( CStringView name, CStringPart defaultValue ) const;

	template <class T>
	T doGetOrCreateValue( CStringView name, const T& defaultValue );
	CString doGetOrCreateValue( CStringView name, const CString& defaultValue );
	CString doGetOrCreateValue( CStringView name, const CStringView& defaultValue );
	CString doGetOrCreateValue( CStringView name, const CStringPart& defaultValue ); 
	CString doGetOrCreateValue( CStringView name, const TPrimitiveString& defaultValue ); 
	CString doGetOrCreateStringValue( CStringView name, CStringPart defaultValue );

	void retrieveStringWriteParams( CUnicodeView value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const;

	void retrieveValueWriteParams( const int& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const;
	void retrieveValueWriteParams( const long long& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const;
	template <class T>
	void retrieveValueWriteParams( const T& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const;

	bool tryReadStringValue( CStringView valueName, CString& result ) const;
	LSTATUS tryGetRegValue( CStringView valueName, void* valuePtr, DWORD& valueSize ) const;
	void setRegValue( CStringView valueName, DWORD dataType, const void* dataPtr, DWORD dataSize ) const;

	template <class T>
	void doSetValue( CStringView valueName, const T& value, Types::FalseType strMarker );
	template <class T>
	void doSetValue( CStringView valueName, const T& value, Types::TrueType strMarker );

	// Copying is prohibited.
	CRegistryKey( CRegistryKey& ) = delete;
	void operator=( CRegistryKey& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class T>
T CRegistryKey::doReadValue( CStringView name, const T& defaultValue ) const
{
	staticAssert( Types::IsPOD<T>::Result );
	T result;
	const DWORD valueSize = sizeof( defaultValue );
	DWORD size = valueSize;
	const auto readStatus = tryGetRegValue( name, &result, size );
	return readStatus == ERROR_SUCCESS && valueSize == size ? move( result ) : defaultValue;
}

template <class T>
auto CRegistryKey::GetValue( CStringView valueName, const T& defaultValue ) const
{
	staticAssert( ( Types::IsPOD<T>::Result || Types::IsString<T, char>::Result ) );
	return doReadValue( valueName, defaultValue );
}

template <class T>
T CRegistryKey::doGetOrCreateValue( CStringView name, const T& defaultValue ) 
{
	staticAssert( Types::IsPOD<T>::Result );
	T result;
	const DWORD valueSize = sizeof( defaultValue );
	DWORD size = valueSize;
	const auto readStatus = tryGetRegValue( name, &result, size );
	if( readStatus != ERROR_SUCCESS || size != valueSize ) {
		SetValue( name, defaultValue );
		return defaultValue;
	}
	return result;
}

template <class T>
T CRegistryKey::GetOrCreateValue( CStringView valueName, const T& defaultValue )
{
	staticAssert( ( Types::IsPOD<T>::Result || Types::IsString<T, char>::Result ) );
	return doGetOrCreateValue( valueName, defaultValue );
}

template <class T>
void CRegistryKey::SetValue( CStringView valueName, const T& newValue )
{
	doSetValue( valueName, newValue, Types::IsString<T, char>() );
}

template<class T>
inline void CRegistryKey::doSetValue( CStringView valueName, const T& newValue, Types::FalseType /*strMarker*/ )
{
	staticAssert( Types::IsPOD<T>::Result );
	const void* dataPtr;
	DWORD dataSize;
	DWORD dataType;
	retrieveValueWriteParams( newValue, dataType, dataPtr, dataSize );
	setRegValue( valueName, dataType, dataPtr, dataSize );
}

template<class T>
inline void CRegistryKey::doSetValue( CStringView valueName, const T& newValue, Types::TrueType /*strMarker*/ )
{
	staticAssert( ( Types::IsString<T, char>::Result ) );
	const auto unicodeVal = UnicodeStr( newValue );
	const void* dataPtr;
	DWORD dataSize;
	DWORD dataType;
	retrieveStringWriteParams( unicodeVal, dataType, dataPtr, dataSize );
	setRegValue( valueName, dataType, dataPtr, dataSize );
}

template <class T>
void CRegistryKey::retrieveValueWriteParams( const T& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const
{
	dataType = REG_BINARY;
	dataPtr = &value;
	dataSize = sizeof( T );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

