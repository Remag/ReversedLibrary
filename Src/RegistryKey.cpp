#include <RegistryKey.h>
#include <BaseStringPart.h>
#include <BaseStringView.h>
#include <EnumDictionary.h>
#include <Errors.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CUnicodeString CRegistryKeyValueEnumerator::operator*() const
{
	CUnicodeString newName;
	DWORD valueLen = maxLength;
	::RegEnumValue( keyHandle, enumPosition, newName.CreateRawBuffer( maxLength ), &valueLen, nullptr, nullptr, nullptr, nullptr );
	return newName;
}

//////////////////////////////////////////////////////////////////////////

const CEnumDictionary<TRegistryRootKey, RRK_EnumCount, HKEY> rootKeys {
	{ RRK_ClassesRoot, HKEY_CLASSES_ROOT },
	{ RRK_CurrentUser, HKEY_CURRENT_USER },
	{ RRK_LocalMachine, HKEY_LOCAL_MACHINE },
	{ RRK_Users, HKEY_USERS },
	{ RRK_CurrentConfig, HKEY_CURRENT_CONFIG }
};
const CEnumDictionary<TRegistryRootKey, RRK_EnumCount> rootKeyNames {
	{ RRK_ClassesRoot, L"CLASSES_ROOT" },
	{ RRK_CurrentUser, L"CURRENT_USER"},
	{ RRK_LocalMachine, L"LOCAL_MACHINE" },
	{ RRK_Users, L"USERS" },
	{ RRK_CurrentConfig, L"CURRENT_CONFIG" }
};
extern const CError Err_RegistryOpenError;
CRegistryKey::CRegistryKey( TRegistryRootKey rootKey, CUnicodeView keyName, TRegistryAccessType accessType )
{
	const auto rootHandle = rootKeys[rootKey];
	initializeRegistryKey( rootHandle, keyName, accessType );
}

CRegistryKey::CRegistryKey( const CRegistryKey& parentKey, CUnicodeView keyName, TRegistryAccessType accessType )
{
	initializeRegistryKey( parentKey.keyHandle, keyName, accessType );
}

void CRegistryKey::initializeRegistryKey( HKEY parent, CUnicodeView name, TRegistryAccessType accessType )
{
	const auto openResult = ::RegCreateKeyEx( parent, name.Ptr(), 0, nullptr, 0, accessType, nullptr, &keyHandle, nullptr );
	check( openResult == ERROR_SUCCESS, Err_RegistryOpenError, name, static_cast<int>( openResult ) );
}

CRegistryKey::~CRegistryKey()
{
	::RegCloseKey( keyHandle );
}

CRegistryKeyValueEnumerator CRegistryKey::ValueNames() const
{
	DWORD valueCount;
	DWORD maxValueLen;
	::RegQueryInfoKey( keyHandle, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &valueCount, &maxValueLen, nullptr, nullptr, nullptr );
	return CRegistryKeyValueEnumerator( keyHandle, 0, valueCount, maxValueLen );
}

bool CRegistryKey::HasValue( CUnicodeView valueName ) const
{
	DWORD size;
	return tryGetRegValue( valueName, nullptr, size ) != ERROR_FILE_NOT_FOUND;
}

CUnicodeString CRegistryKey::doReadValue( CUnicodeView name, const CUnicodeString& defaultValue ) const
{
	return readStringValue( name, defaultValue );
}

CUnicodeString CRegistryKey::doReadValue( CUnicodeView name, const CUnicodeView& defaultValue ) const
{
	return readStringValue( name, defaultValue );
}

CUnicodeString CRegistryKey::doReadValue( CUnicodeView name, const CUnicodePart& defaultValue ) const
{
	return readStringValue( name, defaultValue );
}

CUnicodeString CRegistryKey::doReadValue( CUnicodeView name, const TPrimitiveString& defaultValue ) const
{
	return readStringValue( name, CUnicodePart( defaultValue ) );
}

CUnicodeString CRegistryKey::readStringValue( CUnicodeView name, CUnicodePart defaultValue ) const
{
	CUnicodeString result;
	const auto readSuccess = tryReadStringValue( name, result );
	return readSuccess ? move( result ) : UnicodeStr( defaultValue );
}

CUnicodeString CRegistryKey::doGetOrCreateValue( CUnicodeView name, const CUnicodeString& defaultValue ) 
{
	return doGetOrCreateStringValue( name, defaultValue );
}

CUnicodeString CRegistryKey::doGetOrCreateValue( CUnicodeView name, const CUnicodeView& defaultValue ) 
{
	return doGetOrCreateStringValue( name, defaultValue );
}

CUnicodeString CRegistryKey::doGetOrCreateValue( CUnicodeView name, const CUnicodePart& defaultValue ) 
{
	return doGetOrCreateStringValue( name, defaultValue );
}

CUnicodeString CRegistryKey::doGetOrCreateValue( CUnicodeView name, const TPrimitiveString& defaultValue )
{
	return doGetOrCreateStringValue( name, CUnicodePart( defaultValue ) );
}

CUnicodeString CRegistryKey::doGetOrCreateStringValue( CUnicodeView name, CUnicodePart defaultValue )
{
	CUnicodeString result;
	const auto readSuccess = tryReadStringValue( name, result );
	if( !readSuccess ) {
		auto newValue = UnicodeStr( defaultValue );
		SetValue( name, newValue );
		return newValue;
	}
	return result;
}

bool CRegistryKey::tryReadStringValue( CUnicodeView valueName, CUnicodeString& result ) const
{
	LSTATUS readStatus;
	do {
		DWORD requiredSize;
		tryGetRegValue( valueName, nullptr, requiredSize );
		auto resultBuffer = result.CreateRawBuffer( requiredSize / 2 + 1 );
		readStatus = tryGetRegValue( valueName, resultBuffer.Ptr(), requiredSize );

	} while( readStatus == ERROR_MORE_DATA );

	return readStatus == ERROR_SUCCESS;
}

LSTATUS CRegistryKey::tryGetRegValue( CUnicodeView valueName, void* valuePtr, DWORD& valueSize ) const
{
	return ::RegQueryValueEx( keyHandle, valueName.Ptr(), 0, nullptr, reinterpret_cast<BYTE*>( valuePtr ), &valueSize );
}

void CRegistryKey::retrieveValueWriteParams( const CUnicodeString& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const
{
	retrieveStringWriteParams( value, dataType, dataPtr, dataSize );
}

void CRegistryKey::retrieveValueWriteParams( const CUnicodeView& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const
{
	retrieveStringWriteParams( value, dataType, dataPtr, dataSize );
}

void CRegistryKey::retrieveValueWriteParams( wchar_t const* const& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const
{
	retrieveStringWriteParams( CUnicodeView( value ), dataType, dataPtr, dataSize );
}

void CRegistryKey::retrieveStringWriteParams( CUnicodeView value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const
{
	dataPtr = value.Ptr();
	dataType = REG_SZ;
	dataSize = ( value.Length() + 1 ) * sizeof( wchar_t );
}

void CRegistryKey::retrieveValueWriteParams( const int& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const
{
	staticAssert( sizeof( value ) == 4 );
	dataType = REG_DWORD;
	dataPtr = &value;
	dataSize = 4;
}

void CRegistryKey::retrieveValueWriteParams( const long long& value, DWORD& dataType, const void*& dataPtr, DWORD& dataSize ) const
{
	staticAssert( sizeof( value ) == 8 );
	dataType = REG_QWORD;
	dataPtr = &value;
	dataSize = 8;
}

extern const CError Err_RegistryWriteError;
void CRegistryKey::setRegValue( CUnicodeView valueName, DWORD dataType, const void* dataPtr, DWORD dataSize ) const
{
	const auto writeResult = ::RegSetKeyValue( keyHandle, nullptr, valueName.Ptr(), dataType, dataPtr, dataSize );
	check( writeResult == ERROR_SUCCESS, Err_RegistryWriteError, valueName, static_cast<int>( writeResult ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
