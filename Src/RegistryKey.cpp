#include <RegistryKey.h>
#include <BaseStringPart.h>
#include <BaseStringView.h>
#include <EnumDictionary.h>
#include <Errors.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CString CRegistryKeyValueEnumerator::operator*() const
{
	CUnicodeString newName;
	DWORD valueLen = maxLength;
	::RegEnumValue( keyHandle, enumPosition, newName.CreateRawBuffer( maxLength ), &valueLen, nullptr, nullptr, nullptr, nullptr );
	return Str( newName );
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
CRegistryKey::CRegistryKey( TRegistryRootKey rootKey, CStringView keyName, TRegistryAccessType accessType )
{
	const auto rootHandle = rootKeys[rootKey];
	initializeRegistryKey( rootHandle, keyName, accessType );
}

CRegistryKey::CRegistryKey( const CRegistryKey& parentKey, CStringView keyName, TRegistryAccessType accessType )
{
	initializeRegistryKey( parentKey.keyHandle, keyName, accessType );
}

void CRegistryKey::initializeRegistryKey( HKEY parent, CStringView name, TRegistryAccessType accessType )
{
	const auto openResult = ::RegCreateKeyEx( parent, UnicodeStr( name ).Ptr(), 0, nullptr, 0, accessType, nullptr, &keyHandle, nullptr );
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

bool CRegistryKey::HasValue( CStringView valueName ) const
{
	DWORD size;
	return tryGetRegValue( valueName, nullptr, size ) != ERROR_FILE_NOT_FOUND;
}

CString CRegistryKey::doReadValue( CStringView name, const CString& defaultValue ) const
{
	return readStringValue( name, defaultValue );
}

CString CRegistryKey::doReadValue( CStringView name, const CStringView& defaultValue ) const
{
	return readStringValue( name, defaultValue );
}

CString CRegistryKey::doReadValue( CStringView name, const CStringPart& defaultValue ) const
{
	return readStringValue( name, defaultValue );
}

CString CRegistryKey::doReadValue( CStringView name, const TPrimitiveString& defaultValue ) const
{
	return readStringValue( name, CStringPart( defaultValue ) );
}

CString CRegistryKey::readStringValue( CStringView name, CStringPart defaultValue ) const
{
	CString result;
	const auto readSuccess = tryReadStringValue( name, result );
	return readSuccess ? move( result ) : Str( defaultValue );
}

CString CRegistryKey::doGetOrCreateValue( CStringView name, const CString& defaultValue ) 
{
	return doGetOrCreateStringValue( name, defaultValue );
}

CString CRegistryKey::doGetOrCreateValue( CStringView name, const CStringView& defaultValue ) 
{
	return doGetOrCreateStringValue( name, defaultValue );
}

CString CRegistryKey::doGetOrCreateValue( CStringView name, const CStringPart& defaultValue ) 
{
	return doGetOrCreateStringValue( name, defaultValue );
}

CString CRegistryKey::doGetOrCreateValue( CStringView name, const TPrimitiveString& defaultValue )
{
	return doGetOrCreateStringValue( name, CStringPart( defaultValue ) );
}

CString CRegistryKey::doGetOrCreateStringValue( CStringView name, CStringPart defaultValue )
{
	CString result;
	const auto readSuccess = tryReadStringValue( name, result );
	if( !readSuccess ) {
		SetValue( name, defaultValue );
		return Str( defaultValue );
	}
	return result;
}

bool CRegistryKey::tryReadStringValue( CStringView valueName, CString& result ) const
{
	CUnicodeString utf16Value;
	LSTATUS readStatus;
	do {
		DWORD requiredSize;
		tryGetRegValue( valueName, nullptr, requiredSize );
		auto resultBuffer = utf16Value.CreateRawBuffer( requiredSize / 2 + 1 );
		readStatus = tryGetRegValue( valueName, resultBuffer.Ptr(), requiredSize );

	} while( readStatus == ERROR_MORE_DATA );

	result = Str( utf16Value );
	return readStatus == ERROR_SUCCESS;
}

LSTATUS CRegistryKey::tryGetRegValue( CStringView valueName, void* valuePtr, DWORD& valueSize ) const
{
	return ::RegQueryValueEx( keyHandle, UnicodeStr( valueName ).Ptr(), 0, nullptr, reinterpret_cast<BYTE*>( valuePtr ), &valueSize );
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
void CRegistryKey::setRegValue( CStringView valueName, DWORD dataType, const void* dataPtr, DWORD dataSize ) const
{
	const auto writeResult = ::RegSetKeyValue( keyHandle, nullptr, UnicodeStr( valueName ).Ptr(), dataType, dataPtr, dataSize );
	check( writeResult == ERROR_SUCCESS, Err_RegistryWriteError, valueName, static_cast<int>( writeResult ) );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
