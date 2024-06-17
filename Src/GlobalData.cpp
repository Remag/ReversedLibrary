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
extern const CStringView DefaultAssertFailedMessage = "Assertion failed: %0\nFile: %1\nFunction: %2, line: %3.";
extern const CStringView UnknownComErrorMessage = "Unknown COM Error:\nResult code: %0.";
extern const CStringView GeneralMultiCurlError = "LibCurl multi interface returned an error.";
const CStringView CMemoryException::NotEnoughMemoryMessage = "Not enough memory!";
extern const CStringView TempFilePrefix = "relibtmp";
extern const CStringView TempFileExt = "tmp";
extern const CStringView CreatedFromStrName = "Document created from string.";


// Message handlers.
namespace Log {
	CWindowMessageLog WindowLog;
	CStdOutputLog StdLog;
}

// Strings.
CString ApplicationTitle;
namespace FileSystem {
	CString SpecificUserAppDataPath;
	CString AllUsersAppDataPath;
}
// File error descriptions.
extern const CStringView UnknownLastError = "Unknown last error!\n Error code: %0.";
extern const CStringView GeneralFileError = "General File Error! Error code: %1.\nFile name: %0.";
extern const CStringView FileNotFoundError = "File not found!\nFile name: %0.";
extern const CStringView InvalidFileError = "Invalid file!\nFile name: %0.";
extern const CStringView FileTooBigError = "The file is too big!\nFile name: %0.";
extern const CStringView BadPathFileError = "Invalid path!\nFile name: %0.";
extern const CStringView ObjectAlreadyExistsError = "An object with this name already exists!\nFile name: %0.";
extern const CStringView AccessDeniedError = "Access to file denied!\nFile name: %0.";
extern const CStringView SharingViolationFileError = "File sharing violation!\nFile name: %0.";
extern const CStringView DiskFullError = "The disk is too large to store the file!\nFile name: %0.";
extern const CStringView EarlyEndFileError = "Unexpected end of file!\nFile name: %0.";
extern const CStringView HardwareFileError= "Hardware IO error!\nFile name: %0.";
extern const CStringView XmlParsingError = "XML parsing error at position %0:\n%1.";
extern const CStringView JsonParsingError = "JSON parsing error at line %0, position %1.";
extern const CStringView JsonConversionError = "JSON value was expected to be %0, the actual value was %1.";
extern const CStringView JsonMissingKeyError = "JSON object is missing a key: \"%0\"";
extern const CStringView GeneralCurlError = "Curl error. Error string buffer: %0.";
extern const CStringView GeneralPngFileError = "JPEG parsing error: %1.\nFile name: %0";
extern const CStringView GeneralGifFileError = "GIF parsing error: %1.\nFile name: %0";
extern const CStringView GeneralJpgFileError = "PNG parsing error: %1.\nFile name: %0";
extern const CStringView UncommitedArchiveError = "Archive was written to but never flushed to a file or buffer.";

// Symbol sets.
extern const CUnicodeSet InvalidFileNameSymbols{
	L'*', L'?', L'<', L'>', L':', L'\"', L'|', L'\\', L'/',
	L'\1', L'\2', L'\3', L'\4', L'\5', L'\6', L'\7', L'\10',
	L'\11', L'\12', L'\13', L'\14', L'\15', L'\16', L'\17', L'\20',
	L'\21', L'\22', L'\23', L'\24', L'\25', L'\26', L'\27', L'\30',
	L'\31', L'\32', L'\33', L'\34', L'\35', L'\36', L'\37'
};

// Errors.
extern const CError REAPI Err_SmallArchive( "Trying to read an archive value after its end." );
extern const CError REAPI Err_BadArchive( "Unable to serialize with the given archive." );
extern const CError REAPI Err_BadArchiveVersion( "Archive version is incompatible with the current program." );
extern const CError REAPI Err_CompressedArchive( "This library build doesn't support compressed archives." );
extern const CError Err_BadIniFile( "INI contains an invalid string.\nFile name: %0. String position: %1." );
extern const CError Err_DuplicateIniKey( "INI file contains a duplicate key.\nFile name: %0. Key name: %1." );
extern const CError Err_RegistryOpenError{ "Failed to open a registry key. Key name: %0.  Error code: %1." };
extern const CError Err_RegistryWriteError{ "Failed to write to a registry key. Value name: %0. Error code: %1." };
extern const CError Err_ZlibInitError{ "Failed to initialize ZLib. Error code: %0." };
extern const CError Err_ZlibInflateError{ "Failed to unzip data. Error code: %0." };
extern const CError Err_BadCollectionData{ "File collection data is corrupted." };
extern const CError Err_CreateTempFile( "Unable to open the temporary files folder.\nFolder name: %0.");

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

CArray<CString, CProcessHeap> TempFileNames;
CRandomGenerator TempFileSuffixGenerator;

namespace Log {
	extern thread_local CStringView CurrentMessageSource = CStringView();
}

//////////////////////////////////////////////////////////////////////////

}	// namespace Relib.

// XML API data.
namespace rapidxml {
	// Node name (anything but space \n \r \t / > ? \0)
	const CUnicodeSet xml_document::node_name_pred::nodeNameExcludeSet{ L' ', L'\n', L'\r', L'\t', L'/', L'>', L'?', 0 };
	// Attribute name (anything but space \n \r \t / < > = ? ! \0)
	const CUnicodeSet xml_document::attribute_name_pred::attributeNameExcludeSet{ L' ', L'\n', L'\r', L'\t', L'/', L'<', L'>', L'=', L'?', L'!', 0 };
}




