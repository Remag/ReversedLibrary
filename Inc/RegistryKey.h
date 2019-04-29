#pragma once
#include <Redefs.h>
#include <BaseStringView.h>
#include <TemplateUtils.h>

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

// Mechanism for writing data to the Windows registry.
// The class owns an open key handle. The handle is freed on deletion.
class REAPI CRegistryKey {
public:
	// Create a key with a given name at a given registry path.
	CRegistryKey( TRegistryRootKey rootKey, CUnicodeView keyName, TRegistryAccessType accessType );
	// Create a subkey of a given key.
	CRegistryKey( const CRegistryKey& parentKey, CUnicodeView keyName, TRegistryAccessType accessType );
	~CRegistryKey();

	// Enumerate all values of this key.
	void GetValueNames( CArray<CUnicodeString>& result );

	// Check if a given value exists.
	bool HasValue( CUnicodeView valueName ) const;

	// Value creation functions.

	// Read the value with the given name. If the value does not exist or the type does not match the requested one, the default value is returned.
	template <class T>
	auto GetValue( CUnicodeView valueName, const T& defaultValue ) const;
	// Read the value with the given name. If the value does not exist or the type does not match the requested one, the new value is created.
	template <class T>
	T GetOrCreateValue( CUnicodeView valueName, const T& defaultValue );
	template <class T>
	void SetValue( CUnicodeView valueName, const T& newValue );

private:
	HKEY keyHandle;

	typedef const wchar_t* TPrimitiveString;
	
	void initializeRegistryKey( HKEY parent, CUnicodeView name, TRegistryAccessType accessType );

	template <class T>
	T doReadValue( CUnicodeView name, const T& defaultValue ) const;
	CUnicodeString doReadValue( CUnicodeView name, const CUnicodeString& defaultValue ) const;
	CUnicodeString doReadValue( CUnicodeView name, const CUnicodeView& defaultValue ) const;
	CUnicodeString doReadValue( CUnicodeView name, const CUnicodePart& defaultValue ) const;
	CUnicodeString doReadValue( CUnicodeView name, const TPrimitiveString& defaultValue ) const;
	CUnicodeString readStringValue( CUnicodeView name, CUnicodePart defaultValue ) const;

	template <class T>
	T doGetOrCreateValue( CUnicodeView name, const T& defaultValue );
	CUnicodeString doGetOrCreateValue( CUnicodeView name, const CUnicodeString& defaultValue );
	CUnicodeString doGetOrCreateValue( CUnicodeView name, const CUnicodeView& defaultValue );
	CUnicodeString doGetOrCreateValue( CUnicodeView name, const CUnicodePart& defaultValue ); 
	CUnicodeString doGetOrCreateValue( CUnicodeView name, const TPrimitiveString& defaultValue ); 
	CUnicodeString doGetOrCreateStringValue( CUnicodeView name, CUnicodePart defaultValue );

	void retrieveStringWriteParams( CUnicodeView value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const;

	void retrieveValueWriteParams( const CUnicodeString& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const;
	void retrieveValueWriteParams( const CUnicodeView& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const;
	void retrieveValueWriteParams( wchar_t const* const& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const;
	void retrieveValueWriteParams( const int& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const;
	void retrieveValueWriteParams( const long long& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const;
	template <class T>
	void retrieveValueWriteParams( const T& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const;

	bool tryReadStringValue( CUnicodeView valueName, CUnicodeString& result ) const;
	LSTATUS tryGetRegValue( CUnicodeView valueName, void* valuePtr, DWORD& valueSize ) const;
	void setRegValue( CUnicodeView valueName, DWORD dataType, const void* dataPtr, DWORD dataSize ) const;

	// Copying is prohibited.
	CRegistryKey( CRegistryKey& ) = delete;
	void operator=( CRegistryKey& ) = delete;
};

//////////////////////////////////////////////////////////////////////////

template <class T>
T CRegistryKey::doReadValue( CUnicodeView name, const T& defaultValue ) const
{
	staticAssert( Types::IsFundamental<T>::Result );
	T result;
	const DWORD valueSize = sizeof( defaultValue );
	DWORD size = valueSize;
	const auto readStatus = tryGetRegValue( name, &result, size );
	return readStatus == ERROR_SUCCESS && valueSize == size ? move( result ) : defaultValue;
}

template <class T>
auto CRegistryKey::GetValue( CUnicodeView valueName, const T& defaultValue ) const
{
	staticAssert( ( Types::IsFundamental<T>::Result || Types::IsString<T, wchar_t>::Result ) );
	return doReadValue( valueName, defaultValue );
}

template <class T>
T CRegistryKey::doGetOrCreateValue( CUnicodeView name, const T& defaultValue ) 
{
	staticAssert( Types::IsFundamental<T>::Result );
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
T CRegistryKey::GetOrCreateValue( CUnicodeView valueName, const T& defaultValue )
{
	staticAssert( ( Types::IsFundamental<T>::Result || Types::IsString<T, wchar_t>::Result ) );
	return doGetOrCreateValue( valueName, defaultValue );
}

template <class T>
void CRegistryKey::SetValue( CUnicodeView valueName, const T& newValue )
{
	staticAssert( ( Types::IsFundamental<T>::Result || Types::IsString<T, wchar_t>::Result ) );
	const void* dataPtr;
	DWORD dataSize;
	DWORD dataType;
	retrieveValueWriteParams( newValue, dataType, dataPtr, dataSize );
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

