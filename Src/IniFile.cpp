#include <IniFile.h>
#include <Comparators.h>
#include <Errors.h>
#include <FileOperations.h>
#include <PtrOwner.h>
#include <Pair.h>
#include <StrConversions.h>

namespace Relib {

//////////////////////////////////////////////////////////////////////////

CPair<CStringPart> CIniSectionKeyRange::operator*() const
{
	const auto& data = *mapIterator;
	return CPair<CStringPart>( data.Key(), values[data.Value()] );
}

//////////////////////////////////////////////////////////////////////////
extern const CError Err_BadIniFile;
extern const CError Err_DuplicateIniKey;

static CStringPart deleteWhitespace( CStringPart str )
{
	return str.TrimSpaces();
}

bool CIniFileSection::IsEmpty() const
{
	const auto idCount = valueStrings.Size();
	for( int id = 0; id < idCount; id++ ) {
		if( HasKey( id ) ) {
			return false;
		}
	}
	return true;
}

void CIniFileSection::Empty()
{
	valueNameToId.Empty();
	valueStrings.Empty();
}

int CIniFileSection::GetKeyId( CStringPart keyName ) const
{
	return valueNameToId[deleteWhitespace( keyName )];
}

int CIniFileSection::GetOrCreateKeyId( CStringPart keyName )
{
	const auto valueCount = valueStrings.Size();
	const auto resultId = valueNameToId.GetOrCreate( deleteWhitespace( keyName ), valueCount ).Value();
	if( resultId == valueCount ) {
		valueStrings.Add();
	}
	return resultId;
}

int CIniFileSection::GetOrCreateKeyId( CStringPart keyName, CString defaultValue )
{
	const auto valueCount = valueStrings.Size();
	const auto resultId = valueNameToId.GetOrCreate( deleteWhitespace( keyName ), valueCount ).Value();
	if( resultId == valueCount ) {
		valueStrings.Add( move( defaultValue ) );
	}
	return resultId;
}

bool CIniFileSection::HasKey( CStringPart keyName ) const
{
	const auto keyId = valueNameToId.Get( deleteWhitespace( keyName ) );
	return keyId != nullptr && HasKey( *keyId );
}

bool CIniFileSection::HasKey( int keyId ) const
{
	// Empty key values are marked with a whitespace char.
	const auto& targetStr = valueStrings[keyId];
	return targetStr.Length() != 1 || targetStr[0] != ' ';
}

const CString* CIniFileSection::LookupString( CStringPart keyName ) const
{
	const auto keyId = valueNameToId.Get( deleteWhitespace( keyName ) );
	return keyId == nullptr ? nullptr : LookupString( *keyId );
}

const CString* CIniFileSection::LookupString( int keyId ) const
{
	return HasKey( keyId ) ? &valueStrings[keyId] : nullptr;
}

static const char* prohibitedIniSymbols = "\r\n";
void CIniFileSection::SetString( CStringPart keyName, CStringPart newValue )
{
	assert( keyName.FindOneOf( prohibitedIniSymbols ) == NotFound );
	const auto keyId = GetOrCreateKeyId( keyName );
	SetString( keyId, newValue );
}

void CIniFileSection::SetString( int keyId, CStringPart newValue )
{
	assert( newValue.FindOneOf( prohibitedIniSymbols ) == NotFound );
	valueStrings[keyId] = deleteWhitespace( newValue );
}

void CIniFileSection::DeleteKey( CStringPart keyName )
{
	const auto keyId = valueNameToId.Get( deleteWhitespace( keyName ) );
	if( keyId != nullptr ) {
		DeleteKey( *keyId );
	}
}

void CIniFileSection::DeleteKey( int keyId )
{
	valueStrings[keyId] = CString();
}

CIniSectionKeyRange CIniFileSection::KeyValuePairs() const
{
	return CIniSectionKeyRange( valueNameToId, valueStrings, valueNameToId.begin() );
}

CString CIniFileSection::GetKeyValuePairsString() const
{
	typedef CPair<CStringPart, int> TKeyIdPair;
	CArray<TKeyIdPair> keyIdPairs;
	keyIdPairs.ReserveBuffer( valueNameToId.Size() );
	for( const auto& value : valueNameToId ) {
		keyIdPairs.Add( TKeyIdPair( value.Key(), value.Value() ) );
	}
	keyIdPairs.QuickSort( LessByAction( &TKeyIdPair::Second ) );

	CString result;
	for( auto keyIdPair : keyIdPairs ) {
		if( !HasKey( keyIdPair.Second ) ) {
			continue;
		}
		const CStringPart valueStr = valueStrings[keyIdPair.Second];
		result += keyIdPair.First + '=' + valueStr + "\r\n";
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////

CIniFile::CIniFile( CStringPart name ) :
	isModified( false )
{
	readFile( name );
}

CIniFile::CIniFile( CIniFile&& other ) :
	filePath( move( other.filePath ) ),
	sectionNameToSectionId( move( other.sectionNameToSectionId ) ),
	sections( move( other.sections ) ),
	isModified( move( other.isModified ) )
{
	other.isModified = false;
}

const CIniFile& CIniFile::operator=( CIniFile&& other )
{
	swap( filePath, other.filePath );
	swap( sectionNameToSectionId, other.sectionNameToSectionId );
	swap( sections, other.sections );
	swap( isModified, other.isModified );
	return *this;
}

static const wchar_t unicodeLineSeparator = 0x2028;
static const wchar_t unicodeParagraphSeparator = 0x2029;
static int skipUnicodeWhitespace( CStringPart substr )
{
	switch( substr[0] ) {
		case L'\r':
			return substr[1] == L'\n' ? 2 : 1;
		case L'\n':
		case L'\v':
		case L'\f':
		case unicodeLineSeparator:
		case unicodeParagraphSeparator:
			return 1;
		default:
			return 0;
	}
}

// Read the given file and fill the class internal structures.
void CIniFile::readFile( CStringPart fileName )
{
	filePath = fileName;
	CString fileContents;
	try {
		fileContents = File::ReadText( filePath );
	} catch( const CFileException& e ) {
		if( e.Type() == CFileException::FET_FileNotFound ) {
			// Ignore the missing file errors.
			return;
		} else {
			throw;
		}
	}

	sectionNameToSectionId.Add( CString(), 0 );
	CIniFileSection* currentSection = &sections.Add( CStringPart() );
	int lineNumber = 0;
	for( auto line : fileContents.SplitByAction( skipUnicodeWhitespace ) ) {
		const auto fileStr = deleteWhitespace( line );
		if( shouldSkip( fileStr ) ) {
			lineNumber++;
			continue;
		}
		CString keyName;
		CString valueName;
		if( parseKeyValuePair( fileStr, keyName, valueName ) ) {
			if( currentSection->HasKey( keyName ) ) {
				check( false, Err_DuplicateIniKey, filePath, deleteWhitespace( keyName ) );
			}
			currentSection->SetString( keyName, valueName );
			lineNumber++;
			continue;
		} 
		CString newSectionName;
		if( parseSection( fileStr, newSectionName ) ) {
			currentSection = &getOrCreateSection( move( newSectionName ) );
			lineNumber++;
			continue;
		}
		check( false, Err_BadIniFile, filePath, UnicodeStr( lineNumber ) );
	}
}

// Determine if str denotes a comment or an empty string in the .ini file.
bool CIniFile::shouldSkip( CStringPart str )
{
	return str.IsEmpty()
		|| str.First() == L';'
		|| ( str.Length() >= 2 && str[0] == L'/' && str[1] == L'/' );
}

// Try and get a key-value pair from str. If str doesn't contain a valid pair, return false.
bool CIniFile::parseKeyValuePair( CStringPart str, CString& key, CString& value )
{
	const int equalIndex = str.Find( L'=' );
	if( equalIndex == NotFound ) {
		return false;
	}

	key = str.Left( equalIndex );
	value = str.Mid( equalIndex + 1 );
	return true;
}

// Try and get a section name from str. If str doesn't contain a section name, return false.
bool CIniFile::parseSection( CStringPart str, CString& section )
{
	if( str.First() != L'[' || str.Last() != L']' ) {
		return false;
	}

	section = str.Mid( 1, str.Length() - 2 );
	return true;
}

// Get an existing section with a given name or create a new one.
CIniFileSection& CIniFile::getOrCreateSection( CStringPart name )
{
	assert( name.FindOneOf( prohibitedIniSymbols ) == NotFound );
	const auto id = GetOrCreateSectionId( name );
	return sections[id];
}

// Get the section with a given name. If no section is found, return 0.
CIniFileSection* CIniFile::getSection( CStringPart name )
{
	const auto idPtr = sectionNameToSectionId.Get( deleteWhitespace( name ) );
	return idPtr == nullptr ? nullptr : &sections[*idPtr];
}

CIniFile::~CIniFile()
{
	if( IsModified() ) {
		try {
			Save();
		} catch( const CException& ) {
			// Ignore exceptions.
		}
	}
}

void CIniFile::Save()
{
	CString result;
	for( const auto& section : sections ) {
		if( section.IsEmpty() ) {
			continue;
		}
		const auto sectionName = section.GetName();
		const auto sectionDisplayName = sectionName.IsEmpty() ? CString() : '[' + sectionName + "]\r\n";
		result += sectionDisplayName;
		result += section.GetKeyValuePairsString() + "\r\n";
	}

	File::WriteText( filePath, result );
	isModified = false;
}

void CIniFile::Empty()
{
	sectionNameToSectionId.Empty();
	sections.Empty();
	isModified = true;	
}

void CIniFile::EmptySection( CStringPart sectionName )
{
	CIniFileSection* section = getSection( sectionName );
	if( section != nullptr ) {
		section->Empty();
		isModified = true;
	}
}

int CIniFile::GetSectionId( CStringPart sectionName ) const
{
	return sectionNameToSectionId[deleteWhitespace( sectionName )];
}

int CIniFile::GetOrCreateSectionId( CStringPart sectionName )
{
	const auto sectionCount = sections.Size();
	const auto& newId = sectionNameToSectionId.GetOrCreate( deleteWhitespace( sectionName ), sectionCount );
	if( newId.Value() == sectionCount ) {
		sections.Add( newId.Key() );
	}
	return newId.Value();
}

const CIniFileSection* CIniFile::GetSection( CStringPart sectionName ) const
{
	return getSection( sectionName );
}

const CIniFileSection& CIniFile::GetSection( int sectionId ) const
{
	return sections[sectionId];
}

CIniFileSection* CIniFile::GetSection( CStringPart sectionName )
{
	isModified = true;
	return getSection( sectionName );
}

CIniFileSection& CIniFile::GetSection( int sectionId )
{
	return sections[sectionId];
}

int CIniFile::GetKeyId( int sectionId, CStringPart keyName ) const
{
	return sections[sectionId].GetKeyId( keyName );
}

int CIniFile::getOrCreateKeyId( int sectionId, CStringPart keyName, CString defaultValue ) 
{
	return sections[sectionId].GetOrCreateKeyId( keyName, move( defaultValue ) );
}

const CString* CIniFile::LookupString( CStringPart sectionName, CStringPart keyName ) const
{
	const CIniFileSection* section = getSection( sectionName );
	if( section == nullptr ) {
		return nullptr;
	}
	return section->LookupString( keyName );
}

const CString* CIniFile::LookupString( int sectionId, int keyId ) const
{
	return sections[sectionId].LookupString( keyId );
}

void CIniFile::SetString( CStringPart sectionName, CStringPart keyName, CStringPart newValue )
{
	CIniFileSection& section = getOrCreateSection( sectionName );
	section.SetString( keyName, newValue );
	isModified = true;
}

void CIniFile::SetString( int sectionId, int keyId, CStringPart newValue )
{
	sections[sectionId].SetString( keyId, newValue );
	isModified = true;
}

void CIniFile::DeleteKey( CStringPart sectionName, CStringPart keyName )
{
	CIniFileSection* section = getSection( sectionName );
	if( section != 0 ) {
		section->DeleteKey( keyName );
		isModified = true;
	}
}

void CIniFile::DeleteKey( int sectionId, int keyId )
{
	sections[sectionId].DeleteKey( keyId );
	isModified = true;
}

bool CIniFile::HasSection( CStringPart sectionName ) const
{
	return sectionNameToSectionId.Has( deleteWhitespace( sectionName ) );
}

bool CIniFile::HasKey( CStringPart sectionName, CStringPart keyName ) const
{
	const CIniFileSection* section = getSection( sectionName );
	return section != nullptr && section->HasKey( keyName );
}

bool CIniFile::HasKey( int sectionId, int keyId ) const
{
	const auto& section = sections[sectionId];
	return section.HasKey( keyId );
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.
