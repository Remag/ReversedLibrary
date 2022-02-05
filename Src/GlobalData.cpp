// Global library variables. All global variables should be defined here to avoid race conditions.
#include <Redefs.h>
#include <BaseStringPart.h>
#include <Relimits.h>
#include <BaseString.h>
#include <Errors.h>
#include <LibraryAllocators.h>
#include <MessageLogImpls.h>
#include <UnicodeSet.h>
#include <RandomGenerator.h>
#include <NamedInlineComponent.h>
#include <StrConversions.h>
#include <ActionOwner.h>
#include <ActionUtils.h>
#include <EnumDictionary.h>
#include <Pair.h>
#include <Atomic.h>
#include <ObjectCreationUtils.h>
#include <Mutex.h>
#include <RapidXml\rapidxml.hpp>

#pragma warning( disable : 4074 )
// Global data should be initialized as fast as possible. Next directive sets the highest priority to the current translation unit.
#pragma init_seg( compiler )

namespace Relib {

// Numeric limits.
const float CLimits<float>::Min = -FLT_MAX;
const float CLimits<float>::Max = FLT_MAX;

const double CLimits<double>::Min = -DBL_MAX;
const double CLimits<double>::Max = DBL_MAX;

namespace RelibInternal {
	// Allocators.
	CVirtualAllocDynamicManager VirtualMemoryAllocator;
	REAPI CThreadSafeAllocator<CGeneralBlockAllocator<CUnicodeSet::TStorageType::PageSizeInBytes>> UnicodeSetAllocator;
	REAPI CActionOwnerAllocator ActionOwnerAllocator;
}

REAPI CStringAllocator StringAllocator;

// Critical sections.
CCriticalSection ConsoleWriteSection;
CCriticalSection FileWriteSection;
CCriticalSection ApplicationTitleSection;
CCriticalSection ObjectCreationFunctionsSection;
CCriticalSection StringAllocatorSection;
CCriticalSection TempFileLock;
namespace RelibInternal {
	CCriticalSection DebugAllocatorGlobalSection;
	REAPI CCriticalSection ComponentIdsSection;
	REAPI CCriticalSection EventClassIdSection;
}

namespace FileSystem {
	CCriticalSection ApplicationDataSection;
}

// Empty string buffers.
char CStringView::emptyBufferStr = 0;
wchar_t CUnicodeView::emptyBufferStr = 0;

// Constant Strings.
extern const CUnicodeView DefaultAssertFailedMessage = L"Assertion failed: %0\nFile: %1\nFunction: %2, line: %3.";
extern const CUnicodeView UnknownComErrorMessage = L"Unknown COM Error:\nResult code: %0.";
extern const CUnicodeView GeneralMultiCurlError = L"LibCurl multi interface returned an error.";
const CUnicodeView CMemoryException::NotEnoughMemoryMessage = L"Not enough memory!";
extern const CUnicodeView TempFilePrefix = L"relibtmp";
extern const CUnicodeView TempFileExt = L"tmp";

// Message handlers.
namespace Log {
	CWindowMessageLog WindowLog;
	CStdOutputLog StdLog;
}

// Strings.
CUnicodeString ApplicationTitle;
namespace FileSystem {
	CUnicodeString SpecificUserAppDataPath;
	CUnicodeString AllUsersAppDataPath;
}
// File error descriptions.
extern const CUnicodeView UnknownLastError = L"Unknown last error!\n Error code: %0.";
extern const CUnicodeView GeneralFileError = L"General File Error! Error code: %1.\nFile name: %0.";
extern const CUnicodeView FileNotFoundError = L"File not found!\nFile name: %0.";
extern const CUnicodeView InvalidFileError = L"Invalid file!\nFile name: %0.";
extern const CUnicodeView FileTooBigError = L"The file is too big!\nFile name: %0.";
extern const CUnicodeView BadPathFileError = L"Invalid path!\nFile name: %0.";
extern const CUnicodeView ObjectAlreadyExistsError = L"An object with this name already exists!\nFile name: %0.";
extern const CUnicodeView AccessDeniedError = L"Access to file denied!\nFile name: %0.";
extern const CUnicodeView SharingViolationFileError = L"File sharing violation!\nFile name: %0.";
extern const CUnicodeView DiskFullError = L"The disk is too large to store the file!\nFile name: %0.";
extern const CUnicodeView EarlyEndFileError = L"Unexpected end of file!\nFile name: %0.";
extern const CUnicodeView HardwareFileError= L"Hardware IO error!\nFile name: %0.";
extern const CUnicodeView XmlParsingError = L"XML parsing error at position %0:\n%1.";
extern const CUnicodeView JsonParsingError = L"JSON parsing error at line %0, position %1.";
extern const CUnicodeView JsonConversionError = L"JSON value was expected to be %0, the actual value was %1.";
extern const CUnicodeView JsonMissingKeyError = L"JSON object is missing a key: \"%0\"";
extern const CUnicodeView GeneralCurlError = L"Curl error. Error string buffer: %0.";
extern const CUnicodeView GeneralPngFileError = L"JPEG parsing error: %1.\nFile name: %0";
extern const CUnicodeView GeneralGifFileError = L"GIF parsing error: %1.\nFile name: %0";
extern const CUnicodeView GeneralJpgFileError = L"PNG parsing error: %1.\nFile name: %0";
extern const CUnicodeView UncommitedArchiveError = L"Archive was written to but never flushed to a file or buffer.";

// Symbol sets.
extern const CUnicodeSet InvalidFileNameSymbols{
	L'*', L'?', L'<', L'>', L':', L'\"', L'|', L'\\', L'/',
	L'\1', L'\2', L'\3', L'\4', L'\5', L'\6', L'\7', L'\10',
	L'\11', L'\12', L'\13', L'\14', L'\15', L'\16', L'\17', L'\20',
	L'\21', L'\22', L'\23', L'\24', L'\25', L'\26', L'\27', L'\30',
	L'\31', L'\32', L'\33', L'\34', L'\35', L'\36', L'\37'
};

// Errors.
extern const CError REAPI Err_SmallArchive( L"Trying to read an archive value after its end." );
extern const CError REAPI Err_BadArchive( L"Unable to serialize with the given archive." );
extern const CError REAPI Err_BadArchiveVersion( L"Archive version is incompatible with the current program." );
extern const CError Err_BadIniFile( L"INI contains an invalid string.\nFile name: %0. String position: %1." );
extern const CError Err_DuplicateIniKey( L"INI file contains a duplicate key.\nFile name: %0. Key name: %1." );
extern const CError Err_RegistryOpenError{ L"Failed to open a registry key. Key name: %0.  Error code: %1." };
extern const CError Err_RegistryWriteError{ L"Failed to write to a registry key. Value name: %0. Error code: %1." };
extern const CError Err_ZlibInitError{ L"Failed to initialize ZLib. Error code: %0." };
extern const CError Err_ZlibInflateError{ L"Failed to unzip data. Error code: %0." };
extern const CError Err_BadCollectionData{ L"File collection data is corrupted." };
extern const CError Err_CreateTempFile( L"Unable to open the temporary files folder.\nFolder name: %0.");

// Other.

// Maps of external objects' names.
CMap<CUnicodeString, CPtrOwner<CBaseObjectCreationFunction, CProcessHeap>, CDefaultHash<CUnicodeString>, CProcessHeap> ObjectCreationFunctions;
CMap<CStringView, CUnicodeString, CDefaultHash<CStringView>, CProcessHeap> ObjectRegisteredNames;

namespace RelibInternal {
	// Event system class name to its unique identifier.
	REAPI CMap<CStringView, int, CDefaultHash<CStringView>, CProcessHeap> EventClassNameToId;

	// Current maximum identifier for the non-inline components.
	REAPI CAtomic<int> CurrentComponentId{ 0 };

	// Component class name to its components maximum unique identifier.
	REAPI CMap<CStringView, CComponentClassInfo, CDefaultHash<CStringView>, CProcessHeap> ComponentClassNameToId;
	// Named component database.
	REAPI CMap<CString, CNamedInlineComponent, CDefaultHash<CString>, CProcessHeap> NamedInlineComponents;
	REAPI CMap<CPair<int>, CNamedInlineComponent, CDefaultHash<CPair<int>>, CProcessHeap> NamedInlineComponentIds;

	REAPI CEnumDictionary<TNamedComponentType, NCT_EnumCount, CPair<size_t>> NamedComponentSizeDict {
		{ NCT_Bool, CreatePair( sizeof( bool ), alignof( bool ) ) },
		{ NCT_Int, CreatePair( sizeof( int ), alignof( int ) ) },
		{ NCT_Float, CreatePair( sizeof( float ), alignof( float ) ) },
		{ NCT_Double, CreatePair( sizeof( double ), alignof( double ) ) },
		{ NCT_Vec2, CreatePair( sizeof( CVector2<float> ), alignof( CVector2<float> ) ) },
		{ NCT_Vec3, CreatePair( sizeof( CVector3<float> ), alignof( CVector3<float> ) ) },
		{ NCT_IntVec2, CreatePair( sizeof( CVector2<int> ), alignof( CVector2<int> ) ) },
		{ NCT_IntVec3, CreatePair( sizeof( CVector3<int> ), alignof( CVector3<int> ) ) },
		{ NCT_AString, CreatePair( sizeof( CString ), alignof( CString ) ) },
		{ NCT_WString, CreatePair( sizeof( CString ), alignof( CString ) ) }
	};

	const CStringView CStrConversionFunctions<char>::trueStrings[2] = { "true", "1" };
	const CStringView CStrConversionFunctions<char>::falseStrings[2] = { "false", "0" };
	const CUnicodeView CStrConversionFunctions<wchar_t>::trueUnicodeStrings[2] = { L"true", L"1" };
	const CUnicodeView CStrConversionFunctions<wchar_t>::falseUnicodeStrings[2] = { L"false", L"0" };
}

CArray<CUnicodeString, CProcessHeap> TempFileNames;
CRandomGenerator TempFileSuffixGenerator;

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

// XML API data.
namespace rapidxml {
	// Node name (anything but space \n \r \t / > ? \0)
	const CUnicodeSet xml_document::node_name_pred::nodeNameExcludeSet{ L' ', L'\n', L'\r', L'\t', L'/', L'>', L'?', 0 };
	// Attribute name (anything but space \n \r \t / < > = ? ! \0)
	const CUnicodeSet xml_document::attribute_name_pred::attributeNameExcludeSet{ L' ', L'\n', L'\r', L'\t', L'/', L'<', L'>', L'=', L'?', L'!', 0 };
}




